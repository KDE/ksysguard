/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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

class QListWidget;

#include <QtXml/qdom.h>


#include <SensorDisplay.h>

class Ui_LogFileSettings;

class LogFile : public KSGRD::SensorDisplay
{
	Q_OBJECT
public:
	LogFile(QWidget *parent, const QString& title, SharedSettings *workSheetSettings);
	~LogFile(void);

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& sensorType, const QString& sensorDescr) Q_DECL_OVERRIDE;
	void answerReceived(int id, const QList<QByteArray>& answer) Q_DECL_OVERRIDE;

	bool restoreSettings(QDomElement& element) Q_DECL_OVERRIDE;
	bool saveSettings(QDomDocument& doc, QDomElement& element) Q_DECL_OVERRIDE;

	void updateMonitor(void);

	void configureSettings(void) Q_DECL_OVERRIDE;

	void timerTick() Q_DECL_OVERRIDE
	{
		updateMonitor();
	}

	bool hasSettingsDialog() const Q_DECL_OVERRIDE
	{
		return true;
	}

public Q_SLOTS:
	void applySettings() Q_DECL_OVERRIDE;
	void applyStyle() Q_DECL_OVERRIDE;

	void settingsAddRule();
	void settingsDeleteRule();
	void settingsChangeRule();
	void settingsRuleListSelected(int index);
	void settingsRuleTextChanged();

private:
	Ui_LogFileSettings* lfs;
	QListWidget* monitor;
	QStringList filterRules;

	unsigned long logFileID;
};

#endif // _LogFile_h
