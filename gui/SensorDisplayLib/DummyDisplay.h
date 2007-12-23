/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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

#ifndef KSG_DUMMYDISPLAY_H
#define KSG_DUMMYDISPLAY_H

#include <SensorDisplay.h>
#include <QEvent>

class DummyDisplay : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    DummyDisplay( QWidget* parent, SharedSettings *workSheetSettings);
    virtual ~DummyDisplay() {}

    virtual bool eventFilter( QObject*, QEvent* );
};

#endif
