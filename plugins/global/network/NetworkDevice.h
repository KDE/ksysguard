/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "SensorObject.h"

class NetworkDevice : public SensorObject
{
    Q_OBJECT

public:
    NetworkDevice(const QString& id, const QString& name);
    ~NetworkDevice() override = default;

    virtual void update() = 0;

protected:
    SensorProperty *m_networkSensor;
    SensorProperty *m_signalSensor;
    SensorProperty *m_ipv4Sensor;
    SensorProperty *m_ipv6Sensor;
    SensorProperty *m_downloadSensor;
    SensorProperty *m_uploadSensor;
    SensorProperty *m_totalDownloadSensor;
    SensorProperty *m_totalUploadSensor;
};
