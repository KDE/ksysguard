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

#include <qwhatsthis.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include "Workspace.h"
#include "WorkSheet.h"
#include "WorkSheetSetup.h"
#include "Workspace.moc"

Workspace::Workspace(QWidget* parent, const char* name)
	: QTabWidget(parent, name)
{
	sheets.setAutoDelete(TRUE);
	autoSave = TRUE;

	QWhatsThis::add(this, i18n(
		"This is you work space. It holds your work sheets. You need "
		"to create a new work sheet (Menu File->New) before "
		"you can drag sensors here."));
}

void
Workspace::saveProperties(KConfig* cfg)
{
	cfg->writeEntry("WorkDir", workDir);
	cfg->writeEntry("CurrentSheet", tabLabel(currentPage()));

	QString sheetList;
	QListIterator<WorkSheet> it(sheets);
	int i;
	QStringList list;
	for (i = 0; it.current(); ++it, ++i)
		list.append((*it)->getFileName());
	cfg->writeEntry("Sheets", list);
}

void
Workspace::readProperties(KConfig* cfg)
{
	QString currentSheet;

	workDir = cfg->readEntry("WorkDir");

	if (workDir.isEmpty())
	{
		/* If workDir is not specified in the config file, it's
		 * probably the first time the user has started KSysGuard. We
		 * then "restore" a special default configuration. */
		KStandardDirs* kstd = KGlobal::dirs();
		kstd->addResourceType("data", "share/apps/ksysguard");

		workDir = kstd->saveLocation("data", "ksysguard");

		QString f = kstd->findResource("data", "SystemLoad.sgrd");
		QString fNew = workDir + "/" + i18n("System Load") + ".sgrd";
		if (!f.isEmpty())
			restoreWorkSheet(f, fNew);

		f = kstd->findResource("data", "ProcessTable.sgrd");
		fNew = workDir + "/" + i18n("Process Table") + ".sgrd";
		if (!f.isEmpty())
			restoreWorkSheet(f, fNew);

		currentSheet = i18n("System Load");
	}
	else
	{
		currentSheet = cfg->readEntry("CurrentSheet");
		QStringList list = cfg->readListEntry("Sheets");
		for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
			restoreWorkSheet(*it);
	}

	// Determine visible sheet.
	QListIterator<WorkSheet> it(sheets);
	for (; it.current(); ++it)
		if (currentSheet == tabLabel(*it))
		{
			showPage(*it);
			break;
		}
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
		insertTab(sheet, s->getSheetName());
		sheets.append(sheet);
		showPage(sheet);
	}
	delete s;
}

bool
Workspace::saveOnQuit()
{
	QListIterator<WorkSheet> it(sheets);
	for (; it.current(); ++it)
		if ((*it)->hasBeenModified())
		{
			if (!autoSave || (*it)->getFileName().isEmpty())
			{
				int res = KMessageBox::warningYesNoCancel(
					this,
					QString(i18n("The worksheet '%1' contains unsaved data\n"
								 "Do you want to save the worksheet?"))
					.arg(tabLabel(*it)));
				if (res == KMessageBox::Yes)
					saveWorkSheet(*it);
				else if (res == KMessageBox::Cancel)
					return FALSE;	// abort quit
			}
			else
				saveWorkSheet(*it);
		}

	return (TRUE);
}

void
Workspace::loadWorkSheet()
{
	KFileDialog fd(0, "*.sgrd", this, "LoadFileDialog", TRUE);
	KURL url = fd.getOpenURL(workDir, "*.sgrd", 0,
								i18n("Select a work sheet to load"));
	loadWorkSheet(url);
}

void
Workspace::loadWorkSheet(const KURL& url)
{
	if (url.isEmpty())
		return;

	/* It's probably not worth the effort to make this really network
	 * transparent. Unless s/o beats me up I use this pseudo transparent
	 * code. */
	QString tmpFile;
    KIO::NetAccess::download(url, tmpFile);
	workDir = tmpFile.left(tmpFile.findRev('/'));
	// Load sheet from file.
	restoreWorkSheet(tmpFile);
	/* If we have loaded a non-local file we clear the file name so that
	 * the users is prompted for a new name for saving the file. */
	KURL tmpFileUrl;
	tmpFileUrl.setPath(tmpFile);
	if (tmpFileUrl != url.url())
		sheets.last()->setFileName(QString::null);
    KIO::NetAccess::removeTempFile(tmpFile);

	emit announceRecentURL(KURL(url));
}

void
Workspace::saveWorkSheet(WorkSheet* sheet)
{
	if (!sheet)
	{
		KMessageBox::sorry(
			this, i18n("You don't have a worksheet that could be saved!"));
		return;
	}

	QString fileName = sheet->getFileName();
	if (fileName.isEmpty())
	{
		KFileDialog fd(0, "*.sgrd", this, "LoadFileDialog", TRUE);
		fileName = fd.getSaveFileName(workDir + "/" +
			tabLabel(currentPage()) + ".sgrd", "*.sgrd", 0,
			i18n("Save current work sheet as"));
		if (fileName.isEmpty())
			return;
		workDir = fileName.left(fileName.findRev('/'));
		// extract filename without path
		QString baseName = fileName.right(fileName.length() -
										  fileName.findRev('/') - 1);
		// chop off extension (usually '.sgrd')
		baseName = baseName.left(baseName.findRev('.'));
		changeTab(currentPage(), baseName);
	}
	/* If we cannot save the file is probably write protected. So we need
	 * to ask the user for a new name. */
	if (!sheet->save(fileName))
	{
		saveWorkSheetAs();
		return;
	}

	/* Add file to recent documents menue. */
	KURL url;
	url.setPath(fileName);
	emit announceRecentURL(url);
}

void
Workspace::saveWorkSheetAs()
{
	WorkSheet* sheet = (WorkSheet*) currentPage();
	if (!sheet)
	{
		KMessageBox::sorry(
			this, i18n("You don't have a worksheet that could be saved!"));
		return;
	}

	QString fileName;
	do
	{
		KFileDialog fd(0, "*.sgrd", this, "LoadFileDialog", TRUE);
		fileName = fd.getSaveFileName(workDir + "/" +
			tabLabel(currentPage()) + ".sgrd", "*.sgrd");
		if (fileName.isEmpty())
			return;
		workDir = fileName.left(fileName.findRev('/'));
		// extract filename without path
		QString baseName = fileName.right(fileName.length() -
										  fileName.findRev('/') - 1);
		// chop off extension (usually '.sgrd')
		baseName = baseName.left(baseName.findRev('.'));
		changeTab(currentPage(), baseName);
	} while (!sheet->save(fileName));

	/* Add file to recent documents menue. */
	KURL url;
	url.setPath(fileName);
	emit announceRecentURL(url);
}

void
Workspace::deleteWorkSheet()
{
	WorkSheet* current = (WorkSheet*) currentPage();

	if (current)
	{
		removePage(current);
		sheets.remove(current);

		setMinimumSize(sizeHint());
	}
	else
	{
		QString msg = i18n("There are no work sheets that\n"
						   "could be deleted!");
		KMessageBox::error(this, msg);
	}
}

void
Workspace::deleteWorkSheet(const QString& fileName)
{
	QListIterator<WorkSheet> it(sheets);
	for (; it.current(); ++it)
		if ((*it)->getFileName() == fileName)
		{
			removePage(*it);
			sheets.remove(*it);

			setMinimumSize(sizeHint());
			return;
		}
}

bool
Workspace::restoreWorkSheet(const QString& fileName, const QString& newName)
{
	/* We might want to save the worksheet under a different name later. This
	 * name can be specified by newName. If newName is empty we use the
	 * original name to save the work sheet. */
	QString fName;
	if (newName.isEmpty())
		fName = fileName;
	else
		fName = newName;

	// extract filename without path
	QString baseName = fName.right(fName.length() -
									  fName.findRev('/') - 1);
	// chop off extension (usually '.sgrd')
	baseName = baseName.left(baseName.findRev('.'));

	WorkSheet* sheet = new WorkSheet(this);
	CHECK_PTR(sheet);
	insertTab(sheet, baseName);
	showPage(sheet);
	if (!sheet->load(fileName))
	{
		delete sheet;
		return (FALSE);
	}
	sheets.append(sheet);

	/* Force the file name to be the new name. This also sets the modified
	 * flag, so that the file will get saved on exit. */
	if (!newName.isEmpty())
		sheet->setFileName(newName);

	return (TRUE);
}

void
Workspace::showProcesses()
{
	KStandardDirs* kstd = KGlobal::dirs();
	kstd->addResourceType("data", "share/apps/ksysguard");

	QString f = kstd->findResource("data", "ProcessTable.sgrd");
	if (!f.isEmpty())
		restoreWorkSheet(f);
	else
		KMessageBox::error(
			this, i18n("Cannot find file ProcessTable.sgrd!"));
}
