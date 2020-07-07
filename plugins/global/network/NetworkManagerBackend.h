/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <memory>
#include <QHash>

#include "NetworkBackend.h"
#include "NetworkDevice.h"

class QTimer;

namespace NetworkManager
{
    class Device;
    class DeviceStatistics;
    class WirelessDevice;
}

class NetworkManagerDevice : public NetworkDevice
{
public:
    NetworkManagerDevice(const QString &id, QSharedPointer<NetworkManager::Device> device);
    ~NetworkManagerDevice() override;

    void update() override;

private:
    void updateWifi(NetworkManager::WirelessDevice *device);

    QSharedPointer<NetworkManager::Device> m_device;
    QSharedPointer<NetworkManager::DeviceStatistics> m_statistics;
    std::unique_ptr<QTimer> m_statisticsTimer;
};

class NetworkManagerBackend : public NetworkBackend
{
    Q_OBJECT

public:
    NetworkManagerBackend(QObject *parent = nullptr);
    ~NetworkManagerBackend() override;

    bool isSupported() override;
    void start() override;
    void stop() override;

private:
    void onDeviceAdded(const QString &uni);
    void onDeviceRemoved(const QString &uni);

    QHash<QString, NetworkManagerDevice *> m_devices;
};

