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

BasicSensor::BasicSensor(const QString &name, const QString &hostName, const QString &type, const QString &regexpName) {
    mNameList.append(name);
    init(hostName,type,regexpName);
}

BasicSensor::BasicSensor(const QList<QString> &name, const QString &hostName, const QString &type, const QString &regexpName)  {
    mNameList = name;
    init(hostName,type,regexpName);
}

BasicSensor::~BasicSensor() {
}

void BasicSensor::init(const QString &hostName, const QString &type, const QString &regexpName)
{
    mHostName = hostName;
    mType = type;
    mRegexpName = regexpName;
    mLocalHost = (hostName.toLower() == "localhost" || hostName.isEmpty());
    mOk = true;
    mInteger = (mType == "integer");
}
QString BasicSensor::name() const {
    return mNameList.at(0);
}

bool BasicSensor::isOk() const {
    return mOk;
}

void BasicSensor::setOk(bool value)  {
    mOk = value;
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

void BasicSensor::addTitle(const QString &title)  {
    mTitleList.append(title);
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
