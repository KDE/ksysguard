/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

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

#include "SensorAgent.h"
#include "SensorClient.h"
#include "SensorAgent.moc"

SensorAgent::SensorAgent()
{
	ktopd = 0;
	ktopdOnLine = false;
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
SensorAgent::start(const QString& host, const QString& shell)
{
	ktopd = new KProcess;

	connect(ktopd, SIGNAL(processExited(KProcess *)),
			this, SLOT(ktopdExited(KProcess*)));
	connect(ktopd, SIGNAL(receivedStdout(KProcess *, char*, int)),
			this, SLOT(msgRcvd(KProcess*, char*, int)));
	connect(ktopd, SIGNAL(receivedStderr(KProcess *, char*, int)),
			this, SLOT(errMsgRcvd(KProcess*, char*, int)));
	connect(ktopd, SIGNAL(wroteStdin(KProcess*)), this,
			SLOT(msgSent(KProcess*)));
	*ktopd << "ktopd/Linux/ktopd";

	if (!ktopd->start(KProcess::NotifyOnExit, KProcess::All))
		cout << "Can't start ktopd" << endl;
}

bool
SensorAgent::sendRequest(const QString& req, SensorClient* client, int id)
{
	/* The request is registered with the FIFO so that the answer can be
	 * routed back to the requesting client. */
	inputFIFO.prepend(new SensorRequest(req, client, id));
	if (ktopdOnLine && (inputFIFO.count() == 1))
		executeCommand();
}

void
SensorAgent::msgSent(KProcess* proc)
{
	// remove oldest (sent) request from input FIFO
	SensorRequest* req = inputFIFO.last();
	inputFIFO.removeLast();
	delete req;

	// Try to send next request if available.
	executeCommand();
}

void 
SensorAgent::msgRcvd(KProcess* proc, char* buffer, int buflen)
{
	answerBuffer += QString(buffer).left(buflen);

	int end;
	while ((end = answerBuffer.find("\nktopd> ")) > 0)
	{
		if (!ktopdOnLine)
		{
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
SensorAgent::errMsgRcvd(KProcess* proc, char* buffer, int buflen)
{
	cout << "RCVD Error: " << buffer << endl;
}

void
SensorAgent::ktopdExited(KProcess* proc)
{
	ktopdOnLine = false;
	cout << "ktopd exited" << endl;
}

void
SensorAgent::executeCommand(void)
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
