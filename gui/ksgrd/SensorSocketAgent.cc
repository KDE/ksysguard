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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id$
*/

#include <stdlib.h>

#include <klocale.h>
#include <kpassdlg.h> 
#include <kdebug.h>

#include "SensorManager.h"
#include "SensorClient.h"
#include "SensorSocketAgent.moc"

using namespace KSGRD;

SensorSocketAgent::SensorSocketAgent(SensorManager* sm) :
	SensorAgent(sm)
{
	connect(&socket, SIGNAL(error(int)), this, SLOT(error(int)));
	connect(&socket, SIGNAL(bytesWritten(int)),
			this, SLOT(msgSent(int)));
	connect(&socket, SIGNAL(readyRead()),
			this, SLOT(msgRcvd()));
	connect(&socket, SIGNAL(connectionClosed()),
			this, SLOT(connectionClosed()));
}

SensorSocketAgent::~SensorSocketAgent()
{
	socket.writeBlock("quit\n", strlen("quit\n"));
	socket.flush();
}
	
bool
SensorSocketAgent::start(const QString& host_, const QString&,
				   const QString&, int port_)
{
	if (port_ <= 0)
		kdDebug(1215) << "SensorSocketAgent::start: Illegal port " << port_
				  << endl;

	host = host_;
	port = port_;

	socket.connectToHost(host, port);
	return (true);
}

void
SensorSocketAgent::msgSent(int)
{
	if (socket.bytesToWrite() != 0)
		return;

	transmitting = false;
	// Try to send next request if available.
	executeCommand();
}

void 
SensorSocketAgent::msgRcvd()
{
	int buflen = socket.bytesAvailable();
	char* buffer = new char[buflen];
	socket.readBlock(buffer, buflen);
	QString buf = QString::fromLocal8Bit(buffer, buflen);
	delete [] buffer;

	processAnswer(buf);
}

void
SensorSocketAgent::connectionClosed()
{
	daemonOnLine = false;
	sensorManager->hostLost(this);
	sensorManager->requestDisengage(this);
}

void
SensorSocketAgent::error(int id)
{
	switch (id)
	{
	case QSocket::ErrConnectionRefused:
		SensorMgr->notify(QString(i18n("Connection to %1 refused").arg(host)));
		break;

	case QSocket::ErrHostNotFound:
		SensorMgr->notify(QString(i18n("Host %1 not found").arg(host)));
		break;

	case QSocket::ErrSocketRead:
		SensorMgr->notify(QString(i18n("Read error at host %1").arg(host)));
		break;

	default:
		kdDebug(1215) << "SensorSocketAgent::error() unkown error " << id << endl;
	}

	daemonOnLine = false;
	sensorManager->requestDisengage(this);
}

bool
SensorSocketAgent::writeMsg(const char* msg, int len)
{
	return (socket.writeBlock(msg, len) == len);
}
