/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    
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

*/

#ifndef KSG_WORKSHEETSETTINGS_H
#define KSG_WORKSHEETSETTINGS_H

#include <kdialogbase.h>

class KLineEdit;
class KIntNumInput;

class WorkSheetSettings : public KDialogBase
{
  Q_OBJECT

  public:
    WorkSheetSettings( QWidget* parent = 0, const char* name = 0 );
    ~WorkSheetSettings();

    void setRows( int rows );
    int rows() const;

    void setColumns( int columns );
    int columns() const;

    void setInterval( int interval );
    int interval() const;

    void setSheetTitle( const QString &title );
    QString sheetTitle() const;

  private:
    KLineEdit* mSheetTitle;

    KIntNumInput* mColumns;
    KIntNumInput* mInterval;
    KIntNumInput* mRows;
};

#endif
