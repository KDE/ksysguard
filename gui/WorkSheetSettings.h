/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef KSG_WORKSHEETSETTINGS_H
#define KSG_WORKSHEETSETTINGS_H

#include <QDialog>

class QSpinBox;

class KLineEdit;
class KDoubleNumInput;

class WorkSheetSettings : public QDialog
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

    QSpinBox* mColumns;
    KDoubleNumInput* mInterval;
    QSpinBox* mRows;
};

#endif
