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

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#include <qdom.h>
#include <qimage.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "FancyPlotterSettings.h"

#include "FancyPlotter.h"

FancyPlotter::FancyPlotter( QWidget* parent, const char* name,
                            const QString &title, double, double,
                            bool nf)
  : KSGRD::SensorDisplay( parent, name, title )
{
  mBeams = 0;
  setNoFrame( nf );

  if ( noFrame() ) {
    mPlotter = new SignalPlotter( this );
    mPlotter->setShowTopBar( true );
	} else
    mPlotter = new SignalPlotter( frame() );

  if ( !title.isEmpty() )
    mPlotter->setTitle( title );

  setMinimumSize( sizeHint() );

  /* All RMB clicks to the mPlotter widget will be handled by 
   * SensorDisplay::eventFilter. */
	mPlotter->installEventFilter( this );

  setPlotterWidget( mPlotter );

  setModified( false );
}

FancyPlotter::~FancyPlotter()
{
}

void FancyPlotter::configureSettings()
{
  mSettingsDialog = new FancyPlotterSettings( this );

  mSettingsDialog->setTitle( title() );
  mSettingsDialog->setUseAutoRange( mPlotter->useAutoRange() );
  mSettingsDialog->setMinValue( mPlotter->minValue() );
  mSettingsDialog->setMaxValue( mPlotter->maxValue() );

  mSettingsDialog->setUsePolygonStyle( mPlotter->graphStyle() == GRAPH_POLYGON );
  mSettingsDialog->setHorizontalScale( mPlotter->horizontalScale() );

  mSettingsDialog->setShowVerticalLines( mPlotter->showVerticalLines() );
  mSettingsDialog->setVerticalLinesColor( mPlotter->verticalLinesColor() );
  mSettingsDialog->setVerticalLinesDistance( mPlotter->verticalLinesDistance() );
  mSettingsDialog->setVerticalLinesScroll( mPlotter->verticalLinesScroll() );

  mSettingsDialog->setShowHorizontalLines( mPlotter->showHorizontalLines() );
  mSettingsDialog->setHorizontalLinesColor( mPlotter->horizontalLinesColor() );
  mSettingsDialog->setHorizontalLinesCount( mPlotter->horizontalLinesCount() );

  mSettingsDialog->setShowLabels( mPlotter->showLabels() );
  mSettingsDialog->setShowTopBar( mPlotter->showTopBar() );
  mSettingsDialog->setFontSize( mPlotter->fontSize() );

  mSettingsDialog->setBackgroundColor( mPlotter->backgroundColor() );

  QList< QStringList > list;
  for ( uint i = 0; i < mBeams; ++i ) {
    QStringList entry;
    entry << QString( "%1" ).arg( i + 1 );
    entry << sensors().at( i )->hostName();
    entry << KSGRD::SensorMgr->translateSensor( sensors().at( i )->name() );
    entry << KSGRD::SensorMgr->translateUnit( sensors().at( i )->unit() );
    entry << ( sensors().at( i )->isOk() ? i18n( "OK" ) : i18n( "Error" ) );
    entry << ( mPlotter->beamColors()[ i ].name() );

    list.append( entry );
  }
  mSettingsDialog->setSensors( list );

  connect( mSettingsDialog, SIGNAL( applyClicked() ), SLOT( applySettings() ) );

  if ( mSettingsDialog->exec() )
    applySettings();

  delete mSettingsDialog;
  mSettingsDialog = 0;
}

void FancyPlotter::applySettings()
{
  setTitle( mSettingsDialog->title() );
  mPlotter->setTitle( title() );

  if ( mSettingsDialog->useAutoRange() )
    mPlotter->setUseAutoRange( true );
  else {
    mPlotter->setUseAutoRange( false );
    mPlotter->changeRange( 0, mSettingsDialog->minValue(),
                          mSettingsDialog->maxValue() );
  }

  if ( mSettingsDialog->usePolygonStyle() )
    mPlotter->setGraphStyle( GRAPH_POLYGON );
  else
    mPlotter->setGraphStyle( GRAPH_ORIGINAL );

  if ( mPlotter->horizontalScale() != mSettingsDialog->horizontalScale() ) {
    mPlotter->setHorizontalScale( mSettingsDialog->horizontalScale() );
    // Can someone think of a useful QResizeEvent to pass?
    // It doesn't really matter anyway because it's not used.
    emit resizeEvent( 0 );
  }

  mPlotter->setShowVerticalLines( mSettingsDialog->showVerticalLines() );
  mPlotter->setVerticalLinesColor( mSettingsDialog->verticalLinesColor() );
  mPlotter->setVerticalLinesDistance( mSettingsDialog->verticalLinesDistance() );
  mPlotter->setVerticalLinesScroll( mSettingsDialog->verticalLinesScroll() );

  mPlotter->setShowHorizontalLines( mSettingsDialog->showHorizontalLines() );
  mPlotter->setHorizontalLinesColor( mSettingsDialog->horizontalLinesColor() );
  mPlotter->setHorizontalLinesCount( mSettingsDialog->horizontalLinesCount() );

  mPlotter->setShowLabels( mSettingsDialog->showLabels() );
  mPlotter->setShowTopBar( mSettingsDialog->showTopBar() );
  mPlotter->setFontSize( mSettingsDialog->fontSize() );

  mPlotter->setBackgroundColor( mSettingsDialog->backgroundColor() );

  /* Iterate through registered sensors and through the items of the
   * listview. Where a sensor cannot be matched, it is removed. */
  uint delCount = 0;

  QList< QStringList > list = mSettingsDialog->sensors();
  QList< QStringList >::Iterator it;

  for ( uint i = 0; i < sensors().count(); ++i ) {
    bool found = false;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( (*it)[ 0 ].toInt() == (int)( i + 1 + delCount ) ) {
        mPlotter->beamColors()[ i ] = QColor( (*it)[ 5 ] );
        found = true;
        if ( delCount > 0 )
          (*it)[ 0 ] = QString( "%1" ).arg( i + 1 );
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

  mPlotter->repaint();
  setModified( true );
}

void FancyPlotter::applyStyle()
{
  mPlotter->setVerticalLinesColor( KSGRD::Style->firstForegroundColor() );
  mPlotter->setHorizontalLinesColor( KSGRD::Style->secondForegroundColor() );
  mPlotter->setBackgroundColor( KSGRD::Style->backgroundColor() );
  mPlotter->setFontSize( KSGRD::Style->fontSize() );
  for ( int i = 0; i < mPlotter->beamColors().count() &&
        i < KSGRD::Style->numSensorColors(); ++i )
    mPlotter->beamColors()[ i ] = KSGRD::Style->sensorColor( i );

  mPlotter->update();
  setModified( true );
}

bool FancyPlotter::addSensor( const QString &hostName, const QString &name,
                              const QString &type, const QString &title )
{
  return addSensor( hostName, name, type, title,
                    KSGRD::Style->sensorColor( mBeams ) );
}

bool FancyPlotter::addSensor( const QString &hostName, const QString &name,
                              const QString &type, const QString &title,
                              const QColor &color )
{
  if ( type != "integer" && type != "float" )
    return false;

  if ( mBeams > 0 && hostName != sensors().at( 0 )->hostName() ) {
    KMessageBox::sorry( this, QString( "All sensors of this display need "
                                       "to be from the host %1!" )
                        .arg( sensors().at( 0 )->hostName() ) );

    /* We have to enforce this since the answers to value requests
     * need to be received in order. */
    return false;
  }

  if ( !mPlotter->addBeam( color ) )
    return false;

  registerSensor( new FPSensorProperties( hostName, name, type, title, color ) );

  /* To differentiate between answers from value requests and info
   * requests we add 100 to the beam index for info requests. */
  sendRequest( hostName, name + "?", mBeams + 100 );

  ++mBeams;

  QString tooltip;
  for ( uint i = 0; i < mBeams; ++i ) {
    tooltip += QString( "%1%2:%3" ).arg( i != 0 ? "\n" : "" )
                                   .arg( sensors().at( mBeams - i - 1 )->hostName() )
                                   .arg( sensors().at( mBeams - i - 1  )->name() );
  }

  QToolTip::remove( mPlotter );
  mPlotter->setToolTip( tooltip );

  return true;
}

bool FancyPlotter::removeSensor( uint pos )
{
  if ( pos >= mBeams ) {
    kdDebug(1215) << "FancyPlotter::removeSensor: idx out of range ("
                  << pos << ")" << endl;
    return false;
  }

  mPlotter->removeBeam( pos );
  mBeams--;
  KSGRD::SensorDisplay::removeSensor( pos );

  QString tooltip;
  for ( uint i = 0; i < mBeams; ++i ) {
    tooltip += QString( "%1%2:%3" ).arg( i != 0 ? "\n" : "" )
                                   .arg( sensors().at( mBeams - i - 1 )->hostName() )
                                   .arg( sensors().at( mBeams - i - 1  )->name() );
  }

  QToolTip::remove( mPlotter );
  mPlotter->setToolTip( tooltip );

  return true;
}

void FancyPlotter::resizeEvent( QResizeEvent* )
{
  if ( noFrame() )
    mPlotter->setGeometry( 0, 0, width(), height() );
  else
    frame()->setGeometry( 0, 0, width(), height() );
}

QSize FancyPlotter::sizeHint()
{
  if ( noFrame() )
    return mPlotter->sizeHint();
  else
    return frame()->sizeHint();
}

void FancyPlotter::answerReceived( int id, const QString &answer )
{
  if ( (uint)id < mBeams ) {
    if ( id != (int)mSampleBuf.count() ) {
      if ( id == 0 )
        sensorError( mBeams - 1, true );
      else
        sensorError( id - 1, true );
    }
    mSampleBuf.append( answer.toDouble() );

    /* We received something, so the sensor is probably ok. */
    sensorError( id, false );

    if ( id == (int)mBeams - 1 ) {
      mPlotter->addSample( mSampleBuf );
      mSampleBuf.clear();
    }
  } else if ( id >= 100 ) {
    KSGRD::SensorFloatInfo info( answer );
    if ( !mPlotter->useAutoRange() && mPlotter->minValue() == 0.0 &&
         mPlotter->maxValue() == 0.0 ) {
      /* We only use this information from the sensor when the
       * display is still using the default values. If the
       * sensor has been restored we don't touch the already set
       * values. */
      mPlotter->changeRange( id - 100, info.min(), info.max() );
      if ( info.min() == 0.0 && info.max() == 0.0 )
        mPlotter->setUseAutoRange( true );
    }
    sensors().at( id - 100 )->setUnit( info.unit() );
  }
}

bool FancyPlotter::restoreSettings( QDomElement &element )
{
  /* autoRage was added after KDE 2.x and was brokenly emulated by
   * min == 0.0 and max == 0.0. Since we have to be able to read old
   * files as well we have to emulate the old behaviour as well. */
  double min = element.attribute( "min", "0.0" ).toDouble();
  double max = element.attribute( "max", "0.0" ).toDouble();
  if ( element.attribute( "autoRange", min == 0.0 && max == 0.0 ? "1" : "0" ).toInt() )
    mPlotter->setUseAutoRange( true );
  else {
    mPlotter->setUseAutoRange( false );
    mPlotter->changeRange( 0, element.attribute( "min" ).toDouble(),
                           element.attribute( "max" ).toDouble() );
  }

  mPlotter->setShowVerticalLines( element.attribute( "vLines", "1" ).toUInt() );
  mPlotter->setVerticalLinesColor( restoreColor( element, "vColor",
                                   KSGRD::Style->firstForegroundColor() ) );
  mPlotter->setVerticalLinesDistance( element.attribute( "vDistance", "30" ).toUInt() );
  mPlotter->setVerticalLinesScroll( element.attribute( "vScroll", "1" ).toUInt() );
  mPlotter->setGraphStyle( element.attribute( "graphStyle", "0" ).toUInt() );
  mPlotter->setHorizontalScale( element.attribute( "hScale", "1" ).toUInt() );

  mPlotter->setShowHorizontalLines( element.attribute( "hLines", "1" ).toUInt() );
  mPlotter->setHorizontalLinesColor( restoreColor( element, "hColor",
                                     KSGRD::Style->secondForegroundColor() ) );
  mPlotter->setHorizontalLinesCount( element.attribute( "hCount", "5" ).toUInt() );

  mPlotter->setShowLabels( element.attribute( "labels", "1" ).toUInt() );
  mPlotter->setShowTopBar( element.attribute( "topBar", "0" ).toUInt() );
  mPlotter->setFontSize( element.attribute( "fontSize",
                   QString( "%1" ).arg( KSGRD::Style->fontSize() ) ).toUInt() );

  mPlotter->setBackgroundColor( restoreColor( element, "bColor",
                                   KSGRD::Style->backgroundColor() ) );

  QDomNodeList dnList = element.elementsByTagName( "beam" );
  for ( int i = 0; i < dnList.count(); ++i ) {
    QDomElement el = dnList.item( i ).toElement();
    addSensor( el.attribute( "hostName" ), el.attribute( "sensorName" ),
               ( el.attribute( "sensorType" ).isEmpty() ? "integer" :
               el.attribute( "sensorType" ) ), "", restoreColor( el, "color",
               KSGRD::Style->sensorColor( i ) ) );
  }

  SensorDisplay::restoreSettings( element );

  if ( !title().isEmpty() )
    mPlotter->setTitle( title() );

  setModified( false );

  return true;
}

bool FancyPlotter::saveSettings( QDomDocument &doc, QDomElement &element,
                                 bool save )
{
  element.setAttribute( "min", mPlotter->minValue() );
  element.setAttribute( "max", mPlotter->maxValue() );
  element.setAttribute( "autoRange", mPlotter->useAutoRange() );
  element.setAttribute( "vLines", mPlotter->showVerticalLines() );
  saveColor( element, "vColor", mPlotter->verticalLinesColor() );
  element.setAttribute( "vDistance", mPlotter->verticalLinesDistance() );
  element.setAttribute( "vScroll", mPlotter->verticalLinesScroll() );

  element.setAttribute( "graphStyle", mPlotter->graphStyle() );
  element.setAttribute( "hScale", mPlotter->horizontalScale() );

  element.setAttribute( "hLines", mPlotter->showHorizontalLines() );
  saveColor( element, "hColor", mPlotter->horizontalLinesColor() );
  element.setAttribute( "hCount", mPlotter->horizontalLinesCount() );

  element.setAttribute( "labels", mPlotter->showLabels() );
  element.setAttribute( "topBar", mPlotter->showTopBar() );
  element.setAttribute( "fontSize", mPlotter->fontSize() );

  saveColor( element, "bColor", mPlotter->backgroundColor() );

  for ( uint i = 0; i < mBeams; ++i ) {
    QDomElement beam = doc.createElement( "beam" );
    element.appendChild( beam );
    beam.setAttribute( "hostName", sensors().at( i )->hostName() );
    beam.setAttribute( "sensorName", sensors().at( i )->name() );
    beam.setAttribute( "sensorType", sensors().at( i )->type() );
    saveColor( beam, "color", mPlotter->beamColors()[ i ] );
  }

  SensorDisplay::saveSettings( doc, element );

  if ( save )
    setModified( false );

  return true;
}

bool FancyPlotter::hasSettingsDialog() const
{
  return true;
}



FPSensorProperties::FPSensorProperties()
{
}

FPSensorProperties::FPSensorProperties( const QString &hostName,
                                        const QString &name,
                                        const QString &type,
                                        const QString &description,
                                        const QColor &color )
  : KSGRD::SensorProperties( hostName, name, type, description ),
    mColor( color )
{
}
                                        
FPSensorProperties::~FPSensorProperties()
{
}

void FPSensorProperties::setColor( const QColor &color )
{
  mColor = color;
}

QColor FPSensorProperties::color() const
{
  return mColor;
}

#include "FancyPlotter.moc"
