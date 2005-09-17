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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#include <qimage.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <klocale.h>
#include <QList>

#include "StyleSettings.h"

#include "StyleEngine.h"

using namespace KSGRD;

StyleEngine* KSGRD::Style;

StyleEngine::StyleEngine()
{
  mFirstForegroundColor = QColor( 0x6894c9 );  // light blue
  mSecondForegroundColor = QColor( 0x6894c9 ); // light blue
  mAlarmColor = QColor( 255, 0, 0 );
  mBackgroundColor = QColor( 0x313031 );       // almost black
  mFontSize = 9;

  mSensorColors.append( QColor( 0x1889ff ) );  // soft blue
  mSensorColors.append( QColor( 0xff7f08 ) );  // reddish
  mSensorColors.append( QColor( 0xffeb14 ) );  // bright yellow

  uint v = 0x00ff00;
  for ( uint i = mSensorColors.count(); i < 32; ++i ) {
    v = ( ( ( v + 82 ) & 0xff ) << 23 ) | ( v >> 8 );
    mSensorColors.append( QColor( v & 0xff, ( v >> 16 ) & 0xff, ( v >> 8 ) & 0xff ) );
  }
}

StyleEngine::~StyleEngine()
{
}

void StyleEngine::readProperties( KConfig *cfg )
{
  mFirstForegroundColor = cfg->readColorEntry( "fgColor1", &mFirstForegroundColor );
  mSecondForegroundColor = cfg->readColorEntry( "fgColor2", &mSecondForegroundColor );
  mAlarmColor = cfg->readColorEntry( "alarmColor", &mAlarmColor );
  mBackgroundColor = cfg->readColorEntry( "backgroundColor", &mBackgroundColor );
  mFontSize = cfg->readNumEntry( "fontSize", mFontSize );

  QStringList list = cfg->readListEntry( "sensorColors" );
  if ( !list.isEmpty() ) {
    mSensorColors.clear();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      mSensorColors.append( QColor( *it ) );
  }
}

void StyleEngine::saveProperties( KConfig *cfg )
{
  cfg->writeEntry( "fgColor1", mFirstForegroundColor );
  cfg->writeEntry( "fgColor2", mSecondForegroundColor );
  cfg->writeEntry( "alarmColor", mAlarmColor );
  cfg->writeEntry( "backgroundColor", mBackgroundColor );
  cfg->writeEntry( "fontSize", mFontSize );

  QStringList list;
  QList<QColor>::Iterator it;
  for ( it = mSensorColors.begin(); it != mSensorColors.end(); ++it )
    list.append( (*it).name() );

  cfg->writeEntry( "sensorColors", list );
}

const QColor &StyleEngine::firstForegroundColor() const
{
  return mFirstForegroundColor;
}

const QColor &StyleEngine::secondForegroundColor() const
{
  return mSecondForegroundColor;
}

const QColor &StyleEngine::alarmColor() const
{
  return mAlarmColor;
}

const QColor &StyleEngine::backgroundColor() const
{
  return mBackgroundColor;
}

uint StyleEngine::fontSize() const
{
  return mFontSize;
}

const QColor& StyleEngine::sensorColor( uint pos )
{
  static QColor dummy;

  if ( pos < mSensorColors.count() )
    return mSensorColors.at( pos );
  else
    return dummy;
}

uint StyleEngine::numSensorColors() const
{
  return mSensorColors.count();
}

void StyleEngine::configure()
{
  mSettingsDialog = new StyleSettings( 0 );

  mSettingsDialog->setFirstForegroundColor( mFirstForegroundColor );
	mSettingsDialog->setSecondForegroundColor( mSecondForegroundColor );
  mSettingsDialog->setAlarmColor( mAlarmColor );
  mSettingsDialog->setBackgroundColor( mBackgroundColor );
  mSettingsDialog->setFontSize( mFontSize );
  mSettingsDialog->setSensorColors( mSensorColors );

  connect( mSettingsDialog, SIGNAL( applyClicked() ),
           this, SLOT( applyToWorksheet() ) );

  if ( mSettingsDialog->exec() )
    apply();

  delete mSettingsDialog;
  mSettingsDialog = 0;
}

void StyleEngine::applyToWorksheet()
{
  apply();
  emit applyStyleToWorksheet();
}

void StyleEngine::apply()
{
  if ( !mSettingsDialog )
    return;

  mFirstForegroundColor = mSettingsDialog->firstForegroundColor();
  mSecondForegroundColor = mSettingsDialog->secondForegroundColor();
  mAlarmColor = mSettingsDialog->alarmColor();
  mBackgroundColor = mSettingsDialog->backgroundColor();
  mFontSize = mSettingsDialog->fontSize();

  mSensorColors = mSettingsDialog->sensorColors();
}

#include "StyleEngine.moc"
