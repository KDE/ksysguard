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

CpuObject::CpuObject(const QString &id, const QString &name, SensorContainer *parent)
    : SensorObject(id, name, parent)
{
    auto n = new SensorProperty(QStringLiteral("name"), i18nc("@title", "Name"), name, this);
    n->setVariantType(QVariant::String);

    m_usage = new SensorProperty(QStringLiteral("usage"), i18nc("@title", "Total Usage"), this);
    m_usage->setShortName(i18nc("@title, Short for 'Total Usage'", "Usage"));
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_usage->setVariantType(QVariant::Double);
    m_usage->setMax(100);

    m_system = new SensorProperty(QStringLiteral("system"), i18nc("@title", "System Usage"), this);
    m_system->setShortName(i18nc("@title, Short for 'System Usage'", "System"));
    m_system->setUnit(KSysGuard::UnitPercent);
    m_system->setVariantType(QVariant::Double);
    m_system->setMax(100);

    m_user = new SensorProperty(QStringLiteral("user"), i18nc("@title", "User Usage"), this);
    m_user->setShortName(i18nc("@title, Short for 'User Usage'", "User"));
    m_user->setUnit(KSysGuard::UnitPercent);
    m_user->setVariantType(QVariant::Double);
    m_user->setMax(100);

    m_wait = new SensorProperty(QStringLiteral("wait"), i18nc("@title", "Wait"), this);
    m_wait->setUnit(KSysGuard::UnitPercent);
    m_wait->setVariantType(QVariant::Double);
    m_wait->setMax(100);

    if (id != QStringLiteral("all"))
    {
        m_frequency = new SensorProperty(QStringLiteral("frequency"), i18nc("@title", "Current Frequency"), this);
        m_frequency->setShortName(i18nc("@title, Short for 'Current Frequency'", "Frequency"));
        m_frequency->setDescription(i18nc("@info", "Current frequency of the CPU"));
        m_frequency->setVariantType(QVariant::Double);
        m_frequency->setUnit(KSysGuard::Unit::UnitMegaHertz);
    }
}
