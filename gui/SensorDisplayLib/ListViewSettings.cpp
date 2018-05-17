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
#include "ui_ListViewSettingsWidget.h"

#include <KLocalizedString>

ListViewSettings::ListViewSettings(QWidget *parent, const QString &name )
    : QDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setWindowTitle( i18n( "List View Settings" ) );

  QWidget *widget = new QWidget( this );
  m_settingsWidget = new Ui_ListViewSettingsWidget;
  m_settingsWidget->setupUi( widget );

  connect(m_settingsWidget->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_settingsWidget->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->addWidget(widget);
  setLayout(vlayout);
}

ListViewSettings::~ListViewSettings()
{
  delete m_settingsWidget;
}

QString ListViewSettings::title() const
{
  return m_settingsWidget->m_title->text();
}

QColor ListViewSettings::textColor() const
{
  return m_settingsWidget->m_textColor->color();
}

QColor ListViewSettings::backgroundColor() const
{
  return m_settingsWidget->m_backgroundColor->color();
}

QColor ListViewSettings::gridColor() const
{
  return m_settingsWidget->m_gridColor->color();
}

void ListViewSettings::setTitle( const QString &title )
{
  m_settingsWidget->m_title->setText( title );
}

void ListViewSettings::setBackgroundColor( const QColor &c )
{
  m_settingsWidget->m_backgroundColor->setColor( c );
}

void ListViewSettings::setTextColor( const QColor &c )
{
  m_settingsWidget->m_textColor->setColor( c );
}

void ListViewSettings::setGridColor( const QColor &c )
{
  m_settingsWidget->m_gridColor->setColor( c );
}



/* vim: et sw=2 ts=2
*/

