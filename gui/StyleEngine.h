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

#ifndef KSG_STYLEENGINE_H
#define KSG_STYLEENGINE_H

#include <QColor>
#include <QObject>
#include <QList>

class KConfigGroup;
namespace KSGRD {
class StyleEngine : public QObject
{
  Q_OBJECT

  public:
    StyleEngine(QObject * parent = 0);
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

  private:

    QColor mFirstForegroundColor;
    QColor mSecondForegroundColor;
    QColor mAlarmColor;
    QColor mBackgroundColor;
    uint mFontSize;
    QList<QColor> mSensorColors;
};
extern StyleEngine* Style;

}

#endif
