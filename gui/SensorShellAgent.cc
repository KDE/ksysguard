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
#include "SensorShellAgent.moc"

SensorShellAgent::SensorShellAgent(SensorManager* sm) :
	SensorAgent(sm)
{
}

SensorShellAgent::~SensorShellAgent()
{
	if (daemon)
	{
		daemon->writeStdin("quit\n", strlen("quit\n"));
		delete daemon;
		daemon = 0;
	}
}
	
bool
SensorShellAgent::start(const QString& host_, const QString& shell_,
						const QString& command_, int)
{
	daemon = new KShellProcess;
	CHECK_PTR(daemon);

	host = host_;
	shell = shell_;
	command = command_;

	connect(daemon, SIGNAL(processExited(KProcess *)),
			this, SLOT(daemonExited(KProcess*)));
	connect(daemon, SIGNAL(receivedStdout(KProcess *, char*, int)),
			this, SLOT(msgRcvd(KProcess*, char*, int)));
	connect(daemon, SIGNAL(receivedStderr(KProcess *, char*, int)),
			this, SLOT(errMsgRcvd(KProcess*, char*, int)));
	connect(daemon, SIGNAL(wroteStdin(KProcess*)), this,
			SLOT(msgSent(KProcess*)));

	QString cmd;
	if (command != "")
		cmd =  command;
	else
		cmd = shell + " " + host + " ksysguardd";
	*daemon << cmd;

	if (!daemon->start(KProcess::NotifyOnExit, KProcess::All))
	{
		sensorManager->hostLost(this);
		kdDebug () << "Command '" << cmd << "' failed"  << endl;
		return (false);
	}

	return (true);
}

void
SensorShellAgent::msgSent(KProcess*)
{
	// remove oldest (sent) request from input FIFO
	SensorRequest* req = inputFIFO.last();
	inputFIFO.removeLast();
	delete req;

	// Try to send next request if available.
	executeCommand();
}

void 
SensorShellAgent::msgRcvd(KProcess*, char* buffer, int buflen)
{
	if (!buffer || buflen == 0)
		return;

	QString aux = QString::fromLocal8Bit(buffer, buflen);
	answerBuffer += aux;

	int end;
	while ((end = answerBuffer.find("\nksysguardd> ")) > 0)
	{
		if (!daemonOnLine)
		{
			/* First '\nksysguardd> ' signals that daemon is ready to serve
			 * requests now. */
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
					<< "ERROR: Received answer but have no pending request!"
					<< endl;
				return;
			}
			processingFIFO.removeLast();
			
			if (!req->client)
			{
				kdDebug ()
					<< "ERROR: No client registered for request!" << endl;
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
				req->client->answerReceived(req->id, answerBuffer.left(end));
			}
			delete req;
		}

		// chop of processed part answer buffer
		answerBuffer.remove(0, end + strlen("\nksysguardd> "));
	}
}

void
SensorShellAgent::errMsgRcvd(KProcess*, char* buffer, int buflen)
{
	if (!buffer || buflen == 0)
		return;

	errorBuffer += QString::fromLocal8Bit(buffer, buflen);

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

void
SensorShellAgent::daemonExited(KProcess*)
{
	daemonOnLine = false;
	sensorManager->hostLost(this);
	sensorManager->disengage(this);
}

void
SensorShellAgent::executeCommand()
{
	if (inputFIFO.isEmpty())
		return;

	// take request for input FIFO
	SensorRequest* req = inputFIFO.last();

	// send request to daemon
	QString cmdWithNL = req->request + "\n";
	daemon->writeStdin(cmdWithNL.ascii(), cmdWithNL.length());

	// add request to processing FIFO
	processingFIFO.prepend(new SensorRequest(*req));
}
