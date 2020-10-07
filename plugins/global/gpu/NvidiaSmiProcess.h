/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <memory>
#include <QObject>
#include <QProcess>

class NvidiaSmiProcess : public QObject
{
    Q_OBJECT

public:
    struct GpuData {
        int index = -1;
        uint power = 0;
        uint temperature = 0;
        uint usage = 0;
        uint memoryUsed = 0;
        uint coreFrequency = 0;
        uint memoryFrequency = 0;
    };

    struct GpuQueryResult {
        QString name;
        uint totalMemory = 0;
        uint maxCoreFrequency = 0;
        uint maxMemoryFrequency = 0;
        uint maxTemperature = 0;
    };

    NvidiaSmiProcess();

    bool isSupported() const;

    QVector<GpuQueryResult> query();

    void ref();
    void unref();

    Q_SIGNAL void dataReceived(const GpuData &data);

private:
    void readStatisticsData();

    QString m_smiPath;
    std::unique_ptr<QProcess> m_process = nullptr;
    int m_references = 0;
};
