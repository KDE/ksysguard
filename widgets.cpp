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

/* $Id$ */

/*=============================================================================
  HEADERs
 =============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __FreeBSD__
#include <signal.h>
#include <sys/syslimits.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#endif

#include <signal.h>
#include <qevent.h> 
#include <qpalette.h>
#include <qcombo.h>
#include <qpainter.h>
#include <qpushbt.h>
#include <qtabdlg.h>
#include <qtimer.h>
#include <qmsgbox.h>
#include <qlabel.h>
#include <qobject.h>
#include <qlistbox.h>
#include <qgrpbox.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qbttngrp.h>
#include <qpalette.h>
#include <qlined.h>
#include <qtabbar.h>
#include <qpopmenu.h>
#include <qfontmet.h>

#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ktablistbox.h>

#include "settings.h"
#include "cpu.h"
#include "memory.h"
#include "comm.h"
#include "ptree.h"
#include "widgets.moc"

/*=============================================================================
  #DEFINEs
 =============================================================================*/
#define ktr           klocale->translate
#ifndef __FreeBSD__
// Actually it would be muy nifty if we could use getmntent for this :^)
// BSD doesn't need this, so fail if something tries to use it..
#define PROC_BASE     "/proc"
#endif
#define KDE_ICN_DIR   "/share/icons/mini"
#define KTOP_ICN_DIR  "/share/apps/ktop/pics"
#define INIT_PID      1
#define NONE         -1

//-----------------------------------------------------------------------------
//#define DEBUG_MODE    // uncomment to active "printf lines"
//-----------------------------------------------------------------------------

/*=============================================================================
  GLOBALs
 =============================================================================*/
extern KConfig *config;

// exec.xpm = default Icon for processes tree.
// drawn  by Mark Donohoe for the K Desktop Environment 
static const char* execXpm[]={
"16 16 7 1",
"# c #000000",
"d c #008080",
"a c #beffff",
"c c #00c0c0",
"b c #585858",
"e c #00c0c0",
". c None",
"................",
".....######.....",
"..###abcaba###..",
".#cabcbaababaa#.",
".#cccaaccaacaa#.",
".###accaacca###.",
"#ccccca##aaccaa#",
"#dd#ecc##cca#cc#",
"#d#dccccccacc#c#",
"##adcccccccccd##",
".#ad#c#cc#d#cd#.",
".#a#ad#cc#dd#d#.",
".###ad#dd#dd###.",
"...#ad#cc#dd#...",
"...####cc####...",
"......####......"};

static QPixmap *defaultIcon;

static const char *refreshrates[] = {
    "Refresh rate : Slow", 
    "Refresh rate : Medium", 
    "Refresh rate : Fast",
     0
};

static const char *sortmethodsTree[] = {
    "Sort by ID",
    "Sort by Name",
    "Sort by Owner (UID)",
     0
};

static const char *sig[] = {
    "send SIGINT\t(ctrl-c)" ,
    "send SIGQUIT\t(core)" ,
    "send SIGTERM\t(term.)" ,
    "send SIGKILL\t(term.)" ,
    "send SIGUSR1\t(user1)" ,
    "send SIGUSR2\t(user2)" ,
};

#define NUM_COL 10
// I have to find a better method...
static const char *col_headers[] = {
     " "      ,
     "procID" ,
     "Name"   ,
     "userID" ,
     "CPU"    ,
     "Time"   ,
     "Status" ,
     "VmSize" ,
     "VmRss"  ,
#ifdef __FreeBSD__
     "Prior"   ,
#else
     "VmLib"  ,
#endif
     0
};

static const char *dummies[] = {
     "++++"              ,
     "procID++"          ,
     "kfontmanager++"    ,
     "rootuseroot"       ,
     "100.00%+"          ,
     "100:00++"          ,
     "Status+++"         ,
     "VmSize++"          ,
     "VmSize++"          ,
     "VmSize++"          ,
     0
};

static KTabListBox::ColumnType col_types[] = {
     KTabListBox::MixedColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn
};

/*=============================================================================
 Class : IconListElem (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : IconListElem::IconListElem (constructor)
 -----------------------------------------------------------------------------*/
IconListElem::IconListElem(const char* fName,const char* iName)
{
  QPixmap  new_xpm;

  pm = new QPixmap(fName);
  if ( pm && pm->isNull() ) {
       delete pm;
       pm = defaultIcon;
  }
  strcpy(icnName,iName);
}

/*-----------------------------------------------------------------------------
  Routine : IconListElem::~IconListElem (destructor)
 -----------------------------------------------------------------------------*/
IconListElem::~IconListElem()
{
  if ( pm ) delete pm;
}

/*-----------------------------------------------------------------------------
  Routine : IconListElem::getPixmap
 -----------------------------------------------------------------------------*/
const QPixmap* IconListElem::getPixmap()
{
  return pm;
}

/*-----------------------------------------------------------------------------
  Routine : IconListElem::getName()
 -----------------------------------------------------------------------------*/
const char* IconListElem::getName()
{
  return icnName;
}

/*=============================================================================
 Class : ProcList (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : ProcList::ProcList (constructor)
 -----------------------------------------------------------------------------*/
ProcList::ProcList(QWidget *parent=0,const char* name=0,int nCol=1)
         :KTabListBox(parent,name,nCol)
{
  initMetaObject();
  setSeparator(';');
}

/*-----------------------------------------------------------------------------
  Routine : ProcList::~ProcList  (destructor)
 -----------------------------------------------------------------------------*/
ProcList::~ProcList ()
{
}

/*-----------------------------------------------------------------------------
  Routine :  ProcList::cellHeight
 -----------------------------------------------------------------------------*/
int ProcList::cellHeight(int row)
{
  #ifdef DEBUG_MODE
     printf("KTop debug : cellHeight called for row %d.\n",row);
  #endif

  const QPixmap *pix = TaskMan::TaskMan_getProcIcon(text(row,2));
  if ( pix ) 
       return(pix->height());
  return(18);
}


/*=============================================================================
 Class : TaskMan (methods)
 =============================================================================*/

/*-----------------------------------------------------------------------------
 definition of the TaskMan's static member "icons".
 -----------------------------------------------------------------------------*/
QList<IconListElem>* TaskMan::icons = NULL;

/*-----------------------------------------------------------------------------
  Routine : TaskMan::TaskMan (constructor)
            creates the actual QTabDialog. We create it as a modeless dialog, 
	    using the toplevel widget as its parent, so the dialog won't get 
	    its own window.
 -----------------------------------------------------------------------------*/
TaskMan::TaskMan( QWidget *parent, const char *name, int sfolder )
        :QTabDialog( parent, name, FALSE, 0)
{
    QString tmp;
    QWidget *p0,*p1,*p2;

    initMetaObject();

    pages[0] = NULL;
    pages[1] = NULL;
    pages[2] = NULL;

    settings = NULL;
    ps       = NULL;
    ps_list  = NULL;

    pTree_updating     = FALSE;
    restoreStartupPage = FALSE;
    mouseRightButDown  = FALSE;

    pTree_lastSelectionPid = getpid();
    pList_lastSelectionPid = getpid();

    setStyle(WindowsStyle);
    
    connect(tabBar(),SIGNAL(selected(int)),SLOT(tabBarSelected(int)));
     
    /*----------------------------------------------
     set up pSig (QPopupMenu)
     ----------------------------------------------*/ 
    pSig = new QPopupMenu(NULL,"_psig");
    CHECK_PTR(pSig);
    pSig->insertItem(ktr(sig[0]),MENU_ID_SIGINT);
    pSig->insertItem(ktr(sig[1]),MENU_ID_SIGQUIT);
    pSig->insertItem(ktr(sig[2]),MENU_ID_SIGTERM);
    pSig->insertItem(ktr(sig[3]),MENU_ID_SIGKILL);
    pSig->insertSeparator();
    pSig->insertItem(ktr(sig[4]),MENU_ID_SIGUSR1);
    pSig->insertItem(ktr(sig[5]),MENU_ID_SIGUSR2);
    connect(pSig,SIGNAL(activated(int)), this, SLOT(pSigHandler(int)));
  
    /*----------------------------------------------
      set up page 0 (process list viewer)
     ----------------------------------------------*/ 

    pages[0] = p0 = new QWidget(this,"page0"); 
    CHECK_PTR(p0);
    pList_box = new QGroupBox(p0, "pList_box"); 
    CHECK_PTR(pList_box);
    pList = new ProcList(p0,"pList",NUM_COL);    
    CHECK_PTR(pList);
    connect(pList,SIGNAL(headerClicked(int)),SLOT(pList_headerClicked(int)));
    connect(pList,SIGNAL(highlighted(int,int)),SLOT(pList_procHighlighted(int,int)));
    connect(pList,SIGNAL(popupMenu(int,int)),SLOT(pList_popupMenu(int,int)));
    
    QFontMetrics fm = pList->fontMetrics();
    for ( int cnt=0 ; col_headers[cnt] ; cnt++ ) {
         pList->setColumn(cnt,col_headers[cnt]
                             ,fm.width(dummies[cnt])
                             ,col_types[cnt]);
    }

    // now, three buttons which should appear on the sheet (just below the listbox)
    pList_bRefresh = new QPushButton(ktr("Refresh Now"), p0,"pList_bRefresh");
    CHECK_PTR(pList_bRefresh);
    connect(pList_bRefresh, SIGNAL(clicked()), this, SLOT(pList_update()));
    pList_bKill = new QPushButton(ktr("Kill task"), p0, "pList_bKill");
    CHECK_PTR(pList_bKill);
    connect(pList_bKill,SIGNAL(clicked()), this, SLOT(pList_killTask()));
  
    pList_cbRefresh = new QComboBox(p0,"pList_cbRefresh");
    CHECK_PTR(pList_cbRefresh);
    for ( int i=0 ; refreshrates[i] ; i++ ) {
      pList_cbRefresh->insertItem( klocale->translate(refreshrates[i]),-1);
    } 
    pList_cbRefresh->setCurrentItem(2); //fast = default value;
    connect(pList_cbRefresh,SIGNAL(activated(int)),SLOT(pList_cbRefreshActivated(int)));

    pList_box->setTitle(ktr("Running processes"));

    /*----------------------------------------------
     set up page 1 (process tree)
     ----------------------------------------------*/
    pages[1] = p1 = new QWidget(this, "page1"); 
    CHECK_PTR(p1);          
    pTree_box = new QGroupBox(p1,"pTree_box"); 
    CHECK_PTR(pTree_box); 
    pTree = new ProcTree(p1,"pTree"); 
    CHECK_PTR(pTree);          
    pTree->setExpandLevel(20); 
    pTree->show();
    pTree->setSmoothScrolling(TRUE);
    connect(pTree,SIGNAL(highlighted(int)),SLOT(pTree_procHighlighted(int)));
    connect(pTree,SIGNAL(clicked(QMouseEvent*)),SLOT(pTree_clicked(QMouseEvent*)));

    // now, three buttons which should appear on the sheet (just below the tree box)
    pTree_bRefresh = new QPushButton(ktr("Refresh Now"), p1, "pTree_bRefresh");
    CHECK_PTR(pTree_bRefresh);
    connect(pTree_bRefresh, SIGNAL(clicked()), this, SLOT(pTree_update()));
    pTree_bRoot = new QPushButton(ktr("Change Root"), p1,"pTree_bRoot");
    CHECK_PTR(pTree_bRoot);
    connect(pTree_bRoot,SIGNAL(clicked()), this,SLOT(pTree_changeRoot()));
    pTree_bKill = new QPushButton(ktr("Kill task"), p1, "pTree_bKill");
    CHECK_PTR(pTree_bKill);
    connect(pTree_bKill,SIGNAL(clicked()), this, SLOT(pTree_killTask()));
    pTree_box->setTitle(ktr("Running processes"));

    pTree_cbSort = new QComboBox(p1,"pTree_cbSort");
    CHECK_PTR(pTree_cbSort);
    for ( int i=0 ; sortmethodsTree[i] ; i++ ) {
        pTree_cbSort->insertItem( klocale->translate(sortmethodsTree[i]),-1);
    } 
    pTree_cbSort->setCurrentItem(1); //by proc name = default value;
    connect(pTree_cbSort,SIGNAL(activated(int)),SLOT(pTree_cbSortActivated(int)));
    
    /*----------------------------------------------
     set up page 2 (This is the performance monitor)
     ----------------------------------------------*/
    pages[2] = p2 = new QWidget(this, "page2");
    CHECK_PTR(p2); 
    cpubox = new QGroupBox(p2, "_cpumon");
    CHECK_PTR(cpubox); 
    cpubox->setTitle(ktr("CPU load"));
    cpubox1 = new QGroupBox(p2, "_cpumon1");
    CHECK_PTR(cpubox1); 
    cpubox1->setTitle(ktr("CPU load history"));
    // cpu_cur is the left display (current load).
    cpu_cur = new QWidget(p2, "cpu_child");
    CHECK_PTR(cpu_cur); 
    cpu_cur->setBackgroundColor(black);
    // cpumon is the "real" load monitor. It contains all the functionality.
    // cpu_cur is passed as a parameter
    cpumon = new CpuMon (cpubox1, "cpumon", cpu_cur);
    CHECK_PTR(cpumon);
    // now, we do the same for the memory monitor
    membox = new QGroupBox(p2, "_memmon");
    CHECK_PTR(membox);
    membox->setTitle(ktr("Memory"));
    membox1 = new QGroupBox(p2, "_memhistory");
    CHECK_PTR(membox1);
    membox1->setTitle(ktr("Memory usage history"));
    mem_cur = new QWidget(p2, "mem_child");
    CHECK_PTR(mem_cur);
    mem_cur->setBackgroundColor(black);
    memmon = new MemMon (membox1, "memmon", mem_cur);

    // adjust some geometry
    pList_box->move(5, 5);
    pList_box->resize(380, 380);
    pList->move(10, 30);
    pList->resize(370, 300);

    /*----------------------------------------------
     settings
     ----------------------------------------------*/
    strcpy(cfgkey_startUpPage,"startUpPage");
    strcpy(cfgkey_pListUpdate,"pListUpdate");
    strcpy(cfgkey_pListSort,"pListSort");
    strcpy(cfgkey_pTreeSort,"pTreeSort");

    // restore refresh rate settings...
    pList_refreshRate=UPDATE_MEDIUM;
    tmp = config->readEntry(QString(cfgkey_pListUpdate));
    if( ! tmp.isNull() ) {
        bool res = FALSE;
        pList_refreshRate = tmp.toInt(&res);
        if (!res) pList_refreshRate=UPDATE_MEDIUM;
    }
    tid = NONE;
    pList_cbRefreshActivated(pList_refreshRate);
    pList_cbRefresh->setCurrentItem(pList_refreshRate);

    // restore sort method for pList...
    pList_sortby=SORTBY_CPU;
    tmp = config->readEntry(QString(cfgkey_pListSort));
    if( ! tmp.isNull() ) {
        bool res = FALSE;
        pList_sortby = tmp.toInt(&res);
        if (!res) pList_sortby=SORTBY_CPU;
    }

    // restore sort method for pTree...
    pTree_sortby=SORTBY_NAME;
    tmp = config->readEntry(QString(cfgkey_pTreeSort));
    if( ! tmp.isNull() ) {
        bool res = FALSE;
        pTree_sortby = tmp.toInt(&res);
        if (!res) pTree_sortby=SORTBY_NAME;
    }
    pTree_cbSort->setCurrentItem(pTree_sortby);

    // startup_page settings...
    startup_page = PAGE_PLIST;
    tmp = config->readEntry(QString(cfgkey_startUpPage));
    if( ! tmp.isNull() ) {
            startup_page = tmp.toInt();
            #ifdef DEBUG_MODE
               printf("KTop debug : startup_page (config val) = %d.\n",startup_page);
            #endif
    }  

    if ( sfolder >= 0 ) { 
         restoreStartupPage = TRUE;
         startup_page = sfolder;
         #ifdef DEBUG_MODE
            printf("KTop debug : startup_page (cmd line val) = %d.\n",startup_page);
         #endif
    }

    installEventFilter(this);

    pList_update();        /* create process list the first time */
    pTree_update();        /* create process tree the first time */

    // add pages...
    addTab(p0,ktr("Processes &List"));
    addTab(p1,ktr("Processes &Tree"));
    addTab(p2,ktr("&Performance"));
    move(0,0);
}


/*-----------------------------------------------------------------------------
  Routine : TaskMan::~TaskMan() (destructor)
	    writes back config entries (current)
 -----------------------------------------------------------------------------*/
TaskMan::~TaskMan()
{  
   if ( settings )
      delete settings;

   while( ps_list ) {
      psPtr tmp = ps_list;
      ps_list = ps_list->next;
      delete tmp;
   }
   
   delete pList;
   delete pTree;
   delete cpumon;
   delete memmon;
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::raiseStartUpPage
 -----------------------------------------------------------------------------*/
void TaskMan::raiseStartUpPage()
{ 
    QString tmp;

    tabBar()->setCurrentTab(startup_page);

    // in case startup_page has been modified from cmd line...
    if ( restoreStartupPage ) {
       tmp = config->readEntry(QString(cfgkey_startUpPage));
       if( ! tmp.isNull() )
           startup_page = tmp.toInt();
    }
} 

/*-----------------------------------------------------------------------------
  Routine : TaskMan::initIconList
 -----------------------------------------------------------------------------*/
void TaskMan::TaskMan_initIconList()
{
    DIR           *dir;
    struct dirent *de;
    char           path[PATH_MAX+1];
    char           icnFile[PATH_MAX+1];
    char           prefix[32];

    if ( icons ) return;

    #ifdef DEBUG_MODE
       printf("KTop debug : Looking for mini-icons.\n");
    #endif

    defaultIcon = new QPixmap(execXpm);
    CHECK_PTR(defaultIcon);

    icons = new QList<IconListElem>;
    CHECK_PTR(icons);
    
    char *kde_dir = getenv("KDEDIR");

    if ( kde_dir ) {
         #ifdef DEBUG_MODE
            printf("KTop debug : KDEDIR : %s.\n",kde_dir);
         #endif
         sprintf(path,"%s%s",kde_dir,KDE_ICN_DIR);
         dir = opendir(path);
     } else {
          #ifdef DEBUG_MODE
            printf("KTop debug : trying /opt/kde/share...\n");
          #endif
          sprintf(prefix,"/opt/kde");
          sprintf(path,"%s%s",prefix,KDE_ICN_DIR);
          dir = opendir(path);
          if ( ! dir ) {
              #ifdef DEBUG_MODE
                printf("KTop debug : trying /usr/local/kde/share...\n");
              #endif
              sprintf(prefix,"/usr/local");
              sprintf(path,"%s%s",prefix,KDE_ICN_DIR);
              dir = opendir(path);
          }
     }

    if ( ! dir ) return;  // default icon will be used
  
    while ( ( de = readdir(dir) ) )
     {
	if( strstr(de->d_name,".xpm") ) { 

            sprintf(icnFile,"%s/%s",path,de->d_name);

            #ifdef DEBUG_MODE
		//printf("KTop debug : found xpm : %s.\n",icnFile);
            #endif

            IconListElem *newElem = new IconListElem(icnFile,de->d_name);
            CHECK_PTR(newElem);
            icons->append(newElem);  
	}       
     }

    (void)closedir(dir);
  
    if ( kde_dir ) {
         sprintf(path,"%s%s",kde_dir,KTOP_ICN_DIR);
    } else {
          sprintf(path,"%s%s",prefix,KTOP_ICN_DIR);
          #ifdef DEBUG_MODE
            printf("KTop debug : trying %s...\n",path);
          #endif
    }

    dir = opendir(path);     
    if ( !dir ) return; // default icon will be used
  
    while ( ( de = readdir(dir) ) )
     {
	if( strstr(de->d_name,".xpm") ) 
         { 
            sprintf(icnFile,"%s/%s",path,de->d_name);

            #ifdef DEBUG_MODE
		//printf("KTop debug : found xpm : %s.\n",icnFile);
            #endif

            IconListElem *newElem = new IconListElem(icnFile,de->d_name);
            CHECK_PTR(newElem);
            icons->append(newElem);  
	}       
     }

    (void)closedir(dir);
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::clearIconList
 -----------------------------------------------------------------------------*/
void TaskMan::TaskMan_clearIconList()
{
  icons->setAutoDelete(TRUE);
  icons->clear();
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::TaskMan_getProcIcon
 -----------------------------------------------------------------------------*/
const QPixmap* TaskMan::TaskMan_getProcIcon( const char* pname )
{
 IconListElem* cur  = icons->first();
 IconListElem* last = icons->getLast();
 bool          goOn = TRUE;
 char          iName[128];

 sprintf(iName,"%s.xpm",pname);
 do {
    if ( cur == last ) goOn = FALSE;
    if (! cur ) goto end;
    if ( !strcmp(cur->getName(),iName) )
       return( cur->getPixmap() );
    cur = icons->next(); 
 } while ( goOn ) ;

 end: 
   return defaultIcon;
}
/*-----------------------------------------------------------------------------
  Routine : TaskMan::resizeEvent
 -----------------------------------------------------------------------------*/
void TaskMan::resizeEvent(QResizeEvent *ev)
{
  
    QTabDialog::resizeEvent(ev);

    if( ! pages[1] || !pages[0] )
          return;

    int w = pages[0]->width();
    int h = pages[0]->height();

    // processes list
    pList_box->setGeometry(5, 5, w - 10, h - 20);
    pList->setGeometry(10,25, w - 20, h - 75);
    pList_cbRefresh->setGeometry(10, h - 45,140, 25);
    pList_bRefresh->setGeometry(w - 180, h - 45, 80, 25);
    pList_bKill->setGeometry(w - 90, h - 45, 80, 25);
    
    // processes tree
    pTree_box->setGeometry(5, 5, w - 10, h - 20);
    pTree->setGeometry(10, 30, w - 20, h - 90);
    pTree_cbSort->setGeometry(10, h - 50,140, 25);
    pTree_bRefresh->setGeometry(w - 270, h - 50, 80, 25);
    pTree_bRoot->setGeometry(w - 180, h - 50, 80, 25);
    pTree_bKill->setGeometry(w - 90, h - 50, 80, 25);
 
    // performances page
    cpubox->setGeometry(10, 10, 80, (h / 2) - 30);
    cpubox1->setGeometry(100, 10, w - 110, (h / 2) - 30);
    cpu_cur->setGeometry(20, 30, 60, (h / 2) - 60);
    cpumon->setGeometry(10, 20, cpubox1->width() - 20, cpubox1->height() - 30);
    membox->setGeometry(10, h / 2, 80, (h / 2) - 30);
    membox1->setGeometry(100, h / 2, w - 110, (h / 2) - 30);
    mem_cur->setGeometry(20, h / 2 + 20, 60, (h / 2) - 60);
    memmon->setGeometry(10, 20, membox1->width() - 20, membox1->height() - 30);
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::TaskMan_timerEvent(QTimerEvent *)
 -----------------------------------------------------------------------------*/
void TaskMan::timerEvent(QTimerEvent *)
{
   pList_update();
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pSigHandler
 -----------------------------------------------------------------------------*/
void TaskMan::pSigHandler( int id )
{
  int the_sig;

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
    	default:
          return;
          break;
  }

  int selection;
  switch ( tabBar()->currentTab() ) {
     case PAGE_PLIST:
      if ( pList_lastSelectionPid == NONE ) return;
      selection = pList_lastSelectionPid;
      break;
     case PAGE_PTREE:
      if ( pTree_lastSelectionPid == NONE ) return;
      selection = pTree_lastSelectionPid;
      break;
     default:
      return;
      break;
  }
  
  int err = kill(selection,the_sig);
  if ( err == -1 ) {
       QMessageBox::warning(this,"ktop",
       "Kill error...\nSpecified process does not exists\nor permission denied.",
       "Ok", 0);
  }

  switch ( tabBar()->currentTab() ) {
    case PAGE_PLIST: 
        if ( err != -1 ) {
      	     pList_lastSelectionPid = getpid();
      	     pList_update();
        }
      	break;
    case PAGE_PTREE:
        if ( err != -1 ) {
      	     pTree_lastSelectionPid = getpid();
      	     pTree_update();
        }
      	break;
    default:
      	return;
      	break;
  }
 
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::tabBarSelected
 -----------------------------------------------------------------------------*/
void TaskMan::tabBarSelected ( int tabIndx )
{ 
  #ifdef DEBUG_MODE
    printf("KTop debug : tabBar selected. indx=%d.\n",tabIndx);
  #endif

  switch ( tabIndx ) {
     case PAGE_PLIST :
       pList_update();
       break;
     case PAGE_PTREE :
       pTree_update();
       break;
     default:
       break;
  }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_cbRefreshActivated
 -----------------------------------------------------------------------------*/
void TaskMan::pList_cbRefreshActivated(int indx)
{ 

 #ifdef DEBUG_MODE
    printf("KTop debug : cbRefreshActivated - item = %d.\n",indx);
 #endif

 int value;

 pList_refreshRate = indx;

 switch ( indx ) {
	case UPDATE_SLOW:
	  value = UPDATE_SLOW_VALUE;
	  break;
        case UPDATE_MEDIUM:
	  value = UPDATE_MEDIUM_VALUE;
          break;
        case UPDATE_FAST:
	  value = UPDATE_FAST_VALUE;
          break;
    	default:
	  value = UPDATE_FAST_VALUE;
	  break;    	
 }
 setUpdateInterval(value);
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::setUpdateInterval
 	    sets new timer interval, return old value
 -----------------------------------------------------------------------------*/
int TaskMan::setUpdateInterval(int new_interval)
{
    int old = timer_interval;
    
    if ( tid != NONE ) killTimer(tid);
    timer_interval = new_interval * 1000;
    tid = startTimer(timer_interval);
    return (old / 1000);
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::getUpdateInterval()
            returns current timer interval
 -----------------------------------------------------------------------------*/
int TaskMan::getUpdateInterval()
{
    return (timer_interval / 1000);
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::invokeSettings(void)
 -----------------------------------------------------------------------------*/
void TaskMan::invokeSettings(void)
{
    if( ! settings ){
        settings = new AppSettings(0,"proc_options");
        CHECK_PTR(settings);      
    }

    settings->setStartUpPage(startup_page);
    if( settings->exec() ) {
        startup_page = settings->getStartUpPage();
        #ifdef DEBUG_MODE
           printf("KTop debug : startup_page (new val) = %d.\n",startup_page);
        #endif
        saveSettings();
    }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::saveSettings(void)
 -----------------------------------------------------------------------------*/
void TaskMan::saveSettings()
{
 QString  t;
 char     temp[32];
 char    *g_format = "%04d:%04d:%04d:%04d";

 sprintf( temp                    , g_format
        , parentWidget()->x()     , parentWidget()->y()
        , parentWidget()->width() , parentWidget()->height() );

 config->writeEntry(QString("G_Toplevel"), QString(temp));
 config->writeEntry(QString(cfgkey_startUpPage),t.setNum(startup_page),TRUE);
 config->writeEntry(QString(cfgkey_pListUpdate),t.setNum(pList_refreshRate),TRUE);
 config->writeEntry(QString(cfgkey_pListSort),t.setNum(pList_sortby),TRUE);
 config->writeEntry(QString(cfgkey_pTreeSort),t.setNum(pTree_sortby),TRUE);
 config->sync();
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan : Processes list routines
 -----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_update
 -----------------------------------------------------------------------------*/
void TaskMan::pList_update(void)
{
    int top_Item   = pList->topItem();
    pList->setAutoUpdate(FALSE);
      pList_load();
      pList_restoreSelection();
      pList->setTopItem(top_Item);
    pList->setAutoUpdate(TRUE);
    if( pList->isVisible() ) pList->repaint();
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_load
 -----------------------------------------------------------------------------*/
void TaskMan::pList_load()
{
#ifdef __FreeBSD__
	char line[256];
	struct passwd *pwent;
	pList_clearProcVisit();

	int mib[2];
	size_t len;
	struct kinfo_proc *p;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;
	sysctl(mib, 3, NULL, &len, NULL, 0);
	p = (struct kinfo_proc *)malloc(len);
	sysctl(mib, 3, p, &len, NULL, 0);

	int num;
	for (num = len / sizeof(struct kinfo_proc) - 1; num > -1; num--) {
		char s[10];
		snprintf(s, 10,"%d",p[num].kp_proc.p_pid);
		ps = pList_getProcItem(s);
		if ( ! ps ) return;

		strcpy(ps->name, p[num].kp_proc.p_comm);
		ps->status = p[num].kp_proc.p_stat;
		strcpy(ps->statusTxt, p[num].kp_eproc.e_wmesg);
	 	ps->visited = 1;
		ps->uid = p[num].kp_eproc.e_ucred.cr_uid;
		ps->gid = p[num].kp_eproc.e_pgid;
		ps->pid = p[num].kp_proc.p_pid;
		ps->ppid = p[num].kp_eproc.e_ppid;

		struct timeval tv;
		gettimeofday(&tv,0);

		ps->oabstime = ps->abstime;
		ps->abstime = tv.tv_sec*100+tv.tv_usec/10000;

		// if no time between two cycles - don't show any usable cpu percentage
		if(ps->oabstime==ps->abstime)
			ps->oabstime=ps->abstime-100000;
		ps->otime=ps->time;
//		ps->time=p[num].kp_proc.p_rtime.tv_sec*100+p[num].kp_proc.p_rtime.tv_usec/10000;
		ps->time=0;

		// set other data
		ps->vm_size = (p[num].kp_eproc.e_vm.vm_tsize +
			p[num].kp_eproc.e_vm.vm_dsize +
			p[num].kp_eproc.e_vm.vm_ssize) * getpagesize() / 1024;
		ps->vm_lock = 0;
		ps->vm_rss = p[num].kp_eproc.e_vm.vm_rssize * getpagesize() / 1024;
		ps->vm_data = p[num].kp_eproc.e_vm.vm_dsize * getpagesize() / 1024;
		ps->vm_stack = p[num].kp_eproc.e_vm.vm_ssize * getpagesize() / 1024;
		ps->vm_exe = p[num].kp_eproc.e_vm.vm_tsize * getpagesize() / 1024;
		ps->priority = p[num].kp_proc.p_priority - PZERO;
	}
	free(p);
#else
    DIR *dir;
    char line[256];
    struct dirent *entry;
    struct passwd *pwent;
    
    pList_clearProcVisit();
      dir = opendir(PROC_BASE);
      while( (entry = readdir(dir)) ) {
        if( isdigit(entry->d_name[0]) ) 
            pList_getProcStatus(entry->d_name);
      }
      closedir(dir);
#endif
    pList_removeProcUnvisited();
    pList_sort();

    pList->clear();
    pList->dict().clear();  

    psPtr tmp;
    const QPixmap *pix;
    char  usrName[32];
    int   i;
    for( tmp=ps_list, i=1 ; tmp ; tmp=tmp->next , i++) {
        pwent = getpwuid(tmp->uid);
        if ( pwent ) 
          strncpy(usrName,pwent->pw_name,31);
        else 
          strcpy(usrName,"????");
        pix = pList_getProcIcon((const char*)tmp->name);
        pList->dict().insert((const char*)tmp->name,pix);
        sprintf(line, "{%s};%d;%s;%s;%5.2f%%;%d:%02d;%s;%d;%d;%d", 
                    tmp->name,
	            tmp->pid, 
                    tmp->name, 
                    usrName,
                    1.0*(tmp->time-tmp->otime)/(tmp->abstime-tmp->oabstime)*100, 
                    (tmp->time/100)/60,(tmp->time/100)%60, 
                    tmp->statusTxt,
                    tmp->vm_size, 
                    tmp->vm_rss, 
#ifdef __FreeBSD__
		    tmp->priority
#else
                    tmp->vm_lib
#endif
		    );         
        pList->appendItem(line);
    }

}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_sort()
 -----------------------------------------------------------------------------*/
void TaskMan::pList_sort() 
{

    int   swap;
    psPtr start, 
          item, 
          tmp;

    for ( start=ps_list ; start ; start=start->next ) { 

        for ( item=ps_list ; item && item->next ; item=item->next ) {

	    switch ( pList_sortby ) {
	        case SORTBY_PID:
		    swap = item->pid > item->next->pid;
		    break;
	        case SORTBY_UID:
		    swap = item->uid > item->next->uid;
		    break;
	        case SORTBY_NAME:
                    swap = strcmp(item->name, item->next->name) > 0;
		    break;
	        case SORTBY_TIME:
		    swap = item->time < item->next->time;
		    break;
	        case SORTBY_STATUS:
		    swap = item->status > item->next->status;
		    break;
	        case SORTBY_VMSIZE:
		    swap = item->vm_size < item->next->vm_size;
		    break;
	        case SORTBY_VMRSS:
		    swap = item->vm_rss < item->next->vm_rss;
		    break;
#ifdef __FreeBSD__
	        case SORTBY_PRIOR:
		    swap = item->priority < item->next->priority;
#else
	        case SORTBY_VMLIB:
		    swap = item->vm_lib < item->next->vm_lib;
#endif
		    break;
	        case SORTBY_CPU:
	        default        :
		    swap = (item->time-item->otime) < (item->next->time-item->next->otime);
	    }

	    if ( swap ) {
	        tmp = item->next;
	        if ( item->prev ) 
                     item->prev->next = tmp;
		else 
                     ps_list = tmp;
	        if( tmp->next ) 
                     tmp->next->prev = item;
		tmp->prev  = item->prev;
		item->prev = tmp;
		item->next = tmp->next;
		tmp->next  = item;
		if( (start=item) ) start=tmp;
		item=tmp;
	    }

	}
    } 
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_clearProcVisit()
 -----------------------------------------------------------------------------*/
void TaskMan::pList_clearProcVisit() 
{
    psPtr tmp;
    for( tmp=ps_list ; tmp ; tmp->visited=0 , tmp=tmp->next );
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_getProcItem(char* aName)
 -----------------------------------------------------------------------------*/
psPtr TaskMan::pList_getProcItem(char* aName) 
{
    psPtr tmp;
    int   pid;

    sscanf(aName,"%d",&pid);
    for ( tmp=ps_list ; tmp && (tmp->pid!=pid) ; tmp=tmp->next );

    if( ! tmp ) {
        // create an new elem & insert it 
        // at the top of the linked list
        tmp = new psStruct;
        if ( tmp ) {
           memset(tmp,0,sizeof(psStruct));
           tmp->pid=pid;
           tmp->next=ps_list;
           if( ps_list )
               ps_list->prev=tmp;
           ps_list=tmp;
        }
    }
    return tmp;
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_removeProcUnvisited()
 -----------------------------------------------------------------------------*/
void TaskMan::pList_removeProcUnvisited() 
{
    psPtr item, tmp;

    for ( item=ps_list ; item ; ) {
        if( ! item->visited ) {
            tmp = item;
            if ( item->prev )
                 item->prev->next = item->next;
	    else
	         ps_list = item->next;
            if ( item->next )
	         item->next->prev = item->prev;
            item = item->next; 
            delete tmp;
        }
	item = item->next;
    }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_getProcStatus(char * pid)
 -----------------------------------------------------------------------------*/
#ifndef __FreeBSD__
// BSD doesn't use this
int TaskMan::pList_getProcStatus(char * pid)
{
    char buffer[1024], temp[128];
    FILE *fd;
    int u1, u2, u3, u4, time1, time2;
    
    #ifdef DEBUG_MODE
      //printf("KTop debug: read entered\n");
    #endif

    ps = pList_getProcItem(pid);
    if ( ! ps ) return 0;
    
    sprintf(buffer, "/proc/%s/status", pid);
    if((fd = fopen(buffer, "r")) == 0)
        return 0;

    fscanf(fd, "%s %s", buffer, ps->name);
    fscanf(fd, "%s %c %s", buffer, &ps->status, temp);
    switch ( ps->status ) {
       case 'R':
           strcpy(ps->statusTxt,"Run");
           break;
       case 'S':
           strcpy(ps->statusTxt,"Sleep");
           break;
       case 'D': 
           strcpy(ps->statusTxt,"Disk");
           break;
       case 'Z': 
           strcpy(ps->statusTxt,"Zombie");
           break;
       case 'T': 
           strcpy(ps->statusTxt,"Stop");
           break;
       case 'W': 
           strcpy(ps->statusTxt,"Swap");
           break;
       default:
           strcpy(ps->statusTxt,"????");
           break;
    }
    fscanf(fd, "%s %d", buffer, &ps->pid);
    fscanf(fd, "%s %d", buffer, &ps->ppid);
    fscanf(fd, "%s %d %d %d %d", buffer, &u1, &u2, &u3, &u4);
    ps->uid = u1;
    fscanf(fd, "%s %d %d %d %d", buffer, &u1, &u2, &u3, &u4);
    ps->gid = u1;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_size);
    if(strcmp(buffer, "VmSize:"))
        ps->vm_size=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_lock);
    if(strcmp(buffer, "VmLck:"))
        ps->vm_lock=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_rss);
    if(strcmp(buffer, "VmRSS:"))
        ps->vm_rss=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_data);
    if(strcmp(buffer, "VmData:"))
        ps->vm_data=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_stack);
    if(strcmp(buffer, "VmStk:"))
        ps->vm_stack=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_exe);
    if(strcmp(buffer, "VmExe:"))
        ps->vm_exe=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_lib);
    if(strcmp(buffer, "VmLib:"))
        ps->vm_lib=0;
    fclose(fd);

    #ifdef DEBUG_MODE
      //printf("KTop debug: read completed\n");
    #endif
    sprintf(buffer, "/proc/%s/stat", pid);
    if((fd = fopen(buffer, "r")) == 0)
        return 0;
    
    fscanf(fd, "%*s %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d", &time1, &time2);
    #ifdef DEBUG_MODE
      //printf("KTop debug: %d %d\n", time1, time2);
    #endif

    struct timeval tv;
    gettimeofday(&tv,0);
    ps->oabstime=ps->abstime;
    ps->abstime=tv.tv_sec*100+tv.tv_usec/10000;

    // if no time between two cycles - don't show any usable cpu percentage
    if(ps->oabstime==ps->abstime)
        ps->oabstime=ps->abstime-100000;
    ps->otime=ps->time;
    ps->time=time1+time2;

    fclose(fd);
    ps->visited=1;

    return 1;
}
#endif

/*-----------------------------------------------------------------------------
  Routine : TaskMan::headerClicked
 -----------------------------------------------------------------------------*/
void TaskMan::pList_headerClicked(int indxCol)
{
  #ifdef DEBUG_MODE
    printf("KTop debug : pList_headerClicked : col. : %d.\n",indxCol);
  #endif

  if ( indxCol ) {
       switch ( indxCol-1 ) {
          case SORTBY_PID: 
          case SORTBY_NAME: 
          case SORTBY_UID: 
          case SORTBY_CPU: 
          case SORTBY_TIME:
          case SORTBY_STATUS:
          case SORTBY_VMSIZE:
          case SORTBY_VMRSS:
#ifdef __FreeBSD__
          case SORTBY_PRIOR:
#else
          case SORTBY_VMLIB:
#endif
               pList_sortby = indxCol-1;
               break;
          default: 
               return;
               break;
       }
       pList_update();
  }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_procHighlighted
 -----------------------------------------------------------------------------*/
void TaskMan::pList_procHighlighted(int indx,int)
{ 
  #ifdef DEBUG_MODE
    printf("KTop debug : item %d selected.\n",indx);
  #endif
  
  pList_lastSelectionPid = NONE;
  sscanf(pList->text(indx,1),"%d",&pList_lastSelectionPid);
  #ifdef DEBUG_MODE
    printf("KTop debug : pList_lastSelectionPid = %d.\n",pList_lastSelectionPid);
  #endif 
} 
 
/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_popupMenu
 -----------------------------------------------------------------------------*/
void TaskMan::pList_popupMenu(int indx,int)
{ 
  #ifdef DEBUG_MODE
    printf("KTop debug : item %d selected.\n",indx);
  #endif

  pList->setCurrentItem(indx);
  pSig->popup(QCursor::pos());
  
} 

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_killTask()
 -----------------------------------------------------------------------------*/
void TaskMan::pList_killTask()
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
   printf("KTop debug : current selection pid   = %d\n",pid); 
   printf("KTop debug : current selection pname = %s\n",pname);
   printf("KTop debug : current selection uname = %s\n",uname);
 #endif

 if ( pList_lastSelectionPid != pid ) {
      QMessageBox::warning(this,"ktop",
                                "Selection changed !\n\n",
                                "Abort",0);
      pList_lastSelectionPid = NONE;
      pList->setCurrentItem(0);
      return;
 }

 int  err;
 char msg[256];
 sprintf(msg,"Kill process %d (%s - %s) ?\n",pid,pname,uname);

 switch( QMessageBox::warning(this,"ktop",
                                    msg,
                                   "Continue", "Abort",
                                    0, 1 )
       )
    { 
      case 0: // continue
          err = kill(pList_lastSelectionPid ,SIGKILL);
          if ( err == -1 ) 
	       QMessageBox::warning(this,"ktop",
                                    "Kill error...\nSpecified process does not exists\nor permission denied.",
                                    "Ok", 0);
          pList_lastSelectionPid = NONE;
          pList_update();
          pList->setCurrentItem(0);
          break;
      case 1: // abort
          break;
    }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_restoreSelection
 -----------------------------------------------------------------------------*/
void TaskMan::pList_restoreSelection(int lastColVis = 0)
{
  if ( pList_lastSelectionPid==NONE ) return;

  QString txt;
  int     cnt = pList->count();
  int     pid;
  bool    res = FALSE;

  for ( int i=0 ; i< cnt  ; i++ ) {
      txt = pList->text(i,1);
      res = FALSE;
      pid = txt.toInt(&res);
      if ( res && (pid == pList_lastSelectionPid) ) {
           pList->setCurrentItem(i,lastColVis);
           #ifdef DEBUG_MODE
             printf("KTop debug : pList_restoreSelection - cur pid : %d\n",pid);    
           #endif
           return;
      }
  }
 
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pList_getProcIcon
 -----------------------------------------------------------------------------*/
const QPixmap* TaskMan::pList_getProcIcon( const char* pname )
{
 return TaskMan_getProcIcon(pname);
}



/*-----------------------------------------------------------------------------
  Routine : TaskMan : Processes tree routines
 -----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_update
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_update( void )
{
  pTree_updating = TRUE;
  	pTree->setUpdatesEnabled(FALSE);  
    		pTree->setExpandLevel(0); 
    		pTree->clear();
    		pTree_readProcDir();
                #ifdef DEBUG_MODE
                  printf("KTop debug : pTree_update : pTree_readProcDir : ok.\n");
                #endif
    		pTree_sort();
                #ifdef DEBUG_MODE
                  printf("KTop debug : pTree_update : pTree_sort: ok.\n");
                #endif
    		pTree->setExpandLevel(50);
  	pTree->setUpdatesEnabled(TRUE);
  	if ( pTree->isVisible() )
       		pTree->repaint(TRUE);
        pTree_restoreSelection(pTree->itemAt(0));
        #ifdef DEBUG_MODE
           printf("KTop debug : pTree_update : pTree_restoreSelection : ok.\n");
        #endif
 pTree_updating = FALSE;
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_cbSortActivated
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_cbSortActivated(int indx)
{ 
 #ifdef DEBUG_MODE
    printf("KTop debug : pTree_cbSortActivated - item = %d.\n",indx);
 #endif
 
 pTree_sortby = indx;
 pTree_sortUpdate();
 pTree_restoreSelection(pTree->itemAt(0));
 
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_sort
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_sort( void )
{
 ProcTreeItem *mainItem = pTree->itemAt(0);
 
 if ( ! mainItem ) return;
 
 switch ( pTree_sortby ) {
   case SORTBY_PID:
	mainItem->setSortPidText(); 
        mainItem->setChild(pTree_sortByPid(mainItem->getChild(),false));
	break; 
   case SORTBY_NAME:
        mainItem->setSortNameText(); 
        mainItem->setChild(pTree_sortByName(mainItem->getChild()));
        break; 
   case SORTBY_UID:
        mainItem->setSortUidText(); 
        mainItem->setChild(pTree_sortByUid(mainItem->getChild()));
        break;
 }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_sortUpdate
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_sortUpdate( void )
{    
  pTree->setUpdatesEnabled(FALSE);  
    pTree_sort();
  pTree->setUpdatesEnabled(TRUE);
  if ( pTree->isVisible() )
       pTree->repaint(TRUE); 
}


/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_getParentItem
 -----------------------------------------------------------------------------*/
ProcTreeItem* TaskMan::pTree_getParentItem(ProcTreeItem* item, int ppid )
{
 ProcTreeItem* anItem;

 if ( ! item ) return NULL ; 
 if ( item->getProcId() == ppid ) 
      return item;
 if ( (anItem = pTree_getParentItem(item->getSibling(),ppid)) )
    return anItem;
 return pTree_getParentItem(item->getChild(),ppid);
 
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_restoreSelection
 -----------------------------------------------------------------------------*/
ProcTreeItem* TaskMan::pTree_restoreSelection(ProcTreeItem* item)
{
 ProcTreeItem* anItem;

 if ( !item || (pTree_lastSelectionPid==NONE) ) {
      return NULL;
 }

 if ( item->getProcId() == pTree_lastSelectionPid ) {
      int indx  = pTree->itemVisibleIndex(item);
      #ifdef DEBUG_MODE
        printf("KTop debug : item->getProcId()= %d =?= pTree_lastSelectionPid=%d\n"
	        ,item->getProcId(),pTree_lastSelectionPid);       
	printf("KTop debug : pTree last selection : %d\n",indx);
      #endif
      pTree->setCurrentItem(indx);
      return item;
 }

 if ( (anItem = pTree_restoreSelection(item->getSibling())) )
    return anItem;

 return pTree_restoreSelection(item->getChild());
 
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_sortByName
 -----------------------------------------------------------------------------*/
ProcTreeItem* TaskMan::pTree_sortByName( ProcTreeItem* ref ) 
{
 ProcTreeItem *newTop,*cur,*aTempItem,*prev=NULL;

 if ( ! ref ) return ref;

 ref->setSortNameText();

 if ( ref->hasChild() )
   ref->setChild(pTree_sortByName(ref->getChild()) );

 if ( ! ref->hasSibling() ) 
   return ref;

 newTop = pTree_sortByName(ref->getSibling());
 ref->setSibling(newTop);  
 if ( ! newTop ) return newTop;  
 
 cur = newTop;

 int counter = 0; 

 while ( cur )   
   {     
     aTempItem = cur->getSibling();
     if ( strcmp(ref->getProcName(),cur->getProcName()) > 0 ) {
	 cur->setSibling(ref); 
	 ref->setSibling(aTempItem);
	 if ( prev ) prev->setSibling(cur);
	 prev = cur;
	 counter++;   
     } 
     cur = aTempItem;
   }

 if ( counter ) return newTop; else return ref; 

}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_sortByPid
 -----------------------------------------------------------------------------*/
ProcTreeItem* TaskMan::pTree_sortByPid(ProcTreeItem* ref, bool reverse) 
{
 ProcTreeItem *newTop,*cur,*aTempItem,*prev=NULL;

 if ( ! ref ) return ref;
 
 ref->setSortPidText();

 if ( ref->hasChild() )
   ref->setChild( pTree_sortByPid(ref->getChild(),reverse) );

 if ( ! ref->hasSibling() ) 
   return ref;
 
 newTop = pTree_sortByPid(ref->getSibling(),reverse);
 ref->setSibling(newTop);  
 if ( ! newTop ) return newTop;  
 
 cur = newTop;

 int   counter = 0; 

 while ( cur )   
   { 
     aTempItem = cur->getSibling();
     
     if ( ! reverse ) {
     	if ( ref->getProcId() > cur->getProcId() ) {
	 	cur->setSibling(ref); 
	 	ref->setSibling(aTempItem);
	 	if ( prev ) prev->setSibling(cur);
	 	prev = cur;
	 	counter++;   
     	}
      }
      else {
     	if ( ref->getProcId() < cur->getProcId() ) {
	 	cur->setSibling(ref); 
	 	ref->setSibling(aTempItem);
	 	if ( prev ) prev->setSibling(cur);
	 	prev = cur;
	 	counter++;   
     	}
      }
 
     cur = aTempItem;
   }
 if ( counter ) return newTop; else return ref; 

}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_sortByUid
 -----------------------------------------------------------------------------*/
ProcTreeItem* TaskMan::pTree_sortByUid(ProcTreeItem* ref) 
{
 ProcTreeItem *newTop,*cur,*aTempItem,*prev=NULL;

 if ( ! ref ) return ref;
 
 ref->setSortUidText();

 if ( ref->hasChild() )
   ref->setChild( pTree_sortByUid(ref->getChild()) );

 if ( ! ref->hasSibling() ) 
   return ref;
 
 newTop = pTree_sortByUid(ref->getSibling());
 ref->setSibling(newTop);  
 if ( ! newTop ) return newTop;  
 
 cur = newTop;

 int   counter = 0; 

 while ( cur )   
   { 
     aTempItem = cur->getSibling();
     
     if ( ref->getProcUid() > cur->getProcUid() ) {
	 	cur->setSibling(ref); 
	 	ref->setSibling(aTempItem);
	 	if ( prev ) prev->setSibling(cur);
	 	prev = cur;
	 	counter++;   
     }
     cur = aTempItem;
   }

 if ( counter ) return newTop; else return ref; 

}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_reorder
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_reorder( ProcTree* alist )
{
  ProcTreeItem   *cur = alist->itemAt(0),*token,*next;
  if ( !cur ) return;

  while ( cur ) 
    { 
      ProcTreeItem *parent = pTree_getParentItem(pTree->itemAt(0),cur->getParentId()); 
          next  = cur->getSibling();
      int indx  = alist->itemVisibleIndex(cur);
          token = alist->takeItem(indx);
      if ( parent ) 
	{ 
          ProcTreeItem *child = new ProcTreeItem(cur->getProcInfo()
                                       ,pTree_getProcIcon(cur->getProcName()));
          CHECK_PTR(child);
	  parent->appendChild(child);
	}
        else { // parent not already in tree => move item to bottom
           alist->insertItem(token,alist->count()-1,false);
        }
        cur = next;
    }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_changeRoot
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_changeRoot()
{  
  int newRootIndx = pTree->currentItem();
  if ( newRootIndx == -1 ) return;
  
  ProcTreeItem *newRoot = pTree->takeItem(newRootIndx);
  if ( ! newRoot ) return;

  pTree->setUpdatesEnabled(FALSE);  
    pTree->clear();
    pTree->insertItem(newRoot);
    pTree->setCurrentItem(0);
  pTree->setUpdatesEnabled(TRUE);
  pTree->repaint(TRUE); 
}

/*-----------------------------------------------------------------------------
  Routine : pTree_readProcDir
  Most of this code (this routine) is :
  Copyright 1993-1998 Werner Almesberger (pstree author). All rights reserved.
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_readProcDir(  )
{
#ifdef __FreeBSD__
    int mib[2];
    size_t len;
    struct kinfo_proc *p;
#else
    DIR           *dir;
    struct dirent *de;
    FILE          *file;
    struct stat    st;
    char           path[PATH_MAX+1];
    int            empty,dummy;
#endif
    ProcInfo       pi;
    struct passwd *pwent;

#ifdef __FreeBSD__
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    sysctl(mib, 3, NULL, &len, NULL, 0);
    p = (struct kinfo_proc *)malloc(len);
    sysctl(mib, 3, p, &len, NULL, 0);
#endif
    ProcTree      *alist = new ProcTree();  
    CHECK_PTR(alist);

#ifndef __FreeBSD__
    if (!(dir = opendir(PROC_BASE))) 
      {
	perror(PROC_BASE);
	exit(1);
      }

    empty = 0;

    while ((de = readdir(dir)))
         
         if(isdigit(de->d_name[0])) { 
      
            sscanf(de->d_name,"%d",&(pi.pid));

	    sprintf(path,"%s/%d/stat",PROC_BASE,pi.pid);

	    if ( ( file = fopen(path,"r") ) ) {

		if (fstat(fileno(file),&st) < 0) {
		    perror(path);
		    exit(1);
		}

                pi.uid = st.st_uid;
		if (fscanf(file,"%d (%[^)]) %c %d"
			       ,&dummy
			       ,pi.name
			       ,(char*)&dummy
			       ,&(pi.ppid)) == 4) 
		  { 
                    
                    // get user name
                    pwent = getpwuid(pi.uid);
                    if( pwent )
			strcpy(pi.uname,pwent->pw_name);
                    else 
                        strcpy(pi.uname,"????");

		    strcpy(pi.arg,"Not implemented");
                    
                    // create new item
                    ProcTreeItem *child = new ProcTreeItem(pi,pTree_getProcIcon(pi.name));
                    CHECK_PTR(child);
                     
                    //insert new item in tree
		    if ( pi.pid == INIT_PID )
		         pTree->insertItem(child);
		    else if ( pi.ppid == INIT_PID ) 
			 pTree->addChildItem(child,0); 
		    else { 
		      ProcTreeItem *item=pTree_getParentItem(pTree->itemAt(0),pi.ppid);   
		      if ( item )  
			  item->appendChild(child); 
		      else  
			  alist->insertItem(child); 
		    }	
	          }
		(void)fclose(file);
	    }
	}
    (void)closedir(dir);

    if ( empty ) {
        if ( alist ) delete alist;
	fprintf(stderr,PROC_BASE " is empty (not mounted ???)\n");
	exit(1);
    }
#else
    int num;
    for (num = len / sizeof(struct kinfo_proc) - 1; num > -1; num--) {
	pi.pid = p[num].kp_proc.p_pid;
	pi.ppid = p[num].kp_eproc.e_ppid;
	pi.uid =  p[num].kp_eproc.e_ucred.cr_uid;
	strcpy(pi.name, p[num].kp_proc.p_comm);

	// get user name
	pwent = getpwuid(pi.uid);
	if( pwent )
	    strcpy(pi.uname,pwent->pw_name);
	else
	    strcpy(pi.uname,"????");

	strcpy(pi.arg,"Not implemented");
                    
        // create new item
        ProcTreeItem *child = new ProcTreeItem(pi,pTree_getProcIcon(pi.name));
        CHECK_PTR(child);

        //insert new item in tree
	if ( pi.pid == INIT_PID )
	    pTree->insertItem(child);
	else if ( pi.ppid == INIT_PID )
	    pTree->addChildItem(child,0); 
	else if (pi.ppid) {  // kernel processes can't be viewed in tree
	    ProcTreeItem *item=pTree_getParentItem(pTree->itemAt(0),pi.ppid);   
	    if ( item )  
		item->appendChild(child); 
	    else  
		alist->insertItem(child);
	}

    }
#endif

    pTree_reorder(alist);
    
    delete alist;

}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_clicked
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_clicked(QMouseEvent* e)
{
  #ifdef DEBUG_MODE
      printf("KTop debug : mousePressEvent : button : %d.\n",e->button());
  #endif
  mouseRightButDown = (e->button() == RightButton) ? TRUE : FALSE;
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_procHighlighted
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_procHighlighted(int indx)
{ 
  if ( pTree_updating ) return;

  #ifdef DEBUG_MODE
    printf("KTop debug : item %d selected.\n",indx);
  #endif
 
  pTree_lastSelectionPid = NONE;

  ProcTreeItem *item = pTree->itemAt(indx);
  if ( ! item ) return;
  
  pTree_lastSelectionPid = item->getProcId();

  #ifdef DEBUG_MODE
    printf("KTop debug : pTree_lastSelectionPid = %d.\n"
                                              ,pTree_lastSelectionPid);
  #endif  
  
  if ( mouseRightButDown ) {
       pSig->popup(QCursor::pos());
       mouseRightButDown = FALSE;
  }
} 

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_killTask()
 -----------------------------------------------------------------------------*/
void TaskMan::pTree_killTask()
{
 int cur = pTree->currentItem();
 if ( (cur == -1) || (pTree_lastSelectionPid==NONE) ) {
        pTree->setCurrentItem(0);
	pTree_lastSelectionPid = NONE;
        return;
 }

 ProcTreeItem *item = pTree->itemAt(cur);
 if ( ! item ) return;
 
 ProcInfo pInfo = item->getProcInfo();
 
 #ifdef DEBUG_MODE
   printf("KTop debug : current selection pid   = %d\n",pInfo.pid); 
   printf("KTop debug : current selection pname = %s\n",pInfo.name);
   printf("KTop debug : current selection uname = %s\n",pInfo.uname);
 #endif

 if ( pTree_lastSelectionPid != pInfo.pid ) {
      QMessageBox::warning(this,"ktop",
                                "Selection changed !\n\n",
                                "Abort",0);
      pTree_lastSelectionPid = NONE;
      pTree->setCurrentItem(0);
      return;
 }

 int  err;
 char msg[256];
 sprintf(msg,"Kill process %d (%s - %s) ?\n",pInfo.pid,pInfo.name,pInfo.uname);

 switch( QMessageBox::warning(this,"ktop",
                                    msg,
                                   "Continue", "Abort",
                                    0, 1 )
       )
    { 
      case 0: // continue
          err = kill(pTree_lastSelectionPid ,SIGKILL);
          if ( err == -1 ) 
	       QMessageBox::warning(this,"ktop",
                                    "Kill error...\nSpecified process does not exists\nor permission denied.",
                                    "Ok", 0);
          pTree_lastSelectionPid = NONE;
          pTree_update();
          pTree->setCurrentItem(0);
          break;
      case 1: // abort
          break;
    }
}

/*-----------------------------------------------------------------------------
  Routine : TaskMan::pTree_getProcIcon
 -----------------------------------------------------------------------------*/
const QPixmap* TaskMan::pTree_getProcIcon( const char* pname )
{
 return TaskMan_getProcIcon(pname);
}










