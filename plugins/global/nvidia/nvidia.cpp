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

#include <kpluginfactory.h>

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
    : SensorObject(QStringLiteral("gpu%1").arg(index), i18n("GPU %1", index), parent)
{
    m_pwr = new SensorProperty(QStringLiteral("power"), this);
    m_pwr->setName(i18n("Power"));
    m_pwr->setUnit(KSysGuard::utils::UnitWatt);
    m_pwr->setVariantType(QVariant::UInt);

    m_temp = new SensorProperty(QStringLiteral("temperature"), this);
    m_temp->setName(i18n("Temperature"));
    m_temp->setUnit(KSysGuard::utils::UnitCelsius);
    m_temp->setVariantType(QVariant::Double);

    m_sm = new SensorProperty(QStringLiteral("sharedMemory"), this);
    m_sm->setName(i18n("Shared memory"));
    m_sm->setUnit(KSysGuard::utils::UnitPercent);
    m_sm->setVariantType(QVariant::UInt);

    m_mem = new SensorProperty(QStringLiteral("memory"), this);
    m_mem->setName(i18n("Memory"));
    m_mem->setUnit(KSysGuard::utils::UnitPercent);
    m_mem->setVariantType(QVariant::UInt);

    m_enc = new SensorProperty(QStringLiteral("encoderUsage"), this);
    m_enc->setName(i18n("Encoder"));
    m_enc->setUnit(KSysGuard::utils::UnitPercent);
    m_enc->setVariantType(QVariant::UInt);

    m_dec = new SensorProperty(QStringLiteral("decoderUsage"), this);
    m_dec->setName(i18n("Decoder"));
    m_dec->setUnit(KSysGuard::utils::UnitPercent);
    m_dec->setVariantType(QVariant::UInt);

    m_memClock = new SensorProperty(QStringLiteral("memoryClock"), this);
    m_memClock->setName(i18n("Memory clock"));
    m_memClock->setUnit(KSysGuard::utils::UnitMegaHertz);
    m_memClock->setVariantType(QVariant::UInt);

    m_processorClock = new SensorProperty(QStringLiteral("processorClock"), this);
    m_processorClock->setName(i18n("Processor clock"));
    m_processorClock->setUnit(KSysGuard::utils::UnitMegaHertz);
    m_processorClock->setVariantType(QVariant::UInt);
}

NvidiaPlugin::NvidiaPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    const auto sniExecutable = QStandardPaths::findExecutable("nvidia-smi");
    if (sniExecutable.isEmpty()) {
        return;
    }

    auto gpuSystem = new SensorContainer("nvidia", "Nvidia", this);

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

K_PLUGIN_FACTORY(PluginFactory, registerPlugin<NvidiaPlugin>();)

#include "nvidia.moc"
