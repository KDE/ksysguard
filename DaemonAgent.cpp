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

#include <iostream.h>

#include <qapplication.h>

#include "DaemonAgent.h"
#include "DaemonAgent.moc"

DaemonAgent::DaemonAgent()
{
	ktopd = 0;
}

DaemonAgent::~DaemonAgent()
{
	executeCommand("quit");

	delete ktopd;
	ktopd = 0;
}
	
bool
DaemonAgent::start(const QString& host, const QString& shell)
{
	ktopd = new KProcess;

	connect(ktopd, SIGNAL(processExited(KProcess *)),
						  this, SLOT(ktopdExited(KProcess*)));
	connect(ktopd, SIGNAL(receivedStdout(KProcess *, char*, int)),
						  this, SLOT(msgRcvd(KProcess*, char*, int)));
	connect(ktopd, SIGNAL(receivedStderr(KProcess *, char*, int)),
						  this, SLOT(eMsgRcvd(KProcess*, char*, int)));

	*ktopd << "ktopd/ktopd";

	if (!ktopd->start(KProcess::NotifyOnExit, KProcess::All))
		cout << "Can't start ktopd" << endl;
}

void
DaemonAgent::msgSent(KProcess* proc)
{
}

void 
DaemonAgent::msgRcvd(KProcess* proc, char* buffer, int buflen)
{
	answerBuf = buffer;
	cout << "RCVD: " << buffer << endl;
	emit reportAnswer(answerBuf);
}

void
DaemonAgent::errMsgRcvd(KProcess* proc, char* buffer, int buflen)
{
	cout << "RCVD Error: " << buffer << endl;
}

void
DaemonAgent::ktopdExited(KProcess* proc)
{
	cout << "ktopd exited" << endl;
}

void
DaemonAgent::executeCommand(const QString& cmd)
{
	*ktopd << cmd.ascii() << "\n";
	cout << "Command " << cmd << " sent" << endl;
}
