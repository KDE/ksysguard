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

#define NONE -1

//#define DEBUG_MODE    // uncomment to activate "printf lines"

/*
 * This constructor creates the actual QTabDialog. It is a modeless dialog,
 * using the toplevel widget as its parent, so the dialog won't get its own
 * window.
 */
TaskMan::TaskMan(QWidget* parent, const char* name, int sfolder)
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
	pSig->insertItem(i18n("send SIGINT\t(ctrl-c)"), MENU_ID_SIGINT);
	pSig->insertItem(i18n("send SIGQUIT\t(core)"), MENU_ID_SIGQUIT);
	pSig->insertItem(i18n("send SIGTERM\t(term.)"), MENU_ID_SIGTERM);
	pSig->insertItem(i18n("send SIGKILL\t(term.)"), MENU_ID_SIGKILL);
	pSig->insertSeparator();
	pSig->insertItem(i18n("send SIGUSR1\t(user1)"), MENU_ID_SIGUSR1);
	pSig->insertItem(i18n("send SIGUSR2\t(user2)"), MENU_ID_SIGUSR2);
	connect(pSig, SIGNAL(activated(int)), this, SLOT(pSigHandler(int)));
  
    /*
     * set up page 0 (process list viewer)
     */
    pages[0] = procListPage = new ProcListPage(this, "ProcListPage");
    CHECK_PTR(procListPage);
	connect(procListPage, SIGNAL(killProcess(int)),
			this, SLOT(killProcess(int)));
	
	/*
	 * set up page 1 (process tree)
	 */
    pages[1] = procTreePage = new ProcTreePage(this, "ProcTreePage"); 
    CHECK_PTR(procTreePage);
	connect(procTreePage, SIGNAL(killProcess(int)),
			this, SLOT(killProcess(int)));

	/*
	 * set up page 2 (performance monitor)
	 */
    pages[2] = perfMonPage = new PerfMonPage(this, "PerfMonPage");
    CHECK_PTR(perfMonPage);

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
    addTab(procListPage, i18n("Processes &List"));
    addTab(procTreePage, i18n("Processes &Tree"));
    addTab(perfMonPage, i18n("&Performance"));
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
		if(!tmp.isNull())
			startup_page = tmp.toInt();
	}
} 

void 
TaskMan::resizeEvent(QResizeEvent *ev)
{
	QTabDialog::resizeEvent(ev);

	if(!procListPage || !procTreePage || !perfMonPage)
		return;
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
		QMessageBox::warning(this,i18n("ktop"),
       		i18n("Kill error...\nSpecified process does not exist\nor permission denied."),
       		i18n("OK"), 0);
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
		QMessageBox::warning(this,i18n("ktop"),
		i18n("Renice error...\nSpecified process does not exist\nor permission denied."),
		i18n("OK"), 0); 
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
			QMessageBox::warning(this,i18n("ktop"),
  			i18n("Renice error...\nSpecified process does not exist\nor permission denied."),
			i18n("OK"), 0);   
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
TaskMan::killProcess(int pid)
{
	OSProcessList pl;
	pl.update();
	OSProcess* ps;
	for (ps = pl.first(); ps && ps->getPid() != pid; ps = pl.next())
		;

	if (!ps)
		return;

	QString msg;
	msg.sprintf(i18n("Kill process %d (%s - %s) ?\n"), ps->getPid(),
				ps->getName(), ps->getUserName().data());

	int err;
	switch(QMessageBox::warning(this, "ktop", msg,
								i18n("Continue"), i18n("Abort"), 0, 1))
    { 
	case 0: // continue
		err = kill(ps->getPid(), SIGKILL);
		if (err)
		{
			QMessageBox::warning(this, "ktop",
								 i18n("Kill error !\n"
								 "The following error occured...\n"),
								 strerror(errno), 0);   
		}
		break;

	case 1: // abort
		break;
	}
}

void 
TaskMan::invokeSettings(void)
{
	if(!settings)
	{
		settings = new AppSettings(0,"proc_options");
		CHECK_PTR(settings);      
	}

	settings->setStartUpPage(startup_page);
	if (settings->exec())
	{
		startup_page = settings->getStartUpPage();
		saveSettings();
	}
}

void 
TaskMan::saveSettings()
{
	QString tmp;

	// save window size
	tmp.sprintf("%04d:%04d:%04d:%04d",
				parentWidget()->x(), parentWidget()->y(),
				parentWidget()->width(), parentWidget()->height());
	Kapp->getConfig()->writeEntry(QString("G_Toplevel"), tmp);

	// save startup page (tab)
	Kapp->getConfig()->writeEntry(QString(cfgkey_startUpPage),
					   tmp.setNum(startup_page), TRUE);

	// save process list settings
	procListPage->saveSettings();

	// save process tree settings
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
	label0 = new QLabel(i18n("Please enter desired priority:"), this);
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
        ok = new QPushButton( i18n("OK"), this );
        ok->setGeometry( 10,110, 100,30 );
        connect( ok, SIGNAL(clicked()), SLOT(ok()));
        cancel = new QPushButton( i18n("Cancel"), this );
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
