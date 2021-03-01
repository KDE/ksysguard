/*
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "lmsensors.h"

#include "SensorsFeatureSensor.h"

#include <SensorContainer.h>
#include <SensorObject.h>

#include <KLocalizedString>
#include <KPluginFactory>

#include <sensors/sensors.h>

LmSensorsPlugin::LmSensorsPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    auto container = new SensorContainer(QStringLiteral("lmsensors"), i18n( "Hardware Sensors" ), this);
    if (sensors_init(nullptr) != 0) {
        return;
    }

    const std::array<QByteArray, 3> blacklist{"coretemp","k10temp","amdgpu"}; //already handled by other plugins
    int chipNumber = 0;
    while (const sensors_chip_name * const chipName = sensors_get_detected_chips(nullptr, &chipNumber)) {
        if (std::find(blacklist.cbegin(), blacklist.cend(), chipName->prefix) != blacklist.cend()) {
            continue;
        }
        int requiredBytes  = sensors_snprintf_chip_name(nullptr, 0, chipName) + 1;
        QByteArray name;
        name.resize(requiredBytes);
        sensors_snprintf_chip_name(name.data(), name.size(), chipName);
        const QString nameString = QString::fromUtf8(name);
        SensorObject *sensorObject =  container->object(nameString);
        if (!sensorObject) {
            sensorObject = new SensorObject(nameString, nameString, container);
        }
        int featureNumber = 0;
        while (const sensors_feature * const feature = sensors_get_features(chipName, &featureNumber)) {
            if (auto sensor = makeSensorsFeatureSensor(chipName, feature, sensorObject)) {
                m_sensors.push_back(sensor);
            }
        }
    }
}

LmSensorsPlugin::~LmSensorsPlugin()
{
    sensors_cleanup();
}


QString LmSensorsPlugin::providerName() const
{
    return QStringLiteral("lmsensors");
}

void LmSensorsPlugin::update()
{
    for (auto sensor : qAsConst(m_sensors)) {
        sensor->update();
    }
}

K_PLUGIN_CLASS_WITH_JSON(LmSensorsPlugin, "metadata.json")
#include "lmsensors.moc"
