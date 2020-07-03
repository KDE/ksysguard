/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <SensorObject.h>

class NetworkDevice;
class AggregateSensor;

/**
 * This object aggregates the network usage of all devices.
 */
class AllDevicesObject : public SensorObject
{
    Q_OBJECT

public:
    AllDevicesObject(SensorContainer* parent);

private:
    AggregateSensor *m_downloadSensor = nullptr;
    AggregateSensor *m_uploadSensor = nullptr;
    AggregateSensor *m_totalDownloadSensor = nullptr;
    AggregateSensor *m_totalUploadSensor = nullptr;
};
