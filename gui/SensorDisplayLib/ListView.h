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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#ifndef _ListView_h_
#define _ListView_h_

#include <qlistview.h>
#include <qpainter.h>

#include <SensorDisplay.h>

typedef const char* (*KeyFunc)(const char*);

class QLabel;
class QBoxGroup;
class ListViewSettings;

class PrivateListView : public QListView
{
	Q_OBJECT
public:
	PrivateListView(QWidget *parent = 0, const char *name = 0);
	
	void addColumn(const QString& label, const QString& type);
	void removeColumns(void);
	void update(const QString& answer);
	QValueList<KeyFunc> getSortFunc(void) { return sortFunc; }

private:
	QValueList<KeyFunc> sortFunc;
  QStringList mColumnTypes;
};

class PrivateListViewItem : public QListViewItem
{
public:
	PrivateListViewItem(PrivateListView *parent = 0);

	void paintCell(QPainter *p, const QColorGroup &, int column, int width, int alignment) {
		QColorGroup cgroup = _parent->colorGroup();
		QListViewItem::paintCell(p, cgroup, column, width, alignment);
		p->setPen(cgroup.color(QColorGroup::Link));
		p->drawLine(0, height() - 1, width - 1, height() - 1);
	}

	void paintFocus(QPainter *, const QColorGroup, const QRect) {}

	virtual QString key(int column, bool) const;

private:
	QWidget *_parent;
};	

class ListView : public KSGRD::SensorDisplay
{
	Q_OBJECT
public:
	ListView(QWidget* parent = 0, const char* name = 0,
			const QString& = QString::null, int min = 0, int max = 0);
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

public slots:
	void applySettings();
	void applyStyle();

private:
	PrivateListView* monitor;
	ListViewSettings* lvs;
};

#endif
