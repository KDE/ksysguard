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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qevent.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>

#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>

#include "ksysguard.h"
#include "SensorManager.h"
#include "HostConnector.h"
#include "SensorShellAgent.h"
#include "SensorSocketAgent.h"
#include "SensorManager.moc"

SensorManager* SensorMgr;

SensorManager::SensorManager()
{
	sensors.setAutoDelete(true);

	dict.setAutoDelete(true);
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
	dict.insert("interfaces", new QString(i18n("Interfaces")));
	dict.insert("receiver", new QString(i18n("Receiver")));
	dict.insert("transmitter", new QString(i18n("Transmitter")));
	dict.insert("data", new QString(i18n("Data")));
	dict.insert("compressed", new QString(i18n("Compressed Packets")));
	dict.insert("drops", new QString(i18n("Dropped Packets")));
	dict.insert("errors", new QString(i18n("Errors")));
	dict.insert("fifo", new QString(i18n("FIFO Overruns")));
	dict.insert("frame", new QString(i18n("Frame Errors")));
	dict.insert("multicast", new QString(i18n("Multicast")));
	dict.insert("packets", new QString(i18n("Packets")));
	dict.insert("carrier", new QString(i18n("Carrier")));
	dict.insert("collisions", new QString(i18n("Collisions")));
	dict.insert("sockets", new QString(i18n("Sockets")));
	dict.insert("count", new QString(i18n("Total Number")));
	dict.insert("list", new QString(i18n("Table")));
	dict.insert("apm", new QString(i18n("Advanced Power Management")));
	dict.insert("batterycharge", new QString(i18n("Battery Charge")));
	dict.insert("remainingtime", new QString(i18n("Remaining Time")));
	dict.insert("interrupts", new QString(i18n("Interrupts")));
	dict.insert("loadavg1", new QString(i18n("Load Average (1 min)")));
	dict.insert("loadavg5", new QString(i18n("Load Average (5 min)")));
	dict.insert("loadavg15", new QString(i18n("Load Average (15 min)")));
	dict.insert("clock", new QString(i18n("Clock Frequency")));
	dict.insert("lmsensors", new QString(i18n("Hardware Sensors")));
	dict.insert("partitions", new QString(i18n("Partition Usage")));
	dict.insert("usedspace", new QString(i18n("Used Space")));
	dict.insert("freespace", new QString(i18n("Free Space")));
	dict.insert("filllevel", new QString(i18n("Fill Level")));

	for (int i = 0; i < 32; i++)
	{
		dict.insert("cpu" + QString::number(i),
					new QString(QString(i18n("CPU%1")).arg(i)));
		dict.insert("disk" + QString::number(i),
					new QString(QString(i18n("Disk%1")).arg(i)));
	}

	for (int i = 0; i < 6; i++)
	{
		dict.insert("fan" + QString::number(i),
					new QString(QString(i18n("Fan%1")).arg(i)));
		dict.insert("temp" + QString::number(i),
					new QString(QString(i18n("Temperature%1")).arg(i)));
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

	descriptions.setAutoDelete(true);
	// TODO: translated descriptions not yet implemented.

	units.setAutoDelete(true);
	units.insert("1/s", new QString(i18n("the unit 1 per second", "1/s")));
	units.insert("kBytes", new QString(i18n("kBytes")));
	units.insert("min", new QString(i18n("the unit minutes", "min")));
	units.insert("MHz", new QString(i18n("the frequency unit", "MHz")));

	types.setAutoDelete(true);
	types.insert("integer", new QString(i18n("Integer Value")));
	types.insert("float", new QString(i18n("Floating Point Value")));
	types.insert("table", new QString(i18n("Process Controller")));
	types.insert("listview", new QString(i18n("Table")));

	broadcaster = 0;

	hostConnector = new HostConnector(0, "HostConnector", true);
	CHECK_PTR(hostConnector);
	connect(hostConnector->helpButton, SIGNAL(clicked()),
			this, SLOT(helpConnectHost()));
}

SensorManager::~SensorManager()
{
	delete hostConnector;
}

bool
SensorManager::engageHost(const QString& hostname)
{
	bool retVal = true;

	if (hostname == "" || sensors.find(hostname) == 0)
	{
		if (hostname == "")
		{
			/* Show combo box, hide fixed label. */
			hostConnector->hostLabel->setText(i18n("Host"));
			hostConnector->host->show();
			hostConnector->host->setEnabled(true);
			hostConnector->host->setFocus();
		}
		else
		{
			/* Show fixed label (hostname) and hide combo box. */
			hostConnector->hostLabel->setText(hostname);
			hostConnector->host->hide();
			hostConnector->host->setEnabled(false);
		}
		if (hostConnector->exec())
		{
			QString shell = "";
			QString command = "";
			int port = -1;

			/* Check which radio button is selected and set parameters
			 * appropriately. */
			if (hostConnector->ssh->isChecked())
				shell = "ssh";
			else if (hostConnector->rsh->isChecked())
				shell = "rsh";
			else if (hostConnector->daemon->isChecked())
				port = hostConnector->port->text().toInt();
			else
				command = hostConnector->command->currentText();

			if (hostname == "")
				retVal = engage(hostConnector->host->currentText(), shell,
								command, port);
			else
				retVal = engage(hostname, shell, command, port);
		}
	}
	return (retVal);
}

bool
SensorManager::engage(const QString& hostname, const QString& shell,
					  const QString& command, int port)
{
	SensorAgent* daemon;

	if ((daemon = sensors.find(hostname)) == 0)
	{
		if (port == -1)
			daemon = new SensorShellAgent(this);
		else
			daemon = new SensorSocketAgent(this);
		CHECK_PTR(daemon);

		if (!daemon->start(hostname.ascii(), shell, command, port))
		{
			delete daemon;
			return (false);
		}
		sensors.insert(hostname, daemon);
		connect(daemon, SIGNAL(reconfigure(const SensorAgent*)),
				this, SLOT(reconfigure(const SensorAgent*)));
		emit update();
		return (true);
	}

	return (false);
}

void
SensorManager::requestDisengage(const SensorAgent* sa)
{
	/* When a sensor agent becomes disfunctional it calles this function
	 * to request that it is being removed from the SensorManager. It must
	 * not call disengage() directly since it would trigger ~SensorAgent()
	 * while we are still in a SensorAgent member function.
	 * So we have to post an event which is later caught by
	 * SensorManger::customEvent(). */
	QCustomEvent* ev = new QCustomEvent(QEvent::User, (void*) sa);
	kapp->postEvent(this, ev);
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
			return (true);
		}

	return (false);
}

bool
SensorManager::disengage(const QString& hostname)
{
	SensorAgent* daemon;
	if ((daemon = sensors.find(hostname)) != 0)
	{
		sensors.remove(hostname);
		emit update();
		return (true);
	}

	return (false);
}

bool
SensorManager::resynchronize(const QString& hostname)
{
	SensorAgent* daemon;

	if ((daemon = sensors.find(hostname)) == 0)
		return (false);

	QString shell, command;
	int port;
	getHostInfo(hostname, shell, command, port);

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
SensorManager::notify(const QString& msg) const
{
	/* This function relays text messages to the toplevel widget that
	 * displays the message in a pop-up box. It must be used for objects
	 * that might have been deleted before the pop-up box is closed. */
	if (broadcaster)
	{
		QCustomEvent* ev = new QCustomEvent(QEvent::User);
		ev->setData(new QString(msg));
		kapp->postEvent(broadcaster, ev);
	}
}

void
SensorManager::reconfigure(const SensorAgent*)
{
	emit(update());
}

bool
SensorManager::event(QEvent* ev)
{
	if (ev->type() == QEvent::User)
	{
		disengage((const SensorAgent*) ((QCustomEvent*) ev)->data());
		return (true);
	}

	return (false);
}

bool
SensorManager::sendRequest(const QString& hostname, const QString& req,
						   SensorClient* client, int id)
{
	SensorAgent* daemon;
	if ((daemon = sensors.find(hostname)) != 0)
	{
		daemon->sendRequest(req, client, id);
		return (true);
	}

	return (false);
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
						   QString& command, int& port)
{
	SensorAgent* daemon;
	if ((daemon = sensors.find(hostName)) != 0)
	{
		daemon->getHostInfo(shell, command, port);
		return (true);
	}

	return (false);
}

QString
SensorManager::translateSensor(const QString& u) const
{
	QString token, out;
	int start = 0, end = 0;
	for ( ; ; )
	{
		end = u.find('/', start);
		if (end > 0)
			out += trSensorPath(u.mid(start, end - start)) + "/";
		else
		{
			out += trSensorPath(u.right(u.length() - start));
			break;
		}
		start = end + 1;
	}
	return (out);
}

void
SensorManager::readProperties(KConfig* cfg)
{
	QStringList sl = cfg->readListEntry("HostList");
	hostConnector->host->insertStringList(sl);
	sl.clear();
	sl = cfg->readListEntry("CommandList");
	hostConnector->command->insertStringList(sl);
}

void
SensorManager::saveProperties(KConfig* cfg)
{
	QComboBox* cb = hostConnector->host;
	QStringList sl;
	for (int i = 0; i < cb->count(); ++i)
		sl.append(cb->text(i));
	cfg->writeEntry("HostList", sl);

	cb = hostConnector->command;
	sl.clear();
	for (int i = 0; i < cb->count(); ++i)
		sl.append(cb->text(i));
	cfg->writeEntry("CommandList", sl);
	
}

void
SensorManager::helpConnectHost()
{
	kapp->invokeHelp("CONNECTINGTOOTHERHOSTS",
					 "ksysguard/the-sensor-browser.html");
}
