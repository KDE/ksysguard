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

#include <errno.h>
#include <signal.h>
#include <config.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

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
	pList = new KtopProcList(this, "pList");    
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
			      (KtopProcList::UPDATE_SLOW));
	cbRefresh->insertItem(i18n("Refresh rate: Medium"),
			      (KtopProcList::UPDATE_MEDIUM));
	cbRefresh->insertItem(i18n("Refresh rate: Fast"),
			      (KtopProcList::UPDATE_FAST));
	cbRefresh->setCurrentItem(pList->updateRate());
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
	bRefresh = new QPushButton(i18n("Refresh Now"), this,
									 "pList_bRefresh");
	CHECK_PTR(bRefresh);
	connect(bRefresh, SIGNAL(clicked()), this, SLOT(update()));

	// Create the 'Kill task' button.
	bKill = new QPushButton(i18n("Kill task"), this, "pList_bKill");
	CHECK_PTR(bKill);
	connect(bKill,SIGNAL(clicked()), this, SLOT(killTask()));

	strcpy(cfgkey_pListUpdate, "pListUpdate");
	strcpy(cfgkey_pListFilter, "pListFilter");
	strcpy(cfgkey_pListSort, "pListSort");

	// restore refresh rate settings...
	refreshRate = KtopProcList::UPDATE_MEDIUM;
	loadSetting(refreshRate, "pListUpdate");
	cbRefresh->setCurrentItem(refreshRate);
	pList->setUpdateRate(refreshRate);

	// restore process filter settings...
	int filter = pList->getFilterMode();
	loadSetting(filter, "pListFilter");
	cbFilter->setCurrentItem(filter);
	pList->setFilterMode(filter);

	// restore sort method for pList...
	int sortby = pList->getSortMethod();
	loadSetting(sortby, "pListSort");
	pList->setSortMethod(sortby);

	// create process list
    pList->update();
}

void
ProcListPage::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    int w = width();
    int h = height();
   
	pList->setGeometry(10,25, w - 20, h - 75);

	box->setGeometry(5, 5, w - 10, h - 20);
	cbRefresh->setGeometry(10, h - 45, 150, 25);
	cbFilter->setGeometry(170, h - 45, 150, 25);
	bRefresh->setGeometry(w - 180, h - 45, 80, 25);
	bKill->setGeometry(w - 90, h - 45, 80, 25);
}

void
ProcListPage::saveSettings(void)
{
	QString t;

	/* save refresh rate */
	Kapp->getConfig()->writeEntry(QString(cfgkey_pListUpdate),
					   t.setNum(refreshRate), TRUE);

	/* save filter mode */
	Kapp->getConfig()->writeEntry(QString(cfgkey_pListFilter),
					   t.setNum(pList->getFilterMode()), TRUE);

	/* save sort criterium */
	Kapp->getConfig()->writeEntry(QString(cfgkey_pListSort),
					   t.setNum(pList->getSortMethod()), TRUE);
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
	update();
}

void 
ProcListPage::popupMenu(int row,int)
{ 
	pList->setCurrentItem(row);
} 

void 
ProcListPage::killTask()
{
	int cur = pList->currentItem();
	if (cur == NONE)
		return;

	int pid = pList->text(cur, 1).toInt();
	QString pname = pList->text(cur, 2);
	QString uname = pList->text(cur, 3);

	if (pList->selectionPid() != pid)
	{
		QMessageBox::warning(this,i18n("ktop"),
							 i18n("Selection changed!\n\n"),
							 i18n("Abort"), 0);
		return;
	}

	QString msg;
	msg.sprintf(i18n("Kill process %d (%s - %s)?"), pid, &pname, &uname);

	int err = 0;
	switch (QMessageBox::warning(this, i18n("ktop"), msg, i18n("Continue"), i18n("Abort"), 0, 1))
	{ 
	case 0: // try to kill the process
		err = kill(pList->selectionPid(), SIGKILL);
		if (err)
		{
			QMessageBox::warning(this,i18n("ktop"),
								 i18n("Kill error !\n"
								 "The following error occured...\n"),
								 strerror(errno), 0);   
		}
		pList->update();
		break;

	case 1: // abort
		break;
	}
}
