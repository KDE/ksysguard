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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _MultiMeter_h_
#define _MultiMeter_h_

#include <SensorDisplay.h>
//Added by qt3to4:
#include <QLabel>
#include <QResizeEvent>

class QLCDNumber;
class QLabel;
class MultiMeterSettings;

class MultiMeter : public KSGRD::SensorDisplay
{
	Q_OBJECT

public:
	MultiMeter(QWidget* parent, const QString& title, bool isApplet);
	virtual ~MultiMeter()
	{
	}

	bool addSensor(const QString& hostName, const QString& sensorName,
				const QString& sensorType, const QString& sensorDescr);
	void answerReceived(int id, const QString& answer);
	void resizeEvent(QResizeEvent*);

	bool restoreSettings(QDomElement& element);
	bool saveSettings(QDomDocument& doc, QDomElement& element, bool save = true);

	virtual bool hasSettingsDialog() const
	{
		return (true);
	}

	void configureSettings();

public Q_SLOTS:
	void applySettings();
	void applyStyle();

private:
	void setDigitColor(const QColor& col);
	void setBackgroundColor(const QColor& col);

	QLCDNumber* lcd;
	QColor normalDigitColor;
	QColor alarmDigitColor;

	MultiMeterSettings* mms;
	bool lowerLimitActive;
	double lowerLimit;
	bool upperLimitActive;
	double upperLimit;
};

#endif
