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

#ifndef _DaemonAgent_h_
#define _DaemonAgent_h_

#include <qobject.h>
#include <qstring.h>

#include <kprocess.h>

class DaemonAgent : public QObject
{
	Q_OBJECT

public:
	DaemonAgent();
	~DaemonAgent();

	bool start(const QString& host, const QString& shell);

	signals:
	void reportAnswer(const QString& answer);

private slots:
	void msgSent(KProcess* proc);
	void msgRcvd(KProcess* proc, char* buffer, int buflen);
	void errMsgRcvd(KProcess* proc, char* buffer, int buflen);
	void ktopdExited(KProcess* proc);

private:
	void executeCommand(const QString& cmd);

	KProcess* ktopd;
	QString host;
	QString shell;

	QString answerBuf;
} ;
	
#endif
