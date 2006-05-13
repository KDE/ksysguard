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

#include <q3listview.h>
#include <QPainter>
//Added by qt3to4:
#include <QLabel>
#include <QTimerEvent>
#include <QResizeEvent>

#include <SensorDisplay.h>

typedef const char* (*KeyFunc)(const char*);

class QLabel;
class QBoxGroup;
class ListViewSettings;

class PrivateListView : public Q3ListView
{
	Q_OBJECT
public:
  enum ColumnType { Text, Int, Float, Time, DiskStat };

	PrivateListView(QWidget *parent = 0, const char *name = 0);

	void addColumn(const QString& label, const QString& type);
	void removeColumns(void);
	void update(const QString& answer);
	int columnType( int pos ) const;

private:
  QStringList mColumnTypes;
};

class PrivateListViewItem : public Q3ListViewItem
{
public:
	PrivateListViewItem(PrivateListView *parent = 0);

	void paintCell(QPainter *p, const QColorGroup &, int column, int width, int alignment) {
		QColorGroup cgroup = QColorGroup( _parent->palette() );
		Q3ListViewItem::paintCell(p, cgroup, column, width, alignment);
		p->setPen(cgroup.color(QPalette::Link));
		p->drawLine(0, height() - 1, width - 1, height() - 1);
	}

	void paintFocus(QPainter *, const QColorGroup, const QRect) {}

	virtual int compare( Q3ListViewItem*, int column, bool ascending ) const;

private:
	QWidget *_parent;
};

class ListView : public KSGRD::SensorDisplay
{
	Q_OBJECT
public:
	ListView(QWidget* parent, const QString& title, bool isApplet);
	~ListView() {}

	bool addSensor(const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& sensorDescr);
	void answerReceived(int id, const QString& answer);
	void resizeEvent(QResizeEvent*);
	void updateList();

	bool restoreSettings(QDomElement& element);
	bool saveSettings(QDomDocument& doc, QDomElement& element, bool save = true);

	virtual bool hasSettingsDialog() const
	{
		return (true);
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
	PrivateListView* monitor;
	ListViewSettings* lvs;
};

#endif
