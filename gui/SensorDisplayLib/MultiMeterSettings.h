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


#ifndef MULTIMETERSETTINGS_H
#define MULTIMETERSETTINGS_H

#include <kdialogbase.h>

#include <QString>
#include <QColor>

class MultiMeterSettingsWidget;

class MultiMeterSettings : public KDialogBase
{
  Q_OBJECT

  public:

    MultiMeterSettings( QWidget *parent=0, const char *name=0 );

    QString title();
    bool showUnit();
    bool lowerLimitActive();
    bool upperLimitActive();
    double lowerLimit();
    double upperLimit();
    QColor normalDigitColor();
    QColor alarmDigitColor();
    QColor meterBackgroundColor();

    void setTitle( const QString & );
    void setShowUnit( bool );
    void setLowerLimitActive( bool );
    void setUpperLimitActive( bool );
    void setLowerLimit( double );
    void setUpperLimit( double );
    void setNormalDigitColor( const QColor & );
    void setAlarmDigitColor( const QColor & );
    void setMeterBackgroundColor( const QColor & );

  private:

    MultiMeterSettingsWidget *m_settingsWidget;
};

#endif // MULTIMETERSETTINGS_H

/* vim: et sw=2 ts=2
*/

