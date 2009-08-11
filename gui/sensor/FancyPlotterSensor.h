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

class FancyPlotterSensor: public DataPointSensor {
public:
    FancyPlotterSensor(const QString &name, const QString &summationName, const QString &hostName, const QString &type, const QString &regexpName, const QColor &sensorColor);
    FancyPlotterSensor(const QList<QString> &name, const QString &summationName, const QString &hostName, const QString &type, const QString &regexpName, const QColor &sensorColor);
    virtual ~FancyPlotterSensor();
    virtual void setColor(const QColor &color);
    QString summationName() const;
    double maxValue() const;
    double minValue() const;
    virtual void addData(double value);
    QColor lighterColor() const;
    virtual void removeOldestValue(int numberToRemove = 1);

private:
    QString mSummationName;
    QColor mLighterColor;
    double mMaxValue;
    double mMinValue;
};

#endif /* FANCYPLOTTERSENSOR_H_ */
