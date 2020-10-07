/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "NvidiaSmiProcess.h"

#include <QStandardPaths>

NvidiaSmiProcess::NvidiaSmiProcess()
{
    m_smiPath = QStandardPaths::findExecutable("nvidia-smi");
}

bool NvidiaSmiProcess::isSupported() const
{
    return !m_smiPath.isEmpty();
}

QVector<NvidiaSmiProcess::GpuQueryResult> NvidiaSmiProcess::query()
{
    QVector<NvidiaSmiProcess::GpuQueryResult> result;

    if (!isSupported()) {
        return result;
    }

    // Read and parse the result of "nvidia-smi query"
    // This seems to be the only way to get certain values like total memory or
    // maximum temperature. Unfortunately the output isn't very easily parseable
    // so we have to do some trickery to parse things.

    QProcess queryProcess;
    queryProcess.setProgram(m_smiPath);
    queryProcess.setArguments({QStringLiteral("--query")});
    queryProcess.start();

    int gpuCounter = 0;
    GpuQueryResult *data = &result[0];

    bool readMemory = false;
    bool readMaxClocks = false;

    while (queryProcess.waitForReadyRead()) {
        if (!queryProcess.canReadLine()) {
            continue;
        }

        auto line = queryProcess.readLine();
        if (line.startsWith("GPU ")) {
            // Start of GPU properties block.
            result.append(GpuQueryResult{});
            data = &result[gpuCounter];
            gpuCounter++;
        }

        if ((readMemory || readMaxClocks) && !line.startsWith("        ")) {
            readMemory = false;
            readMaxClocks = false;
        }

        if (line.startsWith("    Product Name")) {
            data->name = line.mid(line.indexOf(':') + 1).trimmed();
        }

        if (line.startsWith("    FB Memory Usage") || line.startsWith("    BAR1 Memory Usage")) {
            readMemory = true;
        }

        if (line.startsWith("    Max Clocks")) {
            readMaxClocks = true;
        }

        if (line.startsWith("        Total") && readMemory) {
            data->totalMemory += std::atoi(line.mid(line.indexOf(':') + 1));
        }

        if (line.startsWith("        GPU Shutdown Temp")) {
            data->maxTemperature = std::atoi(line.mid(line.indexOf(':') + 1));
        }

        if (line.startsWith("        Graphics") && readMaxClocks) {
            data->maxCoreFrequency = std::atoi(line.mid(line.indexOf(':') + 1));
        }

        if (line.startsWith("        Memory") && readMaxClocks) {
            data->maxMemoryFrequency = std::atoi(line.mid(line.indexOf(':') + 1));
        }
    }

    return result;
}

void NvidiaSmiProcess::ref()
{
    if (!isSupported()) {
        return;
    }

    m_references++;

    if (m_process) {
        return;
    }

    m_process = std::make_unique<QProcess>();
    m_process->setProgram(m_smiPath);
    m_process->setArguments({
        QStringLiteral("dmon"), // Monitor
        QStringLiteral("-d"), QStringLiteral("2"), // 2 seconds delay, to match daemon update rate
        QStringLiteral("-s"), QStringLiteral("pucm") // Include all relevant statistics
    });
    connect(m_process.get(), &QProcess::readyReadStandardOutput, this, &NvidiaSmiProcess::readStatisticsData);
    m_process->start();
}

void NvidiaSmiProcess::unref()
{
    if (!isSupported()) {
        return;
    }

    m_references--;

    if (!m_process || m_references > 0) {
        return;
    }

    m_process->terminate();
    m_process->waitForFinished();
    m_process.reset();
}

void NvidiaSmiProcess::readStatisticsData()
{
    while (m_process->canReadLine()) {
        const QString line = m_process->readLine();
        if (line.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const QVector<QStringRef> parts = line.splitRef(QLatin1Char(' '), Qt::SkipEmptyParts);

        // format at time of writing is
        // # gpu   pwr gtemp mtemp    sm   mem   enc   dec  mclk  pclk  fb  bar1
        if (parts.count() != 12) {
            continue;
        }

        bool ok;
        int index = parts[0].toInt(&ok);
        if (!ok) {
            continue;
        }

        GpuData data;
        data.index = index;
        data.power = parts[1].toUInt();
        data.temperature = parts[2].toUInt();

        // GPU usage equals "SM" usage + "ENC" usage + "DEC" usage
        data.usage = parts[4].toUInt() + parts[6].toUInt() + parts[7].toUInt();

        // Total memory used equals "FB" usage + "BAR1" usage
        data.memoryUsed = parts[10].toUInt() + parts[11].toUInt();

        data.memoryFrequency = parts[8].toUInt();
        data.coreFrequency = parts[9].toUInt();

        Q_EMIT dataReceived(data);
    }
}
