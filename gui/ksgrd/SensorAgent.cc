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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    $Id$
*/

#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpassdlg.h> 

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
  /* SensorRequests migrate from the inputFIFO to the processingFIFO. So
   * we only have to delete them when they are removed from the
   * processingFIFO. */
  mInputFIFO.setAutoDelete( false );
  mProcessingFIFO.setAutoDelete( true );

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
  mInputFIFO.prepend( new SensorRequest( req, client, id ) );

#if SA_TRACE
  kdDebug(1215) << "-> " << req << "(" << mInputFIFO.count() << "/"
                << mProcessingFIFO.count() << ")" << endl;
#endif
  executeCommand();

  return false;
}

void SensorAgent::processAnswer( const QString &buffer )
{
#if SA_TRACE
  kdDebug(1215) << "<- " << buffer << endl;
#endif

  for ( uint i = 0; i < buffer.length(); i++ ) {
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
        mErrorBuffer = QString::null;
      }
    } else if ( mState == 0 ) // receiving to answerBuffer
      mAnswerBuffer += buffer[ i ];
    else  // receiving to errorBuffer
      mErrorBuffer += buffer[ i ];
  }

  int end;
  // And now the real information
  while ( ( end = mAnswerBuffer.find( "\nksysguardd> " ) ) >= 0 ) {
#if SA_TRACE
    kdDebug(1215) << "<= " << mAnswerBuffer.left( end )
                  << "(" << mInputFIFO.count() << "/"
                  << mProcessingFIFO.count() << ")" << endl;
#endif
    if ( !mDaemonOnLine ) {
      /* First '\nksysguardd> ' signals that the daemon is
       * ready to serve requests now. */
      mDaemonOnLine = true;
#if SA_TRACE
      kdDebug(1215) << "Daemon now online!" << endl;
#endif
      mAnswerBuffer = QString::null;
      break;
    }
	
    // remove pending request from FIFO
    SensorRequest* req = mProcessingFIFO.last();
    if ( !req ) {
      kdDebug(1215)	<< "ERROR: Received answer but have no pending "
                    << "request!" << endl;
      return;
    }
		
    if ( !req->client() ) {
      /* The client has disappeared before receiving the answer
       * to his request. */
      mProcessingFIFO.removeLast();
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
    mProcessingFIFO.removeLast();

    // chop of processed part of the answer buffer
    mAnswerBuffer.remove( 0, end + strlen( "\nksysguardd> " ) );
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
    // take oldest request for input FIFO
    SensorRequest* req = mInputFIFO.last();
    mInputFIFO.removeLast();

#if SA_TRACE
    kdDebug(1215) << ">> " << req->request().ascii() << "(" << mInputFIFO.count()
                  << "/" << mProcessingFIFO.count() << ")" << endl;
#endif
    // send request to daemon
    QString cmdWithNL = req->request() + "\n";
    if ( writeMsg( cmdWithNL.ascii(), cmdWithNL.length() ) )
      mTransmitting = true;
    else
      kdDebug(1215) << "SensorAgent::writeMsg() failed" << endl;

    // add request to processing FIFO
    mProcessingFIFO.prepend( req );
  }
}

void SensorAgent::disconnectClient( SensorClient *client )
{
  for ( SensorRequest *req = mInputFIFO.first(); req; req = mInputFIFO.next() )
    if ( req->client() == client )
      req->setClient( 0 );
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
