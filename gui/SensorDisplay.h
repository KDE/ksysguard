/*
    KSysGuard, the KDE System Guard
   
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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _SensorDisplay_h_
#define _SensorDisplay_h_

#include <qwidget.h>
#include <qvaluelist.h>
#include <qgroupbox.h>
#include <knotifyclient.h>

#include "SensorClient.h"
#include "SensorDisplay.h"

#include "TimerSettings.h"

#define NONE -1

class QDomDocument;
class QDomElement;

class SensorProperties
{
public:
	SensorProperties() { }
	SensorProperties(const QString& hn, const QString& n, const QString& t, const QString& d)
		: hostName(hn), name(n), type(t), description(d)
	{
		ok = false;
	}
	virtual ~SensorProperties() { }

	QString hostName;
	QString name;
	QString type;
	QString description;
	QString unit;

	/* This flag indicates whether the communication to the sensor is
	 * ok or not. */
	bool ok;
} ;

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
	virtual ~SensorDisplay();

	virtual bool addSensor(const QString& hn, const QString& sn, const QString& st,
						   const QString& res1)
	{
		registerSensor(new SensorProperties(hn, sn, st, res1));
		return (true);
	}

	virtual bool removeSensor(uint idx)
	{
		unregisterSensor(idx);
		return (true);
	}

	/**
	 * This function is a wrapper function to SensorManager::sendRequest.
	 * It should be used by all SensorDisplay functions that need to send
	 * a request to a sensor since it performs an appropriate error
	 * handling by removing the display of necessary.
	 */
	void sendRequest(const QString& hostName, const QString& cmd, int id);

	void setUpdateInterval(uint i)
	{
		bool timerActive = timerId != NONE;

		if (timerActive)
			timerOff();
		timerInterval = i * 1000;
		if (timerActive)
			timerOn();
	}

	virtual bool hasSettingsDialog() const
	{
		return (false);
	}

	virtual void settings() { }

	virtual void updateWhatsThis();

	virtual QString additionalWhatsThis()
	{
		return QString::null;
	}

	virtual void sensorLost(int reqId)
	{
		sensorError(reqId, true);
	}

	virtual void sensorError(int sensorId, bool mode);

	virtual bool createFromDOM(QDomElement&)
	{
		// should never been used.
		return (false);
	}
	virtual bool addToDOM(QDomDocument&, QDomElement&, bool = true)
	{
		// should never been used.
		return (false);
	}

	void collectHosts(QValueList<QString>& list);

	void setupTimer(void);

	bool globalUpdateInterval;

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

	virtual void applySettings() { }
	virtual void applyStyle() { }

	void timerToggled(bool);

	virtual void setModified(bool mfd)
	{
		if (mfd != modified)
		{
			modified = mfd;
			emit displayModified(modified);
		}
	}
		
signals:
	void showPopupMenu(SensorDisplay* display);
	void displayModified(bool mfd);

protected:
	virtual void timerEvent(QTimerEvent*);
	virtual bool eventFilter(QObject*, QEvent*);

	void registerSensor(SensorProperties* sp);

	void unregisterSensor(uint idx);

	virtual void focusInEvent(QFocusEvent*)
	{
		frame->setLineWidth(2);
	}

	virtual void focusOutEvent(QFocusEvent*)
	{
		frame->setLineWidth(1);
	}

	QColor restoreColorFromDOM(QDomElement& de, const QString& attr,
							   const QColor& fallback);
	void addColorToDOM(QDomElement& de, const QString& attr,
					   const QColor& col);

	void internAddToDOM(QDomDocument& doc, QDomElement& display);
	void internCreateFromDOM(QDomElement& display);

	QPtrList<SensorProperties> sensors;

	/// The frame around the other widgets.
	QGroupBox* frame;

	bool modified;

private:
	int timerId;
	int timerInterval;

	TimerSettings *ts;
};

#endif
