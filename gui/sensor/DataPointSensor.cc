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

DataPointSensor::DataPointSensor(const QString &name, const QString &hostName, const QString &type, const QString &regexpName, const QColor &sensorColor) : BasicSensor(name, hostName, type, regexpName) {
 init(sensorColor);
}

DataPointSensor::DataPointSensor(const QList<QString> &name, const QString &hostName, const QString &type, const QString &regexpName, const QColor &sensorColor) : BasicSensor(name, hostName, type, regexpName) {
 init(sensorColor);
}

DataPointSensor::~DataPointSensor() {
}

QString DataPointSensor::unit() const  {
    return mUnit;
}

void DataPointSensor::setUnit(const QString &unit)  {
    mUnit = unit;
}

void DataPointSensor::setColor(const QColor &color)  {
    mSensorColor = color;
}

void DataPointSensor::removeOldestValue(int numberToRemove) {
    while (numberToRemove-- > 0)  {
        mSensorData.removeFirst();
    }
}


double DataPointSensor::removeOneOldestValue()  {
    return mSensorData.takeFirst();
}

void DataPointSensor::setReportedMaxValue(double reportedMaxValue)
{
   mReportedMaxValue = reportedMaxValue;
}

void DataPointSensor::addData(double value)  {
    mSensorData.append(value);
}

double DataPointSensor::lastValue(int index) const {
    Q_UNUSED(index);
    return lastValue();
}

void DataPointSensor::init(const QColor &sensorColor)  {
    setColor(sensorColor);
    mReportedMaxValue = 0;
    mLastSeenValue = 0;
    mPrevSeenValue = 0;
}

