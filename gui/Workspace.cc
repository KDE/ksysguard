/*
    KSysGuard, the KDE System Guard

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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qlineedit.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "WorkSheet.h"
#include "WorkSheetSettings.h"
#include "Workspace.moc"

Workspace::Workspace(QWidget* parent, const char* name)
	: QTabWidget(parent, name)
{
	sheets.setAutoDelete(true);
	autoSave = true;

	connect(this, SIGNAL(currentChanged(QWidget*)),
			this, SLOT(updateCaption(QWidget*)));

	QWhatsThis::add(this, i18n(
		"This is your work space. It holds your worksheets. You need "
		"to create a new worksheet (Menu File->New) before "
		"you can drag sensors here."));
}

Workspace::~Workspace()
{
	/* This workaround is necessary to prevent a crash when the last
	 * page is not the current page. It seems like the the signal/slot
	 * administration data is already deleted but slots are still
	 * being triggered. TODO: I need to ask the Trolls about this. */
	disconnect(this, SIGNAL(currentChanged(QWidget*)),
			   this, SLOT(updateCaption(QWidget*)));
}

void
Workspace::saveProperties(KConfig* cfg)
{
	cfg->writeEntry("WorkDir", workDir);
	cfg->writeEntry("CurrentSheet", tabLabel(currentPage()));

	QString sheetList;
	QPtrListIterator<WorkSheet> it(sheets);
	int i;
	QStringList list;
	for (i = 0; it.current(); ++it, ++i)
		if ((*it)->getFileName() != "")
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
	QPtrListIterator<WorkSheet> it(sheets);
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
		QPtrListIterator<WorkSheet> it(sheets);
		found = false;
		for (; it.current() && !found; ++it)
			if (tabLabel(*it) == sheetName)
				found = true;
	} while (found);

	WorkSheetSettings* wss = new WorkSheetSettings(this, "WorkSheetSettings",
												   true);
	Q_CHECK_PTR(wss);
	wss->sheetName->setText(sheetName);
	wss->sheetName->setFocus();
	if (wss->exec())
	{
		WorkSheet* sheet = new WorkSheet(this, wss->rows->text().toUInt(),
										 wss->columns->text().toUInt(),
										 wss->interval->text().toUInt());
		Q_CHECK_PTR(sheet);
		sheet->setName(wss->sheetName->text());
		insertTab(sheet, wss->sheetName->text());
		sheets.append(sheet);
		showPage(sheet);
		connect(sheet, SIGNAL(sheetModified(QWidget*)),
				this, SLOT(updateCaption(QWidget*)));
	}
	delete wss;
}

bool
Workspace::saveOnQuit()
{
	QPtrListIterator<WorkSheet> it(sheets);
	for (; it.current(); ++it)
		if ((*it)->hasBeenModified())
		{
			if (!autoSave || (*it)->getFileName().isEmpty())
			{
				int res = KMessageBox::warningYesNoCancel(
					this,
					QString(i18n("The worksheet '%1' contains unsaved data.\n"
								 "Do you want to save the worksheet?"))
					.arg(tabLabel(*it)));
				if (res == KMessageBox::Yes)
					saveWorkSheet(*it);
				else if (res == KMessageBox::Cancel)
					return false;	// abort quit
			}
			else
				saveWorkSheet(*it);
		}

	return (true);
}

void
Workspace::loadWorkSheet()
{
	KFileDialog fd(0, "*.sgrd", this, "LoadFileDialog", true);
	KURL url = fd.getOpenURL(workDir, "*.sgrd", 0,
								i18n("Select a Worksheet to Load"));
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
	if (!restoreWorkSheet(tmpFile))
		return;

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
		KFileDialog fd(0, "*.sgrd", this, "LoadFileDialog", true);
		fileName = fd.getSaveFileName(workDir + "/" +
			tabLabel(sheet) + ".sgrd", "*.sgrd", 0,
			i18n("Save Current Worksheet As"));
		if (fileName.isEmpty())
			return;
		workDir = fileName.left(fileName.findRev('/'));
		// extract filename without path
		QString baseName = fileName.right(fileName.length() -
										  fileName.findRev('/') - 1);
		// chop off extension (usually '.sgrd')
		baseName = baseName.left(baseName.findRev('.'));
		changeTab(sheet, baseName);
	}
	/* If we cannot save the file is probably write protected. So we need
	 * to ask the user for a new name. */
	if (!sheet->save(fileName))
	{
		saveWorkSheetAs(sheet);
		return;
	}

	/* Add file to recent documents menue. */
	KURL url;
	url.setPath(fileName);
	emit announceRecentURL(url);
}

void
Workspace::saveWorkSheetAs(WorkSheet* sheet)
{
	if (!sheet)
	{
		KMessageBox::sorry(
			this, i18n("You don't have a worksheet that could be saved!"));
		return;
	}

	QString fileName;
	do
	{
		KFileDialog fd(0, "*.sgrd", this, "LoadFileDialog", true);
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
		changeTab(sheet, baseName);
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
		if (current->hasBeenModified())
		{
			if (!autoSave || current->getFileName().isEmpty())
			{
				int res = KMessageBox::warningYesNoCancel(
					this,
					QString(i18n("The worksheet '%1' contains unsaved data.\n"
								 "Do you want to save the worksheet?"))
					.arg(tabLabel(current)));
				if (res == KMessageBox::Yes)
					saveWorkSheet(current);
			}
			else
				saveWorkSheet(current);
		}

		removePage(current);
		sheets.remove(current);

		setMinimumSize(sizeHint());
	}
	else
	{
		QString msg = i18n("There are no worksheets that "
						   "could be deleted!");
		KMessageBox::error(this, msg);
	}
}

void
Workspace::removeAllWorkSheets()
{
	WorkSheet *sheet;
	while ( sheet = ( WorkSheet * )currentPage() )
	{
		removePage( sheet );
		sheets.remove( sheet );
	}
}

void
Workspace::deleteWorkSheet(const QString& fileName)
{
	QPtrListIterator<WorkSheet> it(sheets);
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
	Q_CHECK_PTR(sheet);
	sheet->setName(baseName);
	insertTab(sheet, baseName);
	showPage(sheet);
	if (!sheet->load(fileName))
	{
		delete sheet;
		return (false);
	}
	sheets.append(sheet);
	connect(sheet, SIGNAL(sheetModified(QWidget*)),
			this, SLOT(updateCaption(QWidget*)));

	/* Force the file name to be the new name. This also sets the modified
	 * flag, so that the file will get saved on exit. */
	if (!newName.isEmpty())
		sheet->setFileName(newName);

	return (true);
}

void
Workspace::cut()
{
	WorkSheet* current = (WorkSheet*) currentPage();

	if (current)
		current->cut();
}

void
Workspace::copy()
{
	WorkSheet* current = (WorkSheet*) currentPage();

	if (current)
		current->copy();
}

void
Workspace::paste()
{
	WorkSheet* current = (WorkSheet*) currentPage();

	if (current)
		current->paste();
}

void
Workspace::configure()
{
	WorkSheet* current = (WorkSheet*) currentPage();

	if (!current)
		return;

	current->settings();
}

void
Workspace::updateCaption(QWidget* ws)
{
	if (ws)
		emit setCaption(tabLabel(ws),
						((WorkSheet*) ws)->hasBeenModified());
	else
		emit setCaption(QString::null, false);

	for (WorkSheet* s = sheets.first(); s != 0; s = sheets.next())
		((WorkSheet*) s)->setIsOnTop(s == ws);
}

void
Workspace::applyStyle()
{
	if (currentPage())
		((WorkSheet*) currentPage())->applyStyle();
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
