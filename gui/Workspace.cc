/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "Workspace.h"
#include "WorkSheet.h"
#include "WorkSheetSetup.h"
#include "Workspace.moc"

Workspace::Workspace(QWidget* parent, const char* name)
	: QTabWidget(parent, name)
{
}

void
Workspace::newWorkSheet()
{
	/* Find a name of the form "Sheet %d" that is not yet used by any
	 * of the existing worksheets. */
	int i = 1;
	bool found;
	QString sheetName;
	do
	{
		sheetName = QString(i18n("Sheet %1")).arg(i++);
		QListIterator<WorkSheet> it(sheets);
		found = FALSE;
		for (; it.current() && !found; ++it)
			if (tabLabel(*it) == sheetName)
				found = TRUE;
	} while (found);

	WorkSheetSetup* s = new WorkSheetSetup(sheetName);
	if (s->exec())
		addSheet(s->getSheetName(), s->getRows(), s->getColumns());
	delete s;
}

void
Workspace::loadWorkSheet()
{
	QString fileName = "test.xml";
	QFile file(fileName);
	if (!file.open(IO_ReadOnly))
	{
		KMessageBox::sorry(this, i18n("Can't open the file %1")
						   .arg(fileName));
		return;
	}
    
	// Parse the XML file.
	QDomDocument doc;
	// Read in file and check for a valid XML header.
	if (!doc.setContent(&file))
	{
		KMessageBox::sorry(
			this,
			i18n("The file %1 does not contain valid XML").arg(fileName));
		return;
	}
	// Check for proper document type.
	if (doc.doctype().name() != "ktopWorkSheet")
	{
		KMessageBox::sorry(
			this,
			i18n("The file %1 does not contain a valid work sheet\n"
				 "definition, which must have a document type\n"
				 "'ktopWorkSheet'").arg(fileName));
		return;
	}
	// Check for proper name.
	QDomElement element = doc.documentElement();
	QString name = element.attribute("name");
	if (name.isEmpty())
	{
		KMessageBox::sorry(
			this, i18n("The file %1 lacks a name for the work sheet.")
			.arg(fileName));
		return;
	}
	bool rowsOk;
	uint rows = element.attribute("rows").toUInt(&rowsOk);
	bool columnsOk;
	uint columns = element.attribute("columns").toUInt(&columnsOk);
	if (!(rowsOk && columnsOk))
	{
		KMessageBox::sorry(
			this, i18n("The file %1 has an invalid work sheet size.")
			.arg(fileName));
		return;
	}

	// Add a new sheet to the workspace.
	WorkSheet* sheet = addSheet(name, rows, columns);
	CHECK_PTR(sheet);
	sheet->load(element);
}

void
Workspace::saveWorkSheet()
{
	WorkSheet* sheet = (WorkSheet*) currentPage();
	if (!sheet)
	{
		KMessageBox::sorry(
			this, i18n("You don't have a worksheet that could be saved!"));
		return;
	}

	QString fileName = "test.xml";
	QFile file(fileName);
	if (!file.open(IO_WriteOnly))
	{
		KMessageBox::sorry(this, i18n("Can't open the file %1")
						   .arg(fileName));
		return;
	}
	QTextStream s(&file);
    s << "<!DOCTYPE ktopWorkSheet>\n";

	sheet->save(s, tabLabel(sheet));

	file.close();
}

void
Workspace::deleteWorkSheet()
{
	WorkSheet* current = (WorkSheet*) currentPage();

	if (current)
	{
		removePage(current);
		sheets.remove(current);
		delete current;
	}
	else
	{
		QString msg = i18n("There are no work sheets that\n"
						   "could be deleted!");
		KMessageBox::error(this, msg);
	}
}

WorkSheet*
Workspace::addSheet(const QString& title, int rows, int columns)
{
	WorkSheet* sheet = new WorkSheet(this, rows, columns);
	insertTab(sheet, title);
	sheets.append(sheet);
	showPage(sheet);

	return (sheet);
}

void
Workspace::saveProperties()
{
	QString sheetList;
	QListIterator<WorkSheet> it(sheets);
	for (; it.current(); ++it)
		sheetList += tabLabel(*it);
	// TODO: not finished yet.
}
