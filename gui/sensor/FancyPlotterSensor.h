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

#ifndef FANCYPLOTTERSENSOR_H_
#define FANCYPLOTTERSENSOR_H_

#include "DataPointSensor.h"
#include <QColor>
#include <limits.h>

class FancyPlotterSensor: public DataPointSensor {
public:
	FancyPlotterSensor(const QString argName, const QString argSummationName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor);
	FancyPlotterSensor(const QList<QString> argName, const QString argSummationName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor);
	virtual ~FancyPlotterSensor();
	virtual void setColor(const QColor argColor);
	QString summationName() const;
	double maxValue() const;
	double minValue() const;
	virtual void addData(const double argValue);
    QColor lighterColor() const;
    virtual void removeOldestValue(int argNumberToRemove = 1);

private:
	QString mSummationName;
	QColor mLighterColor;
	double mMaxValue;
	double mMinValue;
};

inline QColor FancyPlotterSensor::lighterColor() const
{
       return mLighterColor;
}

#endif /* FANCYPLOTTERSENSOR_H_ */
