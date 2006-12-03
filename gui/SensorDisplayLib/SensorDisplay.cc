/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

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

#include <QCheckBox>
#include <qdom.h>
#include <QMenu>
#include <QSpinBox>

#include <QBitmap>
#include <QPixmap>
#include <Q3PtrList>
#include <QEvent>
#include <QTimerEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QCustomEvent>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurl.h>
#include <kservice.h>

#include "ksgrd/SensorManager.h"
#include "TimerSettings.h"

#include "SensorDisplay.h"

#define NONE -1

using namespace KSGRD;

SensorDisplay::DeleteEvent::DeleteEvent( SensorDisplay *display )
  : QEvent( QEvent::User ), mDisplay( display )
{
}

SensorDisplay* SensorDisplay::DeleteEvent::display() const
{
  return mDisplay;
}

SensorDisplay::SensorDisplay( QWidget *parent, const QString &title, SharedSettings *workSheetSettings)
  : QWidget( parent )
{
  mSharedSettings = workSheetSettings;

  // default interval is 2 seconds.
  mUpdateInterval = 2;
  mUseGlobalUpdateInterval = true;
  mShowUnit = false;
  mTimerId = NONE;
  mErrorIndicator = 0;
  mPlotterWdg = 0;

  setTimerOn( true );
  this->setWhatsThis( "dummy" );

  setMinimumSize( 16, 16 );
  setSensorOk( false );
  setTitle(title);


  /* Let's call updateWhatsThis() in case the derived class does not do
   * this. */
  updateWhatsThis();
}

SensorDisplay::~SensorDisplay()
{
  if ( SensorMgr != 0 )
    SensorMgr->disconnectClient( this );

  killTimer( mTimerId );
}

void SensorDisplay::registerSensor( SensorProperties *sp )
{
  mSensors.append( sp );
}

void SensorDisplay::unregisterSensor( uint pos )
{
  delete mSensors.takeAt( pos );
}

void SensorDisplay::configureUpdateInterval()
{
  TimerSettings dlg( this );

  dlg.setUseGlobalUpdate( mUseGlobalUpdateInterval );
  dlg.setInterval( mUpdateInterval );

  if ( dlg.exec() ) {
    if ( dlg.useGlobalUpdate() ) {
      mUseGlobalUpdateInterval = true;
      // FIXME: Get update interval from parent
      setUpdateInterval( 2 );
    } else {
      mUseGlobalUpdateInterval = false;
      setUpdateInterval( dlg.interval() );
    }
  }
}

void SensorDisplay::timerEvent( QTimerEvent* )
{
  int i = 0;

  foreach( SensorProperties *s, mSensors)
    sendRequest( s->hostName(), s->name(), i++ );
}

bool SensorDisplay::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::MouseButtonPress &&
     ( (QMouseEvent*)event)->button() == Qt::RightButton ) {

    QMenu pm;
    QAction *action = 0;
    if ( mSharedSettings->isApplet ) {
      action = pm.addAction( i18n( "Launch &System Guard") );
      action->setData( 1 );
      pm.addSeparator();
    }

    if ( hasSettingsDialog() ) {
      action = pm.addAction( i18n( "&Properties" ) );
      action->setData( 2 );
    }
    if(!mSharedSettings->locked) {  
      action = pm.addAction( i18n( "&Remove Display" ) );
      action->setData( 3 );
    }
    action = pm.addSeparator();
    action = pm.addAction( i18n( "&Change Update Interval..." ) );
    action->setData( 4 );

    if ( !timerOn() ) {
      action = pm.addAction( i18n( "Re&sume" ) );
      action->setData( 5 );
    } else {
      action = pm.addAction( i18n( "P&ause" ) );
      action->setData( 6 );
    }

    action = pm.exec( QCursor::pos() );
    if ( action ) {
      switch ( action->data().toInt() ) {
        case 1:
          KRun::run(*KService::serviceByDesktopName("ksysguard"), KUrl::List(), topLevelWidget());
          break;
        case 2:
          configureSettings();
          break;
        case 3: {
            if ( mDeleteNotifier ) {
              DeleteEvent *event = new DeleteEvent( this );
              kapp->postEvent( mDeleteNotifier, event );
            }
          }
          break;
        case 4:
          configureUpdateInterval();
          break;
        case 5:
          setTimerOn( true );
          break;
        case 6:
          setTimerOn( false );
          break;
      }
    }

    return true;
  } 

  return QWidget::eventFilter( object, event );
}
void SensorDisplay::sendRequest( const QString &hostName,
                                 const QString &command, int id )
{
  if ( !SensorMgr->sendRequest( hostName, command, (SensorClient*)this, id ) )
    sensorError( id, true );
}

void SensorDisplay::sensorError( int sensorId, bool err )
{
  if ( sensorId >= (int)mSensors.count() || sensorId < 0 )
    return;

  if ( err == mSensors.at( sensorId )->isOk() ) {
    // this happens only when the sensorOk status needs to be changed.
    mSensors.at( sensorId )->setIsOk( !err );
  }

  bool ok = true;
  for ( uint i = 0; i < (uint)mSensors.count(); ++i )
    if ( !mSensors.at( i )->isOk() ) {
      ok = false;
      break;
    }

  setSensorOk( ok );
}

void SensorDisplay::updateWhatsThis()
{
  this->setWhatsThis( i18n(
    "<qt><p>This is a sensor display. To customize a sensor display click "
    "the right mouse button here "
    "and select the <i>Properties</i> entry from the popup "
    "menu. Select <i>Remove</i> to delete the display from the worksheet."
    "</p>%1</qt>" ,  additionalWhatsThis() ) );
}

void SensorDisplay::hosts( QStringList& list )
{
  foreach( SensorProperties *s, mSensors)
    if ( !list.contains( s->hostName() ) )
      list.append( s->hostName() );
}

QColor SensorDisplay::restoreColor( QDomElement &element, const QString &attr,
                                    const QColor& fallback )
{
  bool ok;
  uint c = element.attribute( attr ).toUInt( &ok );
  if ( !ok )
    return fallback;
  else
    return QColor( (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF );
}

void SensorDisplay::saveColor( QDomElement &element, const QString &attr,
                               const QColor &color )
{
  int r, g, b;
  color.getRgb( &r, &g, &b );
  element.setAttribute( attr, (r << 16) | (g << 8) | b );
}

bool SensorDisplay::addSensor( const QString &hostName, const QString &name,
                               const QString &type, const QString &description )
{
  registerSensor( new SensorProperties( hostName, name, type, description ) );
  return true;
}

bool SensorDisplay::removeSensor( uint pos )
{
  unregisterSensor( pos );
  return true;
}

void SensorDisplay::setUpdateInterval( uint interval )
{
  bool timerActive = timerOn();

  if ( timerActive )
    setTimerOn( false );

  mUpdateInterval = interval;

  if ( timerActive )
    setTimerOn( true );
}

bool SensorDisplay::hasSettingsDialog() const
{
  return false;
}

void SensorDisplay::configureSettings()
{
}

void SensorDisplay::setUseGlobalUpdateInterval( bool value )
{
  mUseGlobalUpdateInterval = value;
}

bool SensorDisplay::useGlobalUpdateInterval() const
{
  return mUseGlobalUpdateInterval;
}

QString SensorDisplay::additionalWhatsThis()
{
  return QString();
}

void SensorDisplay::sensorLost( int reqId )
{
  sensorError( reqId, true );
}

void SensorDisplay::setDeleteNotifier( QObject *object )
{
  mDeleteNotifier = object;
}

bool SensorDisplay::restoreSettings( QDomElement &element )
{
  mShowUnit = element.attribute( "showUnit", "0" ).toInt();
  setUnit( element.attribute( "unit", QString() ) );
  setTitle( element.attribute( "title", QString() ) );

  if ( element.attribute( "updateInterval" ) != QString() ) {
    mUseGlobalUpdateInterval = false;
    setUpdateInterval( element.attribute( "updateInterval", "2" ).toInt() );
  } else {
    mUseGlobalUpdateInterval = true;
    // FIXME: Get update interval from parent
    setUpdateInterval( 2 );
  }

  if ( element.attribute( "pause", "0" ).toInt() == 0 )
    setTimerOn( true );
  else
    setTimerOn( false );

  return true;
}

bool SensorDisplay::saveSettings( QDomDocument&, QDomElement &element )
{
  element.setAttribute( "title", title() );
  element.setAttribute( "unit", unit() );
  element.setAttribute( "showUnit", mShowUnit );

  if ( mUseGlobalUpdateInterval )
    element.setAttribute( "globalUpdate", "1" );
  else {
    element.setAttribute( "globalUpdate", "0" );
    element.setAttribute( "updateInterval", mUpdateInterval );
  }

  if ( !timerOn() )
    element.setAttribute( "pause", 1 );
  else
    element.setAttribute( "pause", 0 );

  return true;
}

void SensorDisplay::setTimerOn( bool on )
{
  if ( on ) {
    if ( mTimerId == NONE )
      mTimerId = startTimer( mUpdateInterval * 1000 );
  } else {
    if ( mTimerId != NONE ) {
      killTimer( mTimerId );
      mTimerId = NONE;
    }
  }
}

bool SensorDisplay::timerOn() const
{
  return ( mTimerId != NONE );
}

QList<SensorProperties *> &SensorDisplay::sensors()
{
  return mSensors;
}

void SensorDisplay::rmbPressed()
{
  emit showPopupMenu( this );
}

void SensorDisplay::applySettings()
{
}

void SensorDisplay::applyStyle()
{
}

void SensorDisplay::setSensorOk( bool ok )
{
  if ( ok ) {
    delete mErrorIndicator;
    mErrorIndicator = 0;
  } else {
    if ( mErrorIndicator )
      return;

    KIconLoader iconLoader;
    QPixmap errorIcon = iconLoader.loadIcon( "connect_creating", K3Icon::Desktop,
                                             K3Icon::SizeSmall );
    if ( !mPlotterWdg )
      return;

    mErrorIndicator = new QWidget( mPlotterWdg );
    QPalette palette = mErrorIndicator->palette();
    palette.setBrush( mErrorIndicator->backgroundRole(), QBrush( errorIcon ) );
    mErrorIndicator->setPalette( palette );
    mErrorIndicator->resize( errorIcon.size() );
    if ( !errorIcon.mask().isNull() )
      mErrorIndicator->setMask( errorIcon.mask() );

    mErrorIndicator->move( 0, 0 );
    mErrorIndicator->show();
  }
}

void SensorDisplay::setTitle( const QString &title )
{
  mTitle = title;
  emit changeTitle(title);
}

QString SensorDisplay::title() const
{
  return mTitle;
}

void SensorDisplay::setUnit( const QString &unit )
{
  mUnit = unit;
}

QString SensorDisplay::unit() const
{
  return mUnit;
}

void SensorDisplay::setShowUnit( bool value )
{
  mShowUnit = value;
}

bool SensorDisplay::showUnit() const
{
  return mShowUnit;
}

void SensorDisplay::setPlotterWidget( QWidget *wdg )
{
  mPlotterWdg = wdg;
}

void SensorDisplay::reorderSensors(const QList<int> &orderOfSensors)
{
  QList<SensorProperties *> newSensors;
  for ( int i = 0; i < orderOfSensors.count(); ++i ) {
    newSensors.append( mSensors.at(orderOfSensors[i] ));
  }
  mSensors = newSensors;
}



SensorProperties::SensorProperties()
{
}

SensorProperties::SensorProperties( const QString &hostName, const QString &name,
                                    const QString &type, const QString &description )
  : mName( name ), mType( type ), mDescription( description )
{
  setHostName(hostName);
  mOk = false;
}

SensorProperties::~SensorProperties()
{
}

void SensorProperties::setHostName( const QString &hostName )
{
  mHostName = hostName;
  mIsLocalhost = (mHostName.toLower() == "localhost" || mHostName.isEmpty());
}

bool SensorProperties::isLocalhost() const
{
  return mIsLocalhost;
}

QString SensorProperties::hostName() const
{
  return mHostName;
}

void SensorProperties::setName( const QString &name )
{
  mName = name;
}

QString SensorProperties::name() const
{
  return mName;
}

void SensorProperties::setType( const QString &type )
{
  mType = type;
}

QString SensorProperties::type() const
{
  return mType;
}

void SensorProperties::setDescription( const QString &description )
{
  mDescription = description;
}

QString SensorProperties::description() const
{
  return mDescription;
}

void SensorProperties::setUnit( const QString &unit )
{
  mUnit = unit;
}

QString SensorProperties::unit() const
{
  return mUnit;
}

void SensorProperties::setIsOk( bool value )
{
  mOk = value;
}

bool SensorProperties::isOk() const
{
  return mOk;
}

#include "SensorDisplay.moc"
