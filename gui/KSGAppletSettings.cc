/*
    This file is part of KSysGuard.
    Copyright ( C ) 2002 Nadeem Hasan ( nhasan@kde.org )

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or ( at your option ) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QLabel>
#include <QLayout>
#include <QSpinBox>

#include <kacceleratormanager.h>
#include <klocale.h>

#include "KSGAppletSettings.h"

KSGAppletSettings::KSGAppletSettings( QWidget *parent)
    : KDialog( parent )
{
  setModal( false );
  setCaption( i18n( "System Guard Applet Settings" ) );
  setButtons( Ok|Apply|Cancel );
  showButtonSeparator( true );

  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Number of displays:" ), page );
  topLayout->addWidget( label, 0, 0 );

  mNumDisplay = new QSpinBox( page );
  mNumDisplay->setMinimum( 1 );
  mNumDisplay->setMaximum( 32 );
  mNumDisplay->setValue(2);
  topLayout->addWidget( mNumDisplay, 0, 1 );
  label->setBuddy( mNumDisplay );

  label = new QLabel( i18n( "Size ratio:" ), page );
  topLayout->addWidget( label, 1, 0 );

  mSizeRatio = new QSpinBox(page );
  mSizeRatio->setValue(100);
  mSizeRatio->setMinimum( 10 );
  mSizeRatio->setMaximum( 500 );
  mSizeRatio->setSingleStep(20);
  mSizeRatio->setSuffix( i18nc( "Number suffix in spinbox", "%" ) );
  topLayout->addWidget( mSizeRatio, 1, 1 );
  label->setBuddy( mSizeRatio );

  label = new QLabel( i18n( "Update interval:" ), page );
  topLayout->addWidget( label, 2, 0 );

  mInterval = new QSpinBox( page );
  mInterval->setMinimum( 1 );
  mInterval->setMaximum( 300 );
  mInterval->setValue(2);
  mInterval->setSuffix( i18n( " sec" ) );
  topLayout->addWidget( mInterval, 2, 1 );
  label->setBuddy( mInterval );

  resize( QSize( 250, 130 ).expandedTo( minimumSizeHint() ) );

  KAcceleratorManager::manage( page );
}

KSGAppletSettings::~KSGAppletSettings()
{
}

int KSGAppletSettings::numDisplay() const
{
  return mNumDisplay->value();
}

void KSGAppletSettings::setNumDisplay( int value )
{
  mNumDisplay->setValue( value );
}

int KSGAppletSettings::sizeRatio() const
{
  return mSizeRatio->value();
}

void KSGAppletSettings::setSizeRatio( int value )
{
  mSizeRatio->setValue( value );
}

double KSGAppletSettings::updateInterval() const
{
  return mInterval->value();
}

void KSGAppletSettings::setUpdateInterval( double value )
{
  mInterval->setValue( value );
}

