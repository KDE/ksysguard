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

BasicSensor::BasicSensor(const QString argName, const QString argHostName, const QString argType, const QString argRegexpName) {
	mNameList.append(argName);
	init(argHostName,argType,argRegexpName);
}

BasicSensor::BasicSensor(const QList<QString> argName, const QString argHostName, const QString argType, const QString argRegexpName)  {
	mNameList = argName;
	init(argHostName,argType,argRegexpName);
}

BasicSensor::~BasicSensor() {
}

void BasicSensor::init(const QString argHostName, const QString argType, const QString argRegexpName)
{
	mHostName = argHostName;
	mType = argType;
	mRegexpName = argRegexpName;
	mLocalHost = (argHostName.toLower() == "localhost" || argHostName.isEmpty());
	mOk = true;
	mInteger = (mType == "integer");
}
QString BasicSensor::name() const {
	return mNameList.at(0);
}

bool BasicSensor::isOk() const {
	return mOk;
}

void BasicSensor::setOk(bool argValue)  {
	mOk = argValue;
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
	return mLocalHost;
}

QString BasicSensor::type() const  {
	return mType;
}

void BasicSensor::addTitle(QString argTitle)  {
	mTitleList.append(argTitle);
}

QList<QString> BasicSensor::titleList() const
{
	return mTitleList;
}

bool BasicSensor::isAggregateSensor() const {
	return false;
}

QList<QString> BasicSensor::nameList() const  {
	return mNameList;
}
