/*
    KTop, the KDE Task Manager and System Monitor
   
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qgroupbox.h>
#include <qwhatsthis.h>

#include <klocale.h>

#include "SensorManager.h"
#include "DummyDisplay.moc"

DummyDisplay::DummyDisplay(QWidget* parent, const char* name,
					   const QString&, double, double)
	: SensorDisplay(parent, name)
{
	frame->setTitle(i18n("Drop sensor here"));
	setMinimumSize(16, 16);

	/* All events to the frame widget will be handled by 
	 * SensorDisplay::eventFilter. */
	frame->installEventFilter(this);

	QWhatsThis::add(this, i18n(
		"This is an empty space in a work sheet. Drag a sensor from "
		"the Sensor Browser and drop it here. A sensor display will "
		"appear that allows you to monitor the values of the sensor "
		"over time."));
}

void
DummyDisplay::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(0, 0, width(), height());
}

bool
DummyDisplay::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::MouseButtonRelease &&
			 ((QMouseEvent*) e)->button() == LeftButton)
	{
		setFocus();
	}
	return QWidget::eventFilter(o, e);
}
