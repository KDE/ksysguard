/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

#include "SensorFrame.h"
#include <QBoxLayout>



SensorFrame::SensorFrame(KSGRD::SensorDisplay* display)
{
  setAlignment(Qt::AlignCenter);
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(2);
  layout->addWidget(display);
  setLayout(layout);
  connect(display, SIGNAL(changeTitle(const QString&)), SLOT(setTitle(const QString&)));
  setTitle(display->title());
  setFlat(true);
}
void SensorFrame::setTitle(const QString& title) {
  QGroupBox::setTitle(title);
}

#include "SensorFrame.moc"

