/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999, 2000, 2001 Chris Schlaeger <cs@kde.org>
    
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCheckBox>
#include <QtXml/qdom.h>
#include <QPushButton>
#include <QHBoxLayout>

#include <kdebug.h>
#include <KLocalizedString>
#include <knumvalidator.h>
#include <ksgrd/SensorManager.h>
#include "StyleEngine.h"

#include "BarGraph.h"
#include "DancingBarsSettings.h"

#include "DancingBars.h"

DancingBars::DancingBars( QWidget *parent, const QString &title, SharedSettings *workSheetSettings)
  : KSGRD::SensorDisplay( parent, title, workSheetSettings)
{
  mBars = 0;
  mFlags = QBitArray( 100 );
  mFlags.fill( false );

  QLayout *layout = new QHBoxLayout(this);
  mPlotter = new BarGraph( this );
  layout->addWidget(mPlotter);

  setMinimumSize( sizeHint() );

  /* All RMB clicks to the mPlotter widget will be handled by 
   * SensorDisplay::eventFilter. */
  mPlotter->installEventFilter( this );

  setPlotterWidget( mPlotter );

}

DancingBars::~DancingBars()
{
}

void DancingBars::configureSettings()
{
  DancingBarsSettings dlg( this );

  dlg.setTitle( title() );
  dlg.setMinValue( mPlotter->getMin() );
  dlg.setMaxValue( mPlotter->getMax() );

  double l, u;
  bool la, ua;
  mPlotter->getLimits( l, la, u, ua );

  dlg.setUseUpperLimit( ua );
  dlg.setUpperLimit( u );

  dlg.setUseLowerLimit( la );
  dlg.setLowerLimit( l );

  dlg.setForegroundColor( mPlotter->normalColor );
  dlg.setAlarmColor( mPlotter->alarmColor );
  dlg.setBackgroundColor( mPlotter->mBackgroundColor );
  dlg.setFontSize( mPlotter->fontSize );

  SensorModelEntry::List list;
  for ( uint i = mBars - 1; i < mBars; i-- ) {
    SensorModelEntry entry;
    entry.setId( i );
    entry.setHostName( sensors().at( i )->hostName() );
    entry.setSensorName( KSGRD::SensorMgr->translateSensor( sensors().at( i )->name() ) );
    entry.setLabel( mPlotter->footers[ i ] );
    entry.setUnit( KSGRD::SensorMgr->translateUnit( sensors().at( i )->unit() ) );
    entry.setStatus( sensors().at( i )->isOk() ? i18n( "OK" ) : i18n( "Error" ) );

    list.append( entry );
  }
  dlg.setSensors( list );

  if ( !dlg.exec() )
    return;

  setTitle( dlg.title() );
  mPlotter->changeRange( dlg.minValue(), dlg.maxValue() );
  mPlotter->setLimits( dlg.useLowerLimit() ?
                       dlg.lowerLimit() : 0,
                       dlg.useLowerLimit(),
                       dlg.useUpperLimit() ?
                       dlg.upperLimit() : 0,
                       dlg.useUpperLimit() );

  mPlotter->normalColor = dlg.foregroundColor();
  mPlotter->alarmColor = dlg.alarmColor();
  mPlotter->mBackgroundColor = dlg.backgroundColor();
  mPlotter->fontSize = dlg.fontSize();

  uint delCount = 0;

  list = dlg.sensors();

  for ( int i = 0; i < sensors().count(); ++i ) {
    bool found = false;
    for ( int j = 0; j < list.count(); ++j ) {
      if ( list[ j ].id() == (int)( i + delCount ) ) {
        mPlotter->footers[ i ] = list[ j ].label();
        found = true;
        if ( delCount > 0 )
          list[ j ].setId( i );

        continue;
      }
    }

    if ( !found ) {
      if ( removeSensor(i) ) {
        i--;
        delCount++;
      }
    }
  }

  repaint();
}

void DancingBars::applyStyle()
{
  mPlotter->normalColor = KSGRD::Style->firstForegroundColor();
  mPlotter->alarmColor = KSGRD::Style->alarmColor();
  mPlotter->mBackgroundColor = KSGRD::Style->backgroundColor();
  mPlotter->fontSize = KSGRD::Style->fontSize();

  repaint();
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
  sendRequest( hostName, name + '?', mBars + 100 );
  ++mBars;
  mSampleBuffer.resize( mBars );

  QString tooltip;
  for ( uint i = 0; i < mBars; ++i ) {
    tooltip += QString( "%1%2:%3" ).arg( i != 0 ? "\n" : "" )
                                   .arg( sensors().at( i )->hostName() )
                                   .arg( sensors().at( i )->name() );
  }
  mPlotter->setToolTip( tooltip );

  return true;
}

bool DancingBars::removeSensor( uint pos )
{
  if ( pos >= mBars ) {
    kDebug(1215) << "DancingBars::removeSensor: idx out of range ("
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
  mPlotter->setToolTip( tooltip );

  return true;
}

void DancingBars::updateSamples( const QVector<double> &samples )
{
  mPlotter->updateSamples( samples );
}

void DancingBars::answerReceived( int id, const QList<QByteArray> &answerlist )
{
  /* We received something, so the sensor is probably ok. */
  sensorError( id, false );
  QByteArray answer;
  if(!answerlist.isEmpty()) answer = answerlist[0];
  if ( id < 100 ) {
    if(id >= mSampleBuffer.count()) {
      kDebug(1215) << "ERROR: DancingBars received invalid data";
      sensorError(id, true);
      return;
    }
    mSampleBuffer[ id ] = answer.toDouble();
    if ( mFlags.testBit( id ) == true ) {
      kDebug(1215) << "ERROR: DancingBars lost sample (" << mFlags
                    << ", " << mBars << ")" << endl;
      sensorError( id, true );
      return;
    }
    mFlags.setBit( id, true );

    bool allBitsAvailable = true;
    for ( uint i = 0; i < mBars; ++i )
      allBitsAvailable &= mFlags.testBit( i );

    if ( allBitsAvailable ) {
      mPlotter->updateSamples( mSampleBuffer );
      mFlags.fill( false );
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
  mPlotter->mBackgroundColor = restoreColor( element, "backgroundColor",
                                            KSGRD::Style->backgroundColor() );
  mPlotter->fontSize = element.attribute( "fontSize", QString( "%1" ).arg(
                                          KSGRD::Style->fontSize() ) ).toInt();

  QDomNodeList dnList = element.elementsByTagName( "beam" );
  for ( int i = 0; i < dnList.count(); ++i ) {
    QDomElement el = dnList.item( i ).toElement();
    addSensor( el.attribute( "hostName" ), el.attribute( "sensorName" ),
               ( el.attribute( "sensorType" ).isEmpty() ? "integer" :
               el.attribute( "sensorType" ) ), el.attribute( "sensorDescr" ) );
  }


  return true;
}

bool DancingBars::saveSettings( QDomDocument &doc, QDomElement &element)
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
	saveColor( element, "backgroundColor", mPlotter->mBackgroundColor );
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

  return true;
}

bool DancingBars::hasSettingsDialog() const
{
  return true;
}


