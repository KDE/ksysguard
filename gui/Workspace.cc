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

#include "Workspace.h"
#include "WorkSheet.h"
#include "WorkSheetSetup.h"
#include "Workspace.moc"

Workspace::Workspace(QWidget* parent, const char* name)
	: QTabWidget(parent, name)
{
	tabCount = 0;
}

void
Workspace::newWorkSheet()
{
	QString sheetName = QString(i18n("Sheet %1")).arg(++tabCount);
	WorkSheetSetup* s = new WorkSheetSetup(sheetName);
	if (s->exec())
		addSheet(s->getSheetName(), s->getRows(), s->getColumns());
	delete s;
}

void
Workspace::loadWorkSheet()
{
	debug("Not yet implemented");
}

void
Workspace::deleteWorkSheet()
{
	WorkSheet* current = (WorkSheet*) currentPage();

	if (current)
	{
		removePage(current);
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
Workspace::addSheet(const QString& title, int rows, int columns)
{
	WorkSheet* sheet = new WorkSheet(this, rows, columns);
	insertTab(sheet, title);
	showPage(sheet);
}
