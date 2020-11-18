/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "NetworkManagerBackend.h"

#include <QTimer>
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

    m_statistics = m_device->deviceStatistics();

    // We want to display speed in bytes per second, so use a fixed one-second
    // update interval here so we are independant of the actual update rate of
    // the daemon.
    m_statistics->setRefreshRateMs(1000);

    // Unfortunately, the statistics interface does not emit change signals if
    // no change happened. This makes the change signals rather useless for our
    // case because we also need to know when no change happened, so that we
    // can update rate sensors to show 0. So instead use a timer and query the
    // statistics every second, updating the sensors as needed.
    m_statisticsTimer = std::make_unique<QTimer>();
    m_statisticsTimer->setInterval(1000);
    connect(m_statisticsTimer.get(), &QTimer::timeout, this, [this]() {
        auto newDownload = m_statistics->rxBytes();
        auto previousDownload = m_totalDownloadSensor->value().toULongLong();
        if (previousDownload > 0) {
            m_downloadSensor->setValue(newDownload - previousDownload);
        }
        m_totalDownloadSensor->setValue(newDownload);

        auto newUpload = m_statistics->txBytes();
        auto previousUpload = m_totalUploadSensor->value().toULongLong();
        if (previousUpload > 0) {
            m_uploadSensor->setValue(newUpload - previousUpload);
        }
        m_totalUploadSensor->setValue(newUpload);
    });

    std::vector<SensorProperty*> statisticSensors{m_downloadSensor, m_totalDownloadSensor, m_uploadSensor, m_totalUploadSensor};
    for (auto property : statisticSensors) {
        connect(property, &SensorProperty::subscribedChanged, this, [this, statisticSensors](bool subscribed) {
            if (subscribed && !m_statisticsTimer->isActive()) {
                m_statisticsTimer->start();
            } else if (std::none_of(statisticSensors.begin(), statisticSensors.end(), [](auto property) { return property->isSubscribed(); })) {
                m_statisticsTimer->stop();
                m_totalDownloadSensor->setValue(0);
                m_totalUploadSensor->setValue(0);
            }
        });
    }

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
    setName(m_device->activeConnection()->connection()->name());
    m_networkSensor->setValue(name());

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

    connect(device.get(), &NetworkManager::Device::activeConnectionChanged, this, [this, device, uni]() {
        if (device->activeConnection()) {
            onDeviceAdded(uni);
        } else {
            onDeviceRemoved(uni);
        }
    }, Qt::UniqueConnection);

    if (!device->activeConnection()) {
        return;
    }

    if (m_devices.contains(uni)) {
        return;
    }

    auto nmDevice = new NetworkManagerDevice(device->interfaceName(), device);
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

