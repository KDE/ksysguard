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
#include <kactioncollection.h>
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
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
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
#include "StyleEngine.h"
#include "HostConnector.h"

#include "ksysguard.h"



//Comment out to stop ksysguard from forking.  Good for debugging
//#define FORK_KSYSGUARD

static const char Description[] = I18N_NOOP( "KDE System Monitor" );
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

  mSensorBrowser = 0;

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
  statusBar()->insertItem( i18n( "Loading CPU Stat.." ), 1, STATUSBAR_STRETCH );
  statusBar()->insertItem( i18n( "Loading Memory Totals.." ), 2, STATUSBAR_STRETCH );
  statusBar()->insertItem( i18n( "Loading Swap Totals.." ), 3, STATUSBAR_STRETCH);
  statusBar()->hide();

  // create actions for menu entries
  QAction *action = actionCollection()->addAction("new_worksheet");
  action->setIcon(KIcon("tab_new"));
  action->setText(i18n( "&New Worksheet..." ));
  connect(action, SIGNAL(triggered(bool)), mWorkSpace, SLOT( newWorkSheet() ));
  action = actionCollection()->addAction("import_worksheet");
  action->setIcon(KIcon("fileopen") );
  action->setText(i18n( "Import Worksheet..." ));
  connect(action, SIGNAL(triggered(bool)), mWorkSpace, SLOT( importWorkSheet() ));
  mTabRemoveAction = actionCollection()->addAction( "remove_worksheet" );
  mTabRemoveAction->setIcon( KIcon("tab_remove") );
  mTabRemoveAction->setText( i18n( "&Remove Worksheet" ) );
  connect(mTabRemoveAction, SIGNAL(triggered(bool)), mWorkSpace, SLOT( removeWorkSheet() ));
  mTabExportAction = actionCollection()->addAction( "export_worksheet" );
  mTabExportAction->setIcon( KIcon("filesaveas") );
  mTabExportAction->setText( i18n( "&Export Worksheet..." ) );
  connect(mTabExportAction, SIGNAL(triggered(bool)), mWorkSpace, SLOT( exportWorkSheet() ));

  KStandardAction::quit( this, SLOT( close() ), actionCollection() );

  mMonitorRemoteAction = actionCollection()->addAction( "connect_host" );
  mMonitorRemoteAction->setIcon( KIcon("connect_established") );
  mMonitorRemoteAction->setText( i18n( "Monitor remote machine..." ) );
  connect(mMonitorRemoteAction, SIGNAL(triggered(bool)), SLOT( connectHost() ));

  action = actionCollection()->addAction( "configure_sheet" );
  action->setIcon( KIcon("configure") );
  action->setText( i18n( "&Worksheet Properties" ) );
  connect(action, SIGNAL(triggered(bool)), mWorkSpace, SLOT( configure() ));

  mColorizeAction = actionCollection()->addAction( "configure_style" );
  mColorizeAction->setIcon( KIcon("colorize") );
  mColorizeAction->setText( i18n( "Configure &Style..." ) );
  connect(mColorizeAction, SIGNAL(triggered(bool)), SLOT( editStyle() ));

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
  mMonitorRemoteAction->setVisible(!locked);
  mColorizeAction->setVisible(!locked);

  if(!locked && !mSensorBrowser) {
    startSensorBrowserWidget();
  }
  if(mSensorBrowser) {
    if(mSensorBrowser->isVisible() && locked) //going from visible to not visible to save the state
      mSplitterSize = mSplitter->sizes();
    mSensorBrowser->setVisible(!locked);

  }
}
void TopLevel::startSensorBrowserWidget()
{
  if(mSensorBrowser) return;
  kDebug() << "Creating sensor browser" << endl;
  mSensorBrowser = new SensorBrowserWidget( 0, KSGRD::SensorMgr );
  mSplitter->insertWidget(0,mSensorBrowser);
  mSplitter->setSizes( mSplitterSize );
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
  startSensorBrowserWidget();
  return mSensorBrowser->listSensors( hostName );
}

QStringList TopLevel::listHosts()
{
  startSensorBrowserWidget();
  return mSensorBrowser->listHosts();
}

void TopLevel::initStatusBar()
{
  KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd" );
  /* Request info about the swap space size and the units it is
   * measured in.  The requested info will be received by
   * answerReceived(). */
  KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/used?",
                                 (KSGRD::SensorClient*)this, 7 );
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
  HostConnector hostConnector( this );

//  hostConnector.setHostNames( mHostList );
//  hostConnector.setCommands( mCommandList );

//  hostConnector.setCurrentHostName( "" );

  if ( !hostConnector.exec() )
    return;

//  mHostList = hostConnector.hostNames();
//  mCommandList = hostConnector.commands();

  QString shell = "";
  QString command = "";
  int port = -1;

  /* Check which radio button is selected and set parameters
   * appropriately. */
  if ( hostConnector.useSsh() )
    shell = "ssh";
  else if ( hostConnector.useRsh() )
    shell = "rsh";
  else if ( hostConnector.useDaemon() )
    port = hostConnector.port();
  else
    command = hostConnector.currentCommand();

  KSGRD::SensorMgr->engage( hostConnector.currentHostName(), shell, command, port );
}

void TopLevel::disconnectHost()
{
  if(mSensorBrowser)
    mSensorBrowser->disconnect();
}

void TopLevel::editToolbars()
{
  saveMainWindowSettings( KGlobal::config().data() );
  KEditToolbar dlg( actionCollection() );
  connect( &dlg, SIGNAL( newToolbarConfig() ), this,
           SLOT( slotNewToolbarConfig() ) );

  dlg.exec();
}

void TopLevel::slotNewToolbarConfig()
{
  createGUI();
  applyMainWindowSettings( KGlobal::config().data() );
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
    KSGRD::SensorMgr->sendRequest( "localhost", "cpu/idle",
                                   (KSGRD::SensorClient*)this, 1 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/physical/free",
                                   (KSGRD::SensorClient*)this, 2 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/physical/used",
                                   (KSGRD::SensorClient*)this, 3 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/physical/application",
                                   (KSGRD::SensorClient*)this, 4 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/free",
                                   (KSGRD::SensorClient*)this, 5 );
    KSGRD::SensorMgr->sendRequest( "localhost", "mem/swap/used",
                                   (KSGRD::SensorClient*)this, 6 );
  }
}

bool TopLevel::queryClose()
{
  if ( !mWorkSpace->saveOnQuit() )
    return false;

  saveProperties( KGlobal::config().data() );
  KGlobal::config()->sync();

  return true;
}

void TopLevel::readProperties( KConfig *cfg )
{

  /* we can ignore 'isMaximized' because we can't set the window
     maximized, so we save the coordinates instead */
  if ( cfg->readEntry( "isMinimized" , false) == true )
    showMinimized();

  mSplitterSize = cfg->readEntry( "SplitterSizeList",QList<int>() );
  if ( mSplitterSize.isEmpty() ) {
    // start with a 30/70 ratio
    mSplitterSize.append( 10 );
    mSplitterSize.append( 90 );
  }

  KSGRD::SensorMgr->readProperties( cfg );
  KSGRD::Style->readProperties( cfg );

  mWorkSpace->readProperties( cfg );

  applyMainWindowSettings( cfg );
}

void TopLevel::saveProperties( KConfig *cfg )
{
  cfg->writeEntry( "isMinimized", isMinimized() );

  if(mSensorBrowser && mSensorBrowser->isVisible())
    cfg->writeEntry( "SplitterSizeList",  mSplitter->sizes());
  else if(mSplitterSize.size() == 2 && mSplitterSize.value(0) != 0 && mSplitterSize.value(1) != 0)
    cfg->writeEntry( "SplitterSizeList", mSplitterSize );

  KSGRD::Style->saveProperties( cfg );
  KSGRD::SensorMgr->saveProperties( cfg );

  saveMainWindowSettings( cfg );
  mWorkSpace->saveProperties( cfg );
}

void TopLevel::answerReceived( int id, const QList<QByteArray> &answerList )
{
  // we have received an answer from the daemon.
  QByteArray answer;
  if(!answerList.isEmpty()) answer = answerList[0];
  QString s;
  static QString unit;
  static long mFree = 0;
  static long mUsedApplication = 0;
  static long mUsedTotal = 0;
  static long sUsed = 0;
  static long sFree = 0;

  switch ( id ) {
    case 0:
      s = i18n( " %1 Processes ", answer.toInt() );
      statusBar()->changeItem( s, 0 );
      break;

    case 1:
      s = i18n( " CPU: %1% ", (int) (100 - answer.toFloat()) );
      statusBar()->changeItem( s, 1 );
      break;

    case 2:
      mFree = answer.toLong();
      break;

    case 3:
      mUsedTotal = answer.toLong();
      break;

    case 4:
      mUsedApplication = answer.toLong();
      s = i18n( " Memory: %1 / %2 " ,
                KGlobal::locale()->formatByteSize( mUsedApplication*1024),
                KGlobal::locale()->formatByteSize( (mFree+mUsedTotal)*1024 ) );
      statusBar()->changeItem( s, 2 );
      setSwapInfo( sUsed, sFree, unit );
      break;

    case 5:
      sFree = answer.toLong();
      break;

    case 6:
      sUsed = answer.toLong();
      break;

    case 7: {
      KSGRD::SensorIntegerInfo info( answer );
      unit = KSGRD::SensorMgr->translateUnit( info.unit() );
      break;
    }
  }
}

void TopLevel::setSwapInfo( long used, long free, const QString & )
{
  QString msg;
  if ( used == 0 && free == 0 ) // no swap available
    msg = i18n( " No swap space available " );
  else {
    msg = i18n( " Swap: %1 / %2 " ,
                KGlobal::locale()->formatByteSize( used*1024 ),
                KGlobal::locale()->formatByteSize( free*1024) );
  }

  statusBar()->changeItem( msg, 3 );
}

static const KCmdLineOptions options[] = {
  { "showprocesses", I18N_NOOP( "Show only process list of local host" ), 0 },
  { "+[worksheet]", I18N_NOOP( "Optional worksheet files to load" ), 0 },
  KCmdLineLastOption
};

/*
 * Once upon a time...
 */
extern "C" KDE_EXPORT int kdemain( int argc, char** argv )
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
                        I18N_NOOP( "(c) 1996-2006 The KDE System Monitor Developers" ) );
  aboutData.addAuthor( "John Tapsell", "Current Maintainer", "john.tapsell@kde.org" );
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
    topLevel->readProperties( KGlobal::config().data() );
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
