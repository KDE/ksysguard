/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999, 2000, 2001 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#include <qcheckbox.h>
#include <qdom.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <klocale.h>
#include <knumvalidator.h>
#include <ksgrd/ColorPicker.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "BarGraph.h"
#include "DancingBarsSettings.h"

#include "DancingBars.h"

DancingBars::DancingBars( QWidget *parent, const char *name, const QString &title,
                          int, int, bool noFrame_ )
  : KSGRD::SensorDisplay( parent, name, title )
{
  mBars = 0;
	mFlags = 0;
	setNoFrame( noFrame_ );

  if ( noFrame() )
    mPlotter = new BarGraph( this );
  else
    mPlotter = new BarGraph( frame() );

  setMinimumSize( sizeHint() );

  /* All RMB clicks to the mPlotter widget will be handled by 
   * SensorDisplay::eventFilter. */
  mPlotter->installEventFilter( this );

  setPlotterWidget( mPlotter );

  setModified( false );
}

DancingBars::~DancingBars()
{
}

void DancingBars::configureSettings()
{
  mSettingsDialog = new DancingBarsSettings( this );

  mSettingsDialog->setTitle( title() );
  mSettingsDialog->setMinValue( mPlotter->getMin() );
  mSettingsDialog->setMaxValue( mPlotter->getMax() );

  double l, u;
  bool la, ua;
  mPlotter->getLimits( l, la, u, ua );

  mSettingsDialog->setUseUpperLimit( ua );
  mSettingsDialog->setUpperLimit( u );

  mSettingsDialog->setUseLowerLimit( la );
  mSettingsDialog->setLowerLimit( l );

  mSettingsDialog->setForegroundColor( mPlotter->normalColor );
  mSettingsDialog->setAlarmColor( mPlotter->alarmColor );
  mSettingsDialog->setBackgroundColor( mPlotter->backgroundColor );
  mSettingsDialog->setFontSize( mPlotter->fontSize );

  QValueList< QStringList > list;
  for ( uint i = mBars - 1; i < mBars; i-- ) {
    QStringList entry;
    entry << sensors().at( i )->hostName();
    entry << KSGRD::SensorMgr->translateSensor( sensors().at( i )->name() );
    entry << mPlotter->footers[ i ];
    entry << KSGRD::SensorMgr->translateUnit( sensors().at( i )->unit() );
    entry << ( sensors().at( i )->isOk() ? i18n( "OK" ) : i18n( "Error" ) );

    list.append( entry );
  }
  mSettingsDialog->setSensors( list );

  connect( mSettingsDialog, SIGNAL( applyClicked() ), SLOT( applySettings() ) );

  if ( mSettingsDialog->exec() )
    applySettings();

  delete mSettingsDialog;
  mSettingsDialog = 0;
}

void DancingBars::applySettings()
{
  setTitle( mSettingsDialog->title() );
  mPlotter->changeRange( mSettingsDialog->minValue(), mSettingsDialog->maxValue() );
  mPlotter->setLimits( mSettingsDialog->useLowerLimit() ?
                       mSettingsDialog->lowerLimit() : 0,
                       mSettingsDialog->useLowerLimit(),
                       mSettingsDialog->useUpperLimit() ?
                       mSettingsDialog->upperLimit() : 0,
                       mSettingsDialog->useUpperLimit() );

  mPlotter->normalColor = mSettingsDialog->foregroundColor();
  mPlotter->alarmColor = mSettingsDialog->alarmColor();
  mPlotter->backgroundColor = mSettingsDialog->backgroundColor();
  mPlotter->fontSize = mSettingsDialog->fontSize();

  QValueList< QStringList > list = mSettingsDialog->sensors();
  QValueList< QStringList >::Iterator it;

  for ( uint i = 0; i < sensors().count(); i++ ) {
    bool found = false;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( (*it)[ 0 ] == sensors().at( i )->hostName() &&
           (*it)[ 1 ] == KSGRD::SensorMgr->translateSensor( sensors().at( i )->name() ) ) {
        mPlotter->footers[ i ] = (*it)[ 2 ];
        found = true;
        break;
      }
    }

    if ( !found )
      removeSensor( i );
  }

  repaint();
  setModified( true );
}

void DancingBars::applyStyle()
{
  mPlotter->normalColor = KSGRD::Style->firstForegroundColor();
  mPlotter->alarmColor = KSGRD::Style->alarmColor();
  mPlotter->backgroundColor = KSGRD::Style->backgroundColor();
  mPlotter->fontSize = KSGRD::Style->fontSize();

  repaint();
  setModified( true );
}

bool DancingBars::addSensor( const QString &hostName, const QString &name,
                             const QString &type, const QString &title )
{
  if ( type != "integer" && type != "float" )
    return false;

  if ( mBars >= 32 )
    return false;

  if ( !mPlotter->addBar( title ) )
    return false;

  registerSensor( new KSGRD::SensorProperties( hostName, name, type, title ) );

  /* To differentiate between answers from value requests and info
   * requests we add 100 to the beam index for info requests. */
  sendRequest( hostName, name + "?", mBars + 100 );
  ++mBars;
  mSampleBuffer.resize( mBars );

  QString tooltip;
  for ( uint i = 0; i < mBars; ++i ) {
    tooltip += QString( "%1%2:%3" ).arg( i != 0 ? "\n" : "" )
                                   .arg( sensors().at( i )->hostName() )
                                   .arg( sensors().at( i )->name() );
  }
  QToolTip::remove( mPlotter );
  QToolTip::add( mPlotter, tooltip );

  return true;
}

bool DancingBars::removeSensor( uint pos )
{
  if ( pos >= mBars ) {
    kdDebug(1215) << "DancingBars::removeSensor: idx out of range ("
                  << pos << ")" << endl;
    return false;
  }

  mPlotter->removeBar( pos );
  mBars--;
  KSGRD::SensorDisplay::removeSensor( pos );

  QString tooltip;
  for ( uint i = 0; i < mBars; ++i ) {
    tooltip += QString( "%1%2:%3" ).arg( i != 0 ? "\n" : "" )
                                   .arg( sensors().at( i )->hostName() )
                                   .arg( sensors().at( i )->name() );
  }
  QToolTip::remove( mPlotter );
  QToolTip::add( mPlotter, tooltip );

  return true;
}

void DancingBars::updateSamples( const QMemArray<double> &samples )
{
  mPlotter->updateSamples( samples );
}

void DancingBars::resizeEvent( QResizeEvent* )
{
  if ( noFrame() )
    mPlotter->setGeometry( 0, 0, width(), height() );
  else
    frame()->setGeometry( 0, 0, width(), height() );
}

QSize DancingBars::sizeHint()
{
  if ( noFrame() )
    return ( mPlotter->sizeHint() );
  else
    return ( frame()->sizeHint() );
}

void DancingBars::answerReceived( int id, const QString &answer )
{
  /* We received something, so the sensor is probably ok. */
  sensorError( id, false );
	
  if ( id < 100 ) {
    mSampleBuffer[ id ] = answer.toDouble();
    if ( mFlags & ( 1 << id ) ) {
      kdDebug(1215) << "ERROR: DancingBars lost sample (" << mFlags
                    << ", " << mBars << ")" << endl;
      sensorError( id, true );
    }
    mFlags |= 1 << id;

    if ( mFlags == (uint)( ( 1 << mBars ) - 1 ) ) {
      mPlotter->updateSamples( mSampleBuffer );
      mFlags = 0;
    }
  } else if ( id >= 100 ) {
    KSGRD::SensorIntegerInfo info( answer );
    if ( id == 100 )
      if ( mPlotter->getMin() == 0.0 && mPlotter->getMax() == 0.0 ) {
        /* We only use this information from the sensor when the
         * display is still using the default values. If the
         * sensor has been restored we don't touch the already set
         * values. */
        mPlotter->changeRange( info.min(), info.max() );
			}

    sensors().at( id - 100 )->setUnit( info.unit() );
	}
}

bool DancingBars::restoreSettings( QDomElement &element )
{
  SensorDisplay::restoreSettings( element );

  mPlotter->changeRange( element.attribute( "min", "0" ).toDouble(),
                         element.attribute( "max", "0" ).toDouble() );

  mPlotter->setLimits( element.attribute( "lowlimit", "0" ).toDouble(),
                       element.attribute( "lowlimitactive", "0" ).toInt(),
                       element.attribute( "uplimit", "0" ).toDouble(),
                       element.attribute( "uplimitactive", "0" ).toInt() );

  mPlotter->normalColor = restoreColor( element, "normalColor",
                                        KSGRD::Style->firstForegroundColor() );
  mPlotter->alarmColor = restoreColor( element, "alarmColor",
                                       KSGRD::Style->alarmColor() );
  mPlotter->backgroundColor = restoreColor( element, "backgroundColor",
                                            KSGRD::Style->backgroundColor() );
  mPlotter->fontSize = element.attribute( "fontSize", QString( "%1" ).arg(
                                          KSGRD::Style->fontSize() ) ).toInt();

  QDomNodeList dnList = element.elementsByTagName( "beam" );
  for ( uint i = 0; i < dnList.count(); ++i ) {
    QDomElement el = dnList.item( i ).toElement();
    addSensor( el.attribute( "hostName" ), el.attribute( "sensorName" ),
               ( el.attribute( "sensorType" ).isEmpty() ? "integer" :
               el.attribute( "sensorType" ) ), el.attribute( "sensorDescr" ) );
  }

  setModified( false );

  return true;
}

bool DancingBars::saveSettings( QDomDocument &doc, QDomElement &element,
                                bool save )
{
  element.setAttribute( "min", mPlotter->getMin() );
  element.setAttribute( "max", mPlotter->getMax() );
  double l, u;
  bool la, ua;
  mPlotter->getLimits( l, la, u, ua );
  element.setAttribute( "lowlimit", l );
  element.setAttribute( "lowlimitactive", la );
  element.setAttribute( "uplimit", u );
  element.setAttribute( "uplimitactive", ua );

  saveColor( element, "normalColor", mPlotter->normalColor );
  saveColor( element, "alarmColor", mPlotter->alarmColor );
	saveColor( element, "backgroundColor", mPlotter->backgroundColor );
  element.setAttribute( "fontSize", mPlotter->fontSize );

  for ( uint i = 0; i < mBars; ++i ) {
    QDomElement beam = doc.createElement( "beam" );
    element.appendChild( beam );
    beam.setAttribute( "hostName", sensors().at( i )->hostName() );
    beam.setAttribute( "sensorName", sensors().at( i )->name() );
    beam.setAttribute( "sensorType", sensors().at( i )->type() );
    beam.setAttribute( "sensorDescr", mPlotter->footers[ i ] );
  }

  SensorDisplay::saveSettings( doc, element );

  if ( save )
    setModified( false );

  return true;
}

bool DancingBars::hasSettingsDialog() const
{
  return true;
}

#include "DancingBars.moc"
