/*
    KTop, the KDE Task Manager
   
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

#include <config.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <errno.h>
#include <signal.h>

#include <qmessagebox.h>

#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>

#include "ktop.h"
#include "ProcListPage.moc"

#define NONE -1

ProcListPage::ProcListPage(QWidget* parent = 0, const char* name = 0)
	: QWidget(parent, name)
{
	// Create the box that will contain the other widgets.
	box = new QGroupBox(this, "pList_box"); 
	box->setTitle(i18n("Running Processes"));
	CHECK_PTR(box);
	box->move(5, 5);
	box->resize(380, 380);

	// Create the table that lists the processes.
	pList = new ProcessList(this, "pList");    
	CHECK_PTR(pList);
	connect(pList, SIGNAL(popupMenu(int, int)),
			SLOT(popupMenu(int, int)));
	connect(pList, SIGNAL(popupMenu(int, int)),
			parent, SLOT(popupMenu(int, int)));
	pList->move(10, 30);
	pList->resize(370, 300);

	// Create a combo box to configure the refresh rate.
	cbRefresh = new QComboBox(this, "pList_cbRefresh");
	CHECK_PTR(cbRefresh);
	cbRefresh->insertItem(i18n("Refresh rate: Slow"),
						  (ProcessList::UPDATE_SLOW));
	cbRefresh->insertItem(i18n("Refresh rate: Medium"),
						  (ProcessList::UPDATE_MEDIUM));
	cbRefresh->insertItem(i18n("Refresh rate: Fast"),
						  (ProcessList::UPDATE_FAST));
	cbRefresh->setCurrentItem(pList->getUpdateRate());
	connect(cbRefresh, SIGNAL(activated(int)),
			SLOT(cbRefreshActivated(int)));

	// Create the combo box to configure the process filter.
	cbFilter = new QComboBox(this,"pList_cbFilter");
	CHECK_PTR(cbFilter);
	cbFilter->insertItem(i18n("All processes"), -1);
	cbFilter->insertItem(i18n("System processes"), -1);
	cbFilter->insertItem(i18n("User processes"), -1);
	cbFilter->insertItem(i18n("Own processes"), -1);
	cbFilter->setCurrentItem(pList->getFilterMode());
	connect(cbFilter, SIGNAL(activated(int)),
			SLOT(cbProcessFilter(int)));

	// Create the 'Refresh Now' button.
	bRefresh = new QPushButton(i18n("Refresh Now"), this, "pList_bRefresh");
	CHECK_PTR(bRefresh);
	connect(bRefresh, SIGNAL(clicked()), this, SLOT(update()));

	// Create the 'Kill task' button.
	bKill = new QPushButton(i18n("Kill task"), this, "pList_bKill");
	CHECK_PTR(bKill);
	connect(bKill,SIGNAL(clicked()), this, SLOT(killTask()));

	// restore refresh rate settings...
	refreshRate = ProcessList::UPDATE_MEDIUM;
	loadSetting(refreshRate, "pListUpdate");
	cbRefresh->setCurrentItem(refreshRate);
	pList->setUpdateRate(refreshRate);

	// restore process filter settings...
	int filter = pList->getFilterMode();
	loadSetting(filter, "pListFilter");
	cbFilter->setCurrentItem(filter);
	pList->setFilterMode(filter);

	// restore sort method for pList...
	int sortby = pList->getSortColumn();
	loadSetting(sortby, "pListSort");
	pList->setSortColumn(sortby);

	// create process list
    pList->update();
}

void
ProcListPage::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    int w = width();
    int h = height();
   
	box->setGeometry(5, 5, w - 10, h - 20);

	pList->setGeometry(10,25, w - 20, h - 75);

	cbRefresh->setGeometry(10, h - 45, 165, 25);
	cbFilter->setGeometry(185, h - 45, 140, 25);
	bRefresh->setGeometry(w - 200, h - 45, 90, 25);
	bKill->setGeometry(w - 100, h - 45, 90, 25);
}

void
ProcListPage::saveSettings(void)
{
	QString t;

	/* save refresh rate */
	Kapp->getConfig()->writeEntry("pListUpdate", t.setNum(refreshRate), TRUE);

	/* save filter mode */
	Kapp->getConfig()->writeEntry("pListFilter",
								  t.setNum(pList->getFilterMode()), TRUE);

	/* save sort criterium */
	Kapp->getConfig()->writeEntry("pListSort",
								  t.setNum(pList->getSortColumn()), TRUE);
}

void 
ProcListPage::cbRefreshActivated(int indx)
{ 
	refreshRate = indx;
	pList->setUpdateRate(indx);
}

void 
ProcListPage::cbProcessFilter(int indx)
{
	pList->setFilterMode(indx);
	pList->update();
}

void 
ProcListPage::popupMenu(int row,int)
{ 
	pList->setCurrentItem(row);
} 

void 
ProcListPage::killTask()
{
	int pid = selectionPid();
	if (pid < 0)
	{
		QMessageBox::warning(this, i18n("Task Manager"),
							 i18n("You need to select a process before\n"
								  "pressing the kill button!\n"),
								  i18n("OK"), 0);   
		return;
	}

	emit(killProcess(pid));

	pList->update();
}

