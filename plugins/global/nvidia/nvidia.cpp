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

#include "nvidia.h"

#include <QDateTime>
#include <QProcess>
#include <QStandardPaths>

#include <KPluginFactory>

#include <KLocalizedString>

#include <SensorContainer.h>

class GPU : public SensorObject
{
public:
    GPU(int index, SensorContainer *parent);
    ~GPU() = default;
    SensorProperty *powerProperty() const { return m_pwr; }
    SensorProperty *tempProperty() const { return m_temp; }
    SensorProperty *sharedMemory() const { return m_sm; }
    SensorProperty *memory() const { return m_mem; }
    SensorProperty *encoder() const { return m_enc; }
    SensorProperty *decoder() const { return m_dec; }
    SensorProperty *memoryClock() const { return m_memClock; }
    SensorProperty *processorClock() const { return m_processorClock; }

private:
    SensorProperty *m_pwr;
    SensorProperty *m_temp;
    SensorProperty *m_sm;
    SensorProperty *m_mem;
    SensorProperty *m_enc;
    SensorProperty *m_dec;
    SensorProperty *m_memClock;
    SensorProperty *m_processorClock;
};

GPU::GPU(int index, SensorContainer *parent)
    : SensorObject(QStringLiteral("gpu%1").arg(index), i18nc("@title", "GPU %1", index + 1), parent)
{
    auto displayIndex = index + 1;

    m_pwr = new SensorProperty(QStringLiteral("power"), this);
    m_pwr->setName(i18nc("@title", "GPU %1 Power Usage", displayIndex));
    m_pwr->setShortName(i18nc("@title GPU Power Usage", "Power"));
    m_pwr->setUnit(KSysGuard::UnitWatt);
    m_pwr->setVariantType(QVariant::UInt);

    m_temp = new SensorProperty(QStringLiteral("temperature"), this);
    m_temp->setName(i18nc("@title", "GPU %1 Temperature", displayIndex));
    m_temp->setShortName(i18nc("@title GPU Temperature", "Temperature"));
    m_temp->setUnit(KSysGuard::UnitCelsius);
    m_temp->setVariantType(QVariant::Double);

    m_sm = new SensorProperty(QStringLiteral("sharedMemory"), this);
    m_sm->setName(i18nc("@title", "GPU %1 Shared Memory Usage", displayIndex));
    m_sm->setShortName(i18nc("@title GPU Shared Memory Usage", "Shared Memory"));
    m_sm->setUnit(KSysGuard::UnitPercent);
    m_sm->setVariantType(QVariant::UInt);

    m_mem = new SensorProperty(QStringLiteral("memory"), this);
    m_mem->setName(i18nc("@title", "GPU %1 Memory Usage", displayIndex));
    m_mem->setShortName(i18nc("@title GPU Memory Usage", "Memory"));
    m_mem->setUnit(KSysGuard::UnitPercent);
    m_mem->setVariantType(QVariant::UInt);

    m_enc = new SensorProperty(QStringLiteral("encoderUsage"), this);
    m_enc->setName(i18nc("@title", "GPU %1 Encoder Usage", displayIndex));
    m_enc->setShortName(i18nc("@title GPU Encoder Usage", "Encoder"));
    m_enc->setUnit(KSysGuard::UnitPercent);
    m_enc->setVariantType(QVariant::UInt);

    m_dec = new SensorProperty(QStringLiteral("decoderUsage"), this);
    m_dec->setName(i18nc("@title", "GPU %1 Decoder Usage", displayIndex));
    m_dec->setShortName(i18nc("@title GPU Decoder Usage", "Decoder"));
    m_dec->setUnit(KSysGuard::UnitPercent);
    m_dec->setVariantType(QVariant::UInt);

    m_memClock = new SensorProperty(QStringLiteral("memoryClock"), this);
    m_memClock->setName(i18nc("@title", "GPU %1 Memory Clock", displayIndex));
    m_memClock->setName(i18nc("@title GPU Memory Clock", "Memory Clock"));
    m_memClock->setUnit(KSysGuard::UnitMegaHertz);
    m_memClock->setVariantType(QVariant::UInt);

    m_processorClock = new SensorProperty(QStringLiteral("processorClock"), this);
    m_processorClock->setName(i18nc("@title", "GPU %1 Processor Clock", displayIndex));
    m_processorClock->setName(i18nc("@title GPU Processor Clock", "Processor Clock"));
    m_processorClock->setUnit(KSysGuard::UnitMegaHertz);
    m_processorClock->setVariantType(QVariant::UInt);
}

NvidiaPlugin::NvidiaPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    const auto sniExecutable = QStandardPaths::findExecutable("nvidia-smi");
    if (sniExecutable.isEmpty()) {
        return;
    }

    auto gpuSystem = new SensorContainer("nvidia", i18nc("@title NVidia GPU information", "NVidia"), this);

    //assuming just one GPU for now
    auto gpu0 = new GPU(0, gpuSystem);
    connect(gpu0, &SensorObject::subscribedChanged, this, &NvidiaPlugin::gpuSubscriptionChanged);
    m_gpus[0] = gpu0;

    m_process = new QProcess(this);
    m_process->setProgram(sniExecutable);
    m_process->setArguments({ QStringLiteral("dmon") });

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=]() {
        while (m_process->canReadLine()) {
            const QString line = m_process->readLine();
            if (line.startsWith(QLatin1Char('#'))) {
                continue;
            }
            const QVector<QStringRef> parts = line.splitRef(QLatin1Char(' '), Qt::SkipEmptyParts);

            // format at time of writing is
            // # gpu   pwr gtemp mtemp    sm   mem   enc   dec  mclk  pclk
            if (parts.count() != 10) {
                continue;
            }
            bool ok;
            int index = parts[0].toInt(&ok);
            if (!ok) {
                continue;
            }
            GPU *gpu = m_gpus.value(index);
            if (!gpu) {
                continue;
            }
            gpu->powerProperty()->setValue(parts[1].toUInt());
            gpu->tempProperty()->setValue(parts[2].toUInt());
            // I have no idea what parts[3] mtemp represents..skipping for now
            gpu->sharedMemory()->setValue(parts[4].toUInt());
            gpu->memory()->setValue(parts[5].toUInt());
            gpu->encoder()->setValue(parts[6].toUInt());
            gpu->decoder()->setValue(parts[7].toUInt());
            gpu->memoryClock()->setValue(parts[8].toUInt());
            gpu->processorClock()->setValue(parts[9].toUInt());
        }
    });
}

void NvidiaPlugin::gpuSubscriptionChanged(bool subscribed)
{
    if (subscribed) {
        m_activeWatcherCount++;
        if (m_activeWatcherCount == 1) {
            m_process->start();
        }
    } else {
        m_activeWatcherCount--;
        if (m_activeWatcherCount == 0) {
            m_process->terminate();
        }
    }
}

K_PLUGIN_CLASS_WITH_JSON(NvidiaPlugin, "metadata.json")

#include "nvidia.moc"
