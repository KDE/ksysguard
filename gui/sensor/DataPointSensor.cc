/*
 KSysGuard, the KDE System Guard

 Copyright (c) 2009 Sebastien Martel <sebastiendevel@gmail.com>

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
#include "DataPointSensor.h"

DataPointSensor::DataPointSensor(const QString argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor) : BasicSensor(argName, argHostName, argType, argRegexpName) {
 init(argSensorColor);
}

DataPointSensor::DataPointSensor(const QList<QString> argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor) : BasicSensor(argName, argHostName, argType, argRegexpName) {
 init(argSensorColor);
}

DataPointSensor::~DataPointSensor() {
}

QString DataPointSensor::unit() const  {
    return mUnit;
}

void DataPointSensor::setUnit(const QString &argUnit)  {
    mUnit = argUnit;
}

void DataPointSensor::setColor(const QColor argColor)  {
    mSensorColor = argColor;
}

void DataPointSensor::removeOldestValue(int argNumberToRemove) {
    while (argNumberToRemove-- > 0)  {
        mSensorData.removeFirst();
    }
}


double DataPointSensor::removeOneOldestValue()  {
    return mSensorData.takeFirst();
}

void DataPointSensor::putReportedMaxValue(double argReportedMaxValue)
{
   mTheorethicalMaxValue = argReportedMaxValue;
}

void DataPointSensor::addData(double argValue)  {
    mSensorData.append(argValue);
}

double DataPointSensor::lastValue(int argIndex) const {
    return lastValue();
}

void DataPointSensor::init(const QColor argSensorColor)  {
    setColor(argSensorColor);
    mTheorethicalMaxValue = 0;
    mLastSeenValue = 0;
    mPrevSeenValue = 0;
}

