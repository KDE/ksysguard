/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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

#include <QPushButton>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>

#include "StyleEngine.h"
using namespace KSGRD;

StyleEngine* KSGRD::Style;

StyleEngine::StyleEngine(QObject * parent) : QObject(parent)
{
  mFirstForegroundColor = QColor( 0x888888 );  // Gray
  mSecondForegroundColor = QColor( 0x888888 ); // Gray
  mAlarmColor = QColor( 255, 0, 0 );
  mBackgroundColor = Qt::white;       // white
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


