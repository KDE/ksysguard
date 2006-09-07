/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2006 John Tapsell <john.tapsell@kdemail.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard has been written with some source code and ideas from
    ktop (<1.0). Early versions of ktop have been written by Bernd
    Johannes Wuebben <wuebben@math.cornell.edu> and Nicolas Leclercq
    <nicknet@planete.net>.

*/

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorAgent.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kurl.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <QSplitter>

#include "../version.h"
#include "SensorBrowser.h"
#include "Workspace.h"
#include "WorkSheet.h"

#include "ksysguard.h"



//Comment out to stop ksysguard from forking.  Good for debugging
//#define FORK_KSYSGUARD

static const char Description[] = I18N_NOOP( "KDE system guard" );
TopLevel* topLevel;

/**
  This is the constructor for the main widget. It sets up the menu and the
  TaskMan widget.
 */
TopLevel::TopLevel()
  : KMainWindow( 0 )
{
  QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportScriptableSlots);
  setPlainCaption( i18n( "System Monitor" ) );
  mTimerId = -1;

  mSplitter = new QSplitter( this );
  mSplitter->setOrientation( Qt::Horizontal );
  mSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  setCentralWidget( mSplitter );

  mSensorBrowser = new SensorBrowser( mSplitter, KSGRD::SensorMgr );

  mWorkSpace = new Workspace( mSplitter );
  connect( mWorkSpace, SIGNAL( setCaption( const QString&) ),
           SLOT( setCaption( const QString&) ) );
  connect( mWorkSpace, SIGNAL( currentChanged( int ) ),
           SLOT( currentTabChanged( int ) ) );
  connect( KSGRD::Style, SIGNAL( applyStyleToWorksheet() ), mWorkSpace,
           SLOT( applyStyle() ) );

  /* Create the status bar. It displays some information about the
   * number of processes and the memory consumption of the local
   * host. */
  const int STATUSBAR_STRETCH=1;
  statusBar()->insertItem( i18n( "Loading Processes Count.." ), 0, STATUSBAR_STRETCH );
  statusBar()->insertItem( i18n( "Loading Memory Totals.." ), 1, STATUSBAR_STRETCH );
  statusBar()->insertItem( i18n( "Loading Swap Totals.." ), 2, STATUSBAR_STRETCH);
  statusBar()->hide();

  // create actions for menu entries
  KAction *action = new KAction(KIcon("tab_new"),  i18n( "&New Worksheet..." ), actionCollection(), "new_worksheet" );
  connect(action, SIGNAL(triggered(bool)), mWorkSpace, SLOT( newWorkSheet() ));
  action = new KAction(KIcon("fileopen"),  i18n( "Import Worksheet..." ), actionCollection(), "import_worksheet" );
  connect(action, SIGNAL(triggered(bool)), mWorkSpace, SLOT( importWorkSheet() ));
  mTabRemoveAction = new KAction(KIcon("tab_remove"),  i18n( "&Remove Worksheet" ), actionCollection(), "remove_worksheet" );
  connect(mTabRemoveAction, SIGNAL(triggered(bool)), mWorkSpace, SLOT( removeWorkSheet() ));
  mTabExportAction = new KAction(KIcon("filesaveas"),  i18n( "&Export Worksheet..." ), actionCollection(), "export_worksheet" );
  connect(mTabExportAction, SIGNAL(triggered(bool)), mWorkSpace, SLOT( exportWorkSheet() ));

  KStdAction::quit( this, SLOT( close() ), actionCollection() );

  action = new KAction(KIcon("connect_established"),  i18n( "Monitor remote machine..." ), actionCollection(), "connect_host" );
  connect(action, SIGNAL(triggered(bool)), SLOT( connectHost() ));

  action = new KAction(KIcon("configure"),  i18n( "&Worksheet Properties" ), actionCollection(), "configure_sheet" );
  connect(action, SIGNAL(triggered(bool)), mWorkSpace, SLOT( configure() ));

  action = new KAction(KIcon("colorize"),  i18n( "Configure &Style..." ), actionCollection(), "configure_style" );
  connect(action, SIGNAL(triggered(bool)), SLOT( editStyle() ));

  setupGUI(ToolBar | Keys | StatusBar | Create);
}

void TopLevel::currentTabChanged(int index)
{
  kDebug() << "Current tab changed to " << index << endl;
  QWidget *wdg = mWorkSpace->widget(index);
  WorkSheet *sheet = (WorkSheet *)(wdg);
  Q_ASSERT(sheet);
  bool locked = !sheet || sheet->isLocked();
  mTabRemoveAction->setVisible(!locked);
  mTabExportAction->setVisible(!locked);
  mSensorBrowser->setVisible(!locked);
}


/*
 * DBUS Interface functions
 */

void TopLevel::showOnCurrentDesktop()
{
  KWin::setOnDesktop( winId(), KWin::currentDesktop() );
  kapp->updateUserTimestamp();
  KWin::forceActiveWindow( winId() );
}

void TopLevel::importWorkSheet( const QString &fileName )
{
  mWorkSpace->importWorkSheet( KUrl( fileName ) );
}

void TopLevel::removeWorkSheet( const QString &fileName )
{
  mWorkSpace->removeWorkSheet( fileName );
}

QStringList TopLevel::listSensors( const QString &hostName )
{
  return mSensorBrowser->listSensors( hostName );
}

QStringList TopLevel::listHosts()
{
  return mSensorBrowser->listHosts();
}

void TopLevel::showRequestedSheets()
{
  toolBar( "mainToolBar" )->hide();

  QList<int> sizes;
  sizes.append( 0 );
  sizes.append( 100 );
  mSplitter->setSizes( sizes );
}

void TopLevel::initStatusBar()
{
  KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd" );
  /* Request info about the swap space size and the units it is
   * measured in.  The requested info will be received by
   * answerReceived(). */
  KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/used?",
                                 (KSGRD::SensorClient*)this, 5 );
  updateStatusBar();

  KToggleAction *sb = dynamic_cast<KToggleAction*>(action("options_show_statusbar"));
  if (sb)
     connect(sb, SIGNAL(toggled(bool)), this, SLOT(updateStatusBar()));
}

void TopLevel::updateStatusBar()
{
  if ( mTimerId == -1 )
    mTimerId = startTimer( 2000 );

  // call timerEvent to fill the status bar with real values
  timerEvent( 0 );
}

void TopLevel::connectHost()
{
  KSGRD::SensorMgr->engageHost( "" );
}

void TopLevel::disconnectHost()
{
  mSensorBrowser->disconnect();
}

void TopLevel::editToolbars()
{
  saveMainWindowSettings( KGlobal::config() );
  KEditToolbar dlg( actionCollection() );
  connect( &dlg, SIGNAL( newToolbarConfig() ), this,
           SLOT( slotNewToolbarConfig() ) );

  dlg.exec();
}

void TopLevel::slotNewToolbarConfig()
{
  createGUI();
  applyMainWindowSettings( KGlobal::config() );
}

void TopLevel::editStyle()
{
  KSGRD::Style->configure();
}

bool TopLevel::event( QEvent *e )
{
  if ( e->type() == QEvent::User ) {
    /* Due to the asynchronous communication between ksysguard and its
     * back-ends, we sometimes need to show message boxes that were
     * triggered by objects that have died already. */
    KMessageBox::error( this, static_cast<KSGRD::SensorManager::MessageEvent*>(e)->message() );

    return true;
  }

  return KMainWindow::event( e );
}

void TopLevel::timerEvent( QTimerEvent* )
{
  if ( statusBar()->isVisibleTo( this ) ) {
    /* Request some info about the memory status. The requested
     * information will be received by answerReceived(). */
    KSGRD::SensorMgr->sendRequest( "localhost", "pscount",
                                   (KSGRD::SensorClient*)this, 0 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/physical/free",
                                   (KSGRD::SensorClient*)this, 1 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/physical/used",
                                   (KSGRD::SensorClient*)this, 2 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/free",
                                   (KSGRD::SensorClient*)this, 3 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/used",
                                   (KSGRD::SensorClient*)this, 4 );
  }
}

bool TopLevel::queryClose()
{
  if ( !mWorkSpace->saveOnQuit() )
    return false;

  saveProperties( KGlobal::config() );
  KGlobal::config()->sync();

  return true;
}

void TopLevel::readProperties( KConfig *cfg )
{

  /* we can ignore 'isMaximized' because we can't set the window
     maximized, so we save the coordinates instead */
  if ( cfg->readEntry( "isMinimized" , QVariant(false)).toBool() == true )
    showMinimized();

  QList<int> sizes = cfg->readEntry( "SplitterSizeList",QList<int>() );
  if ( sizes.isEmpty() ) {
    // start with a 30/70 ratio
    sizes.append( 30 );
    sizes.append( 70 );
  }
  mSplitter->setSizes( sizes );

  KSGRD::SensorMgr->readProperties( cfg );
  KSGRD::Style->readProperties( cfg );

  mWorkSpace->readProperties( cfg );

  applyMainWindowSettings( cfg );
}

void TopLevel::saveProperties( KConfig *cfg )
{
  cfg->writeEntry( "isMinimized", isMinimized() );
  cfg->writeEntry( "SplitterSizeList", mSplitter->sizes() );

  KSGRD::Style->saveProperties( cfg );
  KSGRD::SensorMgr->saveProperties( cfg );

  saveMainWindowSettings( cfg );
  mWorkSpace->saveProperties( cfg );
}

void TopLevel::answerReceived( int id, const QStringList &answerList )
{
  // we have received an answer from the daemon.
  QString answer;
  if(!answerList.isEmpty()) answer = answerList[0];
  QString s;
  static QString unit;
  static long mUsed = 0;
  static long mFree = 0;
  static long sUsed = 0;
  static long sFree = 0;

  switch ( id ) {
    case 0:
      s = i18np( "1 Process", "%n Processes", answer.toInt() );
      statusBar()->changeItem( s, 0 );
      break;

    case 1:
      mFree = answer.toLong();
      break;

    case 2:
      mUsed = answer.toLong();
      s = i18n( "Memory: %1 %2 used, %3 %4 free" ,
                KGlobal::locale()->formatNumber( mUsed, 0 ) ,  unit ,
                KGlobal::locale()->formatNumber( mFree, 0 ) ,  unit );
      statusBar()->changeItem( s, 1 );
      break;

    case 3:
      sFree = answer.toLong();
      setSwapInfo( sUsed, sFree, unit );
      break;

    case 4:
      sUsed = answer.toLong();
      setSwapInfo( sUsed, sFree, unit );
      break;

    case 5: {
      KSGRD::SensorIntegerInfo info( answer );
      unit = KSGRD::SensorMgr->translateUnit( info.unit() );
    }
  }
}

void TopLevel::setSwapInfo( long used, long free, const QString &unit )
{
  QString msg;
  if ( used == 0 && free == 0 ) // no swap available
    msg = i18n( "No swap space available" );
  else {
    msg = i18n( "Swap: %1 %2 used, %3 %4 free" ,
                KGlobal::locale()->formatNumber( used, 0 ) ,  unit ,
                KGlobal::locale()->formatNumber( free, 0 ) ,  unit );
  }

  statusBar()->changeItem( msg, 2 );
}

static const KCmdLineOptions options[] = {
  { "showprocesses", I18N_NOOP( "Show only process list of local host" ), 0 },
  { "+[worksheet]", I18N_NOOP( "Optional worksheet files to load" ), 0 },
  KCmdLineLastOption
};

/*
 * Once upon a time...
 */
int main( int argc, char** argv )
{
  // initpipe is used to keep the parent process around till the child
  // has registered with dbus
#ifdef FORK_KSYSGUARD
  int initpipe[ 2 ];
  pipe( initpipe );
#endif
  /* This forking will put ksysguard in it's own session not having a
   * controlling terminal attached to it. This prevents ssh from
   * using this terminal for password requests. Thus, you
   * need a ssh with ssh-askpass support to popup an X dialog to
   * enter the password. */
#ifdef FORK_KSYSGUARD
  pid_t pid;
  if ( ( pid = fork() ) < 0 )
    return -1;
  else
    if ( pid != 0 ) {
      close( initpipe[ 1 ] );

      // wait till init is complete
      char c;
      while( read( initpipe[ 0 ], &c, 1 ) < 0 );

      // then exit
      close( initpipe[ 0 ] );
      exit( 0 );
    }

  close( initpipe[ 0 ] );
  setsid();
#endif

  KAboutData aboutData( "ksysguard", I18N_NOOP( "System Monitor" ),
                        KSYSGUARD_VERSION, Description, KAboutData::License_GPL,
                        I18N_NOOP( "(c) 1996-2006 The KSysGuard Developers" ) );
  aboutData.addAuthor( "John Tapsell", "Current Maintainer", "john.tapsell@kdemail.org" );
  aboutData.addAuthor( "Chris Schlaeger", "Previous Maintainer", "cs@kde.org" );
  aboutData.addAuthor( "Greg Martyn", 0, "greg.martyn@gmail.com" );
  aboutData.addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  aboutData.addAuthor( "Nicolas Leclercq", 0, "nicknet@planete.net" );
  aboutData.addAuthor( "Alex Sanda", 0, "alex@darkstart.ping.at" );
  aboutData.addAuthor( "Bernd Johannes Wuebben", 0, "wuebben@math.cornell.edu" );
  aboutData.addAuthor( "Ralf Mueller", 0, "rlaf@bj-ig.de" );
  aboutData.addAuthor( "Hamish Rodda", 0, "rodda@kde.org" );
  aboutData.addAuthor( "Torsten Kasch", I18N_NOOP( "Solaris Support\n"
                       "Parts derived (by permission) from the sunos5\n"
                       "module of William LeFebvre's \"top\" utility." ),
                       "tk@Genetik.Uni-Bielefeld.DE" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  // initialize KDE application
  KApplication *app = new KApplication;

  KSGRD::SensorMgr = new KSGRD::SensorManager();
  KSGRD::Style = new KSGRD::StyleEngine();


#ifdef FORK_KSYSGUARD
  char c = 0;
  write( initpipe[ 1 ], &c, 1 );
  close( initpipe[ 1 ] );
#endif
  topLevel = new TopLevel();

  // create top-level widget
  if ( app->isSessionRestored() )
    topLevel->restore( 1 );
  else
  {
    topLevel->readProperties( KGlobal::config() );
  }

  topLevel->initStatusBar();
  topLevel->show();
  KSGRD::SensorMgr->setBroadcaster( topLevel );

  // run the application
  int result = app->exec();

  delete KSGRD::Style;
  delete KSGRD::SensorMgr;
  delete app;

  return result;
}

#include "ksysguard.moc"
