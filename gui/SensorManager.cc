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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qevent.h>

#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>

#include "ksysguard.h"
#include "SensorManager.h"
#include "SensorAgent.h"
#include "SensorManager.moc"

SensorManager* SensorMgr;

SensorManager::SensorManager()
{
	sensors.setAutoDelete(true);

	dict.setAutoDelete(TRUE);
	// Fill the sensor description dictionary.
	dict.insert("cpu", new QString(i18n("CPU Load", "Load")));
	dict.insert("idle", new QString(i18n("Idle Load")));
	dict.insert("sys", new QString(i18n("System Load")));
	dict.insert("nice", new QString(i18n("Nice Load")));
	dict.insert("user", new QString(i18n("User Load")));
	dict.insert("mem", new QString(i18n("Memory")));
	dict.insert("physical", new QString(i18n("Physical Memory")));
	dict.insert("swap", new QString(i18n("Swap Memory")));
	dict.insert("cached", new QString(i18n("Cached Memory")));
	dict.insert("buf", new QString(i18n("Buffered Memory")));
	dict.insert("used", new QString(i18n("Used Memory")));
	dict.insert("application", new QString(i18n("Application Memory")));
	dict.insert("free", new QString(i18n("Free Memory")));
	dict.insert("pscount", new QString(i18n("Process Count")));
	dict.insert("ps", new QString(i18n("Process Controller")));
	dict.insert("disk", new QString(i18n("Disk Throughput")));
	dict.insert("load", new QString(i18n("CPU Load", "Load")));
	dict.insert("total", new QString(i18n("Total Accesses")));
	dict.insert("rio", new QString(i18n("Read Accesses")));
	dict.insert("wio", new QString(i18n("Write Accesses")));
	dict.insert("rblk", new QString(i18n("Read Data")));
	dict.insert("wblk", new QString(i18n("Write Data")));
	dict.insert("pageIn", new QString(i18n("Pages In")));
	dict.insert("pageOut", new QString(i18n("Pages Out")));
	dict.insert("context", new QString(i18n("Context Switches")));
	dict.insert("network", new QString(i18n("Network")));
	dict.insert("recBytes", new QString(i18n("Received Bytes")));
	dict.insert("sentBytes", new QString(i18n("Sent Bytes")));
	dict.insert("apm", new QString(i18n("Advanced Power Management")));
	dict.insert("batterycharge", new QString(i18n("Battery Charge")));
	dict.insert("remainingtime", new QString(i18n("Remaining Time")));
	dict.insert("interrupts", new QString(i18n("Interrupts")));
	dict.insert("loadavg1", new QString(i18n("Load Average (1 min)")));
	dict.insert("loadavg5", new QString(i18n("Load Average (5 min)")));
	dict.insert("loadavg15", new QString(i18n("Load Average (15 min)")));
	dict.insert("clock", new QString(i18n("Clock Frequency")));

	for (int i = 0; i < 32; i++)
	{
		dict.insert("cpu" + QString::number(i),
					new QString(QString(i18n("CPU%1")).arg(i)));
		dict.insert("disk" + QString::number(i),
					new QString(QString(i18n("Disk%1")).arg(i)));
	}

	dict.insert("int00", new QString(i18n("Total")));
	for (int i = 1; i < 25; i++)
	{
		QString num = QString::number(i);
		if (i < 10)
			num = "0" + num;
		dict.insert("int" + num,
					new QString(QString(i18n("Int%1")).arg(i - 1, 3)));
	}

	descriptions.setAutoDelete(TRUE);
	// TODO: translated descriptions not yet implemented.

	units.setAutoDelete(TRUE);
	units.insert("1/s", new QString(i18n("the unit 1 per second", "1/s")));
	units.insert("kBytes", new QString(i18n("kBytes")));
	units.insert("min", new QString(i18n("the unit minutes", "min")));
	units.insert("MHz", new QString(i18n("the frequency unit", "MHz")));

	broadcaster = 0;
}

SensorManager::~SensorManager()
{
}

bool
SensorManager::engage(const QString& hostname, const QString& shell,
					  const QString& command)
{
	SensorAgent* daemon;

	if ((daemon = sensors.find(hostname)) == 0)
	{
		daemon = new SensorAgent(this);
		CHECK_PTR(daemon);
		if (!daemon->start(hostname.ascii(), shell, command))
		{
			delete daemon;
			return (FALSE);
		}
		sensors.insert(hostname, daemon);
		connect(daemon, SIGNAL(reconfigure(const SensorAgent*)),
				this, SLOT(reconfigure(const SensorAgent*)));
		emit update();
		return (TRUE);
	}

	return (TRUE);
}

bool
SensorManager::disengage(const SensorAgent* sa)
{
	QDictIterator<SensorAgent> it(sensors);

	for ( ; it.current(); ++it)
		if (it.current() == sa)
		{
			sensors.remove(it.currentKey());
			emit update();
			return (TRUE);
		}

	return (FALSE);
}

bool
SensorManager::disengage(const QString& hostname)
{
	SensorAgent* daemon;
	if ((daemon = sensors.find(hostname)) != 0)
	{
		sensors.remove(hostname);
		emit update();
		return (TRUE);
	}

	return (FALSE);
}

bool
SensorManager::resynchronize(const QString& hostname)
{
	SensorAgent* daemon;

	if ((daemon = sensors.find(hostname)) == 0)
		return (FALSE);

	QString shell, command;
	getHostInfo(hostname, shell, command);
	disengage(hostname);

	kdDebug () << "Re-synchronizing connection to " << hostname << endl;

	return (engage(hostname, shell, command));
}

void
SensorManager::hostLost(const SensorAgent* sensor)
{
	emit hostConnectionLost(sensor->getHostName());

	if (broadcaster)
	{
		QCustomEvent* ev = new QCustomEvent(QEvent::User);
		ev->setData(new QString(
			i18n("Connection to %1 has been lost!")
			.arg(sensor->getHostName())));
		kapp->postEvent(broadcaster, ev);
	}
}

void
SensorManager::reconfigure(const SensorAgent*)
{
	emit(update());
}

bool
SensorManager::sendRequest(const QString& hostname, const QString& req,
						   SensorClient* client, int id)
{
	SensorAgent* daemon;
	if ((daemon = sensors.find(hostname)) != 0)
	{
		daemon->sendRequest(req, client, id);
		return (TRUE);
	}

	return (FALSE);
}

const QString
SensorManager::getHostName(const SensorAgent* sensor) const
{
	QDictIterator<SensorAgent> it(sensors);
	
	while (it.current())
	{
		if (it.current() == sensor)
			return (it.currentKey());
		++it;
	}

	return (QString::null);
}

bool
SensorManager::getHostInfo(const QString& hostName, QString& shell,
						   QString& command)
{
	SensorAgent* daemon;
	if ((daemon = sensors.find(hostName)) != 0)
	{
		daemon->getHostInfo(shell, command);
		return (TRUE);
	}

	return (FALSE);
}
