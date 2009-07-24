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

#include "BasicSensor.h"

BasicSensor::BasicSensor(const QString argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor) {
	mNameList.append(argName);
	init(argHostName,argType,argRegexpName,argSensorColor);
}

BasicSensor::BasicSensor(const QList<QString> argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor)  {
	mNameList = argName;
	init(argHostName,argType,argRegexpName,argSensorColor);
}

BasicSensor::~BasicSensor() {
}

void BasicSensor::init(const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor)
{
	mHostName = argHostName;
	setColor(argSensorColor);
	mType = argType;
	mRegexpName = argRegexpName;
	localHost = (argHostName.toLower() == "localhost" || argHostName.isEmpty());
	mTheorethicalMaxValue = 0;
	mLastSeenValue = 0;
	mPrevSeenValue = 0;
	ok = true;
	integer = (mType == "integer");
}
QString BasicSensor::name() const {
	return mNameList.at(0);
}

void BasicSensor::addData(double argValue)  {
	sensorData.append(argValue);
}

bool BasicSensor::isOk() const {
	return ok;
}

void BasicSensor::setIsOk(const bool argValue)  {
	ok = argValue;
}

QString BasicSensor::hostName() const {
	return mHostName;
}

QString BasicSensor::title() const {
	if (mTitleList.size() > 0)
			return mTitleList.at(0);
		else
			return "";
}

QString BasicSensor::regexpName() const  {
	return mRegexpName;
}

bool BasicSensor::isLocalHost() const  {
	return localHost;
}

QString BasicSensor::unit() const  {
	return mUnit;
}

void BasicSensor::setUnit(QString argUnit)  {
	mUnit = argUnit;
}

QString BasicSensor::type() const  {
	return mType;
}

void BasicSensor::setColor(const QColor argColor)  {
	sensorColor = argColor;
}

void BasicSensor::addTitle(QString argTitle)  {
	mTitleList.append(argTitle);
}

void BasicSensor::removeOldestValue(int argNumberToRemove) {
	while (argNumberToRemove-- > 0)  {
		sensorData.removeFirst();
	}
}

QList<QString> BasicSensor::titleList() const
{
	return mTitleList;
}

double BasicSensor::removeOneOldestValue()  {
	return sensorData.takeFirst();
}

void BasicSensor::putTheoreticalMaxValue(double argTheorethicalMaxValue)
{
   mTheorethicalMaxValue = argTheorethicalMaxValue;
}

bool BasicSensor::isAggregateSensor() const {
	return false;
}

QList<QString> BasicSensor::nameList() const  {
	return mNameList;
}
#include "BasicSensor.moc"
