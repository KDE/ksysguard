/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _SensorLogger_h
#define _SensorLogger_h

#include <QLabel>
#include <q3listview.h>
#include <q3popupmenu.h>
#include <QString>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PtrList>
#include <QTimerEvent>
#include <QResizeEvent>

#include <SensorDisplay.h>

#include "SensorLoggerDlg.h"

#define NONE -1

class SensorLoggerSettings;
class QDomElement;

class SLListViewItem : public Q3ListViewItem
{
public:
	SLListViewItem(Q3ListView *parent = 0);

	void setTextColor(const QColor& color) { textColor = color; }

	void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment) {
		QColorGroup cgroup(cg);
		cgroup.setColor(QPalette::Text, textColor);
		Q3ListViewItem::paintCell(p, cgroup, column, width, alignment);
	
	}

	void paintFocus(QPainter *, const QColorGroup, const QRect) {}

private:
	QColor textColor;
};	

class LogSensor : public QObject, public KSGRD::SensorClient
{
	Q_OBJECT
public:
	LogSensor(Q3ListView *parent);
	~LogSensor(void);

	void answerReceived(int id, const QStringList& answer);

	void setHostName(const QString& name) { hostName = name; lvi->setText(3, name); }
	void setSensorName(const QString& name) { sensorName = name; lvi->setText(2, name); }
	void setFileName(const QString& name)
	{
		fileName = name;
		lvi->setText(4, name);
	}
	void setUpperLimitActive(bool value) { upperLimitActive = value; }
	void setLowerLimitActive(bool value) { lowerLimitActive = value; }
	void setUpperLimit(double value) { upperLimit = value; }
	void setLowerLimit(double value) { lowerLimit = value; }

	void setTimerInterval(int interval) {
		timerInterval = interval;

		if (timerID != NONE)
		{
			timerOff();
			timerOn();
		}

		lvi->setText(1, QString("%1").arg(interval));
	}

	QString getSensorName(void) { return sensorName; }
	QString getHostName(void) { return hostName; }
	QString getFileName(void) { return fileName; }
	int getTimerInterval(void) { return timerInterval; }
	bool getUpperLimitActive(void) { return upperLimitActive; }
	bool getLowerLimitActive(void) { return lowerLimitActive; }
	double getUpperLimit(void) { return upperLimit; }
	double getLowerLimit(void) { return lowerLimit; }
	Q3ListViewItem* getListViewItem(void) { return lvi; }

public Q_SLOTS:
	void timerOff()
	{
		killTimer(timerID);
		timerID = NONE;
	}

	void timerOn()
	{
		timerID = startTimer(timerInterval * 1000);
	}

	bool isLogging() { return timerID != NONE; }

	void startLogging(void);
	void stopLogging(void);

protected:
	virtual void timerEvent(QTimerEvent*);

private:
	Q3ListView* monitor;
	SLListViewItem* lvi;
	QPixmap pixmap_running;
	QPixmap pixmap_waiting;
	QString sensorName;
	QString hostName;
	QString fileName;

	int timerInterval;
	int timerID;

	bool lowerLimitActive;
	bool upperLimitActive;

	double lowerLimit;
	double upperLimit;
};

class SensorLogger : public KSGRD::SensorDisplay
{
	Q_OBJECT
public:
	SensorLogger(QWidget *parent, const QString& title, bool isApplet);
	~SensorLogger(void);

	bool addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType,
				   const QString& sensorDescr);

	bool editSensor(LogSensor*);

	void answerReceived(int id, const QStringList& answer);
	void resizeEvent(QResizeEvent*);

	bool restoreSettings(QDomElement& element);
	bool saveSettings(QDomDocument& doc, QDomElement& element, bool save = true);

	void configureSettings(void);

	virtual bool hasSettingsDialog() const
	{
		return (true);
	}

public Q_SLOTS:
	void applySettings();
	void applyStyle();
	void RMBClicked(Q3ListViewItem*, const QPoint&, int);

protected:
	LogSensor* getLogSensor(Q3ListViewItem*);

private:
	Q3ListView* monitor;

	Q3PtrList<LogSensor> logSensors;

	SensorLoggerDlg *sld;
	SensorLoggerSettings *sls;
};

#endif // _SensorLogger_h
