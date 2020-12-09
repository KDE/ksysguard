/*
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "NetworkBackend.h"
#include "NetworkDevice.h"

#include <QElapsedTimer>

#include <netlink/cache.h>
#include <netlink/socket.h>

struct rtnl_addr;
struct rtnl_link;

class RtNetlinkDevice : public NetworkDevice
{
    Q_OBJECT
public:
    RtNetlinkDevice(const QString &id);
    void update(rtnl_link *link, nl_cache *address_cache, qint64 elapsedTime);
Q_SIGNALS:
    void connected();
    void disconnected();

private:
    bool m_connected = false;
};

class RtNetlinkBackend : public NetworkBackend
{
public:
    RtNetlinkBackend(QObject *parent);
    ~RtNetlinkBackend() override;
    bool isSupported() override;
    void start() override;
    void stop() override;
    void update() override;

private:
    QHash<QByteArray, RtNetlinkDevice *> m_devices;
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> m_socket;
    QElapsedTimer m_updateTimer;
};
