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

#include "power.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/Battery>

#include <AggregateSensor.h>
#include <SensorContainer.h>
#include <SensorObject.h>
#include <SensorProperty.h>

K_PLUGIN_CLASS_WITH_JSON(PowerPlugin, "metadata.json")

class Battery : public SensorObject {
public:
    Battery(const Solid::Battery * const battery, const QString &name,  SensorContainer *parent);
};

Battery::Battery(const Solid::Battery * const battery, const QString &name,  SensorContainer *parent)
    : SensorObject(battery->serial(), name, parent)
{
    auto n = new SensorProperty("name", i18nc("@title", "Name"), name, this);
    n->setVariantType(QVariant::String);

    auto designCapacity = new SensorProperty("design", i18nc("@title", "Design Capacity"), battery->energyFullDesign(), this);
    designCapacity->setShortName(i18nc("@title", "Design Capacity"));
    designCapacity->setPrefix(name);
    designCapacity->setDescription(i18n("Amount of energy that the Battery was designed to hold"));
    designCapacity->setUnit(KSysGuard::UnitWattHour);
    designCapacity->setVariantType(QVariant::Double);;
    designCapacity->setMin(battery->energyFullDesign());
    designCapacity->setMax(battery->energyFullDesign());

    auto currentCapacity = new SensorProperty("capacity", i18nc("@title", "Current Capacity"), battery->energyFull(), this);
    currentCapacity->setShortName(i18nc("@title", "Current Capacity"));
    currentCapacity->setPrefix(name);
    currentCapacity->setDescription(i18n("Amount of energy that the battery can currently hold"));
    currentCapacity->setUnit(KSysGuard::UnitWattHour);
    currentCapacity->setVariantType(QVariant::Double);
    currentCapacity->setMax(designCapacity);
    connect(battery, &Solid::Battery::energyFullChanged, currentCapacity, &SensorProperty::setValue);

    auto health = new SensorProperty("health", i18nc("@title", "Health"), battery->capacity(), this);
    health->setShortName(i18nc("@title", "Health"));
    health->setPrefix(name);
    health->setDescription(i18n("Percentage of the design capacity that the battery can hold"));
    health->setUnit(KSysGuard::UnitPercent);
    health->setVariantType(QVariant::Int);
    health->setMax(100);
    connect(battery, &Solid::Battery::capacityChanged, health, &SensorProperty::setValue);

    auto charge = new SensorProperty("charge", i18nc("@title", "Charge"), battery->energy(), this);
    charge->setShortName(i18nc("@title", "Current Capacity"));
    charge->setPrefix(name);
    charge->setDescription(i18n("Amount of energy that the battery is currently holding"));
    charge->setUnit(KSysGuard::UnitWattHour);
    charge->setVariantType(QVariant::Double);
    charge->setMax(currentCapacity);
    connect(battery, &Solid::Battery::energyChanged, charge, &SensorProperty::setValue);

    auto chargePercent = new SensorProperty("chargePercentage", i18nc("@title", "Charge Percentage"), battery->chargePercent(), this);
    chargePercent->setShortName(i18nc("@title", "Charge Percentage"));
    chargePercent->setPrefix(name);
    chargePercent->setDescription(i18n("Percentage of the current capacity that the battery is currently holding"));
    chargePercent->setUnit(KSysGuard::UnitPercent);
    chargePercent->setVariantType(QVariant::Int);
    chargePercent->setMax(100);
    connect(battery, &Solid::Battery::chargePercentChanged, chargePercent, &SensorProperty::setValue);

    // Solid reports negative of charging and positive for discharging
    auto chargeRate = new SensorProperty("chargeRate", i18nc("@title", "Charging Rate"), -battery->energyRate(), this);
    chargeRate->setShortName(i18nc("@title", "Charging  Rate"));
    chargeRate->setPrefix(name);
    chargeRate->setDescription(i18n("Power that the battery is being charged with (positive) or discharged (negative)"));
    chargeRate->setUnit(KSysGuard::UnitPercent);
    chargeRate->setVariantType(QVariant::Double);
    connect(battery, &Solid::Battery::energyRateChanged, chargeRate, [chargeRate] (double rate) {
        chargeRate->setValue(-rate);
    });

}

PowerPlugin::PowerPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    m_container = new SensorContainer("power", i18nc("@title", "Power"), this);
    const auto batteries = Solid::Device::listFromType(Solid::DeviceInterface::Battery);
    for (const auto &battery : batteries) {
       new Battery(battery.as<Solid::Battery>(), battery.displayName(), m_container);
    }

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, [this] (const QString &udi) {
        const Solid::Device device(udi);
        if (device.isDeviceInterface(Solid::DeviceInterface::Battery)) {
            new Battery(device.as<Solid::Battery>(), device.displayName(), m_container);
        }
    });
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, [this] (const QString &udi) {
        const Solid::Device device(udi);
        if (device.isDeviceInterface(Solid::DeviceInterface::Battery)) {
            const auto object = m_container->object(device.as<Solid::Battery>()->serial());
            if (object) {
                m_container->removeObject(object);
            }
        }
    });
}

#include "power.moc"
