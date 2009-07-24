/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

//#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>

#include "SensorClient.h"
#include "SensorManager.h"

#include "SensorAgent.h"

/**
  This can be used to debug communication problems with the daemon.
  Should be set to 0 in any production version.
*/
#define SA_TRACE 0

using namespace KSGRD;

SensorAgent::SensorAgent( SensorManager *sm ) : QObject(sm)
{
  mSensorManager = sm;
  mDaemonOnLine = false;
}

SensorAgent::~SensorAgent()
{
  for(int i = mInputFIFO.size()-1; i >= 0; --i)
    delete mInputFIFO.takeAt(i);
  for(int i = mProcessingFIFO.size()-1; i >= 0; --i)
    delete mProcessingFIFO.takeAt(i);
}

void SensorAgent::sendRequest(const QString &req, SensorClient *client, const int id) {
	if (mDaemonOnLine) {
		//kDebug() << "This is number of byte available:" << numberByteAvailable();
		if (tryAndBatchRequest(req, client, id)) {
			executeAndEnqueueRequest(new SensorRequest(req, client, id));
		}
		//else request was batched with another one so nothing to do
	} else
		mInputFIFO.enqueue(new SensorRequest(req, client, id));

#if SA_TRACE
	kDebug(1215) << "-> " << req << "(" << mInputFIFO.count() << "/"
	<< mProcessingFIFO.count() << ")" << endl;
#endif
}

void SensorAgent::processAnswer( const char *buf, int buflen )
{
  //It is possible for an answer/error message  to be split across multiple processAnswer calls.  This makes our life more difficult
  //We have to keep track of the state we are in.  Any characters that we have not parsed yet we put in
  //mLeftOverBuffer
  QByteArray buffer = QByteArray::fromRawData(buf, buflen);
  if(!mLeftOverBuffer.isEmpty()) {
	buffer = mLeftOverBuffer + buffer; //If we have data left over from a previous processAnswer, then we have to prepend this on
	mLeftOverBuffer.clear();
  }

#if SA_TRACE
  kDebug(1215) << "<- " << QString::fromUtf8(buffer, buffer.size());
#endif
  int startOfAnswer = 0;  //This can become >= buffer.size(), so check before using!
  for ( int i = 0; i < buffer.size(); ++i ) {
    if ( buffer.at(i) == '\033' ) {  // 033 in octal is the escape character.  The signifies the start of an error
      int startOfError = i;
      bool found = false;
      while(++i < buffer.size()) {
        if(buffer.at(i) == '\033') {
	  QString error = QString::fromUtf8(buffer.constData() + startOfError+1, i-startOfError-1);
	  if ( error.startsWith("RECONFIGURE") ) {
            emit reconfigure( this );
 	  }
          else {
            /* We just received the end of an error message, so we
             * can display it. */
            SensorMgr->notify( i18nc( "%1 is a host name", "Message from %1:\n%2",
                               mHostName ,
                               error ) );
          }
          found = true;
	  break;
	}
      }
      if(found) {
        buffer.remove(startOfError, i-startOfError+1);
	i = startOfAnswer - 1;
	continue;
      } else {
        //We have not found the end of the escape string.  Try checking in the next packet
        mLeftOverBuffer = QByteArray(buffer.constData()+startOfAnswer, buffer.size()-startOfAnswer);
        return;
      }
    }

    //The spec was supposed to be that it returned "\nksysguardd> " but some seem to forget the space, so we have to compensate.  Sigh
    if( (i==startOfAnswer && buffer.size() -i >= (signed)(sizeof("ksysguardd>"  ))-1 && qstrncmp(buffer.constData()+i, "ksysguardd>",   sizeof("ksysguardd>"  )-1) == 0) ||
	(buffer.size() -i >= (signed)(sizeof("\nksysguardd>"))-1 && qstrncmp(buffer.constData()+i, "\nksysguardd>", sizeof("\nksysguardd>")-1) == 0)) {

	QByteArray answer(buffer.constData()+startOfAnswer, i-startOfAnswer);
	if(!answer.isEmpty())
		mAnswerBuffer << answer;
#if SA_TRACE
	kDebug(1215) << "<= " << mAnswerBuffer
		<< "(" << mInputFIFO.count() << "/"
		<< mProcessingFIFO.count() << ")" << endl;
#endif
	if(buffer.at(i) == '\n')
		i++;
	i += sizeof("ksysguardd>") -2;  //Move i on to the next answer (if any). -2 because sizeof adds one for \0  and the for loop will increment by 1 also
	if(i+1 < buffer.size() && buffer.at(i+1) == ' ') i++;
	startOfAnswer = i+1;

	//We have found the end of one reply
	if ( !mDaemonOnLine ) {
		/* First '\nksysguardd> ' signals that the daemon is
	  	 * ready to serve requests now. */
		mDaemonOnLine = true;
#if SA_TRACE
		kDebug(1215) << "Daemon now online!";
#endif
		mAnswerBuffer.clear();
		continue;
	}

	//Deal with the answer we have now read in

	// remove pending request from FIFO
	if ( mProcessingFIFO.isEmpty() ) {
		kDebug(1215)	<< "ERROR: Received answer but have no pending "
				<< "request!" << endl;
		mAnswerBuffer.clear();
		continue;
	}

	SensorRequest *req = mProcessingFIFO.dequeue();
	// we are now responsible for the memory of req - we must delete it!
	if ( req->getClients().isEmpty() ) {
		/* The client has disappeared before receiving the answer
		 * to his request. */
		delete req;
		mAnswerBuffer.clear();
		continue;
	}

	const QList<SensorClient*> clientList = req->getClients();
	const QList<int> listId = req->getIds();
	const int clientListSize = clientList.size();

	if(!mAnswerBuffer.isEmpty() && mAnswerBuffer[0] == "UNKNOWN COMMAND") {
		/* Notify client that the sensor seems to be no longer available. */
        kDebug(1215) << "Received UNKNOWN COMMAND for: " << req->request();
        for (int j = 0;j < clientListSize;++j)  {
        	SensorClient* currentClient = clientList.at(j);
        	currentClient->sensorLost( listId.at(j) );
        }

	} else {
		// Notify client of newly arrived answer.
		for (int j = 0; j < clientListSize; ++j) {
			SensorClient* currentClient = clientList.at(j);
			currentClient->answerReceived( listId.at(j), mAnswerBuffer );
		}
	}
	delete req;
	mAnswerBuffer.clear();
    } else if(buffer.at(i) == '\n'){
	mAnswerBuffer << QByteArray(buffer.constData()+startOfAnswer, i-startOfAnswer);
	startOfAnswer = i+1;
    }
  }

  mLeftOverBuffer += QByteArray(buffer.constData()+startOfAnswer, buffer.size()-startOfAnswer);
  executeCommand();
}

void SensorAgent::executeCommand()
{
	/* This function is called whenever there is a chance that we have a
	 * command to pass to the daemon. But the command may only be sent
	 * if the daemon is online and there is no other command currently
	 * being sent. */
	if (mDaemonOnLine) {
		SensorRequest *req = NULL;
		bool shouldInsert = false;
		while (!mInputFIFO.isEmpty()) {
			req = mInputFIFO.dequeue();
			const QString sRequest = req->request();
			//since we don't batch request in the input queue there will be only one client and one id
			shouldInsert = tryAndBatchRequest(req->request(), req->getClients().at(0), req->getIds().at(0));
			if (shouldInsert) {
#if SA_TRACE
				kDebug(1215) << ">> " << req->request().toAscii() << "(" << mInputFIFO.count()
				<< "/" << mProcessingFIFO.count() << ")" << endl;
#endif
				executeAndEnqueueRequest(req);
			} else
				delete req;
		}
	}
}

void SensorAgent::disconnectClient( SensorClient *client )
{
  for (int i = 0; i < mInputFIFO.size(); ++i)
    mInputFIFO[i]->removeAllRequestWithClient(client);
  for (int i = 0; i < mProcessingFIFO.size(); ++i)
   mProcessingFIFO[i]->removeAllRequestWithClient(client);

}

SensorManager *SensorAgent::sensorManager()
{
  return mSensorManager;
}

void SensorAgent::setDaemonOnLine( bool value )
{
  mDaemonOnLine = value;
}

bool SensorAgent::daemonOnLine() const
{
  return mDaemonOnLine;
}

void SensorAgent::setHostName( const QString &hostName )
{
  mHostName = hostName;
}

QString SensorAgent::hostName() const
{
  return mHostName;
}

QString SensorAgent::reasonForOffline() const
{
  return mReasonForOffline;
}

void SensorAgent::setReasonForOffline(const QString &reasonForOffline)
{
  mReasonForOffline = reasonForOffline;
}

SensorRequest::SensorRequest( const QString &request, SensorClient *client, int id )

{
	mRequest = request;
	clientList.append(client);
	idList.append(id);
}

SensorRequest::~SensorRequest()
{
}






#include "SensorAgent.moc"
