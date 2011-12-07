/* This file is part of the KDE project
   Copyright ( C ) 2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (  at your option ) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef SENSORLOGGERDLG_H
#define SENSORLOGGERDLG_H

#include <kdialog.h>


#include <QColor>

class Ui_SensorLoggerDlgWidget;

class SensorLoggerDlg : public KDialog
{
  Q_OBJECT

  public:

    explicit SensorLoggerDlg( QWidget *parent=0, const char *name=0 );
    ~SensorLoggerDlg();

    QString fileName() const;
    int timerInterval() const;
    bool lowerLimitActive() const;
    bool upperLimitActive() const;
    double lowerLimit() const;
    double upperLimit() const;

    void setFileName( const QString & );
    void setTimerInterval( int );
    void setLowerLimitActive( bool );
    void setUpperLimitActive( bool );
    void setLowerLimit( double );
    void setUpperLimit( double );

  private:

    Ui_SensorLoggerDlgWidget *m_loggerWidget;

  private Q_SLOTS:
    void fileNameTextChanged();
};

#endif // SENSORLOGGERDLG_H

/* vim: et sw=2 ts=2
*/

