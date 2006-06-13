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

#include "ListViewSettings.h"
#include "ListViewSettingsWidget.h"

#include <klocale.h>

ListViewSettings::ListViewSettings( QWidget *parent, const char *name )
    : KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n( "List View Settings" ) );
  setButtons( Ok | Apply | Cancel );
  enableButtonSeparator( true );

  m_settingsWidget = new ListViewSettingsWidget( this, "m_settingsWidget" );
  setMainWidget( m_settingsWidget );
}

QString ListViewSettings::title() const
{
  return m_settingsWidget->title();
}

QColor ListViewSettings::textColor() const
{
  return m_settingsWidget->textColor();
}

QColor ListViewSettings::backgroundColor() const
{
  return m_settingsWidget->backgroundColor();
}

QColor ListViewSettings::gridColor() const
{
  return m_settingsWidget->gridColor();
}

void ListViewSettings::setTitle( const QString &title )
{
  m_settingsWidget->setTitle( title );
}

void ListViewSettings::setBackgroundColor( const QColor &c )
{
  m_settingsWidget->setBackgroundColor( c );
}

void ListViewSettings::setTextColor( const QColor &c )
{
  m_settingsWidget->setTextColor( c );
}

void ListViewSettings::setGridColor( const QColor &c )
{
  m_settingsWidget->setGridColor( c );
}

#include "ListViewSettings.moc"

/* vim: et sw=2 ts=2
*/

