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

#include "cpu.h"

#include <KLocalizedString>

BaseCpuObject::BaseCpuObject(const QString &id, const QString &name, SensorContainer *parent)
    : SensorObject(id, name, parent)
{
}

void BaseCpuObject::makeSensors()
{
    m_usage = new SensorProperty(QStringLiteral("usage"), this);
    m_system = new SensorProperty(QStringLiteral("system"), this);
    m_user = new SensorProperty(QStringLiteral("user"), this);
    m_wait = new SensorProperty(QStringLiteral("wait"), this);
    auto n = new SensorProperty(QStringLiteral("name"), i18nc("@title", "Name"), name(), this);
    n->setVariantType(QVariant::String);
}


void BaseCpuObject::initialize()
{
    makeSensors();

    m_usage->setPrefix(name());
    m_usage->setName(i18nc("@title", "Total Usage"));
    m_usage->setShortName(i18nc("@title, Short for 'Total Usage'", "Usage"));
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_usage->setVariantType(QVariant::Double);
    m_usage->setMax(100);

    m_system->setPrefix(name());
    m_system->setName(i18nc("@title", "System Usage"));
    m_system->setShortName(i18nc("@title, Short for 'System Usage'", "System"));
    m_system->setUnit(KSysGuard::UnitPercent);
    m_system->setVariantType(QVariant::Double);
    m_system->setMax(100);

    m_user->setPrefix(name());
    m_user->setName(i18nc("@title", "User Usage"));
    m_user->setShortName(i18nc("@title, Short for 'User Usage'", "User"));
    m_user->setUnit(KSysGuard::UnitPercent);
    m_user->setVariantType(QVariant::Double);
    m_user->setMax(100);

    m_wait->setPrefix(name());
    m_wait->setName(i18nc("@title", "Wait Usage"));
    m_wait->setShortName(i18nc("@title, Short for 'Wait Load'", "Wait"));
    m_wait->setUnit(KSysGuard::UnitPercent);
    m_wait->setVariantType(QVariant::Double);
    m_wait->setMax(100);
}


CpuObject::CpuObject(const QString &id, const QString &name, SensorContainer *parent)
    : BaseCpuObject(id, name, parent)
{
}

void CpuObject::makeSensors()
{
    BaseCpuObject::makeSensors();

    m_frequency = new SensorProperty(QStringLiteral("frequency"), this);
    m_temperature = new SensorProperty(QStringLiteral("temperature"), this);
}

void CpuObject::initialize()
{
    BaseCpuObject::initialize();

    m_frequency->setPrefix(name());
    m_frequency->setName(i18nc("@title", "Current Frequency"));
    m_frequency->setShortName(i18nc("@title, Short for 'Current Frequency'", "Frequency"));
    m_frequency->setDescription(i18nc("@info", "Current frequency of the CPU"));
    m_frequency->setVariantType(QVariant::Double);
    m_frequency->setUnit(KSysGuard::Unit::UnitMegaHertz);

    m_temperature->setPrefix(name());
    m_temperature->setPrefix(i18nc("@title", "Current Temperature"));
    m_temperature->setShortName(i18nc("@title, Short for Current Temperatur", "Temperature"));
    m_temperature->setVariantType(QVariant::Double);
    m_temperature->setUnit(KSysGuard::Unit::UnitCelsius);
}


AllCpusObject::AllCpusObject(SensorContainer *parent)
    : BaseCpuObject(QStringLiteral("all"), i18nc("@title", "All"), parent)
{
}

void AllCpusObject::makeSensors()
{
    BaseCpuObject::makeSensors();

    m_cpuCount = new SensorProperty(QStringLiteral("cpuCount"), this);
    m_coreCount = new SensorProperty(QStringLiteral("coreCount"), this);
}

void AllCpusObject::initialize()
{
    BaseCpuObject::initialize();

    m_cpuCount->setName(i18nc("@title", "Number of CPUs"));
    m_cpuCount->setShortName(i18nc("@title, Short fort 'Number of CPUs'", "CPUs"));
    m_cpuCount->setDescription(i18nc("@info", "Number of physical CPUs installed in the system"));

    m_coreCount->setName(i18nc("@title", "Number of Cores"));
    m_coreCount->setShortName(i18nc("@title, Short fort 'Number of Cores'", "Cores"));
    m_coreCount->setDescription(i18nc("@info", "Number of CPU cores across all physical CPUS"));
}

void AllCpusObject::setCounts(unsigned int cpuCount, unsigned int coreCount)
{
    m_cpuCount->setValue(cpuCount);
    m_coreCount->setValue(cpuCount);
}
