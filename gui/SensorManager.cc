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

#include "ktop.h"
#include "SensorManager.h"
#include "SensorAgent.h"
#include "SensorManager.moc"

SensorManager* SensorMgr;

SensorManager::SensorManager()
{
	sensors.setAutoDelete(true);
}

SensorManager::~SensorManager()
{
}

bool
SensorManager::engage(const QString& hostname, const QString& shell,
					  const QString& command)
{
	SensorAgent* ktopd;

	if ((ktopd = sensors.find(hostname)) == 0)
	{
		ktopd = new SensorAgent(this);
		CHECK_PTR(ktopd);
		if (!ktopd->start(hostname.ascii(), shell, command))
		{
			delete ktopd;
			return (FALSE);
		}
		sensors.insert(hostname, ktopd);
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
	SensorAgent* ktopd;
	if ((ktopd = sensors.find(hostname)) != 0)
	{
		sensors.remove(hostname);
		emit update();
		return (TRUE);
	}

	return (FALSE);
}

void
SensorManager::hostLost(const SensorAgent* sensor)
{
	debug("SensorManager::hostLost");
	emit hostConnectionLost(sensor->getHostName());

	QCustomEvent* ev = new QCustomEvent(QEvent::User);
	ev->setData(new QString(
		i18n("Connection to %1 has been lost!")
		.arg(sensor->getHostName())));
	kapp->postEvent(Toplevel, ev);
}

void
SensorManager::reconfigure(const SensorAgent*)
{
	// TODO: not yet implemented.
}

bool
SensorManager::sendRequest(const QString& hostname, const QString& req,
						   SensorClient* client, int id)
{
	SensorAgent* ktopd;
	if ((ktopd = sensors.find(hostname)) != 0)
	{
		ktopd->sendRequest(req, client, id);
		return (TRUE);
	}

	return (FALSE);
}

const QString
SensorManager::getHostName(const SensorAgent* sensor) const
{
	static QString dummy;

	QDictIterator<SensorAgent> it(sensors);
	
	while (it.current())
	{
		if (it.current() == sensor)
			return (it.currentKey());
		++it;
	}

	return (dummy);
}

bool
SensorManager::getHostInfo(const QString& hostName, QString& shell,
						   QString& command)
{
	SensorAgent* ktopd;
	if ((ktopd = sensors.find(hostName)) != 0)
	{
		ktopd->getHostInfo(shell, command);
		return (TRUE);
	}

	return (FALSE);
}
