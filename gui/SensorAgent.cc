/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

#include "SensorManager.h"
#include "SensorAgent.h"
#include "SensorClient.h"
#include "SensorAgent.moc"

SensorAgent::SensorAgent(SensorManager* sm) :
	sensorManager(sm)
{
	ktopd = 0;
	ktopdOnLine = false;
	pwSent = false;
}

SensorAgent::~SensorAgent()
{
	ktopd->writeStdin("quit\n", strlen("quit\n"));

	delete ktopd;
	ktopd = 0;

	while (!inputFIFO.isEmpty())
	{
		delete inputFIFO.first();
		inputFIFO.removeFirst();
	}
	while (!processingFIFO.isEmpty())
	{
		delete processingFIFO.first();
		processingFIFO.removeFirst();
	}
}
	
bool
SensorAgent::start(const QString& host_, const QString& shell_,
				   const QString& command_)
{
	ktopd = new KProcess;
	CHECK_PTR(ktopd);

	host = host_;
	shell = shell_;
	command = command_;

	connect(ktopd, SIGNAL(processExited(KProcess *)),
			this, SLOT(ktopdExited(KProcess*)));
	connect(ktopd, SIGNAL(receivedStdout(KProcess *, char*, int)),
			this, SLOT(msgRcvd(KProcess*, char*, int)));
	connect(ktopd, SIGNAL(receivedStderr(KProcess *, char*, int)),
			this, SLOT(errMsgRcvd(KProcess*, char*, int)));
	connect(ktopd, SIGNAL(wroteStdin(KProcess*)), this,
			SLOT(msgSent(KProcess*)));

	if (command != "")
	{
		debug(command);
		// We assume parameters to be seperated by a single blank.
		QString s = command;
		while (s.length() > 0)
		{
			int sep;

			if ((sep = s.find(' ')) < 0)
			{
				*ktopd << s.utf8();
				break;
			}
			else
			{
				*ktopd << s.left(sep).utf8();
				s = s.remove(0, sep + 1);
			}
		}
	}
	else
	{
		*ktopd << shell;
		*ktopd << host;
		*ktopd << "ktopd";
	}

	if (!ktopd->start(KProcess::NotifyOnExit, KProcess::All))
	{
		debug("Can't start ktopd");
		return (false);
	}

	return (true);
}

bool
SensorAgent::sendRequest(const QString& req, SensorClient* client, int id)
{
	/* The request is registered with the FIFO so that the answer can be
	 * routed back to the requesting client. */
	inputFIFO.prepend(new SensorRequest(req, client, id));
	if (ktopdOnLine && (inputFIFO.count() == 1))
	{
		executeCommand();
		return (true);
	}

	return (false);
}

void
SensorAgent::msgSent(KProcess*)
{
	if (pwSent)
	{
		debug("Password sent");
		pwSent = false;
		return;
	}

	// remove oldest (sent) request from input FIFO
	SensorRequest* req = inputFIFO.last();
	inputFIFO.removeLast();
	delete req;

	// Try to send next request if available.
	executeCommand();
}

void 
SensorAgent::msgRcvd(KProcess*, char* buffer, int buflen)
{
	if (buflen > 0)
	{
		char* aux = new char[buflen + 1];
		strncpy(aux, buffer, buflen);
		aux[buflen] = '\0';
		answerBuffer += QString(aux);
		delete [] aux;
	}

	int end;
	while ((end = answerBuffer.find("\nktopd> ")) > 0)
	{
		if (!ktopdOnLine)
		{
			debug("Sensor is on-line");
			/* First '\nktopd> ' signals that ktopd is ready to serve requests
			 * now. */
			ktopdOnLine = true;
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
			processingFIFO.removeLast();
			
			// Notify client of newly arrived answer.
			req->client->answerReceived(req->id, answerBuffer.left(end));
			delete req;
		}

		// chop of processed part answer buffer
		answerBuffer.remove(0, end + strlen("\nktopd> "));
	}
}

void
SensorAgent::errMsgRcvd(KProcess*, char* buffer, int buflen)
{
	QString errorBuffer;
	if (buflen > 0)
	{
		char* aux = new char[buflen + 1];
		strncpy(aux, buffer, buflen);
		aux[buflen] = '\0';
		errorBuffer = aux;
		debug("ERR: %s", aux);
		delete [] aux;
	}

	if (errorBuffer.find("assword: ") > 0)
	{
		QCString password;
		int result = KPasswordDialog::
			getPassword(password, errorBuffer);
		QCString cmdWithNL;
		if (result == KPasswordDialog::Accepted)
			cmdWithNL = password + "\n";
		else
			cmdWithNL = "\n";
		ktopd->writeStdin(cmdWithNL.data(), cmdWithNL.length());
		pwSent = true;
	}
	else
	{
//		sensorManager->disengage(this);
	}
}

void
SensorAgent::ktopdExited(KProcess*)
{
	ktopdOnLine = false;
	debug("ktopd exited");
	sensorManager->disengage(this);
}

void
SensorAgent::executeCommand()
{
	if (inputFIFO.isEmpty())
		return;

	// take request for input FIFO
	SensorRequest* req = inputFIFO.last();

	// send request to ktopd
	QString cmdWithNL = req->request + "\n";
	ktopd->writeStdin(cmdWithNL.ascii(), cmdWithNL.length());

	// add request to processing FIFO
	processingFIFO.prepend(new SensorRequest(*req));
}
