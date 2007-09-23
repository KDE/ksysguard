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

#include <QImage>
#include <QPushButton>
#include <QSpinBox>

#include <kconfig.h>
#include <klocale.h>
#include <QList>

#include "StyleEngine.h"
using namespace KSGRD;

StyleEngine* KSGRD::Style;

StyleEngine::StyleEngine()
{
  mFirstForegroundColor = QColor( 0x0057ae );  // light blue
  mSecondForegroundColor = QColor( 0x0057ae ); // light blue
  mAlarmColor = QColor( 255, 0, 0 );
  mBackgroundColor = QColor( 0x313031 );       // almost black
  mFontSize = 8;

  mSensorColors.append( QColor( 0x0057ae ) );  // soft blue
  mSensorColors.append( QColor( 0xe20800 ) );  // reddish
  mSensorColors.append( QColor( 0xf3c300 ) );  // bright yellow

  uint v = 0x00ff00;
  for ( uint i = mSensorColors.count(); i < 32; ++i ) {
    v = ( ( ( v + 82 ) & 0xff ) << 23 ) | ( v >> 8 );
    mSensorColors.append( QColor( v & 0xff, ( v >> 16 ) & 0xff, ( v >> 8 ) & 0xff ) );
  }
}

StyleEngine::~StyleEngine()
{
}

void StyleEngine::readProperties( const KConfigGroup& cfg )
{
  mFirstForegroundColor = cfg.readEntry( "fgColor1", mFirstForegroundColor );
  mSecondForegroundColor = cfg.readEntry( "fgColor2", mSecondForegroundColor );
  mAlarmColor = cfg.readEntry( "alarmColor", mAlarmColor );
  mBackgroundColor = cfg.readEntry( "backgroundColor", mBackgroundColor );
  mFontSize = cfg.readEntry( "fontSize", mFontSize );

  QStringList list = cfg.readEntry( "sensorColors",QStringList() );
  if ( !list.isEmpty() ) {
    mSensorColors.clear();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      mSensorColors.append( QColor( *it ) );
  }
}

void StyleEngine::saveProperties( KConfigGroup& )
{
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

const QColor& StyleEngine::sensorColor( int pos )
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

#include "StyleEngine.moc"
