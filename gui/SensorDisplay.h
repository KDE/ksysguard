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

#ifndef _SensorDisplay_h_
#define _SensorDisplay_h_

#include <qwidget.h>

#include "SensorClient.h"
#include "SensorDisplay.h"
#include "SensorAgent.h"

#define NONE -1

/**
 * This class is the base class for all displays for sensors. A
 * display is any kind of widget that can display the value of one or
 * more sensors in any form. It must be inherited by all displays that
 * should be inserted into the work sheet.
 */
class SensorDisplay : public QWidget, public SensorClient
{
	Q_OBJECT
public:

	SensorDisplay(QWidget* parent = 0, const char* name = 0);
	~SensorDisplay();

	void registerSensor(SensorAgent* sensorAgent, const QString& sensorName);
	virtual bool addSensor(SensorAgent*, const QString&, const QString&)
	{
		return (false);
	}

public slots:
	/**
	 * This functions stops the timer that triggers the periodic events.
	 */
	void timerOff()
	{
		if (timerId != NONE)
		{
			killTimer(timerId);
			timerId = NONE;
		} 
	}

	/**
	 * This function starts the timer that triggers timer events. It
	 * reads the interval from the member object timerInterval. To
	 * change the interval the timer must be stoped first with
	 * timerOff() and than started again with timeOn().
	 */
	void timerOn()
	{
		if (timerId == NONE)
		{
			timerId = startTimer(timerInterval);
		}
	}


protected:
	virtual void timerEvent(QTimerEvent*);

private:
	int timerId;

	int timerInterval;

	QList<const QString> sensorNames;
	QList<SensorAgent> sensorAgents;
} ;

#endif
