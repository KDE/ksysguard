/*
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LMSENSORS_H
#define LMSENSORS_H

#include <SensorPlugin.h>

class SensorsFeatureSensor;

class LmSensorsPlugin : public SensorPlugin
{
    Q_OBJECT
public:
    LmSensorsPlugin(QObject *parent, const QVariantList &args);
    ~LmSensorsPlugin() override;
    QString providerName() const override;
    void update() override;
private:
    QVector<SensorsFeatureSensor*> m_sensors;
};
#endif
