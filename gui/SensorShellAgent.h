/*
    KSysGuard, the KDE Task Manager
   
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

#ifndef _SensorShellAgent_h_
#define _SensorShellAgent_h_

#include <qobject.h>
#include <qptrlist.h>

#include <SensorAgent.h>

class QString;
class KProcess;
class KShellProcess;
class SensorClient;
class SensorManager;

/**
 * The SensorShellAgent starts a ksysguardd process and handles the
 * asynchronous communication. It keeps a list of pending requests
 * that have not been answered yet by ksysguard. The current
 * implementation only allowes one pending requests. Incoming requests
 * are queued in an input FIFO.
 */
class SensorShellAgent : public SensorAgent
{
	Q_OBJECT

public:
	SensorShellAgent(SensorManager* sm);
	~SensorShellAgent();

	bool start(const QString& host, const QString& shell,
					   const QString& command = "", int port = -1);

	void getHostInfo(QString& sh, QString& cmd, int& port) const
	{
		sh = shell;
		cmd = command;
		port = -1;
	}

private slots:
	void msgSent(KProcess*);
	void msgRcvd(KProcess*, char* buffer, int buflen);
	void errMsgRcvd(KProcess*, char* buffer, int buflen);
	void daemonExited(KProcess*);

private:
	bool writeMsg(const char* msg, int len);
	bool txReady()
	{
		return (!transmitting);
	}

	KShellProcess* daemon;
	QString shell;
	QString command;
} ;
	
#endif
