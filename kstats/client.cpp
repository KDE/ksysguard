/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>

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

#include "client.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>

#include <algorithm>

#include "ksysguarddaemon.h"
#include "SensorPlugin.h"
#include "SensorObject.h"
#include "SensorProperty.h"

Client::Client(KSysGuardDaemon *parent, const QString &serviceName)
    : QObject(parent)
    , m_serviceName(serviceName)
    , m_daemon(parent)
{
    connect(m_daemon, &KSysGuardDaemon::sensorRemoved, this, [this](const QString &sensor) {
        m_subscribedSensors.remove(sensor);
    });
}

Client::~Client()
{
    for (auto sensor : qAsConst(m_subscribedSensors)) {
        sensor->unsubscribe();
    }
}

void Client::subscribeSensors(const QStringList &sensorPaths)
{
    SensorDataList entries;

    for (const QString &sensorPath : sensorPaths) {
        if (auto sensor = m_daemon->findSensor(sensorPath)) {
            m_connections.insert(sensor, connect(sensor, &SensorProperty::valueChanged, this, [this, sensor]() {
                const QVariant value = sensor->value();
                if (!value.isValid()) {
                    return;
                }
                m_pendingUpdates << SensorData(sensor->path(), value);
            }));
            m_connections.insert(sensor, connect(sensor, &SensorProperty::sensorInfoChanged, this, [this, sensor]() {
                m_pendingMetaDataChanges[sensor->path()] = sensor->info();
            }));

            sensor->subscribe();

            m_subscribedSensors.insert(sensorPath, sensor);
        }
    }
}

void Client::unsubscribeSensors(const QStringList &sensorPaths)
{
    for (const QString &sensorPath : sensorPaths) {
        if (auto sensor = m_subscribedSensors.take(sensorPath)) {
            disconnect(m_connections.take(sensor));
            disconnect(m_connections.take(sensor));
            sensor->unsubscribe();
        }
    }
}

void Client::sendFrame()
{
    sendMetaDataChanged(m_pendingMetaDataChanges);
    sendValues(m_pendingUpdates);
    m_pendingUpdates.clear();
    m_pendingMetaDataChanges.clear();
}

void Client::sendValues(const SensorDataList &entries)
{
    if (entries.isEmpty()) {
        return;
    }
    auto msg = QDBusMessage::createTargetedSignal(m_serviceName, "/", "org.kde.KSysGuardDaemon", "newSensorData");
    msg.setArguments({QVariant::fromValue(entries)});
    QDBusConnection::sessionBus().send(msg);
}

void Client::sendMetaDataChanged(const SensorInfoMap &sensors)
{
    if (sensors.isEmpty()) {
        return;
    }
    auto msg = QDBusMessage::createTargetedSignal(m_serviceName, "/", "org.kde.KSysGuardDaemon", "sensorMetaDataChanged");
    msg.setArguments({QVariant::fromValue(sensors)});
    QDBusConnection::sessionBus().send(msg);
}
