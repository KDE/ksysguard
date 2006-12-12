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

#ifndef _ListView_h_
#define _ListView_h_

#include <QStandardItemModel>
#include <QList>
#include <SensorDisplay.h>

class ListViewSettings;
class QTreeView;

class ListView : public KSGRD::SensorDisplay
{
	Q_OBJECT
public:
	ListView(QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
	~ListView() {}

	bool addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& sensorDescr);
	void answerReceived(int id, const QList<QByteArray>& answerlist);
	void updateList();

	bool restoreSettings(QDomElement& element);
	bool saveSettings(QDomDocument& doc, QDomElement& element);

	virtual bool hasSettingsDialog() const
	{
		return true;
	}

	virtual void timerEvent(QTimerEvent*)
	{
		updateList();
	}

	void configureSettings();

public Q_SLOTS:
	void applySettings();
	void applyStyle();

private:

	typedef enum { Text, Int, Float, Time, DiskStat } ColumnType;

	QStandardItemModel mModel;
	QTreeView *mView;
	ListViewSettings* lvs;
	
	QList<ColumnType> mColumnTypes;
        ColumnType convertColumnType(const QString &type) const;
};

#endif
