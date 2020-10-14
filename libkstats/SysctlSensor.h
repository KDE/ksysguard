/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

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

#ifndef SYSCTLSENSOR_H
#define SYSCTLSENSOR_H

#include <SensorProperty.h>

/**
 * Convenience subclass of SensorProperty that reads a value of type T from FreeBSD's sysctl interface
 */
template <typename T>
class SysctlSensor : public SensorProperty {
public:
    /**
     * Contstrucor.
     * @param sysctlName The name of the sysctl entry
     */
    SysctlSensor(const QString &id, const QByteArray &sysctlName, SensorObject *parent);
    SysctlSensor(const QString &id, const QString &name, const QByteArray &sysctlName, SensorObject *parent);

    /**
     * Update this sensor.
     *
     * This will cause the sensor to call syscctl and update the value from that.
     * It should be called periodically so values are updated properly.
     */
    void update() override;
    /**
     * Set the function used to convert the data from sysctl to the value of this sensor.
     *
     * This accepts a function that takes a reference to an object of type T and converts that to a
     * QVariant. Use this if you need to convert the value into another unit or do a calculation to
     * arrive at the sensor value. By default the value is stored as is in a QVariant.
     */
    void setConversionFunction(const std::function<QVariant(const T&)> &function) {m_conversionFunction = function;}
private:
    const QByteArray m_sysctlName;
    std::function<QVariant(const T&)> m_conversionFunction = [](const T& t){return QVariant::fromValue(t);};
};

#ifdef Q_OS_FREEBSD
#include <sys/types.h>
#include <sys/sysctl.h>

template <typename T>
SysctlSensor<T>::SysctlSensor(const QString& id, const QByteArray &sysctlName, SensorObject* parent)
    : SensorProperty(id, parent)
    , m_sysctlName(sysctlName)
{
}

template<typename T>
SysctlSensor<T>::SysctlSensor(const QString& id, const QString& name, const QByteArray& sysctlName, SensorObject* parent)
    : SensorProperty(id, name, parent)
    , m_sysctlName(sysctlName)
{
}

template <typename T>
void SysctlSensor<T>::update()
{
    if (!isSubscribed()) {
        return;
    }
    T value{};
    size_t size = sizeof(T);
    if (sysctlbyname(m_sysctlName.constData(), &value, &size, nullptr, 0) != -1) {
        setValue(m_conversionFunction(value));
    }
}
#endif

#endif
