/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "SensorPlugin.h"

class NetworkPrivate;
class NetworkDevice;

class NetworkPlugin : public SensorPlugin
{
    Q_OBJECT
public:
    NetworkPlugin(QObject *parent, const QVariantList &args);
    ~NetworkPlugin() override;

    QString providerName() const override
    {
        return QStringLiteral("network");
    }

    void onDeviceAdded(NetworkDevice *device);
    void onDeviceRemoved(NetworkDevice *device);

private:
    std::unique_ptr<NetworkPrivate> d;
};
