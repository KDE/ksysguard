/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>

#include <kcolordialog.h>
#include <klocale.h>

#include <ColorPicker.h>

#include "StyleSettings.h"

StyleSettings::StyleSettings( QWidget *parent, const char *name )
  : KDialogBase( Tabbed, i18n( "Global Style Settings" ), Help | Ok | Apply |
                 Cancel, Ok, parent, name, true, true )
{
  QFrame *page = addPage( i18n( "Display Style" ) );
  QGridLayout *layout = new QGridLayout( page, 5, 2, marginHint(), spacingHint() );

  mFirstForegroundColor = new ColorPicker( page );
  mFirstForegroundColor->setText( i18n( "First foreground color" ) );
  layout->addMultiCellWidget( mFirstForegroundColor, 0, 0, 0, 1 );

  mSecondForegroundColor = new ColorPicker( page );
  mSecondForegroundColor->setText( i18n( "Second foreground color" ) );
  layout->addMultiCellWidget( mSecondForegroundColor, 1, 1, 0, 1 );

  mAlarmColor = new ColorPicker( page );
  mAlarmColor->setText( i18n( "Alarm color" ) );
  layout->addMultiCellWidget( mAlarmColor, 2, 2, 0, 1 );

  mBackgroundColor = new ColorPicker( page );
  mBackgroundColor->setText( i18n( "Background color" ) );
  layout->addMultiCellWidget( mBackgroundColor, 3, 3, 0, 1 );

  QLabel *label = new QLabel( i18n( "Font size:" ), page );
  layout->addWidget( label, 4, 0 );

  mFontSize = new QSpinBox( 7, 48, 1, page );
  mFontSize->setValue( 8 );
  layout->addWidget( mFontSize, 4, 1 );
  label->setBuddy( mFontSize );


  page = addPage( i18n( "Sensor Colors" ) );
  layout = new QGridLayout( page, 1, 2, marginHint(), spacingHint() );

  mColorListBox = new QListBox( page );
  layout->addWidget( mColorListBox, 0, 0 );

  mEditColorButton = new QPushButton( i18n( "Change Color..." ), page );
  mEditColorButton->setEnabled( false );
  layout->addWidget( mEditColorButton, 0, 1, Qt::AlignTop );

  connect( mColorListBox, SIGNAL( selectionChanged( QListBoxItem* ) ),
           SLOT( selectionChanged( QListBoxItem* ) ) );
  connect( mColorListBox, SIGNAL( doubleClicked( QListBoxItem* ) ),
           SLOT( editSensorColor() ) );
  connect( mEditColorButton, SIGNAL( clicked() ),
           SLOT( editSensorColor() ) );
}

StyleSettings::~StyleSettings()
{
}

void StyleSettings::setFirstForegroundColor( const QColor &color )
{
  mFirstForegroundColor->setColor( color );
}

QColor StyleSettings::firstForegroundColor() const
{
  return mFirstForegroundColor->color();
}

void StyleSettings::setSecondForegroundColor( const QColor &color )
{
  mSecondForegroundColor->setColor( color );
}

QColor StyleSettings::secondForegroundColor() const
{
  return mSecondForegroundColor->color();
}

void StyleSettings::setAlarmColor( const QColor &color )
{
  mAlarmColor->setColor( color );
}

QColor StyleSettings::alarmColor() const
{
  return mAlarmColor->color();
}

void StyleSettings::setBackgroundColor( const QColor &color )
{
  mBackgroundColor->setColor( color );
}

QColor StyleSettings::backgroundColor() const
{
  return mBackgroundColor->color();
}

void StyleSettings::setFontSize( uint size )
{
  mFontSize->setValue( size );
}

uint StyleSettings::fontSize() const
{
  return mFontSize->value();
}

void StyleSettings::setSensorColors( const QValueList<QColor> &list )
{
  mColorListBox->clear();

  for ( uint i = 0; i < list.count(); ++i ) {
    QPixmap pm( 12, 12 );
		pm.fill( *list.at( i ) );
    mColorListBox->insertItem( pm, QString( i18n( "Color %1" ) ).arg( i ) );
	}
}

QValueList<QColor> StyleSettings::sensorColors()
{
  QValueList<QColor> list;

  for ( uint i = 0; i < mColorListBox->count(); ++i )
    list.append( QColor( mColorListBox->pixmap( i )->convertToImage().pixel( 1, 1 ) ) );

  return list;
}

void StyleSettings::editSensorColor()
{
  int pos = mColorListBox->currentItem();

  if ( pos < 0 )
    return;

  QColor color = mColorListBox->pixmap( pos )->convertToImage().pixel( 1, 1 );

  if ( KColorDialog::getColor( color ) == KColorDialog::Accepted ) {
    QPixmap pm( 12, 12 );
		pm.fill( color );
    mColorListBox->changeItem( pm, mColorListBox->text( pos ), pos );
	}
}

void StyleSettings::selectionChanged( QListBoxItem *item )
{
  mEditColorButton->setEnabled( item != 0 );
}

#include "StyleSettings.moc"
