/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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

/*=============================================================================
  HEADERs
 =============================================================================*/
#include <qpopmenu.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qpushbt.h>
#include <qdialog.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qobject.h>
#include <qlistbox.h>
#include <qgrpbox.h>
#include <qevent.h>
#include <qcombo.h>
#include <qlined.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qtabdlg.h>
#include <qtooltip.h>
#include <qmsgbox.h>
#include <qpalette.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>

#include "settings.h"
#include "cpu.h"
#include "memory.h"
#include "widgets.h"
#include "ktop.moc"
#include "version.h"

/*=============================================================================
  #DEFINEs
 =============================================================================*/
#define ktr klocale->translate
//-----------------------------------------------------------------------------
//#define DEBUG_MODE    // uncomment to active "printf lines"
//-----------------------------------------------------------------------------

/*=============================================================================
  GLOBALs
 =============================================================================*/
KConfig       *config;
KApplication  *mykapp;

#ifdef __FreeBSD__
#include <kvm.h>
#include <fcntl.h>
#include <limits.h>
kvm_t         *kvm;
#endif

/*=============================================================================
 Class : TopLevel (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : TopLevel::TopLevel (constructor)
 -----------------------------------------------------------------------------*/
TopLevel::TopLevel( QWidget *parent, const char *name, int sfolder )
         :QWidget( parent, name )
{
    QString t;
    
    setCaption(i18n("KDE Taskmanager"));
    setMinimumSize(515,468);

    taskman = new TaskMan(this, "", sfolder);
    connect(taskman,SIGNAL(applyButtonPressed()),this,SLOT(quitSlot()));

    file = new QPopupMenu();
    file->insertItem(i18n("&Quit"), MENU_ID_QUIT, -1);
    connect(file, SIGNAL(activated(int)), this, SLOT(menuHandler(int)));

    QString about;
    about.sprintf(i18n(
      "KDE Taskmanager Version %s\n\n"
      "Copyright:\n"
      "1996 A. Sanda <alex@darkstar.ping.at>\n"
      "1997 Ralf Mueller <ralf@bj-ig.de>\n"
      "1997-98 Bernd Johannes Wuebben  <wuebben@kde.org>\n"
      "1998 Nicolas Leclercq <nicknet@planete.net>"), KTOP_VERSION);
    help = kapp->getHelpMenu(TRUE, about);

    settings = new QPopupMenu();
    settings->insertItem(i18n("StartUp Preferences...")
			                        ,MENU_ID_PROCSETTINGS, -1);
    connect(settings, SIGNAL(activated(int)), this, SLOT(menuHandler(int)));

    menubar = new QMenuBar(this, "menubar");
    menubar->setLineWidth(1);
    menubar->insertItem(i18n("&File"), file, 2, -1);
    menubar->insertItem(i18n("&Options"), settings, 3, -1);
    menubar->insertSeparator(-1);
    menubar->insertItem(i18n("&Help"), help, 2, -1);

    t = config->readEntry(QString("G_Toplevel"));
    if( ! t.isNull() ) {
        if ( t.length() == 19 ) { 
            int xpos,ypos,ww,wh;
            sscanf(t.data(),"%04d:%04d:%04d:%04d",&xpos,&ypos,&ww,&wh);
            #ifdef DEBUG_MODE
              printf("KTop debug : %04d:%04d:%04d:%04d\n",xpos,ypos,ww,wh);
            #endif
            setGeometry(xpos,ypos,ww,wh);
        }
    }

    adjustSize();
    show();
    taskman->raiseStartUpPage();
}

/*-----------------------------------------------------------------------------
  Routine : TopLevel::~TopLevel (desstructor)
 -----------------------------------------------------------------------------*/
TopLevel::~TopLevel()
{
}

/*-----------------------------------------------------------------------------
  Routine : TopLevel::closeEvent
 -----------------------------------------------------------------------------*/
void  TopLevel::closeEvent( QCloseEvent * )
{
  quitSlot();
}

/*-----------------------------------------------------------------------------
  Routine : TopLevel::quitSlot()
 -----------------------------------------------------------------------------*/
void TopLevel::quitSlot()
{
  taskman->saveSettings();
  config->sync();
  qApp->quit();
}

/*-----------------------------------------------------------------------------
  Routine : TopLevel::resizeEvent
 -----------------------------------------------------------------------------*/
void TopLevel::resizeEvent( QResizeEvent * )
{
    taskman->setGeometry(0, menubar->height() + 2, width(), 
			    height() - menubar->height() - 5);
}

/*-----------------------------------------------------------------------------
  Routine : TopLevel::menuHandler
 -----------------------------------------------------------------------------*/
void TopLevel::menuHandler(int id)
{
  switch(id) {

    case MENU_ID_QUIT:
      quitSlot();
      break;

    case MENU_ID_PROCSETTINGS:
        taskman->invokeSettings();
        break;
        
    default:
        break;
  }
}

/*-----------------------------------------------------------------------------
  Routine : usage
 -----------------------------------------------------------------------------*/
static void usage(char *name) 
{
  printf("%s [kdeopts] [--help] [-p (list|tree|perf)]\n", name);
}

/*-----------------------------------------------------------------------------
  Routine : main
 -----------------------------------------------------------------------------*/
int main( int argc, char ** argv )
{
    int i, 
    sfolder=-1;

#ifdef __FreeBSD__          // obtain kvm handle for reading kernel data
    kvm = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open");
    // we don't have to be sgid anymore
    setgid(getgid());
#endif
    for(i=1; i<argc; i++) {
        if( ! strcmp(argv[i],"--help") ) {
	    usage(argv[0]);
	    return 0;
	}
	if( strstr(argv[i],"-p") ) {
	    i++;
	           if ( strstr(argv[i],"perf")   ) {
	                sfolder=TaskMan::PAGE_PERF;
	    } else if ( strstr(argv[i],"list") ) {
	                sfolder=TaskMan::PAGE_PLIST;
	    } else if ( strstr(argv[i],"tree") ) {
	                sfolder=TaskMan::PAGE_PTREE;
	    } else {
	        usage(argv[0]);
		return 1;
	    }
	}
    }

    KApplication a( argc, argv, "ktop" );
#ifdef __FreeBSD__
    if (kvm == NULL) {
	QMessageBox::critical(NULL, NULL,
		i18n("Could not open kernel memory file. "
		"Make sure your ktop binary is installed sgid kmem.\n\n"
		"chgrp kmem KDEDIR/bin/ktop\n"
		"chmod 2555 KDEDIR/bin/ktop"),
			      i18n("&OK"));
	
	exit(1);
    }
#endif
    a.enableSessionManagement(true);
    mykapp = &a;
    config = a.getConfig();

    TaskMan::TaskMan_initIconList();

    QWidget *toplevel = new TopLevel(0,"Taskmanager", sfolder);
    a.setTopWidget( toplevel );
 
    int result = a.exec();

    TaskMan::TaskMan_clearIconList();
    

    return result;
}















