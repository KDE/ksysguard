/*
    KTop, the KDE Task Manager
   
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
#include <iostream.h>

#include <qevent.h>
#include <qapplication.h>
#include <qstring.h>

#include <klocale.h>
#include <kprocess.h>
#include <kpassdlg.h> 
#include <kmessagebox.h>
#include <kdebug.h>

#include "SensorManager.h"
#include "SensorClient.h"
#include "SensorSocketAgent.moc"

SensorSocketAgent::SensorSocketAgent(SensorManager* sm) :
	SensorAgent(sm)
{
	connect(&socket, SIGNAL(error(int)), this, SLOT(error(int)));
	connect(&socket, SIGNAL(bytesWritten(int)),
			this, SLOT(msgSent(int)));
	connect(&socket, SIGNAL(readyRead()),
			this, SLOT(msgRcvd()));
	connect(&socket, SIGNAL(delayedCloseFinished()),
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
		kdDebug() << "SensorSocketAgent::start: Illegal port " << port_
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

	// remove oldest (sent) request from input FIFO
	SensorRequest* req = inputFIFO.last();
	inputFIFO.removeLast();
	delete req;

	// Try to send next request if available.
	executeCommand();
}

void 
SensorSocketAgent::msgRcvd()
{
	int buflen = socket.bytesAvailable();
	if (buflen <= 0)
	{
		executeCommand();
		return;
	}

	char* buffer = new char[buflen];
	socket.readBlock(buffer, buflen);
	QString buf = QString::fromLocal8Bit(buffer, buflen);
	delete [] buffer;

	if (buf.left(3) == "\033\033\033")
	{
		errorBuffer += buf;
		
		int start = 0;
		int end;
		while ((end = errorBuffer.find("\n", start)) > 0)
		{
			if (errorBuffer.mid(start, end - start) == "RECONFIGURE")
			{
				emit reconfigure(this);
				errorBuffer.remove(start, end - start + 1);
			}
			start = end + 1;
		}
		
		if (!errorBuffer.isEmpty())
		{
			SensorMgr->notify(QString(i18n("Message from %1:\n%2")
									  .arg(host).arg(errorBuffer)));
			errorBuffer = "";
		}
	}
	else
	{
		int end;
		answerBuffer += buf;
		
		while ((end = answerBuffer.find("\nksysguardd> ")) > 0)
		{
			if (!daemonOnLine)
			{
				/* First '\nksysguardd> ' signals that daemon is
				 * ready to serve requests now. */
				daemonOnLine = true;
				answerBuffer = QString();
				if (!inputFIFO.isEmpty())
				{
					// If FIFO is not empty send out first request.
					executeCommand();
				}
				return;
			}
			
			if (!processingFIFO.isEmpty())
			{
				// remove pending request from FIFO
				SensorRequest* req = processingFIFO.last();
				if (!req)
				{
					kdDebug()
						<< "ERROR: Received answer but have no pending "
						<< "request!" << endl;
					return;
				}
				processingFIFO.removeLast();
				
				if (!req->client)
				{
					kdDebug ()
						<< "ERROR: No client registered for request!"
						<< endl;
					return;
				}
				if (answerBuffer.left(end) == "UNKNOWN COMMAND")
				{
					// Notify client of newly arrived answer.
					req->client->sensorLost(req->id);
				}
				else
				{
					// Notify client of newly arrived answer.
					req->client->answerReceived(req->id,
												answerBuffer.left(end));
				}
				delete req;
			}

			// chop of processed part answer buffer
			answerBuffer.remove(0, end + strlen("\nksysguardd> "));
		}
	}

	executeCommand();
}

void
SensorSocketAgent::connectionClosed()
{
	daemonOnLine = false;
	sensorManager->hostLost(this);
	sensorManager->requestDisengage(this);
}

void
SensorSocketAgent::executeCommand()
{
	if (inputFIFO.isEmpty() || processingFIFO.count() >= 1)
		return;

	// take request for input FIFO
	SensorRequest* req = inputFIFO.last();

	// send request to daemon
	QString cmdWithNL = req->request + "\n";
	socket.writeBlock(cmdWithNL.ascii(), cmdWithNL.length());

	// add request to processing FIFO
	processingFIFO.prepend(new SensorRequest(*req));
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
		kdDebug() << "SensorSocketAgent::error() unkown error " << id << endl;
	}

	daemonOnLine = false;
	sensorManager->requestDisengage(this);
}
