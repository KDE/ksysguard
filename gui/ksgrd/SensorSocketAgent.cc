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

#include "SensorSocketAgent.h"

using namespace KSGRD;

SensorSocketAgent::SensorSocketAgent( SensorManager *sm )
  : SensorAgent( sm )
{
  connect( &mSocket, SIGNAL( error( int ) ), SLOT( error( int ) ) );
  connect( &mSocket, SIGNAL( bytesWritten( int ) ), SLOT( msgSent( int ) ) );
  connect( &mSocket, SIGNAL( readyRead() ), SLOT( msgRcvd() ) );
  connect( &mSocket, SIGNAL( connectionClosed() ), SLOT( connectionClosed() ) );
}

SensorSocketAgent::~SensorSocketAgent()
{
  mSocket.write( "quit\n", strlen( "quit\n" ) );
  mSocket.flush();
}
	
bool SensorSocketAgent::start( const QString &host, const QString&,
                               const QString&, int port )
{
  if ( port <= 0 )
    kDebug(1215) << "SensorSocketAgent::start: Illegal port " << port << endl;

  setHostName( host );
  mPort = port;

  mSocket.connectToHost( hostName(), mPort );

  return true;
}

void SensorSocketAgent::hostInfo( QString &shell, QString &command, int &port ) const
{
  shell.clear();
  command.clear();
  port = mPort;
}

void SensorSocketAgent::msgSent( int )
{
  if ( mSocket.bytesToWrite() != 0 )
    return;

  setTransmitting( false );

  // Try to send next request if available.
  executeCommand();
}

void SensorSocketAgent::msgRcvd()
{
  int buflen = mSocket.bytesAvailable();
  char* buffer = new char[ buflen ];

  mSocket.read( buffer, buflen );

  processAnswer( buffer, buflen ); 
  delete [] buffer;
}

void SensorSocketAgent::connectionClosed()
{
  setDaemonOnLine( false );
  sensorManager()->hostLost( this );
  sensorManager()->requestDisengage( this );
}

void SensorSocketAgent::error( int id )
{
  switch ( id ) {
    case Q3Socket::ErrConnectionRefused:
      SensorMgr->notify( i18n( "Connection to %1 refused" ,
                           hostName() ) );
      break;
    case Q3Socket::ErrHostNotFound:
      SensorMgr->notify( i18n( "Host %1 not found" ,
                           hostName() ) );
      break;
    case Q3Socket::ErrSocketRead:
      SensorMgr->notify( i18n( "Read error at host %1",
                           hostName() ) );
      break;
    default:
      kDebug(1215) << "SensorSocketAgent::error() unknown error " << id << endl;
  }

  setDaemonOnLine( false );
  sensorManager()->requestDisengage( this );
}

bool SensorSocketAgent::writeMsg( const char *msg, int len )
{
  return ( mSocket.write( msg, len ) == len );
}

bool SensorSocketAgent::txReady()
{
  return !transmitting();
}

#include "SensorSocketAgent.moc"
