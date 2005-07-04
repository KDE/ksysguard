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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_STYLEENGINE_H
#define KSG_STYLEENGINE_H

#include <qcolor.h>
#include <qobject.h>
#include <qptrlist.h>

#include <kdemacros.h>

class KConfig;

class QListBoxItem;

class StyleSettings;

namespace KSGRD {

class KDE_EXPORT StyleEngine : public QObject
{
  Q_OBJECT

  public:
    StyleEngine();
    ~StyleEngine();

    void readProperties( KConfig* );
    void saveProperties( KConfig* );

    const QColor& firstForegroundColor() const;
    const QColor& secondForegroundColor() const;
    const QColor& alarmColor() const;
    const QColor& backgroundColor() const;

    uint fontSize() const;

    const QColor& sensorColor( uint pos );
    uint numSensorColors() const;

  public slots:
    void configure();
    void applyToWorksheet();

  signals:
	  void applyStyleToWorksheet();

  private:
    void apply();

    QColor mFirstForegroundColor;
    QColor mSecondForegroundColor;
    QColor mAlarmColor;
    QColor mBackgroundColor;
    uint mFontSize;
    QValueList<QColor> mSensorColors;

    StyleSettings *mSettingsDialog;
};

KDE_EXPORT extern StyleEngine* Style;

}

#endif
