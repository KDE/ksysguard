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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qvaluelist.h>
#include <qwidget.h>

#include <knotifyclient.h>

#include <ksgrd/SensorClient.h>

#define NONE -1

class QDomDocument;
class QDomElement;
class TimerSettings;

namespace KSGRD {

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

	SensorDisplay(QWidget* parent = 0, const char* name = 0, 
				  const QString& title = 0);
	virtual ~SensorDisplay();

	virtual bool addSensor(const QString& hostName, const QString& sensorName,
						   const QString& sensorType,
						   const QString& description);
	virtual bool removeSensor(uint idx);

	/**
	 * This function is a wrapper function to SensorManager::sendRequest.
	 * It should be used by all SensorDisplay functions that need to send
	 * a request to a sensor since it performs an appropriate error
	 * handling by removing the display of necessary.
	 */
	void sendRequest(const QString& hostName, const QString& cmd, int id);

	void setUpdateInterval(uint interval);

	virtual bool hasSettingsDialog() const
	{
		return (false);
	}

	virtual void settings() { }

	virtual void updateWhatsThis();
	virtual QString additionalWhatsThis();

	virtual void sensorLost(int reqId);

	/**
	 * Normaly you shouldn't reimplement this methode
	 */
	virtual void sensorError(int sensorId, bool mode);

	virtual bool createFromDOM(QDomElement&);
	virtual bool addToDOM(QDomDocument&, QDomElement&, bool = true);

	void collectHosts(QValueList<QString>& list);

	void setupTimer(void);

	void setIsOnTop(bool onTop);

	void setTitle(const QString& title);
	QString getTitle();

	void setUnit(const QString& unit);
	QString getUnit();

	void registerPlotterWidget(QWidget *plotter);

	bool showUnit;
	bool globalUpdateInterval;

public slots:
	/**
	 * This function starts the timer that triggers timer events. It
	 * reads the interval from the member object timerInterval. To
	 * change the interval the timer must be stoped first with
	 * timerOff() and than started again with timeOn().
	 */
	void timerOn();

	/**
	 * This functions stops the timer that triggers the periodic events.
	 */
	void timerOff();

	void rmbPressed();

	virtual void applySettings() { }
	virtual void applyStyle() { }

	void timerToggled(bool);

	virtual void setModified(bool mfd);
		
signals:
	void showPopupMenu(KSGRD::SensorDisplay* display);
	void displayModified(bool mfd);

protected:
	virtual void timerEvent(QTimerEvent*);
	virtual void resizeEvent(QResizeEvent*);
	virtual bool eventFilter(QObject*, QEvent*);

	virtual void focusInEvent(QFocusEvent*);
	virtual void focusOutEvent(QFocusEvent*);

	void registerSensor(SensorProperties* sp);
	void unregisterSensor(uint idx);

	QColor restoreColorFromDOM(QDomElement& de, const QString& attr,
						   const QColor& fallback);
	void addColorToDOM(QDomElement& de, const QString& attr,
					   const QColor& col);

	void internAddToDOM(QDomDocument& doc, QDomElement& display);
	void internCreateFromDOM(QDomElement& display);

	void setSensorOk(bool ok);

	QPtrList<SensorProperties> sensors;

	/// The frame around the other widgets.
	QGroupBox* frame;

	bool modified;
	bool noFrame;

	bool pauseOnHide;
	bool pausedWhileHidden;

private:
	int timerId;
	int timerInterval;

	QWidget* errorIndicator;

	QString title;
	QString unit;

	QWidget* plotterWdg;

	TimerSettings* ts;
};

}; // namespace KSGRD

#endif
