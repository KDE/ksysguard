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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

#include <kaccelmanager.h>
#include <kcolorbutton.h>
#include <kcolordialog.h>
#include <klocale.h>

#include "StyleSettings.h"

StyleSettings::StyleSettings( QWidget *parent, const char *name )
  : KDialogBase( Tabbed, i18n( "Global Style Settings" ), Help | Ok | Apply |
                 Cancel, Ok, parent, name, true, true )
{
  QFrame *page = addPage( i18n( "Display Style" ) );
  QGridLayout *layout = new QGridLayout( page, 5, 2, marginHint(), spacingHint() );

  QLabel *label = new QLabel( i18n( "First foreground color:" ), page );
  layout->addWidget( label, 0, 0 );

  mFirstForegroundColor = new KColorButton( page );
  layout->addWidget( mFirstForegroundColor, 0, 1 );
  label->setBuddy( mFirstForegroundColor );

  label = new QLabel( i18n( "Second foreground color:" ), page );
  layout->addWidget( label, 1, 0 );

  mSecondForegroundColor = new KColorButton( page );
  layout->addWidget( mSecondForegroundColor, 1, 1 );
  label->setBuddy( mSecondForegroundColor );

  label = new QLabel( i18n( "Alarm color:" ), page );
  layout->addWidget( label, 2, 0 );

  mAlarmColor = new KColorButton( page );
  layout->addWidget( mAlarmColor, 2, 1 );
  label->setBuddy( mAlarmColor );

  label = new QLabel( i18n( "Background color:" ), page );
  layout->addWidget( label, 3, 0 );

  mBackgroundColor = new KColorButton( page );
  layout->addWidget( mBackgroundColor, 3, 1 );
  label->setBuddy( mBackgroundColor );

  label = new QLabel( i18n( "Font size:" ), page );
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

  KAcceleratorManager::manage( this );
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
    mColorListBox->insertItem( pm, i18n( "Color %1" ).arg( i ) );
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
