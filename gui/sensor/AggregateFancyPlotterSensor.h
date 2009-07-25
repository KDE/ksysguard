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

#ifndef AGGREGATEFANCYPLOTTERSENSOR_H_
#define AGGREGATEFANCYPLOTTERSENSOR_H_

#include <QtCore/QList>
#include "FancyPlotterSensor.h"


class AggregateFancyPlotterSensor: public FancyPlotterSensor {
public:
	AggregateFancyPlotterSensor(QList<QString> argName, QString argSummationName, QString argHostName, QString argType, QString argRegexpName, QColor argSensorColor);
	virtual ~AggregateFancyPlotterSensor();
	virtual bool isAggregateSensor() const;
	virtual void addData(const double argValue);
	virtual void putReportedMaxValue(double argTheorethicalMaxValue);
	virtual double lastValue(int argIndex) const;
	double lastIndividualSensorValue(int argIndex) const;
	virtual void removeOldestValue(int argNumberToRemove = 1);

private:
	int mNumDataReceived;
	int mNumberOfSensor;
	double mTempAggregateValue;
	QList<double>* mIndividualSensorData;
};

inline double AggregateFancyPlotterSensor::lastIndividualSensorValue(int argIndex) const {
    return mIndividualSensorData[argIndex].last();
}

#endif /* AGGREGATEFANCYPLOTTERSENSOR_H_ */
