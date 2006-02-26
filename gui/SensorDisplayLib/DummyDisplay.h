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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_DUMMYDISPLAY_H
#define KSG_DUMMYDISPLAY_H

#include <SensorDisplay.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QEvent>

class DummyDisplay : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    DummyDisplay( QWidget* parent = 0, const char* name = 0,
                  const QString& = QString(), double min = 0,
                  double max = 0 );
    virtual ~DummyDisplay() {}

    void resizeEvent( QResizeEvent* );

    virtual bool eventFilter( QObject*, QEvent* );
};

#endif
