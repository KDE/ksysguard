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

#ifndef _SensorAgent_h_
#define _SensorAgent_h_

#include <qobject.h>
#include <qlist.h>

class QString;
class KProcess;
class KShellProcess;
class SensorClient;
class SensorManager;

/**
 * This auxilliary class is used to store requests during their processing.
 */
class SensorRequest
{
	friend class SensorAgent;

public:
	SensorRequest(const QString& r, SensorClient* c, int i) :
		request(r), client(c), id(i) { }
	~SensorRequest() { }

private:
	QString request;
	SensorClient* client;
	int id;
} ;

/**
 * The SensorAgent starts a ksysguardd process and handles the asynchronous
 * communication. It keeps a list of pending requests that have not been
 * answered yet by ksysguard. The current implementation only allowes one
 * pending requests. Incoming requests are queued in an input FIFO.
 */
class SensorAgent : public QObject
{
	Q_OBJECT

public:
	SensorAgent(SensorManager* sm);
	~SensorAgent();

	bool start(const QString& host, const QString& shell,
			   const QString& command = "");

	/**
	 * This function should only be used by the the SensorManager and
	 * never by the SensorClients directly since the pointer returned by
	 * engaged is not guaranteed to be valid. Only the SensorManager knows
	 * whether a SensorAgent pointer is still valid or not.
	 *
	 * This function sends out a command to the sensor and notifies the
	 * agent to return the answer to 'client'. The 'id' can be used by the
	 * client to identify the answer. It is only passed through and never
	 * used by the SensorAgent. So it can be any value the client suits to
	 * use.
	 */
	bool sendRequest(const QString& req, SensorClient* client, int id = 0);

	const QString& getHostName() const
	{
		return (host);
	}

	void getHostInfo(QString& sh, QString& cmd) const
	{
		sh = shell;
		cmd = command;
	}

signals:
	void reconfigure(const SensorAgent*);

private slots:
	void msgSent(KProcess*);
	void msgRcvd(KProcess*, char* buffer, int buflen);
	void errMsgRcvd(KProcess*, char* buffer, int buflen);
	void daemonExited(KProcess*);

private:
	void executeCommand();

	SensorManager* sensorManager;

	KShellProcess* daemon;
	bool daemonOnLine;
	bool pwSent;
	QString host;
	QString shell;
	QString command;

	QList<SensorRequest> inputFIFO;
	QList<SensorRequest> processingFIFO;
	QString answerBuffer;
	QString errorBuffer;
} ;
	
#endif
