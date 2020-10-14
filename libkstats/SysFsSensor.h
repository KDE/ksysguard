/*
    Copyright (c) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include <QObject>

#include "SensorProperty.h"

/**
 * Convenience subclass of SensorProperty that reads a sysfs file and uses the result as value.
 */
class Q_DECL_EXPORT SysFsSensor : public SensorProperty
{
    Q_OBJECT

public:
    SysFsSensor(const QString &id, const QString &path, SensorObject *parent);

    /**
     * Set the function used to convert the data from sysfs to the value of this sensor.
     *
     * This accepts a function that takes a QByteArray and converts that to a QVariant.
     * By default this is set to `std::atoll` or in other words, any numeric value
     * should automatically be converted to a proper QVariant.
     */
    void setConvertFunction(const std::function<QVariant(const QByteArray&)> &function);

    /**
     * Update this sensor.
     *
     * This will cause the sensor to read sysfs and update the value from that.
     * It should be called periodically so values are updated properly.
     */
    void update() override;

private:
    QString m_path;
    std::function<QVariant(const QByteArray&)> m_convertFunction;
};
