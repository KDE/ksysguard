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

#include "ksysguarddaemon.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusServiceWatcher>

#include <QTimer>

#include "SensorPlugin.h"
#include "SensorObject.h"
#include "SensorContainer.h"
#include "SensorProperty.h"

#include <KPluginLoader>
#include <KPluginMetaData>
#include <KPluginFactory>

#include "ksysguard_ifaceadaptor.h"
#include "client.h"

KSysGuardDaemon::KSysGuardDaemon()
    : m_serviceWatcher(new QDBusServiceWatcher(this))
{
    qDBusRegisterMetaType<SensorData>();
    qDBusRegisterMetaType<SensorInfo>();
    qRegisterMetaType<SensorDataList>("SDL");
    qDBusRegisterMetaType<SensorDataList>();
    qDBusRegisterMetaType<SensorInfoMap>();
    qDBusRegisterMetaType<QStringList>();

    new KSysGuardDaemonAdaptor(this);

    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &KSysGuardDaemon::onServiceDisconnected);

    auto timer = new QTimer(this);
    timer->setInterval(2000);
    connect(timer, &QTimer::timeout, this, &KSysGuardDaemon::sendFrame);
    timer->start();
}

void KSysGuardDaemon::init()
{
    loadProviders();
    QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAdaptors);
    QDBusConnection::sessionBus().registerService("org.kde.ksystemstats");
}

void KSysGuardDaemon::loadProviders()
{
    //instantiate all plugins

    QPluginLoader loader;
    KPluginLoader::forEachPlugin(QStringLiteral("ksysguard"), [&loader, this](const QString &pluginPath) {
        loader.setFileName(pluginPath);
        QObject* obj = loader.instance();
        auto factory = qobject_cast<KPluginFactory*>(obj);
        if (!factory) {
            qWarning() << "Failed to load ksysguard factory";
            return;
        }
        SensorPlugin *provider = factory->create<SensorPlugin>(this);
        if (!provider) {
            return;
        }
        registerProvider(provider);
    });

    if (m_providers.isEmpty()) {
        qWarning() << "No plugins found";
    }
}

void KSysGuardDaemon::registerProvider(SensorPlugin *provider) {
    auto itr = std::find_if(m_providers.cbegin(), m_providers.cend(), [provider](SensorPlugin *plugin) {
        return plugin->providerName() == provider->providerName();
    });
    if (itr != m_providers.cend()) {
        qWarning() << "Skipping" << provider->providerName() << "as it is already registered";
        return;
    }

    m_providers.append(provider);
    const auto containers = provider->containers();
    for (auto container : containers) {
        m_containers[container->id()] = container;
        connect(container, &SensorContainer::objectAdded, this, [this](SensorObject *obj) {
            for (auto sensor: obj->sensors()) {
                emit sensorAdded(sensor->path());
            }
        });
        connect(container, &SensorContainer::objectRemoved, this, [this](SensorObject *obj) {
            for (auto sensor: obj->sensors()) {
                emit sensorRemoved(sensor->path());
            }
        });
    }
}

SensorInfoMap KSysGuardDaemon::allSensors() const
{
    SensorInfoMap infoMap;
    for (auto c : qAsConst(m_containers)) {
        auto containerInfo = SensorInfo{};
        containerInfo.name = c->name();
        infoMap.insert(c->id(), containerInfo);

        const auto objects = c->objects();
        for(auto object : objects) {
            auto objectInfo = SensorInfo{};
            objectInfo.name = object->name();
            infoMap.insert(object->path(), objectInfo);

            const auto sensors = object->sensors();
            for (auto sensor : sensors) {
                infoMap[sensor->path()] = sensor->info();
            }
        }
    }
    return infoMap;
}

SensorInfoMap KSysGuardDaemon::sensors(const QStringList &sensorPaths) const
{
    SensorInfoMap si;
    for (const QString &path : sensorPaths) {
        if (auto sensor = findSensor(path)) {
            si[path] = sensor->info();
        }
    }
    return si;
}

void KSysGuardDaemon::subscribe(const QStringList &sensorIds)
{
    const QString sender = QDBusContext::message().service();
    m_serviceWatcher->addWatchedService(sender);

    Client *client = m_clients.value(sender);
    if (!client) {
        client = new Client(this, sender);
        m_clients[sender] = client;
    }
    client->subscribeSensors(sensorIds);
}

void KSysGuardDaemon::unsubscribe(const QStringList &sensorIds)
{
    const QString sender = QDBusContext::message().service();
    Client *client = m_clients.value(sender);
    if (!client) {
        return;
    }
    client->unsubscribeSensors(sensorIds);
}

SensorDataList KSysGuardDaemon::sensorData(const QStringList &sensorIds)
{
    SensorDataList sensorData;
    for (const QString &sensorId: sensorIds) {
        if (SensorProperty *sensorProperty = findSensor(sensorId)) {
            const QVariant value = sensorProperty->value();
            if (value.isValid()) {
                sensorData << SensorData(sensorId, value);
            }
        }
    }
    return sensorData;
}

SensorProperty *KSysGuardDaemon::findSensor(const QString &path) const
{
    int subsystemIndex = path.indexOf('/');
    int propertyIndex = path.lastIndexOf('/');

    const QString subsystem = path.left(subsystemIndex);
    const QString object = path.mid(subsystemIndex + 1, propertyIndex - (subsystemIndex + 1));
    const QString property = path.mid(propertyIndex + 1);

    auto c = m_containers.value(subsystem);
    if (!c) {
        return nullptr;
    }
    auto o = c->object(object);
    if (!o) {
        return nullptr;
    }
    return o->sensor(property);
}

void KSysGuardDaemon::onServiceDisconnected(const QString &service)
{
    delete m_clients.take(service);
}

void KSysGuardDaemon::sendFrame()
{
    for (auto provider : qAsConst(m_providers)) {
        provider->update();
    }

    for (auto client: qAsConst(m_clients)) {
        client->sendFrame();
    }
}
