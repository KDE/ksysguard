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

#include "SensorManager.h"
#include "SensorAgent.h"
#include "SensorManager.moc"

SensorManager* SensorMgr;

SensorManager::SensorManager()
{
}

SensorManager::~SensorManager()
{
	QListIterator<SensorAgent> it(daList);
	for (; it.current(); ++it)
		delete(it.current());
}

SensorAgent*
SensorManager::engage(const QString& hostname)
{
	SensorAgent* ktopd;
	ktopd = new SensorAgent;
	ktopd->start(hostname.ascii(), "rsh");
	daList.append(ktopd);

	return (ktopd);
}

void
SensorManager::disengage(const SensorAgent* da)
{
	// remove daemon agent for daList
}
