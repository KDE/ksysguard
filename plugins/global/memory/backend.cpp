/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3 of
    the license or (at your option) at any later version that is
    accepted by the membership of KDE e.V. (or its successor
    approved by the membership of KDE e.V.), which shall act as a
    proxy as defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
*/

#include "backend.h"

#include <AggregateSensor.h>
#include <SensorObject.h>
#include <SensorProperty.h>

#include <KLocalizedString>

MemoryBackend::MemoryBackend(SensorContainer *container)
{
    m_physicalObject = new SensorObject(QStringLiteral("physical"), i18nc("@title", "Physical Memory"), container);
    m_swapObject = new SensorObject(QStringLiteral("swap"), i18nc("@title", "Swap Memory"), container);
}

void MemoryBackend::makeSensors()
{
    m_total = new SensorProperty(QStringLiteral("total"), m_physicalObject);
    m_used = new SensorProperty(QStringLiteral("used"), m_physicalObject);
    m_free = new SensorProperty(QStringLiteral("free"), m_physicalObject);
    m_application = new SensorProperty(QStringLiteral("application"), m_physicalObject);
    m_cache = new SensorProperty(QStringLiteral("cache"), m_physicalObject);
    m_buffer = new SensorProperty(QStringLiteral("buffer"), m_physicalObject);

    m_swapTotal = new SensorProperty(QStringLiteral("total"), m_swapObject);
    m_swapUsed = new SensorProperty(QStringLiteral("used"), m_swapObject);
    m_swapFree = new SensorProperty(QStringLiteral("free"), m_swapObject);
}

void MemoryBackend::initSensors()
{
    makeSensors();

    m_total->setName(i18nc("@title", "Total Physical Memory"));
    m_total->setShortName(i18nc("@title, Short for 'Total Physical Memory'", "Total"));
    m_total->setUnit(KSysGuard::UnitByte);
    m_total->setVariantType(QVariant::ULongLong);

    m_used->setName(i18nc("@title", "Used Physical Memory"));
    m_used->setShortName(i18nc("@title, Short for 'Used Physical Memory'", "Used"));
    m_used->setUnit(KSysGuard::UnitByte);
    m_used->setVariantType(QVariant::ULongLong);
    m_used->setMax(m_total);
    auto usedPercentage = new PercentageSensor(m_physicalObject, QStringLiteral("usedPercent"), i18nc("@title", "Used Physical Memory Percentage"));
    usedPercentage->setBaseSensor(m_used);

    m_free->setName(i18nc("@title", "Free Physical Memory"));
    m_free->setShortName(i18nc("@title, Short for 'Free Physical Memory'", "Free"));
    m_free->setUnit(KSysGuard::UnitByte);
    m_free->setVariantType(QVariant::ULongLong);
    m_free->setMax(m_total);
    auto freePercentage = new PercentageSensor(m_physicalObject, QStringLiteral("freePercent"), i18nc("@title", "Free Physical Memory Percentage"));
    freePercentage->setBaseSensor(m_free);

    m_application->setName(i18nc("@title", "Application Memory"));
    m_application->setShortName(i18nc("@title, Short for 'Application Memory'", "Application"));
    m_application->setUnit(KSysGuard::UnitByte);
    m_application->setVariantType(QVariant::ULongLong);
    m_application->setMax(m_total);
    auto applicationPercentage = new PercentageSensor(m_physicalObject, QStringLiteral("applicationPercent"), i18nc("@title", "Application Memory Percentage"));
    applicationPercentage->setBaseSensor(m_application);

    m_cache->setName(i18nc("@title", "Cache Memory"));
    m_cache->setShortName(i18nc("@title, Short for 'Cache Memory'", "Cache"));
    m_cache->setUnit(KSysGuard::UnitByte);
    m_cache->setVariantType(QVariant::ULongLong);
    m_cache->setMax(m_total);
    auto cachePercentage = new PercentageSensor(m_physicalObject, QStringLiteral("cachePercent"), i18nc("@title", "Cache Memory Percentage"));
    cachePercentage->setBaseSensor(m_cache);

    m_buffer->setName(i18nc("@title", "Buffer Memory"));
    m_buffer->setShortName(i18nc("@title, Short for 'Buffer Memory'", "Buffer"));
    m_buffer->setDescription(i18n("Amount of memory used for caching disk blocks"));
    m_buffer->setUnit(KSysGuard::UnitByte);
    m_buffer->setVariantType(QVariant::ULongLong);
    m_buffer->setMax(m_total);
    auto bufferPercentage = new PercentageSensor(m_physicalObject, QStringLiteral("bufferPercent"), i18nc("@title", "Buffer Memory Percentage"));
    bufferPercentage->setBaseSensor(m_buffer);

    m_swapTotal->setName(i18nc("@title", "Total Swap Memory"));
    m_swapTotal->setShortName(i18nc("@title, Short for 'Total Swap Memory'", "Total"));
    m_swapTotal->setUnit(KSysGuard::UnitByte);
    m_swapTotal->setVariantType(QVariant::ULongLong);

    m_swapUsed->setName(i18nc("@title", "Used Swap Memory"));
    m_swapUsed->setShortName(i18nc("@title, Short for 'Used Swap Memory'", "Used"));
    m_swapUsed->setUnit(KSysGuard::UnitByte);
    m_swapUsed->setVariantType(QVariant::ULongLong);
    m_swapUsed->setMax(m_swapTotal);
    auto usedSwapPercentage = new PercentageSensor(m_swapObject, QStringLiteral("usedPercent"), i18nc("@title", "Used Swap Memory Percentage"));
    usedSwapPercentage->setBaseSensor(m_swapUsed);

    m_swapFree->setName(i18nc("@title", "Free Swap Memory"));
    m_swapFree->setShortName(i18nc("@title, Short for 'Free Swap Memory'", "Free"));
    m_swapFree->setUnit(KSysGuard::UnitByte);
    m_swapFree->setVariantType(QVariant::ULongLong);
    m_swapFree->setMax(m_swapTotal);
    auto freeSwapPercentage = new PercentageSensor(m_swapObject, QStringLiteral("freePercent"), i18nc("@title", "Free Swap Memory Percentage"));
    freeSwapPercentage->setBaseSensor(m_swapFree);
}
