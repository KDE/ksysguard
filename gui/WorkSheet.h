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

#ifndef _WorkSheet_h_
#define _WorkSheet_h_

#include <qgrid.h>

#include "SensorDisplay.h"

class QDragEnterEvent;
class QDropEvent;
class QString;

/**
 * A WorkSheet contains the displays to visualize the sensor results. When
 * creating the WorkSheet you must specify the number of columns. Displays
 * can be added and removed on the fly. The grid layout will handle the
 * layout. The number of columns can not be changed. Displays are added by
 * dragging a sensor from the sensor browser over the WorkSheet.
 */
class WorkSheet : public QGrid
{
	Q_OBJECT
public:
	WorkSheet(int columns, QWidget* parent);
	~WorkSheet() { }

	void addDisplay(const QString& hostname, const QString& monitor,
					SensorDisplay* current = 0);

protected:
	void dragEnterEvent(QDragEnterEvent* ev);
	void dropEvent(QDropEvent* ev);

private:
	QList<SensorDisplay> displays;
} ;

#endif
