/*
    KTop, the KDE Task Manager
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
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
*/

// $Id$

#include <kapp.h>

#include "ktop.h"
#include <klocale.h>
#include "ProcListPage.moc"

#define NONE -1

ProcListPage::ProcListPage(QWidget* parent, const char* name)
	: QWidget(parent, name)
{
	// Create the box that will contain the other widgets.
	box = new QGroupBox(this, "pList_box"); 
	box->setTitle(i18n("Running Processes"));
	CHECK_PTR(box);

	// Create the table that lists the processes.
	pList = new ProcessList(this, "pList");    
	CHECK_PTR(pList);

	treeViewCB = new QCheckBox("Show Tree", this, "TreeViewCB");
	CHECK_PTR(treeViewCB);
	treeViewCB->setMinimumSize(treeViewCB->sizeHint());

	/*
	 * Create the combo box to configure the process filter. The
	 * cbFilter must be created prior to constructing pList as the
	 * pList constructor sets cbFilter to its start value.
	 */
	cbFilter = new QComboBox(this, "pList_cbFilter");
	CHECK_PTR(cbFilter);
	cbFilter->insertItem(i18n("All processes"), 0);
	cbFilter->insertItem(i18n("System processes"), 1);
	cbFilter->insertItem(i18n("User processes"), 2);
	cbFilter->insertItem(i18n("Own processes"), 3);
	cbFilter->setMinimumSize(cbFilter->sizeHint());

	/*
	 * When the both cbFilter and pList are constructed we can connect the
	 * missing link.
	 */
	connect(cbFilter, SIGNAL(activated(int)),
			pList, SLOT(setFilterMode(int)));
	// Same for treeViewCB and pList, but bi-directional
	connect(treeViewCB, SIGNAL(toggled(bool)),
			pList, SLOT(setTreeView(bool)));
	connect(pList, SIGNAL(treeViewChanged(bool)),
			this, SLOT(treeViewChanged(bool)));

	// Create the 'Refresh Now' button.
	bRefresh = new QPushButton(i18n("Refresh Now"), this, "pList_bRefresh");
	CHECK_PTR(bRefresh);
	bRefresh->setMinimumSize(bRefresh->sizeHint());
	connect(bRefresh, SIGNAL(clicked()), pList, SLOT(update()));

	// Create the 'Kill task' button.
	bKill = new QPushButton(i18n("Kill task"), this, "pList_bKill");
	CHECK_PTR(bKill);
	bKill->setMinimumSize(bKill->sizeHint());
	connect(bKill,SIGNAL(clicked()), pList, SLOT(killProcess()));

	// Setup the geometry management.
	gm = new QVBoxLayout(this, 10);
	gm->addSpacing(15);
	gm->addWidget(pList, 1);

	gm1 = new QHBoxLayout();
	gm->addLayout(gm1, 0);
	gm1->addStretch();
	gm1->addWidget(treeViewCB);
	gm1->addStretch();
	gm1->addWidget(cbFilter);
	gm1->addStretch();
	gm1->addWidget(bRefresh);
	gm1->addStretch();
	gm1->addWidget(bKill);
	gm1->addStretch();
	gm->addSpacing(5);

	gm->activate();

	setMinimumSize(sizeHint());

	pList->loadSettings();

	// create process list
    pList->update();
	printf("ProcListPage: %d, %d\n", sizeHint().width(), sizeHint().height());
}

void
ProcListPage::resizeEvent(QResizeEvent* ev)
{
	box->setGeometry(5, 5, width() - 10, height() - 10);

    QWidget::resizeEvent(ev);
}
