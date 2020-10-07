/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "GpuDevice.h"

#include <KLocalizedString>

GpuDevice::GpuDevice(const QString& id, const QString& name)
    : SensorObject(id, name)
{
}

void GpuDevice::initialize()
{
    makeSensors();

    m_nameProperty->setName(i18nc("@title", "Name"));
    m_nameProperty->setPrefix(name());
    m_nameProperty->setValue(name());

    m_usageProperty->setName(i18nc("@title", "Usage"));
    m_usageProperty->setPrefix(name());
    m_usageProperty->setMin(0);
    m_usageProperty->setMax(100);
    m_usageProperty->setUnit(KSysGuard::UnitPercent);

    m_totalVramProperty->setName(i18nc("@title", "Total Video Memory"));
    m_totalVramProperty->setPrefix(name());
    m_totalVramProperty->setShortName(i18nc("@title Short for Total Video Memory", "Total"));
    m_totalVramProperty->setUnit(KSysGuard::UnitByte);

    m_usedVramProperty->setName(i18nc("@title", "Video Memory Used"));
    m_usedVramProperty->setPrefix(name());
    m_usedVramProperty->setShortName(i18nc("@title Short for Video Memory Used", "Used"));
    m_usedVramProperty->setMax(m_totalVramProperty);
    m_usedVramProperty->setUnit(KSysGuard::UnitByte);

    m_coreFrequencyProperty->setName(i18nc("@title", "Frequency"));
    m_coreFrequencyProperty->setPrefix(name());
    m_coreFrequencyProperty->setUnit(KSysGuard::UnitMegaHertz);

    m_memoryFrequencyProperty->setName(i18nc("@title", "Memory Frequency"));
    m_memoryFrequencyProperty->setPrefix(name());
    m_memoryFrequencyProperty->setUnit(KSysGuard::UnitMegaHertz);

    m_temperatureProperty->setName(i18nc("@title", "Temperature"));
    m_temperatureProperty->setPrefix(name());
    m_temperatureProperty->setUnit(KSysGuard::UnitCelsius);

}

void GpuDevice::makeSensors()
{
    m_nameProperty = new SensorProperty(QStringLiteral("name"), this);
    m_usageProperty = new SensorProperty(QStringLiteral("usage"), this);
    m_totalVramProperty = new SensorProperty(QStringLiteral("totalVram"), this);
    m_usedVramProperty = new SensorProperty(QStringLiteral("usedVram"), this);
    m_coreFrequencyProperty = new SensorProperty(QStringLiteral("coreFrequency"), this);
    m_memoryFrequencyProperty = new SensorProperty(QStringLiteral("memoryFrequency"), this);
    m_temperatureProperty = new SensorProperty(QStringLiteral("temperature"), this);
}
