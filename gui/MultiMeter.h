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

#ifndef _MultiMeter_h_
#define _MultiMeter_h_

#include <SensorDisplay.h>

class QGroupBox;
class QLCDNumber;
class QLabel;
class MultiMeterSettings;

class MultiMeter : public SensorDisplay
{
	Q_OBJECT

public:
	MultiMeter(QWidget* parent = 0, const char* name = 0,
			   const QString& = QString::null, double min = 0, double max = 0, bool nf = 0);
	virtual ~MultiMeter()
	{
	}

	bool addSensor(const QString& hostName, const QString& sensorName,
				const QString& sensorType, const QString& sensorDescr);
	void answerReceived(int id, const QString& answer);
	void resizeEvent(QResizeEvent*);

	bool createFromDOM(QDomElement& domEl);
	bool addToDOM(QDomDocument& doc, QDomElement& display, bool save = true);

	virtual bool hasSettingsDialog() const
	{
		return (TRUE);
	}

	void settings();

	virtual void sensorError(int, bool err);

public slots:
	void applySettings();
	void applyStyle();

private:
	void setTitle(const QString& t, const QString& u);
	void setDigitColor(const QColor& col);
	void setBackgroundColor(const QColor& col);

	QLCDNumber* lcd;
	QLabel* errorLabel;
	QColor normalDigitColor;
	QColor alarmDigitColor;

	QString title;
	QString unit;
	MultiMeterSettings* mms;
	bool showUnit;
	bool lowerLimitActive;
	double lowerLimit;
	bool upperLimitActive;
	double upperLimit;
};

#endif
