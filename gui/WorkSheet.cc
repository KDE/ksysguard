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
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "WorkSheet.h"
#include "SensorManager.h"
#include "FancyPlotter.h"
#include "MultiMeter.h"
#include "ProcessController.h"
#include "WorkSheet.moc"

WorkSheet::WorkSheet(QWidget* parent) :
	QWidget(parent)
{
	lm = 0;
	rows = columns = 0;
	displays = 0;
	modified = FALSE;
	fileName = "";

	setAcceptDrops(TRUE);
}

WorkSheet::WorkSheet(QWidget* parent, int r, int c) :
	QWidget(parent)
{
	lm = 0;
	displays = 0;
	modified = FALSE;
	fileName = "";
	createGrid(r, c);

	setAcceptDrops(TRUE);
}

WorkSheet::~WorkSheet()
{
	debug("Deleting work sheet...");
	for (int i = 0; i < rows; ++i)
		delete [] displays[i];

	delete [] displays;
}

bool
WorkSheet::hasBeenModified()
{
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < columns; ++j)
			if (((SensorDisplay*)displays[i][j])->hasBeenModified())
				return (TRUE);

	return (modified);
}

bool
WorkSheet::load(const QString& fN)
{
	QFile file(fileName = fN);
	if (!file.open(IO_ReadOnly))
	{
		KMessageBox::sorry(this, i18n("Can't open the file %1")
						   .arg(fileName));
		return (FALSE);
	}
    
	// Parse the XML file.
	QDomDocument doc;
	// Read in file and check for a valid XML header.
	if (!doc.setContent(&file))
	{
		KMessageBox::sorry(
			this,
			i18n("The file %1 does not contain valid XML").arg(fileName));
		return (FALSE);
	}
	// Check for proper document type.
	if (doc.doctype().name() != "KSysGuardWorkSheet")
	{
		KMessageBox::sorry(
			this,
			i18n("The file %1 does not contain a valid work sheet\n"
				 "definition, which must have a document type\n"
				 "'KSysGuardWorkSheet'").arg(fileName));
		return (FALSE);
	}
	// Check for proper size.
	QDomElement element = doc.documentElement();
	bool rowsOk;
	uint r = element.attribute("rows").toUInt(&rowsOk);
	bool columnsOk;
	uint c = element.attribute("columns").toUInt(&columnsOk);
	if (!(rowsOk && columnsOk))
	{
		KMessageBox::sorry(
			this, i18n("The file %1 has an invalid work sheet size.")
			.arg(fileName));
		return (FALSE);
	}

	createGrid(r, c);

	uint i;

	/* Load lists of hosts that are needed for the work sheet and try
	 * to establish a connection. */
	QDomNodeList dnList = element.elementsByTagName("host");
	for (i = 0; i < dnList.count(); ++i)
	{
		QDomElement element = dnList.item(i).toElement();
		SensorMgr->engage(element.attribute("name"),
						  element.attribute("shell"),
						  element.attribute("command"));
	}

	// Load the displays and place them into the work sheet.
	dnList = element.elementsByTagName("display");
	for (i = 0; i < dnList.count(); ++i)
	{
		QDomElement element = dnList.item(i).toElement();
		int row = element.attribute("row").toUInt();
		int column = element.attribute("column").toUInt();
		if (row >= rows || column >= columns)
		{
			debug("Row or Column out of range (%d/%d)", row, column);
			return (FALSE);
		}

		QString classType = element.attribute("class");
		SensorDisplay* newDisplay;
		if (classType == "FancyPlotter")
			newDisplay = new FancyPlotter(this);
		else if (classType == "MultiMeter")
			newDisplay = new MultiMeter(this);
		else if (classType == "ProcessController")
			newDisplay = new ProcessController(this);
		else
		{
			debug("Unkown class %s", classType.latin1());
			return (FALSE);
		}
		CHECK_PTR(newDisplay);

		// load display specific settings
		if (!newDisplay->load(element))
			return (FALSE);

		replaceDisplay(row, column, newDisplay);
	}

	return (TRUE);
}

bool
WorkSheet::save(const QString& fN)
{
	// Parse the XML file.
	QDomDocument doc("KSysGuardWorkSheet");
	doc.appendChild(doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\""));

	// save work sheet information
	QDomElement ws = doc.createElement("WorkSheet");
	doc.appendChild(ws);
	ws.setAttribute("rows", rows);
	ws.setAttribute("columns", columns);

	QStringList hosts;
	collectHosts(hosts);

	// save host information (name, shell, etc.)
	QStringList::Iterator it;
	for (it = hosts.begin(); it != hosts.end(); ++it)
	{
		QDomElement host = doc.createElement("host");
		ws.appendChild(host);
		QString shell, command;
		SensorMgr->getHostInfo(*it, shell, command);
		host.setAttribute("name", *it);
		host.setAttribute("shell", shell);
		host.setAttribute("command", command);
	}
	
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < columns; ++j)
			if (!displays[i][j]->isA("QGroupBox"))
			{
				SensorDisplay* displayP = (SensorDisplay*) displays[i][j];
				QDomElement display = doc.createElement("display");
				ws.appendChild(display);
				display.setAttribute("row", i);
				display.setAttribute("column", j);
				display.setAttribute("class", displayP->className());

				displayP->save(doc, display);
			}	

	QFile file(fileName = fN);
	if (!file.open(IO_WriteOnly))
	{
		KMessageBox::sorry(this, i18n("Can't open the file %1")
						   .arg(fileName));
		return (FALSE);
	}
	QTextStream s(&file);
	s << doc;
	file.close();

	modified = FALSE;
	return (TRUE);
}

SensorDisplay*
WorkSheet::addDisplay(const QString& hostName, const QString& sensorName,
					  const QString& sensorType, const QString& sensorDescr,
					  int r, int c, const QString& displayType)
{
	if (!SensorMgr->engage(hostName))
	{
		QString msg = i18n("Unknown hostname \'%1\'!").arg(hostName);
		KMessageBox::error(this, msg);
		return (0);
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
		{
			QPopupMenu pm;
			pm.insertItem(i18n("Select a display type"), 0);
			pm.setItemEnabled(0, FALSE);
			pm.insertSeparator();
			pm.insertItem(i18n("&Signal Plotter"), 1);
			pm.insertItem(i18n("&Multimeter"), 2);
			switch (pm.exec(QCursor::pos()))
			{
			case 1:
				newDisplay = new FancyPlotter(this, "FancyPlotter",
											  sensorDescr);
				break;
			case 2:
				newDisplay = new MultiMeter(this, "MultiMeter", sensorDescr);
				break;
			default:
				return (0);
			}
		}
		else if (sensorType == "table")
			newDisplay = new ProcessController(this);
		else
		{
			debug("Unkown sensor type: %s", sensorType.latin1());
			return (0);
		}
		replaceDisplay(r, c, newDisplay);
	}

	((SensorDisplay*) displays[r][c])->
		addSensor(hostName, sensorName, displayType);

	modified = TRUE;
	return ((SensorDisplay*) displays[r][c]);
}

void
WorkSheet::showPopupMenu(SensorDisplay* display)
{
	display->settings();
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
				modified = TRUE;
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
		QString sensorType = dObj.left(dObj.find(' '));
		dObj.remove(0, dObj.find(' ') + 1);
		QString sensorDescr = dObj;

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
					addDisplay(hostName, sensorName, sensorType,
							   sensorDescr, i, j);

					// Notify parent about possibly new minimum size.
					((QWidget*) parent()->parent())->setMinimumSize(
						((QWidget*) parent()->parent())->sizeHint());
					return;
				}
	}
}

void
WorkSheet::customEvent(QCustomEvent* ev)
{
	if (ev->type() == QEvent::User)
	{
		// SensorDisplays send out this event if they want to be removed.
		removeDisplay((SensorDisplay*) ev->data());
		delete ev;
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

void
WorkSheet::replaceDisplay(int r, int c, SensorDisplay* newDisplay)
{
	// remove the old display at this location
	delete displays[r][c];

	// insert new display
	lm->addWidget(newDisplay, r, c);
	newDisplay->show();
	displays[r][c] = newDisplay;
	connect(newDisplay, SIGNAL(showPopupMenu(SensorDisplay*)),
			this, SLOT(showPopupMenu(SensorDisplay*)));
}

void
WorkSheet::collectHosts(QValueList<QString>& list)
{
	for (int r = 0; r < rows; ++r)
		for (int c = 0; c < columns; ++c)
			if (!displays[r][c]->isA("QGroupBox"))
				((SensorDisplay*) displays[r][c])->collectHosts(list);
}

void
WorkSheet::createGrid(uint r, uint c)
{
	if (lm)
		delete lm;

	rows = r;
	columns = c;

	// create grid layout with specified dimentions
	lm = new QGridLayout(this, r, c, 5);
	CHECK_PTR(lm);

	// and fill it with dummy displays
	int i, j;
	displays = new QWidget**[rows];
	CHECK_PTR(displays);
	for (i = 0; i < rows; ++i)
	{
		lm->setRowStretch(i, 1);
		displays[i] = new QWidget*[columns];
		CHECK_PTR(displays[i]);
		for (j = 0; j < columns; ++j)
		{
			lm->setColStretch(j, 1);
			insertDummyDisplay(i, j);
		}
	}
	lm->activate();
}
