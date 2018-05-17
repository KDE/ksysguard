/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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
    explicit StyleEngine(QObject * parent = nullptr);
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
