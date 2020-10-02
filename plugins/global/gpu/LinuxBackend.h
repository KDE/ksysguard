/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "GpuBackend.h"

struct udev;
class GpuDevice;

class LinuxBackend : public GpuBackend
{
    Q_OBJECT

public:
    LinuxBackend(QObject* parent = nullptr);

    void start() override;
    void stop() override;
    void update() override;

private:
    udev *m_udev;
    QVector<GpuDevice*> m_devices;
};
