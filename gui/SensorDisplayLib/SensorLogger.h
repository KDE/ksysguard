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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id$
*/

#ifndef _SensorLogger_h
#define _SensorLogger_h

#include <qdom.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpopupmenu.h>
#include <qspinbox.h>
#include <qstring.h>

#include <SensorDisplay.h>

#include "SensorLoggerDlg.h"

#define NONE -1

class SensorLoggerSettings;

class SLListViewItem : public QListViewItem
{
public:
	SLListViewItem(QListView *parent = 0);

	void setTextColor(const QColor& color) { textColor = color; }

	void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment) {
		QColorGroup cgroup(cg);
		cgroup.setColor(QColorGroup::Text, textColor);
		QListViewItem::paintCell(p, cgroup, column, width, alignment);
	
	}

	void paintFocus(QPainter *, const QColorGroup, const QRect) {}

private:
	QColor textColor;
};	

class LogSensor : public QObject, public KSGRD::SensorClient
{
	Q_OBJECT
public:
	LogSensor(QListView *parent);
	~LogSensor(void);

	void answerReceived(int id, const QString& answer);

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
		if (timerID != NONE)
			timerOff();

		timerInterval = interval;

		if (timerID != NONE)
			timerOn();

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
	QListViewItem* getListViewItem(void) { return lvi; }

public slots:
	void timerOff()
	{
		killTimer(timerID);
		timerID = NONE;
	}

	void timerOn()
	{
		timerID = startTimer(timerInterval * 1000);
	}

	void startLogging(void);
	void stopLogging(void);

protected:
	virtual void timerEvent(QTimerEvent*);

private:
	QFile* logFile;
	QListView* monitor;
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
	SensorLogger(QWidget *parent = 0, const char *name = 0, const QString& title = 0);
	~SensorLogger(void);

	bool addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType,
				   const QString& sensorDescr);

	bool editSensor(LogSensor*);

	void answerReceived(int id, const QString& answer);
	void resizeEvent(QResizeEvent*);

	bool createFromDOM(QDomElement& element);
	bool addToDOM(QDomDocument& doc, QDomElement& element, bool save = true);

	void settings(void);

	virtual bool hasSettingsDialog() const
	{
		return (true);
	}

public slots:
	void applySettings();
	void applyStyle();
	void fileSelect();
	void RMBClicked(QListViewItem*, const QPoint&, int);

protected:
	LogSensor* getLogSensor(QListViewItem*);

private:
	QListView* monitor;

	QPtrList<LogSensor> logSensors;

	SensorLoggerDlg *sld;
	SensorLoggerSettings *sls;
};

#endif // _SensorLogger_h
