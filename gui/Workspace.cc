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

#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>

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
	CHECK_PTR(s);
	if (s->exec())
	{
		WorkSheet* sheet = new WorkSheet(this, s->getRows(), s->getColumns());
		CHECK_PTR(sheet);
		insertTab(sheet, sheetName);
		sheets.append(sheet);
		showPage(sheet);
	}
	delete s;
}

void
Workspace::loadWorkSheet()
{
	KFileDialog fd(workDir, "*.ktop", this, "LoadFileDialog", TRUE);
	QString fileName = fd.getOpenFileName(QString::null, "*.ktop");
	if (fileName.isEmpty())
		return;

	workDir = fileName.left(fileName.findRev('/'));
	// extract filename without path
	QString baseName = fileName.right(fileName.length() -
									  fileName.findRev('/') - 1);
	// chop off extension (usually '.ktop')
	baseName = baseName.left(baseName.findRev('.'));

	// Add a new sheet to the workspace.
	WorkSheet* sheet = new WorkSheet(this);
	CHECK_PTR(sheet);
	insertTab(sheet, baseName);
	sheets.append(sheet);
	showPage(sheet);
	sheet->load(fileName);
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

	KFileDialog fd(workDir, "*.ktop", this, "LoadFileDialog", TRUE);
	QString fileName = fd.getSaveFileName(QString::null, "*.ktop");
	if (fileName.isEmpty())
		return;
	workDir = fileName.left(fileName.findRev('/'));
	// extract filename without path
	QString baseName = fileName.right(fileName.length() -
									  fileName.findRev('/') - 1);
	// chop off extension (usually '.ktop')
	baseName = baseName.left(baseName.findRev('.'));
	changeTab(currentPage(), baseName);

	sheet->save(fileName);
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

void
Workspace::saveProperties()
{
	QString sheetList;
	QListIterator<WorkSheet> it(sheets);
	for (; it.current(); ++it)
		sheetList += tabLabel(*it);
	// TODO: not finished yet.
}
