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

#ifndef CPU_H
#define CPU_H

#include <KLocalizedString>

#include <SensorObject.h>

class CpuObject : public SensorObject {
public:
    CpuObject(const QString &id, const QString &name, SensorContainer *parent);

protected: 
    SensorProperty *m_usage;
    SensorProperty *m_system;
    SensorProperty *m_user;
    SensorProperty *m_wait;
    SensorProperty *m_frequency;
    SensorProperty *m_temperature;
};

template <typename T>
class AllCpusObject : public T {
public:
    AllCpusObject(unsigned int cpus, unsigned int cores, SensorContainer *parent);
    static_assert(std::is_base_of<CpuObject, T>::value, "Base of AllCpuObject must be a CpuObject");
};

template <typename T>
AllCpusObject<T>::AllCpusObject(unsigned int cpus, unsigned int cores, SensorContainer *parent)
    : T(QStringLiteral("all"), i18nc("@title", "All"), parent)
{
    delete T::m_frequency;
    delete T::m_temperature;
    auto cpuCount = new SensorProperty(QStringLiteral("cpuCount"), i18nc("@title", "Number of CPUs"), cpus, this);
    cpuCount->setShortName(i18nc("@title, Short fort 'Number of CPUs'", "CPUs"));
    cpuCount->setDescription(i18nc("@info", "Number of physical CPUs installed in the system"));

    auto coreCount = new SensorProperty(QStringLiteral("coreCount"), i18nc("@title", "Number of Cores"), cores, this);
    coreCount->setShortName(i18nc("@title, Short fort 'Number of Cores'", "Cores"));
    coreCount->setDescription(i18nc("@info", "Number of CPU cores across all physical CPUS"));
}
#endif
