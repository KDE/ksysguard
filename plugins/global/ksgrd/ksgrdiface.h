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

#include "SensorContainer.h"
#include "SensorObject.h"
#include "SensorPlugin.h"
#include "SensorProperty.h"

#include <QTimer>
#include <ksgrd/SensorClient.h>

class AggregateSensor;

class KSGRDIface : public SensorPlugin, public KSGRD::SensorClient
{
    Q_OBJECT

public:
    KSGRDIface(QObject *parent, const QVariantList &args);
    ~KSGRDIface();

    virtual QString providerName() const override
    {
        return QStringLiteral("ksgrd");
    }

    void update() override;

    //  From KSGRD::SensorClient
    void sensorLost(int sensor) override;
    void answerReceived(int id, const QList<QByteArray> &answer) override;

Q_SIGNALS:
    void sensorAdded();

private:
    void updateMonitorsList();
    void onSensorMetaDataRetrieved(int id, const QList<QByteArray> &answer);
    void onSensorListRetrieved(const QList<QByteArray> &answer);
    void onSensorUpdated(int id, const QList<QByteArray> &answer);

    void subscribe(const QString &sensorPath);
    void unsubscribe(const QString &sensorPath);

    KSysGuard::Unit unitFromString(const QString &unitString) const;

    //This qlist is just to have an index mapping because of KSGRD's old API
    //Could be an index in SensorInfo subclass
    QStringList m_sensorIds;
    QStringList m_subscribedSensors;

    QHash<QString, SensorContainer *> m_subsystems;
    QHash<QString, SensorProperty *> m_sensors;
    QHash<QString, QString> m_pendingTypes;
    int m_waitingFor;
};
