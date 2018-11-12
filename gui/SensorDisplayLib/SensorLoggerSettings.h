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


#ifndef SENSORLOGGERSETTINGS_H
#define SENSORLOGGERSETTINGS_H

#include <QDialog>

class QColor;

class Ui_SensorLoggerSettingsWidget;

class SensorLoggerSettings : public QDialog
{
  Q_OBJECT

  public:

    explicit SensorLoggerSettings(QWidget *parent=nullptr, const QString &name = {} );
    ~SensorLoggerSettings();

    QString title() const;
    QColor foregroundColor();
    QColor backgroundColor();
    QColor alarmColor();

    void setTitle( const QString & );
    void setForegroundColor( const QColor & );
    void setBackgroundColor( const QColor & );
    void setAlarmColor( const QColor & );

  private:

    Ui_SensorLoggerSettingsWidget *m_settingsWidget;
};

#endif // SENSORLOGGERSETTINGS_H

/* vim: et sw=2 ts=2
*/
