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

#ifndef KSG_STYLEENGINE_H
#define KSG_STYLEENGINE_H

#include <QColor>
#include <QObject>
#include <QList>

class KConfigGroup;
class StyleSettings;
namespace KSGRD {
class StyleEngine : public QObject
{
  Q_OBJECT

  public:
    StyleEngine();
    ~StyleEngine();

    void readProperties( const KConfigGroup& cfg );
    void saveProperties( KConfigGroup& cfg );

    const QColor& firstForegroundColor() const;
    const QColor& secondForegroundColor() const;
    const QColor& alarmColor() const;
    const QColor& backgroundColor() const;

    uint fontSize() const;

    const QColor& sensorColor( int pos );
    uint numSensorColors() const;

  public Q_SLOTS:
    void configure();
    void applyToWorksheet();

  Q_SIGNALS:
    void applyStyleToWorksheet();

  private:
    void apply();

    QColor mFirstForegroundColor;
    QColor mSecondForegroundColor;
    QColor mAlarmColor;
    QColor mBackgroundColor;
    uint mFontSize;
    QList<QColor> mSensorColors;

    StyleSettings *mSettingsDialog;
};
extern StyleEngine* Style;

}

#endif
