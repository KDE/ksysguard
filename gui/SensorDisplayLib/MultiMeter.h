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

#ifndef _MultiMeter_h_
#define _MultiMeter_h_

#include <SensorDisplay.h>

class QLCDNumber;

class MultiMeter : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    MultiMeter(QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
    ~MultiMeter() Q_DECL_OVERRIDE
    {
    }

    bool addSensor(const QString& hostName, const QString& sensorName,
                   const QString& sensorType, const QString& sensorDescr) Q_DECL_OVERRIDE;
    void answerReceived(int id, const QList<QByteArray>& answerlist) Q_DECL_OVERRIDE;
    bool restoreSettings(QDomElement& element) Q_DECL_OVERRIDE;
    bool saveSettings(QDomDocument& doc, QDomElement& element) Q_DECL_OVERRIDE;

    bool hasSettingsDialog() const Q_DECL_OVERRIDE
    {
      return true;
    }

    void configureSettings() Q_DECL_OVERRIDE;

  public Q_SLOTS:
    void applyStyle() Q_DECL_OVERRIDE;

  private:
    void setDigitColor(const QColor&);
    void setBackgroundColor(const QColor&);

    QLCDNumber* mLcd;
    QColor mNormalDigitColor;
    QColor mAlarmDigitColor;
    QColor mBackgroundColor;

    bool mIsFloat;

    bool mLowerLimitActive;
    double mLowerLimit;
    bool mUpperLimitActive;
    double mUpperLimit;
};

#endif
