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
	numberOfSensor = argName.size();
	numDataReceived = 0;
	tempAggregateValue = 0;
}

AggregateFancyPlotterSensor::~AggregateFancyPlotterSensor() {
}

bool AggregateFancyPlotterSensor::isAggregateSensor() const {
	return true;
}

void AggregateFancyPlotterSensor::addData(const double argValue)  {
	if (numDataReceived < numberOfSensor)  {
		++numDataReceived;
		tempAggregateValue += argValue;
	} else  {
		FancyPlotterSensor::addData(tempAggregateValue);
		numDataReceived = 1;
		tempAggregateValue = argValue;
	}
}

void AggregateFancyPlotterSensor::putTheoreticalMaxValue(double argTheorethicalMaxValue)
{
	BasicSensor::putTheoreticalMaxValue(theorethicalMaxValue()+argTheorethicalMaxValue);
}




#include "AggregateFancyPlotterSensor.moc"
