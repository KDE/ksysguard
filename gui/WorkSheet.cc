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
#include "ProcessController.h"

WorkSheet::WorkSheet(int columns, QWidget* parent) :
	QGrid(columns, parent)
{
	setAcceptDrops(TRUE);
}

void
WorkSheet::addDisplay(const QString& hostName, const QString& sensorName,
					  const QString& sensorType, SensorDisplay* current)
{
	if (!SensorMgr->engage(hostName))
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
		/* TODO: We need to make sure that the sensor display supports
		 * the sensor type. */
		current = it.current();
	}

	if (!current)
	{
		// No existing display has been specified, so we create a new one.
		/* Currently we support one specific sensor display for each
		 * sensor type. This will change for sure and can then be
		 * handled with a popup menu that lists all possible sensor
		 * displays for the sensor type. */
		if (sensorType == "integer")
			current = new FancyPlotter(this, 0, sensorName);
		else if (sensorType == "table")
			current = new ProcessController(this);
		else
		{
			debug("Unkown sensor type: " + sensorType);
			return;
		}
		current->show();
		displays.append(current);
	}

	current->addSensor(hostName, sensorName, "Test");
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
		// The host name, sensor name and type are seperated by a ' '.
		QString hostName = dObj.left(dObj.find(' '));
		dObj.remove(0, dObj.find(' ') + 1);
		QString sensorName = dObj.left(dObj.find(' '));
		dObj.remove(0, dObj.find(' ') + 1);
		QString sensorType = dObj;

		if (hostName.isEmpty() || sensorName.isEmpty() ||
			sensorType.isEmpty())
		{
			return;
		}

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
				addDisplay(hostName, sensorName, sensorType, it.current());
				return;
			}
		}
		// Not dropped over a diplay. Create a new display.
		addDisplay(hostName, sensorName, sensorType);
	}
}
