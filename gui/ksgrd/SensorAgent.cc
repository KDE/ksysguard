/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpassworddialog.h> 

#include "SensorClient.h"
#include "SensorManager.h"

#include "SensorAgent.h"

/**
  This can be used to debug communication problems with the daemon.
  Should be set to 0 in any production version.
*/
#define SA_TRACE 0

using namespace KSGRD;

SensorAgent::SensorAgent( SensorManager *sm )
  : mSensorManager( sm )
{
  mDaemonOnLine = false;
  mTransmitting = false;
  mState = 0;
}

SensorAgent::~SensorAgent()
{
}

bool SensorAgent::sendRequest( const QString &req, SensorClient *client, int id )
{
  /* The request is registered with the FIFO so that the answer can be
   * routed back to the requesting client. */
  if(mInputFIFO.count() < 10)   //If we have too many requests, just simply drop the request.  Not great but better than nothing..
    mInputFIFO.enqueue( new SensorRequest( req, client, id ) );

#if SA_TRACE
  kDebug(1215) << "-> " << req << "(" << mInputFIFO.count() << "/"
                << mProcessingFIFO.count() << ")" << endl;
#endif
  executeCommand();

  return false;
}

void SensorAgent::processAnswer( const QString &buffer )
{
#if SA_TRACE
  kDebug(1215) << "<- " << buffer << endl;
#endif

  for ( int i = 0; i < buffer.length(); i++ ) {
    if ( buffer[ i ] == '\033' ) {
      mState = ( mState + 1 ) & 1;
      if ( !mErrorBuffer.isEmpty() && mState == 0 ) {
        if ( mErrorBuffer == "RECONFIGURE\n" )
          emit reconfigure( this );
        else {
          /* We just received the end of an error message, so we
           * can display it. */
          SensorMgr->notify( i18n( "Message from %1:\n%2" )
                           .arg( mHostName )
                           .arg( mErrorBuffer ) );
        }
        mErrorBuffer.clear();
      }
    } else if ( mState == 0 ) // receiving to answerBuffer
      mAnswerBuffer += buffer[ i ];
    else  // receiving to errorBuffer
      mErrorBuffer += buffer[ i ];
  }

  int end;
  // And now the real information
  while ( ( end = mAnswerBuffer.indexOf( "\nksysguardd> " ) ) >= 0 ) {
#if SA_TRACE
    kDebug(1215) << "<= " << mAnswerBuffer.left( end )
                  << "(" << mInputFIFO.count() << "/"
                  << mProcessingFIFO.count() << ")" << endl;
#endif
    if ( !mDaemonOnLine ) {
      /* First '\nksysguardd> ' signals that the daemon is
       * ready to serve requests now. */
      mDaemonOnLine = true;
#if SA_TRACE
      kDebug(1215) << "Daemon now online!" << endl;
#endif
      mAnswerBuffer.clear();
      break;
    }
	
    // remove pending request from FIFO
    if ( mProcessingFIFO.isEmpty() ) {
      kDebug(1215)	<< "ERROR: Received answer but have no pending "
                    << "request!" << endl;
      return;
    }

    SensorRequest *req = mProcessingFIFO.dequeue();
		
    if ( !req->client() ) {
      /* The client has disappeared before receiving the answer
       * to his request. */
      delete req;
      return;
    }

    if ( mAnswerBuffer.left( end ) == "UNKNOWN COMMAND" ) {
      /* Notify client that the sensor seems to be no longer
       * available. */
      req->client()->sensorLost( req->id() );
    } else {
      // Notify client of newly arrived answer.
      req->client()->answerReceived( req->id(), mAnswerBuffer.left( end ) );
    }
    delete req;
    // chop of processed part of the answer buffer
    mAnswerBuffer.remove( 0, end + sizeof( "\nksysguardd> " )-1 );  /*sizeof(x)-1  is a hackish way to save us from having to do strlen :) */
  }

  executeCommand();
}

void SensorAgent::executeCommand()
{
  /* This function is called whenever there is a chance that we have a
   * command to pass to the daemon. But the command many only be send
   * if the daemon is online and there is no other command currently
   * being sent. */
  if ( mDaemonOnLine && txReady() && !mInputFIFO.isEmpty() ) {
    SensorRequest *req = mInputFIFO.dequeue();

#if SA_TRACE
    kDebug(1215) << ">> " << req->request().ascii() << "(" << mInputFIFO.count()
                  << "/" << mProcessingFIFO.count() << ")" << endl;
#endif
    // send request to daemon
    QString cmdWithNL = req->request() + "\n";
    if ( writeMsg( cmdWithNL.toLatin1(), cmdWithNL.length() ) )
      mTransmitting = true;
    else
      kDebug(1215) << "SensorAgent::writeMsg() failed" << endl;

    // add request to processing FIFO
    mProcessingFIFO.enqueue( req );
  }
}

void SensorAgent::disconnectClient( SensorClient *client )
{
  for (int i = 0; i < mInputFIFO.size(); ++i)
    if ( mInputFIFO[i]->client() == client )
      mInputFIFO[i]->setClient(0);
  for (int i = 0; i < mProcessingFIFO.size(); ++i)
    if ( mProcessingFIFO[i]->client() == client )
      mProcessingFIFO[i]->setClient( 0 );
  
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

void SensorAgent::setTransmitting( bool value )
{
  mTransmitting = value;
}

bool SensorAgent::transmitting() const
{
  return mTransmitting;
}

void SensorAgent::setHostName( const QString &hostName )
{
  mHostName = hostName;
}

const QString &SensorAgent::hostName() const
{
  return mHostName;
}


SensorRequest::SensorRequest( const QString &request, SensorClient *client, int id )
  : mRequest( request ), mClient( client ), mId( id )
{
}

SensorRequest::~SensorRequest()
{
}

void SensorRequest::setRequest( const QString &request )
{
  mRequest = request;
}

QString SensorRequest::request() const
{
  return mRequest;
}

void SensorRequest::setClient( SensorClient *client )
{
  mClient = client;
}

SensorClient *SensorRequest::client()
{
  return mClient;
}

void SensorRequest::setId( int id )
{
  mId = id;
}

int SensorRequest::id()
{
  return mId;
}

#include "SensorAgent.moc"
