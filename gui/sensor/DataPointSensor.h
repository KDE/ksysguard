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

#ifndef DATAPOINTSENSOR_H_
#define DATAPOINTSENSOR_H_

#include "BasicSensor.h"

class DataPointSensor: public BasicSensor {
public:
    DataPointSensor(const QString argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor);
    DataPointSensor(const QList<QString> argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor);
    virtual ~DataPointSensor();

    QColor color() const;
    int dataSize() const;
    double data(int argIndex) const;
    QString unit() const;

    double lastValue() const;
    /* Method for sensors that represent aggregate data. This will return the last value of an individual sensor.
     * By default it return the same as lastValue
     */
    virtual double lastValue(int argIndex) const;

    void setUnit(const QString &argUnit);

    /* this is the reported maximum value that the sensor can have, for example as provided by ksysguardd*/
    double reportedMaxValue() const;
    /* this is implementation specific, for a basic sensor it simply override the previous value, i.e. a set*/
    virtual void putReportedMaxValue(double argReportedMaxValue);

    virtual void removeOldestValue(int argNumberToRemove = 1);

    /* last seen value by a client of this sensor, used to speed up drawing in some instance*/
    double lastSeenValue() const;
    double prevSeenValue() const;
    void updateLastSeenValue(double argLastSeenValue);

    virtual void setColor(const QColor argColor);
    virtual void addData(const double argValue);

protected:
    double removeOneOldestValue();

private:

    void init(const QColor argSensorColor);

    QColor mSensorColor;
    QString mUnit;
    QList<double> mSensorData;
    double mTheorethicalMaxValue;
    double mLastSeenValue;
    double mPrevSeenValue;

};

inline QColor DataPointSensor::color() const {
    return mSensorColor;
}

inline double DataPointSensor::data(int argIndex) const {
    return mSensorData.at(argIndex);
}

inline int DataPointSensor::dataSize() const {
    return mSensorData.size();
}

inline double DataPointSensor::reportedMaxValue() const {
    return mTheorethicalMaxValue;
}

inline double DataPointSensor::lastValue() const {
    return mSensorData.last();
}

inline double DataPointSensor::lastSeenValue() const {
    return mLastSeenValue;
}

inline double DataPointSensor::prevSeenValue() const {
    return mPrevSeenValue;
}

inline void DataPointSensor::updateLastSeenValue(double argLastSeenValue) {
    mPrevSeenValue = mLastSeenValue;
    mLastSeenValue = argLastSeenValue;
}

#endif /* DATAPOINTSENSOR_H_ */
