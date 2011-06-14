/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QCheckBox>
#include <QtXml/qdom.h>
#include <QMenu>
#include <QSpinBox>

#include <QBitmap>
#include <QPixmap>
#include <QEvent>
#include <QMouseEvent>
#include <QCustomEvent>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurl.h>
#include <kservice.h>

#include "ksgrd/SensorManager.h"
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

  mShowUnit = false;
  mTimerId = NONE;
  mErrorIndicator = 0;
  mPlotterWdg = 0;

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

  if ( mTimerId > 0 )
    killTimer( mTimerId );
  for(int i = mSensors.size()-1; i>=0; i--)
    unregisterSensor(i);
}

void SensorDisplay::registerSensor( SensorProperties *sp )
{
  mSensors.append( sp );
}

void SensorDisplay::unregisterSensor( uint pos )
{
  delete mSensors.takeAt( pos );
}

void SensorDisplay::timerTick()
{
  int i = 0;

  foreach( SensorProperties *s, mSensors) {
    sendRequest( s->hostName(), s->name(), i++ );
 }
}

void SensorDisplay::showContextMenu(const QPoint &pos)
{
    QMenu pm;
    QAction *action = 0;
    bool menuEmpty = true;

    if ( hasSettingsDialog() ) {
      action = pm.addAction( i18n( "&Properties" ) );
      action->setData( 0 );
      menuEmpty = false;
    }
    if(mSharedSettings && !mSharedSettings->locked) {
      action = pm.addAction( i18n( "&Remove Display" ) );
      action->setData( 1 );
      menuEmpty = false;
    }

    if(menuEmpty) return;
    action = pm.exec( mapToGlobal(pos) );
    if ( action ) {
      switch ( action->data().toInt() ) {
        case 0:
          configureSettings();
          break;
        case 1: {
            if ( mDeleteNotifier ) {
              DeleteEvent *event = new DeleteEvent( this );
              kapp->postEvent( mDeleteNotifier, event );
            }
          }
          break;
      }
    }
}

bool SensorDisplay::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *e = static_cast<QMouseEvent *> (event);
    if( e->button() == Qt::RightButton ) {
      showContextMenu( mapFromGlobal( e->globalPos() ) );
      return true;
    }
  } 

  return QWidget::eventFilter( object, event );
}
void SensorDisplay::sendRequest( const QString &hostName,
                                 const QString &command, int id )
{
  if ( !SensorMgr->sendRequest( hostName, command, (SensorClient*)this, id ) ) {
    sensorError( id, true );
  }
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
  if(mSharedSettings && mSharedSettings->locked)
      this->setWhatsThis( i18n(
        "<qt><p>This is a sensor display. To customize a sensor display click "
        "the right mouse button here "
        "and select the <i>Properties</i> entry from the popup "
        "menu. Select <i>Remove</i> to delete the display from the worksheet."
        "</p>%1</qt>" ,  additionalWhatsThis() ) );
  else
      this->setWhatsThis( additionalWhatsThis());
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
  int color = element.attribute( attr ).toUInt( &ok, 0 );
  
  if ( !ok ) {
    kDebug(1215) << "Invalid color read in from worksheet for " << attr << " = " << element.attribute(attr) << " (Not a valid number)";
    return fallback;
  }
  QColor c( (color & 0xff0000) >> 16, (color & 0xff00) >> 8, (color & 0xff), (color & 0xff000000) >> 24);
  if( !c.isValid()) {
    kDebug(1215) << "Invalid color read in from worksheet for " << attr << " = " << element.attribute(attr);
    return fallback;
  }

  if(c.alpha() == 0) c.setAlpha(255);
  return c;
}

void SensorDisplay::saveColor( QDomElement &element, const QString &attr,
                               const QColor &color )
{
  element.setAttribute( attr, "0x" + QString::number(color.rgba(),16) );
}

void SensorDisplay::saveColorAppend( QDomElement &element, const QString &attr,
                               const QColor &color )
{
  element.setAttribute( attr, element.attribute(attr) + ",0x" + QString::number(color.rgba(),16) );
}

bool SensorDisplay::addSensor( const QString &hostName, const QString &name,
                               const QString &type, const QString &description )
{
  registerSensor( new SensorProperties( hostName, name, type, description ) );
  return true;
}

bool SensorDisplay::removeSensor( uint pos )
{
  if((int) pos >= mSensors.count())
    return false;
  unregisterSensor( pos );
  return true;
}

bool SensorDisplay::hasSettingsDialog() const
{
  return false;
}

void SensorDisplay::configureSettings()
{
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
  setTitle( element.attribute( "title", title() ) );

  return true;
}

bool SensorDisplay::saveSettings( QDomDocument&, QDomElement &element )
{
  element.setAttribute( "title", title() );
  element.setAttribute( "unit", unit() );
  element.setAttribute( "showUnit", mShowUnit );

  return true;
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
    if ( !mPlotterWdg || mPlotterWdg->isVisible())
      return;

    QPixmap errorIcon = KIconLoader::global()->loadIcon( "dialog-error", KIconLoader::Desktop,
                                             KIconLoader::SizeSmall );

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

void SensorDisplay::changeEvent( QEvent * event ) {
  if (event->type() == QEvent::LanguageChange) {
    setTitle(mTitle);  //retranslate
  }
}

void SensorDisplay::setTitle( const QString &title )
{
  mTitle = title;
  mTranslatedTitle = i18n(title.toUtf8());
  emit titleChanged(mTitle);
  emit translatedTitleChanged(mTranslatedTitle);
}

QString SensorDisplay::translatedTitle() const
{
  return mTranslatedTitle;
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

void SensorProperties::setRegExpName( const QString &name )
{
  mRegExpName = name;
}
QString SensorProperties::regExpName() const
{
  return mRegExpName;
}

#include "SensorDisplay.moc"
