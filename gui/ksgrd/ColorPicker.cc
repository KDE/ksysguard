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

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kcolordialog.h>
#include <klocale.h>

#include "ColorPicker.h"

ColorPicker::ColorPicker( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QHBoxLayout *layout = new QHBoxLayout( this );

  mText = new QLabel( this );
  layout->addWidget( mText );
  layout->addSpacing( 8 );

  mColorBox = new QFrame( this );
  mColorBox->setFixedSize( 16, 16 );
  mColorBox->setFrameShape( QFrame::WinPanel );
  mColorBox->setFrameShadow( QFrame::Sunken );
  layout->addWidget( mColorBox );
  layout->addSpacing( 8 );

  mButton = new QPushButton( this );
  mButton->setText( i18n( "Change Color..." ) );
  layout->addWidget( mButton );

  mText->setBuddy( mButton );

  connect( mButton, SIGNAL( clicked() ), SLOT( raiseColorDialog() ) );
}

ColorPicker::~ColorPicker()
{
}

void ColorPicker::raiseColorDialog()
{
  QColor c = color();

	if ( KColorDialog::getColor( c, this->parentWidget() ) == KColorDialog::Accepted )
    setColor( c );
}

QString ColorPicker::text() const
{
  return mText->text();
}

void ColorPicker::setText( const QString &text )
{
  mText->setText( text );
}

QColor ColorPicker::color() const
{
  return mColorBox->palette().color( QPalette::Normal, QColorGroup::Background );
}

void ColorPicker::setColor( const QColor &color )
{
  QPalette palette = mColorBox->palette();
  palette.setColor( QPalette::Normal, QColorGroup::Background, color );
  mColorBox->setPalette( palette );
}

#include "ColorPicker.moc"
