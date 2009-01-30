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

#ifndef KSG_WORKSHEETSETTINGS_H
#define KSG_WORKSHEETSETTINGS_H

#include <kdialog.h>

class KLineEdit;
class KDoubleNumInput;
class KIntNumInput;

class WorkSheetSettings : public KDialog
{
  Q_OBJECT

  public:
    WorkSheetSettings( QWidget* parent, bool locked );
    ~WorkSheetSettings();

    void setRows( int rows );
    int rows() const;

    void setColumns( int columns );
    int columns() const;

    void setInterval( float interval );
    float interval() const;

    void setSheetTitle( const QString &title );
    QString sheetTitle() const;

  private:
    KLineEdit* mSheetTitle;

    KIntNumInput* mColumns;
    KDoubleNumInput* mInterval;
    KIntNumInput* mRows;
};

#endif
