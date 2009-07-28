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
#include "LogSensor.h"

LogSensor::LogSensor(QString name, QString hostName) : BasicSensor(name, hostName, "file", "") {
    mLowerLimitActive = false;
    mUpperLimitActive = 0;
    mLowerLimit = 0;
    mUpperLimit = 0;
    mLimitReached = false;
}

LogSensor::~LogSensor() {

}

void LogSensor::setFileName(const QString& name) {
    mFileName = name;
}

QString LogSensor::fileName() const {
    return mFileName;
}

void LogSensor::setUpperLimitActive(bool value) {
    mUpperLimitActive = value;
}

bool LogSensor::upperLimitActive() const {
    return mUpperLimitActive;
}

void LogSensor::setLowerLimitActive(bool value) {
    mLowerLimitActive = value;
}

bool LogSensor::lowerLimitActive() const {
    return mLowerLimitActive;
}

void LogSensor::setUpperLimit(double value) {
    mUpperLimit = value;
}

double LogSensor::upperLimit() const {
    return mUpperLimit;
}

void LogSensor::setLowerLimit(double value) {
    mLowerLimit = value;
}

double LogSensor::lowerLimit() const {
    return mLowerLimit;
}

bool LogSensor::limitReached() const {
    return mLimitReached;
}

void LogSensor::setLimitReached(bool limit)  {
    mLimitReached = limit;
}


