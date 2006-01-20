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

#ifndef _LogFile_h
#define _LogFile_h

#define MAXLINES 500

class QFile;
class QListWidget;

#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QTimerEvent>
#include <QResizeEvent>

#include <SensorDisplay.h>

#include "LogFileSettings.h"

class LogFile : public KSGRD::SensorDisplay
{
	Q_OBJECT
public:
	LogFile(QWidget *parent = 0, const char *name = 0, const QString& title = 0);
	~LogFile(void);

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& sensorType, const QString& sensorDescr);
	void answerReceived(int id, const QString& answer);
	void resizeEvent(QResizeEvent*);

	bool restoreSettings(QDomElement& element);
	bool saveSettings(QDomDocument& doc, QDomElement& element, bool save = true);

	void updateMonitor(void);

	void configureSettings(void);

	virtual void timerEvent(QTimerEvent*)
	{
		updateMonitor();
	}

	virtual bool hasSettingsDialog() const
	{
		return (true);
	}

public Q_SLOTS:
	void applySettings();
	void applyStyle();

	void settingsFontSelection();
	void settingsAddRule();
	void settingsDeleteRule();
	void settingsChangeRule();
	void settingsRuleListSelected(int index);

private:
	LogFileSettings* lfs;
	QListWidget* monitor;
	QStringList filterRules;

	unsigned long logFileID;
};

#endif // _LogFile_h
