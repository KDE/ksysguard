/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _SensorBrowser_h_
#define _SensorBrowser_h_

#include <qlistview.h>

#include "SensorClient.h"

class QMouseEvent;
class SensorManager;

class SensorBrowser : public QListView, public SensorClient
{
	Q_OBJECT

public:
	SensorBrowser(QWidget* parent, SensorManager* sm, const char* name);
	~SensorBrowser() { }

public slots:
	void update();

protected:
	virtual void viewportMouseMoveEvent(QMouseEvent* ev);

private:
	void answerReceived(int id, const QString& s);

	SensorManager* sensorManager;
	QList<QListViewItem> sensors;

	// This string stores the drag object.
	QString dragText;
} ;

#endif
