/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000, 2001 Chris Schlaeger <cs@kde.org>

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

*/

#include <klocale.h>
#include <ksgrd/SensorManager.h>
#include <QMouseEvent>
#include <QGridLayout>

#include "DummyDisplay.h"

DummyDisplay::DummyDisplay( QWidget* parent, SharedSettings *workSheetSettings )
  : KSGRD::SensorDisplay( parent, i18n( "Drop Sensor Here" ), workSheetSettings )
{
  setWhatsThis(i18n("This is an empty space in a worksheet. Drag a sensor from "
                    "the Sensor Browser and drop it here. A sensor display will "
                    "appear that allows you to monitor the values of the sensor "
                    "over time." ) );

  QLabel *label = new QLabel(this);
  label->setText("Drop Sensor Here");
  label->setAlignment( Qt::AlignCenter );

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(label);

   this->setLayout(layout);
}

bool DummyDisplay::eventFilter( QObject* object, QEvent* event )
{
  if ( event->type() == QEvent::MouseButtonRelease &&
       ( (QMouseEvent*)event)->button() == Qt::LeftButton )
    setFocus();

  return QWidget::eventFilter( object, event );
}

#include "DummyDisplay.moc"
