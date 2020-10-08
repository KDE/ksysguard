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

template <typename T>
class SysctlSensor : public SensorProperty {
public:
    SysctlSensor(const QString &id, const QByteArray &sysctlName, SensorObject *parent);
    void update();
private:
    const QByteArray m_sysctlName;
};

#ifdef Q_OS_FREEBSD
#include <sys/sysctl.h>

template <typename T>
SysctlSensor<T>::SysctlSensor(const QString& id, const QByteArray &sysctlName, SensorObject* parent)
    : SensorProperty(id, parent)
    , m_sysctlName(sysctlName)
{
}

template <typename T>
void SysctlSensor<T>::update()
{
    T value;
    size_t size = sizeof(T);
    if (sysctlbyname(m_sysctlName.constData(), &value, &size, nullptr, 0) != -1) {
        setValue(value);
    }
}
#endif

#endif
