/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "NetworkManagerBackend.h"

#include <QDebug>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/ModemDevice>

NetworkManagerDevice::NetworkManagerDevice(const QString &id, QSharedPointer<NetworkManager::Device> device)
    : NetworkDevice(id, id)
    , m_device(device)
{
    connect(m_device.data(), &NetworkManager::Device::activeConnectionChanged, this, &NetworkManagerDevice::update);
    connect(m_device.data(), &NetworkManager::Device::ipV4ConfigChanged, this, &NetworkManagerDevice::update);
    connect(m_device.data(), &NetworkManager::Device::ipV6ConfigChanged, this, &NetworkManagerDevice::update);

    connect(this, &NetworkManagerDevice::nameChanged, this, [this]() {
        m_networkSensor->setPrefix(name());
        m_signalSensor->setPrefix(name());
        m_ipv4Sensor->setPrefix(name());
        m_ipv6Sensor->setPrefix(name());
        m_downloadSensor->setPrefix(name());
        m_uploadSensor->setPrefix(name());
        m_totalDownloadSensor->setPrefix(name());
        m_totalUploadSensor->setPrefix(name());
    });

    auto statistics = m_device->deviceStatistics();
    statistics->setRefreshRateMs(2000);
    connect(statistics.data(), &NetworkManager::DeviceStatistics::rxBytesChanged, this, [this](qulonglong bytes) {
        auto previousValue = m_totalDownloadSensor->value().toULongLong();
        if (previousValue > 0) {
            m_downloadSensor->setValue(bytes - previousValue);
        }
        m_totalDownloadSensor->setValue(bytes);
    });
    connect(statistics.data(), &NetworkManager::DeviceStatistics::txBytesChanged, this, [this](qulonglong bytes) {
        auto previousValue = m_totalUploadSensor->value().toULongLong();
        if (previousValue > 0) {
            m_uploadSensor->setValue(bytes - previousValue);
        }
        m_totalUploadSensor->setValue(bytes);
    });

    if (m_device->type() == NetworkManager::Device::Wifi) {
        auto wifi = m_device->as<NetworkManager::WirelessDevice>();
        connect(wifi, &NetworkManager::WirelessDevice::activeConnectionChanged, this, [this, wifi]() { updateWifi(wifi); } );
        connect(wifi, &NetworkManager::WirelessDevice::networkAppeared, this, [this, wifi]() { updateWifi(wifi); });
        connect(wifi, &NetworkManager::WirelessDevice::networkDisappeared, this, [this, wifi]() { updateWifi(wifi); });
        updateWifi(wifi);
    }

    update();
}

NetworkManagerDevice::~NetworkManagerDevice()
{
}

void NetworkManagerDevice::update()
{
    if (m_device->activeConnection()) {
        setName(m_device->activeConnection()->connection()->name());
        m_networkSensor->setValue(name());
    } else {
        setName(m_device->ipInterfaceName());
        m_networkSensor->setValue(QString{});
    }

    if (m_device->ipV4Config().isValid()) {
        m_ipv4Sensor->setValue(m_device->ipV4Config().addresses().at(0).ip().toString());
    } else {
        m_ipv4Sensor->setValue(QString{});
    }

    if (m_device->ipV6Config().isValid()) {
        m_ipv6Sensor->setValue(m_device->ipV6Config().addresses().at(0).ip().toString());
    } else {
        m_ipv4Sensor->setValue(QString{});
    }
}

void NetworkManagerDevice::updateWifi(NetworkManager::WirelessDevice* device)
{
    auto activeConnectionName = device->activeConnection()->connection()->name();
    const auto networks = device->networks();
    std::for_each(networks.begin(), networks.end(), [this, activeConnectionName](QSharedPointer<NetworkManager::WirelessNetwork> network) {
        if (network->ssid() == activeConnectionName) {
            connect(network.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, m_signalSensor, &SensorProperty::setValue, Qt::UniqueConnection);
            m_signalSensor->setValue(network->signalStrength());
        } else {
            network->disconnect(m_signalSensor);
        }
    });
}

NetworkManagerBackend::NetworkManagerBackend(QObject* parent)
    : NetworkBackend(parent)
{
}

NetworkManagerBackend::~NetworkManagerBackend()
{
    qDeleteAll(m_devices);
}

bool NetworkManagerBackend::isSupported()
{
    if (NetworkManager::status() == NetworkManager::Unknown) {
        return false;
    } else {
        return true;
    }
}

void NetworkManagerBackend::start()
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &NetworkManagerBackend::onDeviceAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &NetworkManagerBackend::onDeviceRemoved);

    const auto devices = NetworkManager::networkInterfaces();
    for (auto device : devices) {
        onDeviceAdded(device->uni());
    }
}

void NetworkManagerBackend::stop()
{
    NetworkManager::notifier()->disconnect(this);
}

void NetworkManagerBackend::onDeviceAdded(const QString& uni)
{
    auto device = NetworkManager::findNetworkInterface(uni);
    if (!device) {
        return;
    }

    switch (device->type()) {
        case NetworkManager::Device::Ethernet:
        case NetworkManager::Device::Wifi:
        case NetworkManager::Device::Bluetooth:
        case NetworkManager::Device::Modem:
        case NetworkManager::Device::Adsl:
            break;
        default:
            // Non-hardware devices, ignore them.
            return;
    }

    auto nmDevice = new NetworkManagerDevice(device->ipInterfaceName(), device);
    m_devices.insert(uni, nmDevice);
    Q_EMIT deviceAdded(nmDevice);
}

void NetworkManagerBackend::onDeviceRemoved(const QString& uni)
{
    if (!m_devices.contains(uni)) {
        return;
    }

    auto device = m_devices.take(uni);
    Q_EMIT deviceRemoved(device);
    delete device;
}

