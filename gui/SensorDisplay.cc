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
#include "SensorManager.h"

SensorDisplay::SensorDisplay(QWidget* parent, const char* name) :
	QWidget(parent, name)
{
	// default interval is 2 seconds.
	timerInterval = 2000;

	timerId = startTimer(timerInterval);
	sensorNames.setAutoDelete(true);
}

SensorDisplay::~SensorDisplay()
{
	killTimer(timerId);
}

void
SensorDisplay::registerSensor(const QString& hostName,
							  const QString& sensorName)
{
	hostNames.append(new QString(hostName));
	sensorNames.append(new QString(sensorName));
}

void
SensorDisplay::timerEvent(QTimerEvent*)
{
	QListIterator<const QString> hnIt(hostNames);
	QListIterator<const QString> snIt(sensorNames);

	for (int i = 0; hnIt.current(); ++hnIt, ++snIt, ++i)
		SensorMgr->sendRequest(*hnIt.current(), *snIt.current(),
							   (SensorClient*) this, i);
}

