/*
    KSysGuard, the KDE System Guard
   
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

#ifndef _SensorManager_h_
#define _SensorManager_h_

#include <qobject.h>
#include <qdict.h>

#include <kconfig.h>

#include <SensorAgent.h>

class HostConnector;

namespace KSGRD {

class SensorManagerIterator;

/**
 * The SensorManager handles all interaction with the connected
 * hosts. Connections to a specific hosts are handled by
 * SensorAgents. Use engage() to establish a connection and
 * disengage() to terminate the connection. If you don't know if a
 * certain host is already connected use engageHost(). If there is no
 * connection yet or the hostname is empty, a dialog will be shown to
 * enter the connections details.
 */
class SensorManager : public QObject
{
	Q_OBJECT

	friend class SensorManagerIterator;

public:
	SensorManager();
	~SensorManager();

	bool engageHost(const QString& hostname);
	bool engage(const QString& hostname, const QString& shell = "ssh",
				const QString& command = "", int port = -1);
	void requestDisengage(const SensorAgent* sensor);
	bool disengage(const SensorAgent* sensor);
	bool disengage(const QString& hostname);
	bool resynchronize(const QString& hostname);
	void hostLost(const SensorAgent* sensor);
	void notify(const QString& msg) const;
	void setBroadcaster(QWidget* bc)
	{
		broadcaster = bc;
	}
	virtual bool event(QEvent* e);

	bool sendRequest(const QString& hostname, const QString& req,
					 SensorClient* client, int id = 0);

	const QString getHostName(const SensorAgent* sensor) const;
	bool getHostInfo(const QString& host, QString& shell, QString& command,
					 int& port);

	const QString& translateUnit(const QString& u) const
	{
		if (!u.isEmpty() && units[u])
			return (*units[u]);
		else
			return (u);
	}

	const QString& trSensorPath(const QString& u) const
	{
		if (!u.isEmpty() && dict[u])
			return (*dict[u]);
		else
			return (u);
	}
	
	const QString& trSensorType(const QString& u) const
	{
		if (!u.isEmpty() && types[u])
			return (*types[u]);
		else
			return (u);
	}
	
	QString translateSensor(const QString& u) const;
	void readProperties(KConfig* cfg);
	void saveProperties(KConfig* cfg);

	void unlinkClient(SensorClient* client);
	
public slots:
	void reconfigure(const SensorAgent* sensor);

signals:
	void update();
	void hostConnectionLost(const QString& hostName);

protected:
	QDict<SensorAgent> sensors;

protected slots:
	void helpConnectHost();

private:
	/**
	 * These dictionary stores the localized versions of the sensor
	 * descriptions and units.
	 */
	QDict<QString> descriptions;
	QDict<QString> units;
	QDict<QString> dict;
	QDict<QString> types;

	QWidget* broadcaster;

	HostConnector* hostConnector;
} ;

extern SensorManager* SensorMgr;

class SensorManagerIterator : public QDictIterator<SensorAgent>
{
public:
	SensorManagerIterator(const SensorManager* sm) :
		QDictIterator<SensorAgent>(sm->sensors) { }
	~SensorManagerIterator() { }
};
};

#endif
