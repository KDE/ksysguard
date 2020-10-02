/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>

class GpuDevice;

class GpuBackend : public QObject
{
    Q_OBJECT

public:
    GpuBackend(QObject* parent = nullptr) : QObject(parent) { }
    ~GpuBackend() override = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void update() = 0;

    Q_SIGNAL void deviceAdded(GpuDevice *device);
    Q_SIGNAL void deviceRemoved(GpuDevice *device);
};
