/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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
#include "SensorManager.h"
#include "FancyPlotter.h"
#include "ProcessController.h"
#include "WorkSheet.moc"

WorkSheet::WorkSheet(QWidget* parent, int r, int c) :
	QWidget(parent), rows(r), columns(c)
{
	// create grid layout with specified dimentions
	lm = new QGridLayout(this, rows, columns, 10);
	CHECK_PTR(lm);

	// and fill it with dummy displays
	int i, j;
	displays = new QWidget**[rows];
	CHECK_PTR(displays);
	for (i = 0; i < rows; ++i)
	{
		displays[i] = new QWidget*[columns];
		CHECK_PTR(displays[i]);
		for (j = 0; j < columns; ++j)
			insertDummyDisplay(i, j);
	}
	lm->activate();
	setAcceptDrops(TRUE);
}

WorkSheet::~WorkSheet()
{
	for (int i = 0; i < rows; ++i)
		delete [] displays[i];

	delete [] displays;
}

void
WorkSheet::addDisplay(const QString& hostName, const QString& sensorName,
					  const QString& sensorType, int r, int c)
{
	if (!SensorMgr->engage(hostName))
	{
		QString msg = i18n("Unknown hostname \'%1\' or sensor \'%2\'!")
			.arg(hostName).arg(sensorName);
		KMessageBox::error(this, msg);
		return;
	}

	/* If the by 'r' and 'c' specified display is a QGroupBox dummy
	 * display we replace the widget. Otherwise we just try to add
	 * the new sensor to an existing display. */
	if (displays[r][c]->isA("QGroupBox"))
	{
		SensorDisplay* newDisplay = 0;
		/* Currently we support one specific sensor display for each
		 * sensor type. This will change for sure and can then be
		 * handled with a popup menu that lists all possible sensor
		 * displays for the sensor type. */
		if (sensorType == "integer")
			newDisplay = new FancyPlotter(this, 0, sensorName);
		else if (sensorType == "table")
			newDisplay = new ProcessController(this);
		else
		{
			debug("Unkown sensor type: " + sensorType);
			return;
		}

		// remove the old display at this location
		delete displays[r][c];
		// insert new display
		lm->addWidget(newDisplay, r, c);
		newDisplay->show();
		displays[r][c] = newDisplay;
		connect(newDisplay, SIGNAL(removeDisplay(SensorDisplay*)),
				this, SLOT(removeDisplay(SensorDisplay*)));
	}

	((SensorDisplay*) displays[r][c])->
		addSensor(hostName, sensorName, "Unused");
}

void
WorkSheet::removeDisplay(SensorDisplay* display)
{
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < columns; ++j)
			if (displays[i][j] == display)
			{
				delete display;
				insertDummyDisplay(i, j);
			}
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

		/* Find the sensor display that is supposed to get the drop
		 * event and replace or add sensor. */
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < columns; ++j)
				if (displays[i][j]->geometry().contains(ev->pos()))
				{
					addDisplay(hostName, sensorName, sensorType, i, j);
					return;
				}
	}
}

void
WorkSheet::insertDummyDisplay(int r, int c)
{
	QGroupBox* dummy = new QGroupBox(this, "dummy frame");
	dummy->setMinimumSize(40, 25);
	dummy->setTitle(i18n("Drop sensor here"));
	displays[r][c] = dummy;
	lm->addWidget(dummy, r, c);
	displays[r][c]->show();
}
