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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "SensorLoggerDlg.h"
#include "SensorLoggerDlgWidget.h"

#include <qlayout.h>

#include <klocale.h>

SensorLoggerDlg::SensorLoggerDlg( QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, i18n( "Sensor Logger" ),
      Ok|Cancel, Ok, true )
{
  QWidget *main = new QWidget( this );

  QVBoxLayout *topLayout = new QVBoxLayout( main, 0, KDialog::spacingHint() );

  m_loggerWidget = new SensorLoggerDlgWidget( main, "m_loggerWidget" );
  topLayout->addWidget( m_loggerWidget );
  topLayout->addStretch();

  setMainWidget( main );
}

QString SensorLoggerDlg::fileName() const
{
  return m_loggerWidget->fileName();
}

int SensorLoggerDlg::timerInterval() const
{
  return m_loggerWidget->timerInterval();
}

bool SensorLoggerDlg::lowerLimitActive() const
{
  return m_loggerWidget->lowerLimitActive();
}

bool SensorLoggerDlg::upperLimitActive() const
{
  return m_loggerWidget->upperLimitActive();
}

double SensorLoggerDlg::lowerLimit() const
{
  return m_loggerWidget->lowerLimit();
}

double SensorLoggerDlg::upperLimit() const
{
  return m_loggerWidget->upperLimit();
}

void SensorLoggerDlg::setFileName( const QString &url )
{
  m_loggerWidget->setFileName( url );
}

void SensorLoggerDlg::setTimerInterval( int i )
{
  m_loggerWidget->setTimerInterval( i );
}

void SensorLoggerDlg::setLowerLimitActive( bool b )
{
  m_loggerWidget->setLowerLimitActive( b );
}

void SensorLoggerDlg::setUpperLimitActive( bool b )
{
  m_loggerWidget->setUpperLimitActive( b );
}

void SensorLoggerDlg::setLowerLimit( double limit )
{
  m_loggerWidget->setLowerLimit( limit );
}

void SensorLoggerDlg::setUpperLimit( double limit )
{
  m_loggerWidget->setUpperLimit( limit );
}

#include "SensorLoggerDlg.moc"

/* vim: et sw=2 ts=2
*/

