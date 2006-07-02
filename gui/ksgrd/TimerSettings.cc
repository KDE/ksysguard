/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    
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

#include <kacceleratormanager.h>
#include <klocale.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>
#include <QWhatsThis>

#include "TimerSettings.h"

TimerSettings::TimerSettings( QWidget *parent, const char *name )
  : KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n( "Timer Settings" ) );
  setButtons( Ok | Cancel );
  showButtonSeparator( true );

  QFrame *page = new QFrame( this );
  setMainWidget( page );

  QGridLayout *layout = new QGridLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( 0 );

  mUseGlobalUpdate = new QCheckBox( i18n( "Use update interval of worksheet" ), page );
  layout->addWidget( mUseGlobalUpdate, 0, 0, 1, 2 );

  mLabel = new QLabel( i18n( "Update interval:" ), page );
  layout->addWidget( mLabel, 1, 0 );

  mInterval = new QSpinBox( page );
  mInterval->setRange( 1, 300 );
  mInterval->setSingleStep( 1 );
  mInterval->setValue( 2 );
  mInterval->setSuffix( i18n( " sec" ) );
  layout->addWidget( mInterval, 1, 1 );
  mLabel->setBuddy( mInterval );
  mInterval->setWhatsThis( i18n( "All displays of the sheet are updated at the rate specified here." ) );

  connect( mUseGlobalUpdate, SIGNAL( toggled( bool ) ),
           SLOT( globalUpdateChanged( bool ) ) );

  mUseGlobalUpdate->setChecked( true );

  KAcceleratorManager::manage( this );
}

TimerSettings::~TimerSettings()
{
}

void TimerSettings::setUseGlobalUpdate( bool value )
{
  mUseGlobalUpdate->setChecked( value );
}

bool TimerSettings::useGlobalUpdate() const
{
  return mUseGlobalUpdate->isChecked();
}

void TimerSettings::setInterval( int interval )
{
  mInterval->setValue( interval );
}

int TimerSettings::interval() const
{
  return mInterval->value();
}

void TimerSettings::globalUpdateChanged( bool value )
{
  mInterval->setEnabled( !value );
  mLabel->setEnabled( !value );
}

#include "TimerSettings.moc"
