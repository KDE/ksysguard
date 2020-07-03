/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>
#include <QHash>

class NetworkDevice;

class NetworkBackend : public QObject
{
    Q_OBJECT

public:
    NetworkBackend(QObject* parent = nullptr) : QObject(parent) { }
    ~NetworkBackend() override = default;

    virtual bool isSupported() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    Q_SIGNAL void deviceAdded(NetworkDevice *device);
    Q_SIGNAL void deviceRemoved(NetworkDevice *device);
};
