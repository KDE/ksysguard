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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#include <kaccelmanager.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include "TimerSettings.h"

TimerSettings::TimerSettings( QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Timer Settings" ), Ok | Cancel,
                 Ok, parent, name, true, true )
{
  QFrame *page = plainPage();

  QGridLayout *layout = new QGridLayout( page, 2, 2, 0, spacingHint() );

  mUseGlobalUpdate = new QCheckBox( i18n( "Use update interval of worksheet" ), page );
  layout->addMultiCellWidget( mUseGlobalUpdate, 0, 0, 0, 1 );

  mLabel = new QLabel( i18n( "Update interval:" ), page );
  layout->addWidget( mLabel, 1, 0 );

  mInterval = new QSpinBox( 2, 300, 1, page );
  mInterval->setValue( 2 );
  mInterval->setSuffix( i18n( " sec" ) );
  layout->addWidget( mInterval, 1, 1 );
  mLabel->setBuddy( mInterval );
  QWhatsThis::add( mInterval, i18n( "All displays of the sheet are updated at the rate specified here." ) );

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
