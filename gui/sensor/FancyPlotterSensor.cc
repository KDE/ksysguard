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

#include "FancyPlotterSensor.h"

FancyPlotterSensor::FancyPlotterSensor(const QString argName, const QString argSummationName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor) : BasicSensor(argName, argHostName, argType, argRegexpName, argSensorColor) {
	mSummationName = argSummationName;
	setColor(argSensorColor);
	mMinValue = INT_MAX;
	mMaxValue = INT_MIN;
}

FancyPlotterSensor::FancyPlotterSensor(const QList<QString> argName, const QString argSummationName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor) : BasicSensor(argName, argHostName, argType, argRegexpName, argSensorColor)  {
	mSummationName = argSummationName;
	setColor(argSensorColor);
	mMinValue = INT_MAX;
	mMaxValue = INT_MIN;
}


FancyPlotterSensor::~FancyPlotterSensor() {
}

void FancyPlotterSensor::setColor(const QColor argColor)  {
	mLighterColor = argColor.lighter();
	BasicSensor::setColor(argColor);
}

QString FancyPlotterSensor::summationName() const  {
	return mSummationName;
}

double FancyPlotterSensor::maxValue() const  {
	return mMaxValue;
}

double FancyPlotterSensor::minValue() const  {
	return mMinValue;
}

void FancyPlotterSensor::addData(double argValue) {
	BasicSensor::addData(argValue);
	mMaxValue = qMax(mMaxValue,argValue);
	mMinValue = qMin(mMinValue,argValue);

}

void FancyPlotterSensor::removeOldestValue(int argNumberToRemove)  {
	bool needRescanOfMinMax = false;
	while (argNumberToRemove-- > 0)  {
		double removed = removeOneOldestValue();
		if (removed == mMaxValue || removed == mMinValue)  {
			needRescanOfMinMax = true;
		}
	}
	if (needRescanOfMinMax)  {
		mMinValue = INT_MAX;
		mMaxValue = INT_MIN;
		int listSize = dataSize();
		for (int var = 0; var < listSize; ++var) {
			double result = data(var);
			mMaxValue = qMax(mMaxValue,result);
			mMinValue = qMin(mMinValue,result);
		}
	}
}


#include "FancyPlotterSensor.moc"
