/*
    KTop, the KDE Task Manager and System Monitor
   
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include "SensorDisplay.h"
#include "SensorDisplay.moc"

SensorDisplay::SensorDisplay(QWidget* parent, const char* name) :
	QWidget(parent, name)
{
	timerId = startTimer(2000);
	sensorNames.setAutoDelete(true);
}

SensorDisplay::~SensorDisplay()
{
	killTimer(timerId);
}

void
SensorDisplay::registerSensor(SensorAgent* sensorAgent,
							  const QString& sensorName)
{
	sensorAgents.append(sensorAgent);
	sensorNames.append(new QString(sensorName));
}

void
SensorDisplay::timerEvent(QTimerEvent*)
{
	QListIterator<SensorAgent> saIt(sensorAgents);	
	QListIterator<const QString> snIt(sensorNames);

	for (int i = 0; saIt.current(); ++saIt, ++snIt, ++i)
		saIt.current()->sendRequest(*snIt.current(), (SensorClient*) this, i);
}

