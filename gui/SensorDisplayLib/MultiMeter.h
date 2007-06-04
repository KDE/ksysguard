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

#ifndef _MultiMeter_h_
#define _MultiMeter_h_

#include <SensorDisplay.h>

class QLCDNumber;
class QResizeEvent;

class MultiMeter : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    MultiMeter(QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
    virtual ~MultiMeter()
    {
    }

    bool addSensor(const QString& hostName, const QString& sensorName,
                   const QString& sensorType, const QString& sensorDescr);
    void answerReceived(int id, const QList<QByteArray>& answerlist);
    void resizeEvent(QResizeEvent*);

    bool restoreSettings(QDomElement& element);
    bool saveSettings(QDomDocument& doc, QDomElement& element);

    virtual bool hasSettingsDialog() const
    {
      return true;
    }

    void configureSettings();

  public Q_SLOTS:
    void applyStyle();

  private:
    void setDigitColor(const QColor&);
    void setBackgroundColor(const QColor&);

    QLCDNumber* mLcd;
    QColor mNormalDigitColor;
    QColor mAlarmDigitColor;
    QColor mBackgroundColor;

    bool mLowerLimitActive;
    double mLowerLimit;
    bool mUpperLimitActive;
    double mUpperLimit;
};

#endif
