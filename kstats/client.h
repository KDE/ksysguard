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

#include "types.h"
#include <QObject>

class SensorProperty;
class KSysGuardDaemon;

/**
 * This class represents an individual connection to the daemon
 */
class Client : public QObject
{
    Q_OBJECT
public:
    Client(KSysGuardDaemon *parent, const QString &serviceName);
    ~Client() override;
    void subscribeSensors(const QStringList &sensorIds);
    void unsubscribeSensors(const QStringList &sensorIds);
    void sendFrame();

private:
    void sendValues(const SensorDataList &updates);
    void sendMetaDataChanged(const SensorInfoMap &sensors);

    const QString m_serviceName;
    KSysGuardDaemon *m_daemon;
    QHash<QString, SensorProperty *> m_subscribedSensors;
    QMultiHash<SensorProperty *, QMetaObject::Connection> m_connections;
    SensorDataList m_pendingUpdates;
    SensorInfoMap m_pendingMetaDataChanges;
};
