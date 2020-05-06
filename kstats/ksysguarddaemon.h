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

#pragma once

#include <types.h>
#include <QDBusContext>
#include <QObject>

class SensorPlugin;
class SensorContainer;
class SensorProperty;
class Entity;
class Client;
class QDBusServiceWatcher;

/**
 * The main central application
 */
class KSysGuardDaemon : public QObject, public QDBusContext
{
    Q_OBJECT

public:
    KSysGuardDaemon();
    void init();
    SensorProperty *findSensor(const QString &path) const;

public Q_SLOTS:
    // DBus
    SensorInfoMap allSensors() const;
    SensorInfoMap sensors(const QStringList &sensorsIds) const;

    void subscribe(const QStringList &sensorIds);
    void unsubscribe(const QStringList &sensorIds);

    SensorDataList sensorData(const QStringList &sensorIds);

Q_SIGNALS:
    // DBus
    void sensorAdded(const QString &sensorId);
    void sensorRemoved(const QString &sensorId);
    // not emitted directly as we use targetted signals via lower level API
    void newSensorData(const SensorDataList &sensorData);

protected:
    // virtual for autotest to override and not load real plugins
    virtual void loadProviders();

    void sendFrame();
    void registerProvider(SensorPlugin *);

private:
    void onServiceDisconnected(const QString &service);
    QVector<SensorPlugin *> m_providers;
    QHash<QString /*subscriber DBus base name*/, Client*> m_clients;
    QHash<QString /*id*/, SensorContainer *> m_containers;
    QDBusServiceWatcher *m_serviceWatcher;
};
