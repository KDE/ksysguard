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

#include <config.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>       

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <qtabbar.h>
#include <qmessagebox.h>
#include <qslider.h>
#include <qlabel.h>
#include <qlcdnumber.h>

#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ktablistbox.h>

#include "ktop.h"
#include "settings.h"
#include "cpu.h"
#include "memory.h"
#include "comm.h"
#include "ProcListPage.h"
#include "widgets.moc"

#define PROC_BASE     "/proc"
#define KDE_ICN_DIR   "/share/icons/mini"
#define KTOP_ICN_DIR  "/share/apps/ktop/pics"
#define INIT_PID      1
#define NONE         -1

//#define DEBUG_MODE    // uncomment to activate "printf lines"

static const char *sig[] = 
{
    "send SIGINT\t(ctrl-c)" ,
    "send SIGQUIT\t(core)" ,
    "send SIGTERM\t(term.)" ,
    "send SIGKILL\t(term.)" ,
    "send SIGUSR1\t(user1)" ,
    "send SIGUSR2\t(user2)" ,
};

/*
 * It creates the actual QTabDialog. We create it as a modeless dialog, 
 * using the toplevel widget as its parent, so the dialog won't get 
 * its own window
 */
TaskMan::TaskMan(QWidget *parent, const char *name, int sfolder)
	: QTabDialog(parent, name, FALSE, 0)
{
	QString tmp;

	pages[0] = NULL;
	pages[1] = NULL;
	pages[2] = NULL;
	settings = NULL;
	restoreStartupPage = FALSE;

	setStyle(WindowsStyle);
    
	connect(tabBar(), SIGNAL(selected(int)), SLOT(tabBarSelected(int)));
     
	/*
	 * set up popup menu pSig
	 */
	pSig = new QPopupMenu(NULL,"_psig");
	CHECK_PTR(pSig);
	pSig->insertItem(i18n("Renice Task..."),MENU_ID_RENICE);
	pSig->insertSeparator();
	pSig->insertItem(i18n(sig[0]), MENU_ID_SIGINT);
	pSig->insertItem(i18n(sig[1]), MENU_ID_SIGQUIT);
	pSig->insertItem(i18n(sig[2]), MENU_ID_SIGTERM);
	pSig->insertItem(i18n(sig[3]), MENU_ID_SIGKILL);
	pSig->insertSeparator();
	pSig->insertItem(i18n(sig[4]), MENU_ID_SIGUSR1);
	pSig->insertItem(i18n(sig[5]), MENU_ID_SIGUSR2);
	connect(pSig, SIGNAL(activated(int)), this, SLOT(pSigHandler(int)));
  
    /*
     * set up page 0 (process list viewer)
     */
    pages[0] = procListPage = new ProcListPage(this, "ProcListPage");
    CHECK_PTR(procListPage);
	
	/*
	 * set up page 1 (process tree)
	 */
    pages[1] = procTreePage = new ProcTreePage(this, "ProcTreePage"); 
    CHECK_PTR(procTreePage);
    
    /*----------------------------------------------
     set up page 2 (This is the performance monitor)
     ----------------------------------------------*/
    pages[2] = new QWidget(this, "page2");
    CHECK_PTR(pages[2]); 

    cpubox = new QGroupBox(pages[2], "_cpumon");
    CHECK_PTR(cpubox); 
    cpubox->setTitle(i18n("CPU load"));
    cpubox1 = new QGroupBox(pages[2], "_cpumon1");
    CHECK_PTR(cpubox1); 
    cpubox1->setTitle(i18n("CPU load history"));
    cpu_cur = new QWidget(pages[2], "cpu_child");
    CHECK_PTR(cpu_cur); 
    cpu_cur->setBackgroundColor(black);
    cpumon = new CpuMon (cpubox1, "cpumon", cpu_cur);
    CHECK_PTR(cpumon);

    membox = new QGroupBox(pages[2], "_memmon");
    CHECK_PTR(membox);
    membox->setTitle(i18n("Memory"));
    membox1 = new QGroupBox(pages[2], "_memhistory");
    CHECK_PTR(membox1);
    membox1->setTitle(i18n("Memory usage history"));
    mem_cur = new QWidget(pages[2], "mem_child");
    CHECK_PTR(mem_cur);
    mem_cur->setBackgroundColor(black);
    memmon = new MemMon (membox1, "memmon", mem_cur);

    #ifdef ADD_SWAPMON
    	swapbox = new QGroupBox(pages[2], "_swapmon");
    	CHECK_PTR(swapbox);
    	swapbox->setTitle(i18n("Swap"));
    	swapbox1 = new QGroupBox(pages[2], "_swaphistory");
    	CHECK_PTR(swapbox1);
    	swapbox1->setTitle(i18n("Swap history"));
    	swap_cur = new QWidget(pages[2],"swap_child");
    	CHECK_PTR(swap_cur);
    	swap_cur->setBackgroundColor(black);
    	swapmon = new SwapMon (swapbox1, "swapmon", swap_cur);
    #endif

    /*----------------------------------------------
     settings
     ----------------------------------------------*/
    strcpy(cfgkey_startUpPage, "startUpPage");

    // startup_page settings...
	startup_page = PAGE_PLIST;
	tmp = Kapp->getConfig()->readEntry(QString(cfgkey_startUpPage));
	if(!tmp.isNull())
	{
		startup_page = tmp.toInt();
#ifdef DEBUG_MODE
		printf("KTop debug : TaskMan::TaskMan : startup_page (config val)"
			   "== %d.\n", startup_page);
#endif
    }  

	if (sfolder >= 0)
	{ 
		restoreStartupPage = TRUE;
		startup_page = sfolder;
#ifdef DEBUG_MODE
		printf("KTop debug : TaskMan::TaskMan : startup_page (cmd line val)"
			   "== %d.\n", startup_page);
#endif
	}

    installEventFilter(this);

    // add pages...
    addTab(procListPage, "Processes &List");
    addTab(procTreePage, "Processes &Tree");
    addTab(pages[2], "&Performance");
    move(0,0);
#ifdef DEBUG_MODE
	printf("KTOP debug: Selected start tab: %d\n", startup_page);
#endif
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::raiseStartUpPage
 -----------------------------------------------------------------------------*/
void TaskMan::raiseStartUpPage()
{ 
#ifdef DEBUG_MODE
		printf("KTop debug: TaskMan::raiseStartUpPage : startup_page "
			   "== %d.\n", startup_page);
#endif

	QString tmp;

	showPage(pages[startup_page]);

	/*
	 * In case the startup_page has been forced on the command line we restore
	 * the startup_page variable form the config file again so we use the
	 * forced value only for this session.
	 */
	if (restoreStartupPage)
	{
		tmp = Kapp->getConfig()->readEntry(QString(cfgkey_startUpPage));
		if( ! tmp.isNull() )
			startup_page = tmp.toInt();
	}
} 

void 
TaskMan::resizeEvent(QResizeEvent *ev)
{
	QTabDialog::resizeEvent(ev);

	if(!procListPage || !procTreePage || !pages[2])
		return;

    int w = pages[2]->width();
    int h = pages[2]->height();
   
    // performances page

    #ifdef ADD_SWAPMON
	cpubox->setGeometry (  10,       10,      80,    (h-40)/3);
     	cpubox1->setGeometry( 100,       10, w - 110,    (h-40)/3);
     	cpu_cur->setGeometry(  20,    10+18,      60,   (h-118)/3);
     	cpumon->setGeometry(   10,       18, w - 130,   (h-118)/3);
     	membox->setGeometry (  10,   (h+20)/3,      80,    (h-40)/3);
     	membox1->setGeometry( 100,   (h+20)/3, w - 110,    (h-40)/3);
     	mem_cur->setGeometry(  20,(h+20)/3+18,      60,   (h-118)/3);
     	memmon->setGeometry (  10,         18, w - 130,   (h-118)/3);
     	swapbox->setGeometry ( 10,   (2*h+10)/3,      80,    (h-40)/3);
     	swapbox1->setGeometry(100,   (2*h+10)/3, w - 110,    (h-40)/3);
     	swap_cur->setGeometry( 20,(2*h+10)/3+18,      60,   (h-118)/3);
     	swapmon->setGeometry ( 10,           18, w - 130,   (h-118)/3);
   #else
	cpubox->setGeometry(10, 10, 80, (h / 2) - 30);
    	cpubox1->setGeometry(100, 10, w - 110, (h / 2) - 30);
    	cpu_cur->setGeometry(20, 30, 60, (h / 2) - 60);
    	cpumon->setGeometry(10, 20, cpubox1->width() - 20, cpubox1->height() - 30);
    	membox->setGeometry(10, h / 2, 80, (h / 2) - 30);
    	membox1->setGeometry(100, h / 2, w - 110, (h / 2) - 30);
    	mem_cur->setGeometry(20, h / 2 + 20, 60, (h / 2) - 60);
    	memmon->setGeometry(10, 20, membox1->width() - 20, membox1->height() - 30);
   #endif
}
             
void 
TaskMan::pSigHandler( int id )
{
  int the_sig=0;
  int renice=0;

  switch ( id ) {
	case MENU_ID_SIGINT:
          the_sig = SIGINT;
          break;
	case MENU_ID_SIGQUIT:
          the_sig = SIGQUIT;
          break;
	case MENU_ID_SIGTERM:
          the_sig = SIGTERM;
          break;
	case MENU_ID_SIGKILL:
          the_sig = SIGKILL;
          break;
	case MENU_ID_SIGUSR1:
	  the_sig = SIGUSR1;
          break;
	case MENU_ID_SIGUSR2:
	  the_sig = SIGUSR2;
          break;
	case MENU_ID_RENICE:
	  renice = 1;
	  break;
    	default:
          return;
          break;
  }

  int err=0;
  int selection;
  switch ( tabBar()->currentTab() ) {
     case PAGE_PLIST:
      selection = procListPage->selectionPid();
      if ( selection == NONE ) return;
      break;
     case PAGE_PTREE:
      selection = procTreePage->selectionPid();
      if ( selection == NONE ) return;
      break;
     default:
      return;
      break;
  }

  int lastmode = procListPage->setAutoUpdateMode(FALSE);

  if ( renice == 0 ) 
  { 
	err = kill(selection,the_sig);
  	if ( err == -1 ) 
	{
		QMessageBox::warning(this,"ktop",
       		"Kill error...\nSpecified process does not exists\nor permission denied.",
       		"Ok", 0);
	}
   } 
   else 
   {
	int i;   
        int pprio;
       
        errno=0; // -1 is a possible value for getpriority return value
 	pprio = getpriority (PRIO_PROCESS,selection);
	if ( err == -1 ) 
	{
		QMessageBox::warning(this,"ktop",
		"Renice error...\nSpecified process does not exists\nor permission denied.",
		"Ok", 0); 
	  	procListPage->update();
		procTreePage->update();
		return;
	}

	SetNice m(this,"nice",pprio);
	if ( (i=m.exec())<=20 && (i>=-20) && (i!=pprio) ) 
	{
		err = setpriority (PRIO_PROCESS,selection,i);
		if ( err == -1 ) 
   		{
			QMessageBox::warning(this,"ktop",
  			"Renice error...\nSpecified process does not exists\nor permission denied.",
			"Ok", 0);   
		}
	}
   }
        
   switch ( tabBar()->currentTab() ) 
   {
     case PAGE_PLIST:
      if ( err != -1 ) procListPage->update();
      break;
     case PAGE_PTREE:
      if ( err != -1 ) procTreePage->update();
      break;
     default:
      return;
      break;
   }

   procListPage->setAutoUpdateMode(lastmode);

}

void 
TaskMan::tabBarSelected (int tabIndx)
{ 
#ifdef DEBUG_MODE
	printf("KTop debug: TaskMan::tabBarSelected : indx == %d.\n", tabIndx);
#endif

	switch ( tabIndx )
	{
	case PAGE_PLIST :
		procListPage->setAutoUpdateMode(TRUE);
		procListPage->update();
		break;
	case PAGE_PTREE :
		procListPage->setAutoUpdateMode(FALSE);
		procTreePage->update();
		break;
	case PAGE_PERF  :
		procListPage->setAutoUpdateMode(FALSE);
		break;
	default:
		break;
	}
}

void 
TaskMan::invokeSettings(void)
{
#ifdef DEBUG_MODE
	printf("KTop debug: TaskMan::invokeSettings : startup_page == %d.\n",
		   startup_page);
#endif
    if( ! settings ) {
        settings = new AppSettings(0,"proc_options");
        CHECK_PTR(settings);      
    }

    settings->setStartUpPage(startup_page);
    if( settings->exec() ) {
        startup_page = settings->getStartUpPage();
#ifdef DEBUG_MODE
		printf("KTop debug : TaskMan::invokeSettings : startup_page (new val) = %d.\n"
			   ,startup_page);
#endif
        saveSettings();
    }
}

void 
TaskMan::saveSettings()
{
	QString t;
	char temp[32];
	char *g_format = "%04d:%04d:%04d:%04d";

	/* save window size */
	sprintf(temp, g_format,
			parentWidget()->x(), parentWidget()->y(),
			parentWidget()->width() , parentWidget()->height());
	Kapp->getConfig()->writeEntry(QString("G_Toplevel"), QString(temp));

	/* save startup page (tab) */
	Kapp->getConfig()->writeEntry(QString(cfgkey_startUpPage),
					   t.setNum(startup_page), TRUE);

	procListPage->saveSettings();

	procTreePage->saveSettings();

	Kapp->getConfig()->sync();
}

SetNice::SetNice( QWidget *parent, const char *name , int currentPPrio )
       : QDialog( parent, name, TRUE )
{
	QPushButton *ok, *cancel;
	QSlider *priority;
	QLabel *label0;
	QLCDNumber *lcd0;
	label0 = new QLabel("Please enter desired priority:", this);
	label0->setGeometry( 10, 10 ,210, 15);
	priority = new QSlider( -20, 20, 1, 0, QSlider::Horizontal, this, "prio" );
	priority->setGeometry( 10,35, 210, 25 );
    	priority->setTickmarks((QSlider::TickSetting)2);
    	priority->setFocusPolicy( QWidget::TabFocus );
    	priority->setFixedHeight(priority->sizeHint().height());
	lcd0= new QLCDNumber(3,this,"lcd");
	lcd0->setGeometry( 80, 65 , 70 , 30);
	QObject::connect( priority, SIGNAL(valueChanged(int)),lcd0,  SLOT(display(int)) );
	QObject::connect( priority, SIGNAL(valueChanged(int)), SLOT(setPriorityValue(int)) );
        ok = new QPushButton( "Ok", this );
        ok->setGeometry( 10,110, 100,30 );
        connect( ok, SIGNAL(clicked()), SLOT(ok()));
        cancel = new QPushButton( "Cancel", this );
        cancel->setGeometry( 120,110, 100,30 );
        connect( cancel, SIGNAL(clicked()), SLOT(cancel()));
	value=currentPPrio;
	priority->setValue(value);
	lcd0->display(value);
}

void 
SetNice::setPriorityValue( int i )
{
	value=i;
}

void 
SetNice::ok()
{
	done(value);
}

void 
SetNice::cancel()
{
	done(40);
}
