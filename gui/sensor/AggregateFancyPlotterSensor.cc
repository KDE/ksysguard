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

AggregateFancyPlotterSensor::AggregateFancyPlotterSensor(QList<QString> argName, QString argSummationName, QString argHostName, QString argType, QString argRegexpName, QColor argSensorColor) : FancyPlotterSensor(argName, argSummationName,argHostName, argType, argRegexpName, argSensorColor) {
	mNumberOfSensor = argName.size();
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

void AggregateFancyPlotterSensor::addData(const double argValue)  {
	if (mNumDataReceived < mNumberOfSensor)  {
	    mIndividualSensorData[mNumDataReceived++].append(argValue);
		mTempAggregateValue += argValue;
	} else  {
		FancyPlotterSensor::addData(mTempAggregateValue);
		mIndividualSensorData[0].append(argValue);
		mNumDataReceived = 1;
		mTempAggregateValue = argValue;
	}
}

void AggregateFancyPlotterSensor::putReportedMaxValue(double argTheorethicalMaxValue)
{
    FancyPlotterSensor::putReportedMaxValue(reportedMaxValue()+argTheorethicalMaxValue);
}

double AggregateFancyPlotterSensor::lastValue(int argIndex) const  {
    if (argIndex == -1)
        return DataPointSensor::lastValue();
    else
        return mIndividualSensorData[argIndex].last();
}

void AggregateFancyPlotterSensor::removeOldestValue(int argNumberToRemove)  {
    FancyPlotterSensor::removeOldestValue(argNumberToRemove);
    while (argNumberToRemove-- > 0) {
        for (int var = 0; var < mNumberOfSensor; ++var) {
            mIndividualSensorData[var].removeFirst();
        }
    }
}




