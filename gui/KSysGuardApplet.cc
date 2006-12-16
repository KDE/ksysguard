/*
    KSysGuard, the KDE System Guard

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

*/

#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QCursor>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QFrame>
#include <QtGui/QResizeEvent>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QTimer>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kmenu.h>

#include <ksgrd/SensorClient.h>
#include <ksgrd/SensorManager.h>

#include "DancingBars.h"
#include "FancyPlotter.h"
#include "KSGAppletSettings.h"
#include "MultiMeter.h"
#include "StyleEngine.h"

#include "KSysGuardApplet.h"
#include "SharedSettings.h"

#include "utils.h"

extern "C"
{
  KDE_EXPORT KPanelApplet* init( QWidget *parent, const QString& configFile )
  {
    KGlobal::locale()->insertCatalog( "ksysguard" );
    return new KSysGuardApplet( configFile, Plasma::Normal,
                                Plasma::Preferences, parent );
  }
}

KSysGuardApplet::KSysGuardApplet( const QString& configFile, Plasma::Type type,
                                  int actions, QWidget *parent )
  : KPanelApplet( configFile, type, actions, parent)
{
  mSettingsDlg = 0;

  KSGRD::SensorMgr = new KSGRD::SensorManager();

  KSGRD::Style = new KSGRD::StyleEngine();

  mDockCount = 1;
  mDockList = new QWidget*[ mDockCount ];

  mSizeRatio = 1.0;
  mSharedSettings.isApplet = true;
  addEmptyDisplay( mDockList, 0 );

  setUpdateInterval( 2 );

  load();

  setAcceptDrops( true );
}

KSysGuardApplet::~KSysGuardApplet()
{
  save();

  delete mSettingsDlg;
  mSettingsDlg = 0;

  delete KSGRD::Style;
  delete KSGRD::SensorMgr;
  KSGRD::SensorMgr = 0;
}

int KSysGuardApplet::widthForHeight( int height ) const
{
  return ( (int) ( height * mSizeRatio + 0.5 ) * mDockCount );
}

int KSysGuardApplet::heightForWidth( int width ) const
{
  return ( (int) ( width * mSizeRatio + 0.5 ) * mDockCount );
}

void KSysGuardApplet::resizeEvent( QResizeEvent* )
{
  layout();
}

void KSysGuardApplet::preferences()
{
  mSettingsDlg = new KSGAppletSettings( this );

  connect( mSettingsDlg, SIGNAL( applyClicked() ), SLOT( applySettings() ) );
  connect( mSettingsDlg, SIGNAL( okClicked() ), SLOT( applySettings() ) );
  connect( mSettingsDlg, SIGNAL( finished() ), SLOT( preferencesFinished() ) );

  mSettingsDlg->setNumDisplay( mDockCount );
  mSettingsDlg->setSizeRatio( (int) ( mSizeRatio * 100.0 + 0.5 ) );
  mSettingsDlg->setUpdateInterval( updateInterval() );

  mSettingsDlg->show();
}
void KSysGuardApplet::preferencesFinished()
{
  mSettingsDlg->delayedDestruct();
  mSettingsDlg = 0;
}

void KSysGuardApplet::applySettings()
{
  setUpdateInterval( mSettingsDlg->updateInterval() );
  mSizeRatio = mSettingsDlg->sizeRatio() / 100.0;
  resizeDocks( mSettingsDlg->numDisplay() );

  save();
}

void KSysGuardApplet::layout()
{
  if ( orientation() == Qt::Horizontal ) {
    int h = height();
    int w = (int) ( h * mSizeRatio + 0.5 );
    for ( uint i = 0; i < mDockCount; ++i )
      if ( mDockList[ i ] )
        mDockList[ i ]->setGeometry( i * w, 0, w, h );
  } else {
    int w = width();
    int h = (int) ( w * mSizeRatio + 0.5 );
    for ( uint i = 0; i < mDockCount; ++i )
      if ( mDockList[ i ] )
        mDockList[ i ]->setGeometry( 0, i * h, w, h );
  }
}

int KSysGuardApplet::findDock( const QPoint& point )
{
  if ( orientation() == Qt::Horizontal )
    return ( point.x() / (int) ( height() * mSizeRatio + 0.5 ) );
  else
    return ( point.y() / (int) ( width() * mSizeRatio + 0.5 ) );
}

void KSysGuardApplet::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasText() )
    event->acceptProposedAction();
}

void KSysGuardApplet::dropEvent( QDropEvent *event )
{
  if ( !event->mimeData()->hasText() )
    return;

  const QString dragObject = event->mimeData()->text();

  // The host name, sensor name and type are separated by a ' '.
  QStringList parts = dragObject.split( ' ');

  QString hostName = parts[ 0 ];
  QString sensorName = parts[ 1 ];
  QString sensorType = parts[ 2 ];
  QString sensorDescr = parts[ 3 ];

  if ( hostName.isEmpty() || sensorName.isEmpty() || sensorType.isEmpty() )
    return;

  int dock = findDock( event->pos() );
  if ( QLatin1String( "QFrame" ) == mDockList[ dock ]->metaObject()->className() ) {
    if ( sensorType == "integer" || sensorType == "float" ) {
      KMenu popup;
      KSGRD::SensorDisplay *wdg = 0;

      popup.addTitle( i18n( "Select Display Type" ) );
      QAction *a1 = popup.addAction( i18n( "&Signal Plotter" ) );
      QAction *a2 = popup.addAction( i18n( "&Multimeter" ) );
      QAction *a3 = popup.addAction( i18n( "&Dancing Bars" ) );
      QAction *execed = popup.exec( QCursor::pos() );
      if (execed == a1)
          wdg = new FancyPlotter( this, sensorDescr, &mSharedSettings );
      else if (execed == a2)
          wdg = new MultiMeter( this, sensorDescr, &mSharedSettings );
      else if (execed == a3)
          wdg = new DancingBars( this, sensorDescr, &mSharedSettings );

      if ( wdg ) {
        delete mDockList[ dock ];
        mDockList[ dock ] = wdg;
        layout();

        wdg->setDeleteNotifier( this );

        mDockList[ dock ]->show();
      }
    } else {
      KMessageBox::sorry( this,
        i18n( "The KSysGuard applet does not support displaying of "
              "this type of sensor. Please choose another sensor." ) );

      layout();
    }
  }

  if ( QLatin1String( "QFrame" ) != mDockList[ dock ]->metaObject()->className() )
    ((KSGRD::SensorDisplay*)mDockList[ dock ])->
                addSensor( hostName, sensorName, sensorType, sensorDescr );

  save();
}

bool KSysGuardApplet::event( QEvent *e )
{
  if ( e->type() == QEvent::User ) {
    if ( KMessageBox::warningContinueCancel( this,
         i18n( "Do you really want to delete the display?" ), i18n("Delete Display"),
               KStdGuiItem::del() ) == KMessageBox::Continue ) {
      // SensorDisplays send out this event if they want to be removed.
      KSGRD::SensorDisplay::DeleteEvent *event = static_cast<KSGRD::SensorDisplay::DeleteEvent*>( e );
      removeDisplay( event->display() );
      save();

      return true;
    }
  }

  return KPanelApplet::event( e );
}

void KSysGuardApplet::removeDisplay( KSGRD::SensorDisplay *display )
{
  for ( uint i = 0; i < mDockCount; ++i )
    if ( display == mDockList[i] ) {
      delete mDockList[ i ];

      addEmptyDisplay( mDockList, i );
      return;
    }
}

void KSysGuardApplet::resizeDocks( uint newDockCount )
{
  /* This function alters the number of available docks. The number of
   * docks can be increased or decreased. We try to preserve existing
   * docks if possible. */

  if ( newDockCount == mDockCount ) {
    emit updateLayout();
    return;
  }

  // Create and initialize new dock array.
  QWidget** tmp = new QWidget*[ newDockCount ];

  uint i;
  for ( i = 0; ( i < newDockCount ) && ( i < mDockCount ); ++i )
    tmp[ i ] = mDockList[ i ];

  for ( i = newDockCount; i < mDockCount; ++i )
    if ( mDockList[ i ] )
      delete mDockList[ i ];

  for ( i = mDockCount; i < newDockCount; ++i )
    addEmptyDisplay( tmp, i );

  delete [] mDockList;

  mDockList = tmp;
  mDockCount = newDockCount;

  emit updateLayout();
}

bool KSysGuardApplet::load()
{
  KStandardDirs* kstd = KGlobal::dirs();
  QString fileName = kstd->findResource( "data", "ksysguard/KSysGuardApplet.xml" );

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    KMessageBox::sorry( this, i18n( "Cannot open the file %1." ,  fileName ) );
    return false;
  }

  // Parse the XML file.
  QDomDocument doc;

  // Read in file and check for a valid XML header.
  if ( !doc.setContent( &file ) ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain valid XML." ,
                          fileName ) );
    return false;
  }

  // Check for proper document type.
  if ( doc.doctype().name() != "KSysGuardApplet" ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain a valid applet "
                        "definition, which must have a document type 'KSysGuardApplet'." ,
                          fileName ) );
    return false;
  }

  QDomElement element = doc.documentElement();
  bool ok;
  uint count = element.attribute( "dockCnt" ).toUInt( &ok );
  if ( !ok )
    count = 1;

  mSizeRatio = element.attribute( "sizeRatio" ).toDouble( &ok );
  if ( !ok )
    mSizeRatio = 1.0;

  unsigned int interval = element.attribute( "interval").toUInt( &ok );
  if ( !ok )
    interval = 2;
  
  setUpdateInterval( interval );

  resizeDocks( count );

  /* Load lists of hosts that are needed for the work sheet and try
   * to establish a connection. */
  QDomNodeList dnList = element.elementsByTagName( "host" );
  int i;
  for ( i = 0; i < dnList.count(); ++i ) {
    QDomElement element = dnList.item( i ).toElement();
    int port = element.attribute( "port" ).toInt( &ok );
    if ( !ok )
      port = -1;

    KSGRD::SensorMgr->engage( element.attribute( "name" ),
                              element.attribute( "shell" ),
                              element.attribute( "command" ), port );
  }
  //if no hosts are specified, at least connect to localhost
  if(dnList.count() == 0)
    KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd", -1);
  // Load the displays and place them into the work sheet.
  dnList = element.elementsByTagName( "display" );
  for ( i = 0; i < dnList.count(); ++i ) {
    QDomElement element = dnList.item( i ).toElement();
    uint dock = element.attribute( "dock" ).toUInt();
    if ( i >= (int)mDockCount ) {
      kDebug (1215) << "Dock number " << i << " out of range "
                     << mDockCount << endl;
      return false;
    }

    QString classType = element.attribute( "class" );
    KSGRD::SensorDisplay* newDisplay;
    if ( classType == "FancyPlotter" )
      newDisplay = new FancyPlotter( this, i18n("Dummy"), &mSharedSettings );
    else if ( classType == "MultiMeter" )
      newDisplay = new MultiMeter( this, i18n("Dummy"), &mSharedSettings );
    else if ( classType == "DancingBars" )
      newDisplay = new DancingBars( this, i18n("Dummy"), &mSharedSettings );
    else {
      KMessageBox::sorry( this, i18n( "The KSysGuard applet does not support displaying of "
                          "this type of sensor. Please choose another sensor." ) );
      return false;
    }

    connect(&mTimer, SIGNAL( timeout()), newDisplay, SLOT( timerTick()));
    newDisplay->setDeleteNotifier( this );

    // load display specific settings
    if ( !newDisplay->restoreSettings( element ) ) {
      delete newDisplay;
      return false;
    }

    delete mDockList[ dock ];
    mDockList[ dock ] = newDisplay;

  }

  return true;
}

bool KSysGuardApplet::save()
{
  // Parse the XML file.
  QDomDocument doc( "KSysGuardApplet" );
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

  // save work sheet information
  QDomElement ws = doc.createElement( "WorkSheet" );
  doc.appendChild( ws );
  ws.setAttribute( "dockCnt", mDockCount );
  ws.setAttribute( "sizeRatio", mSizeRatio );
  ws.setAttribute( "interval", updateInterval() );

  QStringList hosts;
  uint i;
  for ( i = 0; i < mDockCount; ++i )
    if ( QLatin1String( "QFrame" ) != mDockList[ i ]->metaObject()->className() )
      ((KSGRD::SensorDisplay*)mDockList[ i ])->hosts( hosts );

  // save host information (name, shell, etc.)
  QStringList::Iterator it;
  for ( it = hosts.begin(); it != hosts.end(); ++it ) {
    QString shell, command;
    int port;

    if ( KSGRD::SensorMgr->hostInfo( *it, shell, command, port ) ) {
      QDomElement host = doc.createElement( "host" );
      ws.appendChild( host );
      host.setAttribute( "name", *it );
      host.setAttribute( "shell", shell );
      host.setAttribute( "command", command );
      host.setAttribute( "port", port );
    }
  }

  for ( i = 0; i < mDockCount; ++i )
    if ( QLatin1String( "QFrame" ) != mDockList[ i ]->metaObject()->className() ) {
      QDomElement element = doc.createElement( "display" );
      ws.appendChild( element );
      element.setAttribute( "dock", i );
      element.setAttribute( "class", mDockList[ i ]->metaObject()->className() );

      ((KSGRD::SensorDisplay*)mDockList[ i ])->saveSettings( doc, element );
    }

  KStandardDirs* kstd = KGlobal::dirs();
  QString fileName = kstd->saveLocation( "data", "ksysguard" );
  fileName += "/KSysGuardApplet.xml";

  KSaveFile file( fileName );

  if ( !file.open() )
  {
    KMessageBox::sorry( this, i18n( "Cannot save file %1", fileName ) );
    return false;
  }
  file.setPermissions(QFile::ReadUser|QFile::WriteUser|QFile::ReadGroup|QFile::ReadOther);
  
  QTextStream ts ( &file );
  ts.setCodec( "UTF-8" );
  ts << doc;
  ts.flush();
  file.finalize(); //check for error here?

  return true;
}

void KSysGuardApplet::addEmptyDisplay( QWidget **dock, uint pos )
{
  dock[ pos ] = new QFrame( this );
  ((QFrame*)dock[ pos ])->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  dock[ pos ]->setToolTip(
                 i18n( "Drag sensors from the KDE System Guard into this cell." ) );

  layout();
  if ( isVisible() )
    dock[ pos ]->show();
}

void KSysGuardApplet::setUpdateInterval( unsigned int secs)
{
  if(secs == 0)
    mTimer.stop();
  else {
    mTimer.setInterval(secs*1000);
    mTimer.start();
  }
}
int KSysGuardApplet::updateInterval() const
{
  if(mTimer.isActive())
    return mTimer.interval();
  else
    return 0;
}

#include "KSysGuardApplet.moc"
