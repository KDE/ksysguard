/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>

#include "GpuDevice.h"
#include "NvidiaSmiProcess.h"

class LinuxNvidiaGpu : public GpuDevice
{
    Q_OBJECT

public:
    LinuxNvidiaGpu(int index, const QString& id, const QString& name);
    ~LinuxNvidiaGpu() override;

    void initialize() override;

private:
    void onDataReceived(const NvidiaSmiProcess::GpuData &data);

    int m_index = 0;

    static NvidiaSmiProcess *s_smiProcess;
};
