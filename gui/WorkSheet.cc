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

#include <qdragobject.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "WorkSheet.h"
#include "WorkSheet.moc"

#include "SensorManager.h"
#include "FancyPlotter.h"

WorkSheet::WorkSheet(int columns, QWidget* parent) :
	QGrid(columns, parent)
{
	setAcceptDrops(TRUE);
}

void
WorkSheet::addDisplay(const QString& hostName, const QString& sensorName,
					  SensorDisplay* current)
{
	SensorAgent* sensor = SensorMgr->engage(hostName);

	if (!sensor)
	{
		QString msg = i18n("Unknown hostname \'%1\' or sensor \'%2\'!")
			.arg(hostName).arg(sensorName);
		KMessageBox::error(this, msg);
		return;
	}

	if (current)
	{
		/* If the sensor should be added to an existing widget we have to
		 * make sure that the specified widget is in fact an existing
		 * display. */
		QListIterator<SensorDisplay> it(displays);
		for ( ; it.current() && (it.current() != current); ++it)
			;
		current = it.current();
	}

	if (!current)
	{
		/* No display has focus and no existing widget has been requested.
		 * So we create a new display. */
		current = new FancyPlotter(this, 0, sensorName);
		current->show();
		displays.append(current);
	}

	current->addSensor(sensor, sensorName, "Test");
}

void
WorkSheet::dragEnterEvent(QDragEnterEvent* ev)
{
    ev->accept(QTextDrag::canDecode(ev));
}

void
WorkSheet::dropEvent(QDropEvent* ev)
{
	QString dObj;

	if (QTextDrag::decode(ev, dObj))
	{
		// The host name and the sensor name are seperated by a ' '.
		QString hostName = dObj.left(dObj.find(' '));
		dObj.remove(0, dObj.find(' ') + 1);
		QString sensorName = dObj;

		if (hostName.isEmpty() || sensorName.isEmpty())
			return;

		/* If the event occured over a display the sensor is added to that
		 * existing widget. If it was dropped over the sheet (background)
		 * a new display is created. Since the displays of a full sheet
		 * cover almost the whole sheet, a margin of 15 pixels at all sides
		 * of all displays is counted to the sheet. This is roughly equivalent
		 * of dropping the object outside of the QFrame line. */
		const int margin = 15;
		QListIterator<SensorDisplay> it(displays);
		for ( ; it.current(); ++it)
		{
			QRect r;
			r.setX(it.current()->x() + margin);
			r.setY(it.current()->y() + margin);
			r.setWidth(it.current()->width() - 2 * margin);
			r.setHeight(it.current()->height() - 2 * margin);
			if (r.contains(ev->pos()))
			{
				// Dropped over a display. Add sensor to existing display.
				addDisplay(hostName, sensorName, it.current());
				return;
			}
		}
		// Not dropped over a diplay. Create a new display.
		addDisplay(hostName, sensorName);
	}
}
