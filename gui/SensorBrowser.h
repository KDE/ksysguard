/*
    KTop, the KDE Task Manager and System Monitor
   
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _SensorBrowser_h_
#define _SensorBrowser_h_

#include <qlistview.h>
#include <qdict.h>

#include "SensorClient.h"

class QMouseEvent;
class KIconLoader;
class SensorManager;
class SensorAgent;

/**
 * The SensorBrowser is the graphical front-end of the SensorManager. It
 * displays the currently available hosts and their sensors.
 */
class SensorInfo
{
public:
	SensorInfo(QListViewItem* l, const QString& n, const QString& d,
			   const QString& t)
		: lvi(l), name(n), description(d), type(t) { }
	~SensorInfo() { }

	QListViewItem* getLVI() const
	{
		return (lvi);
	}

	const QString& getName() const
	{
		return (name);
	}

	const QString& getType() const
	{
		return (type);
	}

	const QString& getDescription() const
	{
		return (description);
	}

private:
	/// pointer to the entry in the browser QListView
	QListViewItem* lvi;

	/// the name of the sensor as provided by ktopd
	QString name;

	/// the localized description of the sensor
	QString description;

	/// qualifies the class of the sensor (table, integer, etc.)
	QString type;
} ;

class HostInfo
{
public:
	HostInfo(int i, const SensorAgent* a, const QString& n,
			 QListViewItem* l) :
		id(i), sensorAgent(a), hostName(n), lvi(l)
	{
		sensors.setAutoDelete(TRUE);
	}
	~HostInfo() { }

	int getId() const
	{
		return (id);
	}

	const SensorAgent* getSensorAgent() const
	{
		return (sensorAgent);
	}

	const QString& getHostName() const
	{
		return (hostName);
	}

	QListViewItem* getLVI() const
	{
		return (lvi);
	}

	const QString& getSensorName(const QListViewItem* lvi) const
	{
		QListIterator<SensorInfo> it(sensors);
		for ( ; it.current() && (*it)->getLVI() != lvi; ++it)
			;
		assert(it.current());

		return ((*it)->getName());
	}

	void appendSensors(QStringList& l)
	{
		QListIterator<SensorInfo> it(sensors);
		for ( ; it.current(); ++it)
			l.append(it.current()->getName());
	}

	const QString& getSensorType(const QListViewItem* lvi) const
	{
		QListIterator<SensorInfo> it(sensors);
		for ( ; it.current() && (*it)->getLVI() != lvi; ++it)
			;
		assert(it.current());

		return ((*it)->getType());
	}

	const QString& getSensorDescription(const QListViewItem* lvi) const
	{
		QListIterator<SensorInfo> it(sensors);
		for ( ; it.current() && (*it)->getLVI() != lvi; ++it)
			;
		assert(it.current());

		return ((*it)->getDescription());
	}

	void addSensor(QListViewItem* lvi, const QString& name,
				   const QString& descr, const QString& type)
	{
		SensorInfo* si = new SensorInfo(lvi, name, descr, type);
		CHECK_PTR(si);
		sensors.append(si);
	}

	bool isRegistered(const QString& name) const
	{
		QListIterator<SensorInfo> it(sensors);
		for ( ; it.current(); ++it)
			if ((*it)->getName() == name)
				return (true);

		return (false);
	}

	bool isRegistered(QListViewItem* lvi) const
	{
		QListIterator<SensorInfo> it(sensors);
		for ( ; it.current(); ++it)
			if ((*it)->getLVI() == lvi)
				return (true);

		return (false);
	}

private:
	// unique ID, used for sendRequests/answerReceived 
	int id;

	const SensorAgent* sensorAgent;

	const QString hostName;
	/// pointer to the entry in the browser QListView
	QListViewItem* lvi;

	QList<SensorInfo> sensors;
} ;
	
/**
 * The SensorBrowser is the graphical front-end of the SensorManager. It
 * displays the currently available hosts and their sensors.
 */
class SensorBrowser : public QListView, public SensorClient
{
	Q_OBJECT

public:
	SensorBrowser(QWidget* parent, SensorManager* sm, const char* name);
	~SensorBrowser();

	QStringList listHosts();
	QStringList listSensors(const QString& hostName);

public slots:
	void disconnect();
	void hostReconfigured(const QString& hostName);
	void update();
	void newItemSelected(QListViewItem* item);

protected:
	virtual void viewportMouseMoveEvent(QMouseEvent* ev);

private:
	void answerReceived(int id, const QString& s);

	SensorManager* sensorManager;

	QList<HostInfo> hostInfos;

	// This string stores the drag object.
	QString dragText;

	KIconLoader* icons;
} ;

#endif
