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

#ifndef _SensorDisplay_h_
#define _SensorDisplay_h_

#include <qwidget.h>
#include <qvaluelist.h>

#include "SensorClient.h"
#include "SensorDisplay.h"

#define NONE -1

class QDomDocument;
class QDomElement;

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

	virtual bool addSensor(const QString&, const QString&, const QString&)
	{
		return (FALSE);
	}

	/**
	 * This function is a wrapper function to SensorManager::sendRequest.
	 * It should be used by all SensorDisplay functions that need to send
	 * a request to a sensor since it performs an appropriate error
	 * handling by removing the display of necessary.
	 */
	void sendRequest(const QString& hostName, const QString& cmd, int id);

	virtual bool load(QDomElement&)
	{
		return (TRUE);
	}

	virtual bool save(QDomDocument&, QDomElement&)
	{
		return (TRUE);
	}

	virtual bool hasBeenModified()
	{
		return (TRUE);
	}

	virtual void settings() { }

	void collectHosts(QValueList<QString>& list);

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

	void rmbPressed()
	{
		emit(showPopupMenu(this));
	}

signals:
	void removeDisplay(SensorDisplay* display);
	void showPopupMenu(SensorDisplay* display);

protected:
	virtual void timerEvent(QTimerEvent*);
	virtual bool eventFilter(QObject*, QEvent*);

	void registerSensor(const QString& hostName, const QString& sensorName);

	QList<const QString> hostNames;
	QList<const QString> sensorNames;

private:
	int timerId;

	int timerInterval;
} ;

#endif
