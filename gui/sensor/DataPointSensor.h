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
    DataPointSensor(const QString &name, const QString &hostName, const QString &type, const QString &regexpName, const QColor &sensorColor);
    DataPointSensor(const QList<QString> &name, const QString &hostName, const QString &type, const QString &regexpName, const QColor &sensorColor);
    virtual ~DataPointSensor();

    QColor color() const;
    int dataSize() const;
    double data(int index) const;
    QString unit() const;

    double lastValue() const;
    /* Method for sensors that represent aggregate data. This will return the last value of an individual sensor.
     * By default it return the same as lastValue
     */
    virtual double lastValue(int index) const;

    void setUnit(const QString &unit);

    /* this is the reported maximum value that the sensor can have, for example as provided by ksysguardd*/
    double reportedMaxValue() const;
    /* this is implementation specific, for a basic sensor it simply override the previous value, i.e. a set*/
    virtual void setReportedMaxValue(double reportedMaxValue);

    virtual void removeOldestValue(int numberToRemove = 1);

    /* last seen value by a client of this sensor, used to speed up drawing in some instance*/
    double lastSeenValue() const;
    double prevSeenValue() const;
    void updateLastSeenValue(double lastSeenValue);

    virtual void setColor(const QColor &color);
    virtual void addData(double value);

protected:
    double removeOneOldestValue();

private:

    void init(const QColor &sensorColor);

    QColor mSensorColor;
    QString mUnit;
    QList<double> mSensorData;
    double mReportedMaxValue;
    double mLastSeenValue;
    double mPrevSeenValue;

};

inline QColor DataPointSensor::color() const {
    return mSensorColor;
}

inline double DataPointSensor::data(int index) const {
    return mSensorData.at(index);
}

inline int DataPointSensor::dataSize() const {
    return mSensorData.size();
}

inline double DataPointSensor::reportedMaxValue() const {
    return mReportedMaxValue;
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

inline void DataPointSensor::updateLastSeenValue(double lastSeenValue) {
    mPrevSeenValue = mLastSeenValue;
    mLastSeenValue = lastSeenValue;
}

#endif /* DATAPOINTSENSOR_H_ */
