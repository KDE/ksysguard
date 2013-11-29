/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <kacceleratormanager.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>

#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>


#include "WorkSheetSettings.h"

WorkSheetSettings::WorkSheetSettings( QWidget* parent, bool locked )
  : KDialog( parent )
{
  setObjectName( "WorkSheetSettings" );
  setModal( true );
  setCaption( i18n( "Tab Properties" ) );
  setButtons( Ok | Cancel );

  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QVBoxLayout *topLayout = new QVBoxLayout( page );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( spacingHint() );

  QGroupBox *group = new QGroupBox( i18n( "Title" ), page );

  QGridLayout *groupLayout = new QGridLayout;
  group->setLayout( groupLayout );
  groupLayout->setAlignment( Qt::AlignTop );

  mSheetTitle = new KLineEdit( group );
  groupLayout->addWidget( mSheetTitle, 0, 0 );

  topLayout->addWidget( group );

  group = new QGroupBox( i18n( "Properties" ), page );

  groupLayout = new QGridLayout;
  group->setLayout( groupLayout );
  groupLayout->setAlignment( Qt::AlignTop );

  int row_num = -1;
  QLabel *label;
  if ( !locked ) {
    label = new QLabel( i18n( "Rows:" ), group );
    groupLayout->addWidget( label, ++row_num, 0 );

    mRows = new KIntNumInput( 3, group );
    mRows->setMaximum( 42 );
    mRows->setMinimum( 1 );
    groupLayout->addWidget( mRows, row_num, 1 );
    label->setBuddy( mRows );

    label = new QLabel( i18n( "Columns:" ), group );
    groupLayout->addWidget( label, ++row_num, 0 );

    mColumns = new KIntNumInput( 1, group );
    mColumns->setMaximum( 42 );
    mColumns->setMinimum( 1 );
    groupLayout->addWidget( mColumns, 1, 1 );
    label->setBuddy( mColumns );
    mRows->setWhatsThis( i18n( "Enter the number of rows the sheet should have." ) );
    mColumns->setWhatsThis( i18n( "Enter the number of columns the sheet should have." ) );
  }
  label = new QLabel( i18n( "Update interval:" ), group );
  groupLayout->addWidget( label, ++row_num, 0 );

  mInterval = new KDoubleNumInput( 0.00/*minimum*/, 1000.0/*maximum*/, 0.5/*default*/, group/*parent*/, 0.5/*stepsize*/, 2/*precision*/ );
  mInterval->setSuffix( i18n( " sec" ) );
  groupLayout->addWidget( mInterval, row_num, 1 );
  label->setBuddy( mInterval );

  topLayout->addWidget( group );

  mInterval->setWhatsThis( i18n( "All displays of the sheet are updated at the rate specified here." ) );
  mSheetTitle->setToolTip( i18n( "Enter the title of the worksheet here." ) );

  KAcceleratorManager::manage( page );

  mSheetTitle->setFocus();

//  resize( QSize( 250, 230 ).expandedTo( minimumSizeHint() ) );
}

WorkSheetSettings::~WorkSheetSettings()
{
}

void WorkSheetSettings::setRows( int rows )
{
  mRows->setValue( rows );
}

int WorkSheetSettings::rows() const
{
  return mRows->value();
}

void WorkSheetSettings::setColumns( int columns )
{
  mColumns->setValue( columns );
}

int WorkSheetSettings::columns() const
{
  return mColumns->value();
}

void WorkSheetSettings::setInterval( float interval )
{
  mInterval->setValue( interval );
}

float WorkSheetSettings::interval() const
{
  return mInterval->value();
}

void WorkSheetSettings::setSheetTitle( const QString &title )
{
  mSheetTitle->setText( title );
}

QString WorkSheetSettings::sheetTitle() const
{
  return mSheetTitle->text();
}

#include "WorkSheetSettings.moc"
