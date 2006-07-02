/* This file is part of the KDE project
   Copyright ( C ) 2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "MultiMeterSettings.h"
#include "MultiMeterSettingsWidget.h"

#include <klocale.h>

MultiMeterSettings::MultiMeterSettings( QWidget *parent, const char *name )
    : KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n( "Multimeter Settings" ) );
  setButtons( Ok|Apply|Cancel );
  showButtonSeparator( true );

  m_settingsWidget = new MultiMeterSettingsWidget( this, "m_settingsWidget" );
  setMainWidget( m_settingsWidget );
}

QString MultiMeterSettings::title()
{
  return m_settingsWidget->title();
}

bool MultiMeterSettings::showUnit()
{
  return m_settingsWidget->showUnit();
}

bool MultiMeterSettings::lowerLimitActive()
{
  return m_settingsWidget->lowerLimitActive();
}

bool MultiMeterSettings::upperLimitActive()
{
  return m_settingsWidget->upperLimitActive();
}

double MultiMeterSettings::lowerLimit()
{
  return m_settingsWidget->lowerLimit();
}

double MultiMeterSettings::upperLimit()
{
  return m_settingsWidget->upperLimit();
}

QColor MultiMeterSettings::normalDigitColor()
{
  return m_settingsWidget->normalDigitColor();
}

QColor MultiMeterSettings::alarmDigitColor()
{
  return m_settingsWidget->alarmDigitColor();
}

QColor MultiMeterSettings::meterBackgroundColor()
{
  return m_settingsWidget->meterBackgroundColor();
}

void MultiMeterSettings::setTitle( const QString &title )
{
  m_settingsWidget->setTitle( title );
}

void MultiMeterSettings::setShowUnit( bool b )
{
  m_settingsWidget->setShowUnit( b );
}

void MultiMeterSettings::setLowerLimitActive( bool b )
{
  m_settingsWidget->setLowerLimitActive( b );
}

void MultiMeterSettings::setUpperLimitActive( bool b )
{
  m_settingsWidget->setUpperLimitActive( b );
}

void MultiMeterSettings::setLowerLimit( double limit )
{
  m_settingsWidget->setLowerLimit( limit );
}

void MultiMeterSettings::setUpperLimit( double limit )
{
  m_settingsWidget->setUpperLimit( limit );
}

void MultiMeterSettings::setNormalDigitColor( const QColor &c )
{
  m_settingsWidget->setNormalDigitColor( c );
}

void MultiMeterSettings::setAlarmDigitColor( const QColor &c )
{
  m_settingsWidget->setAlarmDigitColor( c );
}

void MultiMeterSettings::setMeterBackgroundColor( const QColor &c )
{
  m_settingsWidget->setMeterBackgroundColor( c );
}

#include "MultiMeterSettings.moc"

/* vim: et sw=2 ts=2
*/

