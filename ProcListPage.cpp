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

#include <sys/types.h>
#include <signal.h>

#include <qmessagebox.h>

#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>

#include "ktop.h"
#include "ProcListPage.moc"

#define NONE -1

static const char *rateTxt[] =
{
    "Refresh rate: Slow", 
    "Refresh rate: Medium", 
    "Refresh rate: Fast",
	0
};

static const char *processfilter[] =
{
	"All processes",
	"System processes",
	"User processes",
	"Own processes",
	0
};

ProcListPage::ProcListPage(QWidget* parent = 0, const char* name = 0)
	: QWidget(parent, name)
{
	// Create the box that will contain the other widgets.
    pList_box = new QGroupBox(this, "pList_box"); 
    CHECK_PTR(pList_box);
    pList_box->move(5, 5);
    pList_box->resize(380, 380);

	// Create the table that lists the processes.
    pList = new KtopProcList(this, "pList");    
    CHECK_PTR(pList);
    connect(pList, SIGNAL(popupMenu(int, int)),
			SLOT(pList_popupMenu(int, int)));
	connect(pList, SIGNAL(popupMenu(int, int)),
			parent, SLOT(popupMenu(int, int)));
    pList->move(10, 30);
    pList->resize(370, 300);

	// Create a combo box to configure the refresh rate.
    pList_cbRefresh = new QComboBox(this, "pList_cbRefresh");
    CHECK_PTR(pList_cbRefresh);
    for (int i = 0; rateTxt[i]; i++)
	{
		pList_cbRefresh->insertItem(klocale->translate(rateTxt[i]),
									i + (KtopProcList::UPDATE_SLOW));
    } 
    pList_cbRefresh->setCurrentItem(pList->updateRate());
    connect(pList_cbRefresh, SIGNAL(activated(int)),
			SLOT(pList_cbRefreshActivated(int)));

	// Create the combo box to configure the process filter.
    pList_cbFilter = new QComboBox(this,"pList_cbFilter");
    CHECK_PTR(pList_cbFilter);
    for (int i=0; processfilter[i]; i++)
	{
		pList_cbFilter->insertItem(klocale->translate(processfilter[i]), -1);
    }
    pList_cbFilter->setCurrentItem(pList->filterMode());
    connect(pList_cbFilter, SIGNAL(activated(int)),
			SLOT(pList_cbProcessFilter(int)));

	// Create the 'Refresh Now' button.
    pList_bRefresh = new QPushButton(i18n("Refresh Now"), this,
									 "pList_bRefresh");
    CHECK_PTR(pList_bRefresh);
    connect(pList_bRefresh, SIGNAL(clicked()), this, SLOT(pList_update()));

	// Create the 'Kill task' button.
    pList_bKill = new QPushButton(i18n("Kill task"), this, "pList_bKill");
    CHECK_PTR(pList_bKill);
    connect(pList_bKill,SIGNAL(clicked()), this, SLOT(pList_killTask()));
  
    pList_box->setTitle(i18n("Running processes"));

    strcpy(cfgkey_pListUpdate, "pListUpdate");
	strcpy(cfgkey_pListFilter, "pListFilter");
    strcpy(cfgkey_pListSort, "pListSort");

	// restore refresh rate settings...
	pList_refreshRate = KtopProcList::UPDATE_MEDIUM;
	QString tmp = Kapp->getConfig()->readEntry(QString(cfgkey_pListUpdate));
	if(!tmp.isNull())
	{
		bool res = FALSE;
		pList_refreshRate =tmp.toInt(&res);
		if (!res) 
			pList_refreshRate = KtopProcList::UPDATE_MEDIUM;
	}
	pList_cbRefresh->setCurrentItem(pList_refreshRate);
	pList->setUpdateRate(pList_refreshRate);

	// restore process filter settings...
	tmp = Kapp->getConfig()->readEntry(QString(cfgkey_pListFilter));
	if(!tmp.isNull())
	{
		bool res = FALSE;
		int filter = tmp.toInt(&res);
		if (res)
		{
			pList_cbFilter->setCurrentItem(filter);
			pList->setFilterMode(filter);
		}
	}
	pList->setUpdateRate(pList_refreshRate);

	// restore sort method for pList...
	pList_sortby=KtopProcList::SORTBY_CPU;
	tmp = Kapp->getConfig()->readEntry(QString(cfgkey_pListSort));
	if(!tmp.isNull())
	{
		bool res = FALSE;
		pList_sortby = tmp.toInt(&res);
		if (!res) pList_sortby=KtopProcList::SORTBY_CPU;
	}
	pList->setSortMethod(pList_sortby);

    pList->update(); /* create process list */
}

void
ProcListPage::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

    int w = width();
    int h = height();
   
	pList->setGeometry(10,25, w - 20, h - 75);

	pList_box->setGeometry(5, 5, w - 10, h - 20);
	pList_cbRefresh->setGeometry(10, h - 45, 150, 25);
	pList_cbFilter->setGeometry(170, h - 45, 150, 25);
	pList_bRefresh->setGeometry(w - 180, h - 45, 80, 25);
	pList_bKill->setGeometry(w - 90, h - 45, 80, 25);

}

void
ProcListPage::saveSettings(void)
{
	QString t;

	/* save refresh rate */
	Kapp->getConfig()->writeEntry(QString(cfgkey_pListUpdate),
					   t.setNum(pList_refreshRate), TRUE);

	/* save filter mode */
	Kapp->getConfig()->writeEntry(QString(cfgkey_pListFilter),
					   t.setNum(pList->filterMode()), TRUE);

	/* save sort criterium */
	Kapp->getConfig()->writeEntry(QString(cfgkey_pListSort),
					   t.setNum(pList_sortby), TRUE);
}

void 
ProcListPage::pList_cbRefreshActivated(int indx)
{ 
#ifdef DEBUG_MODE
	printf("KTop debug: TaskMan::pList_cbRefreshActivated:"
		   "item == %d.\n", indx);
#endif

	pList_refreshRate = indx;
	pList->setUpdateRate(indx);
}

void 
ProcListPage::pList_cbProcessFilter(int indx)
{
	pList->setFilterMode(indx);
	pList_update();
}

void 
ProcListPage::pList_popupMenu(int row,int)
{ 
  #ifdef DEBUG_MODE
    printf("KTop debug : TaskMan::pList_popupMenu: on item %d.\n",row);
  #endif

  pList->setCurrentItem(row);
} 

void 
ProcListPage::pList_killTask()
{
 char pname[64];
 char uname[64];
 int  pid;

 int cur = pList->currentItem();
 if ( cur == NONE ) return;

 sscanf((pList->text(cur,1)).data(),"%d",&pid);
 sscanf((pList->text(cur,2)).data(),"%s",pname);
 sscanf((pList->text(cur,3)).data(),"%s",uname); 

 #ifdef DEBUG_MODE
   printf("KTop debug : TaskMan::pList_killTask : selection pid   = %d\n",pid); 
   printf("KTop debug : TaskMan::pList_killTask : selection pname = %s\n",pname);
   printf("KTop debug : TaskMan::pList_killTask : selection uname = %s\n",uname);
 #endif

 if ( pList->selectionPid() != pid ) {
      QMessageBox::warning(this,"ktop",
                                "Selection changed !\n\n",
                                "Abort",0);
      return;
 }

 int  err = 0;
 char msg[256];
 sprintf(msg,"Kill process %d (%s - %s) ?\n",pid,pname,uname);

 switch( QMessageBox::warning(this,"ktop",
                                    msg,
                                   "Continue", "Abort",
                                    0, 1 )
       )
    { 
      case 0: // continue
          err = kill(pList->selectionPid(),SIGKILL);
          if ( err ) {
              QMessageBox::warning(this,"ktop",
              "Kill error !\nThe following error occured...\n",
              strerror(errno),0);   
          }
          pList->update();
          break;
      case 1: // abort
          break;
    }
}
