/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
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
*/

// $Id$

#include <signal.h>
#include <errno.h>

#include <qstring.h>
#include <qmessagebox.h>

#include "ProcTreePage.moc"

#define NONE -1

typedef struct
{
	const char* label;
	OSProcessList::SORTKEY sortMethod;
} SortButton;

static SortButton SortButtons[] =
{
	{ "Sort by ID", OSProcessList::SORTBY_PID },
	{ "Sort by Name", OSProcessList::SORTBY_NAME },
	{ "Sort by Owner (UID)", OSProcessList::SORTBY_UID }
};

static const int NoSortButtons = sizeof(SortButtons) / sizeof(SortButton);

ProcTreePage::ProcTreePage(QWidget* parent = 0, const char* name = 0)
	: QWidget(parent, name)
{
	// surrounding box with title "Running Processes"
	pTree_box = new QGroupBox(this, "pTree_box"); 
	CHECK_PTR(pTree_box); 
	pTree_box->setTitle(i18n("Running Processes"));

	// the tree handling widget
	pTree = new KtopProcTree(this, "pTree"); 
	CHECK_PTR(pTree); 
	pTree->setExpandLevel(20); 
	pTree->setSmoothScrolling(TRUE);
	connect(pTree, SIGNAL(popupMenu(QPoint)),
			parent, SLOT(popupMenu(QPoint)));

	/*
	 * four buttons which should appear on the sheet (just below the tree box)
	 */

	// "Refresh Now" button
	pTree_bRefresh = new QPushButton(i18n("Refresh Now"), this,
									 "pTree_bRefresh");
	CHECK_PTR(pTree_bRefresh);
	connect(pTree_bRefresh, SIGNAL(clicked()), this, SLOT(pTree_update()));

	// "Change Root" button
	pTree_bRoot = new QPushButton(i18n("Change Root"), this, "pTree_bRoot");
	CHECK_PTR(pTree_bRoot);
	connect(pTree_bRoot, SIGNAL(clicked()), this, SLOT(pTree_changeRoot()));

	// "Kill Task" button
	pTree_bKill = new QPushButton(i18n("Kill Task"), this, "pTree_bKill");
	CHECK_PTR(pTree_bKill);
	connect(pTree_bKill, SIGNAL(clicked()), this, SLOT(pTree_killTask()));

	// Sorting Method combo button
	pTree_cbSort = new QComboBox(this, "pTree_cbSort");
	CHECK_PTR(pTree_cbSort);
	for (int i = 0; i < NoSortButtons; i++)
		pTree_cbSort->insertItem(i18n(SortButtons[i].label));
	connect(pTree_cbSort, SIGNAL(activated(int)),
			SLOT(handleSortChange(int)));

	strcpy(cfgkey_pTreeSort, "pTreeSort");

	// restore sort method for pTree...
	pTree_sortby = 0;
	loadSetting(pTree_sortby, "pTreeSort");
	pTree->setSortMethod(SortButtons[pTree_sortby].sortMethod);
	pTree_cbSort->setCurrentItem(pTree_sortby);
    
	// create process tree
	pTree->update();
}

void
ProcTreePage::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    int w = width();
    int h = height();
   
	pTree_box->setGeometry(5, 5, w - 10, h - 20);

   	pTree->setGeometry(10, 30, w - 20, h - 90);

	pTree_cbSort->setGeometry(10, h - 50,140, 25);
	pTree_bRefresh->setGeometry(w - 270, h - 50, 80, 25);
	pTree_bRoot->setGeometry(w - 180, h - 50, 80, 25);
	pTree_bKill->setGeometry(w - 90, h - 50, 80, 25);
}

void
ProcTreePage::handleSortChange(int idx)
{
	pTree->setSortMethod(SortButtons[pTree_sortby = idx].sortMethod);
	pTree->update();
}

void 
ProcTreePage::pTree_killTask()
{
	int pid = selectionPid();
	if (pid < 0)
		return;

	emit(killProcess(pid));

	pTree->update();
}
