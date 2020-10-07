/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "AllGpus.h"

#include <KLocalizedString>

#include "AggregateSensor.h"

AllGpus::AllGpus(SensorContainer *parent)
    : SensorObject(QStringLiteral("all"), i18nc("@title", "All GPUs"), parent)
{
    m_usageSensor = new AggregateSensor(this, QStringLiteral("usage"), i18nc("@title", "All GPUs Usage"));
    m_usageSensor->setMatchSensors(QRegularExpression{"^(?!all).*$"}, QStringLiteral("usage"));
    m_usageSensor->setAggregateFunction([this](const QVariant &first, const QVariant &second) {
        auto gpuCount = m_usageSensor->matchCount();
        return QVariant::fromValue(first.toDouble() + (second.toDouble() / gpuCount));
    });
    m_usageSensor->setUnit(KSysGuard::UnitPercent);

    m_totalVramSensor = new AggregateSensor(this, QStringLiteral("totalVram"), i18nc("@title", "All GPUs Total Memory"));
    m_totalVramSensor->setMatchSensors(QRegularExpression{"^(?!all).*$"}, QStringLiteral("totalVram"));
    m_totalVramSensor->setUnit(KSysGuard::UnitByte);

    m_usedVramSensor = new AggregateSensor(this, QStringLiteral("usedVram"), i18nc("@title", "All GPUs Used Memory"));
    m_usedVramSensor->setMatchSensors(QRegularExpression{"^(?!all).*$"}, QStringLiteral("usedVram"));
    m_usedVramSensor->setUnit(KSysGuard::UnitByte);
}
