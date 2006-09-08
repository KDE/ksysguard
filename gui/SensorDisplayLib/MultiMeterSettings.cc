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
#include "ui_MultiMeterSettingsWidget.h"

#include <klocale.h>
#include <knumvalidator.h>

MultiMeterSettings::MultiMeterSettings( QWidget *parent, const char *name )
    : KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n( "Multimeter Settings" ) );
  setButtons( Ok|Cancel );
  showButtonSeparator( true );

  QWidget *mainWidget = new QWidget( this );

  m_settingsWidget = new Ui_MultiMeterSettingsWidget;
  m_settingsWidget->setupUi( mainWidget );
  m_settingsWidget->m_lowerLimit->setValidator(new KDoubleValidator(m_settingsWidget->m_lowerLimit));
  m_settingsWidget->m_upperLimit->setValidator(new KDoubleValidator(m_settingsWidget->m_upperLimit));

  m_settingsWidget->m_title->setFocus();

  setMainWidget( mainWidget );
}

MultiMeterSettings::~MultiMeterSettings()
{
  delete m_settingsWidget;
}

QString MultiMeterSettings::title()
{
  return m_settingsWidget->m_title->text();
}

bool MultiMeterSettings::showUnit()
{
  return m_settingsWidget->m_showUnit->isChecked();
}

bool MultiMeterSettings::lowerLimitActive()
{
  return m_settingsWidget->m_lowerLimitActive->isChecked();
}

bool MultiMeterSettings::upperLimitActive()
{
  return m_settingsWidget->m_upperLimitActive->isChecked();
}

double MultiMeterSettings::lowerLimit()
{
  return m_settingsWidget->m_lowerLimit->text().toDouble();
}

double MultiMeterSettings::upperLimit()
{
  return m_settingsWidget->m_upperLimit->text().toDouble();
}

QColor MultiMeterSettings::normalDigitColor()
{
  return m_settingsWidget->m_normalDigitColor->color();
}

QColor MultiMeterSettings::alarmDigitColor()
{
  return m_settingsWidget->m_alarmDigitColor->color();
}

QColor MultiMeterSettings::meterBackgroundColor()
{
  return m_settingsWidget->m_backgroundColor->color();
}

void MultiMeterSettings::setTitle( const QString &title )
{
  m_settingsWidget->m_title->setText( title );
}

void MultiMeterSettings::setShowUnit( bool b )
{
  m_settingsWidget->m_showUnit->setChecked( b );
}

void MultiMeterSettings::setLowerLimitActive( bool b )
{
  m_settingsWidget->m_lowerLimitActive->setChecked( b );
}

void MultiMeterSettings::setUpperLimitActive( bool b )
{
  m_settingsWidget->m_upperLimitActive->setChecked( b );
}

void MultiMeterSettings::setLowerLimit( double limit )
{
  m_settingsWidget->m_lowerLimit->setText( QString::number( limit ) );
}

void MultiMeterSettings::setUpperLimit( double limit )
{
  m_settingsWidget->m_upperLimit->setText( QString::number( limit ) );
}

void MultiMeterSettings::setNormalDigitColor( const QColor &c )
{
  m_settingsWidget->m_normalDigitColor->setColor( c );
}

void MultiMeterSettings::setAlarmDigitColor( const QColor &c )
{
  m_settingsWidget->m_alarmDigitColor->setColor( c );
}

void MultiMeterSettings::setMeterBackgroundColor( const QColor &c )
{
  m_settingsWidget->m_backgroundColor->setColor( c );
}

#include "MultiMeterSettings.moc"

/* vim: et sw=2 ts=2
*/

