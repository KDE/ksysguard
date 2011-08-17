/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <kacceleratormanager.h>
#include <klocale.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QDoubleSpinBox>
#include <QWhatsThis>

#include "TimerSettings.h"

TimerSettings::TimerSettings( QWidget *parent, const char *name )
  : KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n( "Timer Settings" ) );
  setButtons( Ok | Cancel );

  QFrame *page = new QFrame( this );
  setMainWidget( page );

  QGridLayout *layout = new QGridLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( 0 );

  mUseGlobalUpdate = new QCheckBox( i18n( "Use update interval of worksheet" ), page );
  layout->addWidget( mUseGlobalUpdate, 0, 0, 1, 2 );

  mLabel = new QLabel( i18n( "Update interval:" ), page );
  layout->addWidget( mLabel, 1, 0 );

  mInterval = new QDoubleSpinBox( page );
  mInterval->setRange( 0.1, 10000 );
  mInterval->setSingleStep( 1 );
  mInterval->setValue( 2 );
  mInterval->setSuffix( i18n( " sec" ) );
  layout->addWidget( mInterval, 1, 1 );
  mLabel->setBuddy( mInterval );
  mInterval->setWhatsThis( i18n( "All displays of the sheet are updated at the rate specified here." ) );

  connect( mUseGlobalUpdate, SIGNAL(toggled(bool)),
           SLOT(globalUpdateChanged(bool)) );

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

void TimerSettings::setInterval( double interval )
{
  mInterval->setValue( interval );
}

double TimerSettings::interval() const
{
  return mInterval->value();
}

void TimerSettings::globalUpdateChanged( bool value )
{
  mInterval->setEnabled( !value );
  mLabel->setEnabled( !value );
}

#include "TimerSettings.moc"
