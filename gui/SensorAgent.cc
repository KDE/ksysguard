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
#include "SensorAgent.h"
#include "SensorClient.h"
#include "SensorAgent.moc"

SensorAgent::SensorAgent(SensorManager* sm) :
	sensorManager(sm)
{
	inputFIFO.setAutoDelete(true);
	processingFIFO.setAutoDelete(true);

	daemonOnLine = false;
	state = 1;
}

SensorAgent::~SensorAgent()
{
}
	
bool
SensorAgent::sendRequest(const QString& req, SensorClient* client, int id)
{
	/* The request is registered with the FIFO so that the answer can be
	 * routed back to the requesting client. */
	inputFIFO.prepend(new SensorRequest(req, client, id));
	if (daemonOnLine && (inputFIFO.count() == 1))
	{
		executeCommand();
		return (true);
	}

	return (false);
}

void
SensorAgent::processAnswer(const QString& buf)
{
	for (uint i = 0; i < buf.length(); i++)
		switch (state)
		{
		case 0: // receiving to answerBuffer
			if (buf[i] == '\n')
				state = 1;
			answerBuffer += buf[i];
			break;
		case 1: // last character was a '\n'
		case 2: // last characters were "\n\033"
		case 3: // last characters were "\n\033\033"
			if (buf[i] == '\033')
				state++;
			else
			{
				for (int j = 0; j < state - 1; j++)
					answerBuffer += '\033';
				answerBuffer += buf[i];
				state = 0;
			}
			break;
		case 4:	// receiving to errorBuffer
			if (buf[i] == '\n')
				state = 0;
			errorBuffer += buf[i];
			break;
		}

	// Now look at the error messages
	int start = 0;
	int end;
	while ((end = errorBuffer.find("\n", start)) >= 0)
	{
		if (errorBuffer.mid(start, end - start) == "RECONFIGURE")
		{
			emit reconfigure(this);
			errorBuffer.remove(start, end - start + 1);
		}
		start = end + 1;
	}
	
	if ((end = errorBuffer.find("\n")) >= 0)
	{
		SensorMgr->notify(QString(i18n("Message from %1:\n%2")
								  .arg(host)
								  .arg(errorBuffer.left(end + 1))));
		errorBuffer.remove(0, end + 1);
	}

	// And now the real information
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
				
			if (!req->client)
			{
				kdDebug ()
					<< "ERROR: No client registered for request!"
					<< endl;
				processingFIFO.removeLast();
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
			processingFIFO.removeLast();
		}

		// chop of processed part of the answer buffer
		answerBuffer.remove(0, end + strlen("\nksysguardd> "));
	}
}
