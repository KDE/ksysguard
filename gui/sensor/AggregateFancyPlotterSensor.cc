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

#include "AggregateFancyPlotterSensor.h"

AggregateFancyPlotterSensor::AggregateFancyPlotterSensor(const QList<QString> &name, const QString &summationName, const QString &hostName, const QString &type, const QString &regexpName, const QColor &sensorColor) : FancyPlotterSensor(name, summationName, hostName, type, regexpName, sensorColor) {
	mNumberOfSensor = name.size();
	mNumDataReceived = 0;
	mTempAggregateValue = 0;
	mIndividualSensorData = new QList<double>[mNumberOfSensor];
}

AggregateFancyPlotterSensor::~AggregateFancyPlotterSensor() {
    delete [] mIndividualSensorData;
}

bool AggregateFancyPlotterSensor::isAggregateSensor() const {
	return true;
}

void AggregateFancyPlotterSensor::addData(double value)  {
	if (mNumDataReceived < mNumberOfSensor)  {
	    mIndividualSensorData[mNumDataReceived++].append(value);
		mTempAggregateValue += value;
	} else  {
		FancyPlotterSensor::addData(mTempAggregateValue);
		mIndividualSensorData[0].append(value);
		mNumDataReceived = 1;
		mTempAggregateValue = value;
	}
}

void AggregateFancyPlotterSensor::setReportedMaxValue(double value)
{
    FancyPlotterSensor::setReportedMaxValue(reportedMaxValue()+value);
}

double AggregateFancyPlotterSensor::lastValue(int index) const  {
    if (index == -1)
        return DataPointSensor::lastValue();
    else
        return mIndividualSensorData[index].last();
}

void AggregateFancyPlotterSensor::removeOldestValue(int numberToRemove)  {
    FancyPlotterSensor::removeOldestValue(numberToRemove);
    while (numberToRemove-- > 0) {
        for (int var = 0; var < mNumberOfSensor; ++var) {
            mIndividualSensorData[var].removeFirst();
        }
    }
}

