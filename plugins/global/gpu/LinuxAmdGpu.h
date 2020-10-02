/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>

#include "GpuDevice.h"

struct udev_device;

class SysFsSensor;

class LinuxAmdGpu : public GpuDevice
{
    Q_OBJECT

public:
    LinuxAmdGpu(const QString& id, const QString& name, udev_device *device);
    ~LinuxAmdGpu() override;

    void initialize() override;
    void update() override;

protected:
    void makeSensors() override;

private:
    udev_device *m_device;
    QVector<SysFsSensor*> m_sysFsSensors;
    QString m_coreTemperatureCurrentPath;
    QString m_coreTemperatureMaxPath;
};
