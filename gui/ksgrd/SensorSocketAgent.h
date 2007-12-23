/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

#ifndef KSG_SENSORSOCKETAGENT_H
#define KSG_SENSORSOCKETAGENT_H

#include <QtNetwork/QTcpSocket>

#include "SensorAgent.h"

class QString;

namespace KSGRD {


/**
  The SensorSocketAgent connects to a ksysguardd via a TCP
  connection. It keeps a list of pending requests that have not been
  answered yet by ksysguard. The current implementation only allowes
  one pending requests. Incoming requests are queued in an input
  FIFO.
 */
class SensorSocketAgent : public SensorAgent
{
  Q_OBJECT

  public:
    explicit SensorSocketAgent( SensorManager *sm );
    ~SensorSocketAgent();

    bool start( const QString &host, const QString &shell,
                const QString &command = "", int port = -1 );

    void hostInfo( QString &shell, QString &command, int &port ) const;

  private Q_SLOTS:
    void connectionClosed();
    void msgSent();
    void msgRcvd();
    void error( QAbstractSocket::SocketError );

  private:
    bool writeMsg( const char *msg, int len );

    QTcpSocket mSocket;
    int mPort;
};

}
	
#endif
