/*  This file is part of the KDE Libraries
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <klocale.h>
#include <knuminput.h>

#include "KSGAppletSettings.h"

KSGAppletSettings::KSGAppletSettings( QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, QString::null, Ok|Apply|Cancel, 
      Ok, true ),
      widget_( 0 ) 
{
  setCaption( i18n( "KSysGuard Applet Settings" ) );

  widget_ = new KSGAppletSettingsWidget( this );
  setMainWidget( widget_ );
}

KSGAppletSettings::~KSGAppletSettings()
{
  delete widget_;
}

int KSGAppletSettings::numDisplay()
{
  return widget_->sbNumDisplay->value();
}

void KSGAppletSettings::setNumDisplay( int n )
{
  widget_->sbNumDisplay->setValue( n );
}

int KSGAppletSettings::sizeRatio()
{
  return widget_->sbSizeRatio->value();
}

void KSGAppletSettings::setSizeRatio( int n )
{
  widget_->sbSizeRatio->setValue( n );
}

int KSGAppletSettings::updateInterval()
{
  return widget_->sbInterval->value();
}

void KSGAppletSettings::setUpdateInterval( int n )
{
  widget_->sbInterval->setValue( n );
}

