/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

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

#include "cpu.h"

#include <cstdio>
#include <unistd.h>

#include <QFile>
#include <QElapsedTimer>
#include <QUrl>

#include <KLocalizedString>
#include <KPluginFactory>

#include <AggregateSensor.h>
#include <SensorContainer.h>
#include <SensorObject.h>

class CpuObject : public SensorObject {
public:
    CpuObject(const QString &id, const QString &name, SensorContainer *parent);

    void setTicks(unsigned long long system, unsigned long long  user, unsigned long long wait, unsigned long long idle);
private: 
    SensorProperty *m_usage;
    SensorProperty *m_system;
    SensorProperty *m_user;
    SensorProperty *m_wait;
    // TODO temperature frequency
    unsigned long long m_totalTicks;
    unsigned long long m_systemTicks;
    unsigned long long m_userTicks;
    unsigned long long m_waitTicks;
};

CpuObject::CpuObject(const QString &id, const QString &name, SensorContainer *parent)
    : SensorObject(id, name, parent)
{
    auto n = new SensorProperty(QStringLiteral("name"), i18nc("@title", "Name"), name, this);
    n->setVariantType(QVariant::String);

    m_usage = new SensorProperty(QStringLiteral("usage"), i18nc("@title", "Total Usage"), this);
    m_usage->setShortName(i18nc("@title, Short for 'Total Usage'", "Usage"));
    m_usage->setUnit(KSysGuard::UnitPercent);
    m_usage->setVariantType(QVariant::Double);
    m_usage->setMax(100);

    m_system = new SensorProperty(QStringLiteral("system"), i18nc("@title", "System Usage"), this);
    m_system->setShortName(i18nc("@title, Short for 'System Usage'", "System"));
    m_system->setUnit(KSysGuard::UnitPercent);
    m_system->setVariantType(QVariant::Double);
    m_system->setMax(100);

    m_user = new SensorProperty(QStringLiteral("user"), i18nc("@title", "User Usage"), this);
    m_user->setShortName(i18nc("@title, Short for 'User Usage'", "User"));
    m_user->setUnit(KSysGuard::UnitPercent);
    m_user->setVariantType(QVariant::Double);
    m_user->setMax(100);

    m_wait = new SensorProperty(QStringLiteral("wait"), i18nc("@title", "Wait"), this);
    m_wait->setUnit(KSysGuard::UnitPercent);
    m_wait->setVariantType(QVariant::Double);
    m_wait->setMax(100);
}

void CpuObject::setTicks(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle)
{
    unsigned long long totalTicks = system + user + wait + idle;
    unsigned long long totalDiff = totalTicks - m_totalTicks;
    auto percentage =  [totalDiff] (unsigned long long tickDiff) {
        // according to the documentation some counters can go backwards in some circumstances
        return tickDiff > 0 ? 100.0 *  tickDiff / totalDiff : 0;
    };

    m_system->setValue(percentage(system - m_systemTicks));
    m_user->setValue(percentage(user - m_userTicks));
    m_wait->setValue(percentage(wait - m_waitTicks));
    m_usage->setValue(percentage((system + user + wait) - (m_systemTicks + m_userTicks + m_waitTicks)));

    m_totalTicks = totalTicks;
    m_systemTicks = system;
    m_userTicks = user;
    m_waitTicks = wait;
}



CpuPlugin::CpuPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
#ifdef Q_OS_LINUX
    , m_container(new SensorContainer(QStringLiteral("cpus"), i18n("CPUs"), this))
{
    parseCpuInfo();
}
#else 
{
}
#endif

void CpuPlugin::parseCpuInfo()
{
    QFile cpuinfo("/proc/cpuinfo");
    cpuinfo.open(QIODevice::ReadOnly);

    QHash<int, int> numCores;
    for (QByteArray line = cpuinfo.readLine(); !line.isEmpty(); line = cpuinfo.readLine()) {
        int physicalId = -1;
        int processor = -1;
        // Processors are divided by empty lines
         for (; (physicalId == -1 && processor == -1) || line != "\n";  line = cpuinfo.readLine()) {
            // we are interested in processor number as identifier for /proc/stat, physical id (the
            // cpu the core belongs to) and the number of the core. However with hyperthreading
            // multiple entries will have the same combination of physical id and core id. So we just
            // count up the core number.
            const int delim = line.indexOf(":");
            const QByteArray field = line.left(delim).trimmed();
            const QByteArray value = line.mid(delim + 1).trimmed();
            if (field == "processor") {
                processor = value.toInt();
            } else if (field == "physical id") {
                physicalId = value.toInt();
            }
         }
        const QString name = i18nc("@title", "CPU %1 Core %2").arg(physicalId + 1).arg(++numCores[physicalId]);
        new CpuObject(QStringLiteral("cpu%1").arg(processor), name, m_container);
    }

    const int cores = m_container->objects().size();
    // Add total usage sensors
    auto total = new CpuObject(QStringLiteral("all"), i18nc("@title", "All"), m_container);
    auto cpuCount = new SensorProperty(QStringLiteral("cpuCount"), i18nc("@title", "Number of CPUs"), numCores.size(), total);
    cpuCount->setShortName(i18nc("@title, Short fort 'Number of CPUs'", "CPUs"));
    cpuCount->setDescription(i18nc("@info", "Number of physical CPUs installed in the system"));

    auto coreCount = new SensorProperty(QStringLiteral("coreCount"), i18nc("@title", "Number of Cores"), cores, total);
    coreCount->setShortName(i18nc("@title, Short fort 'Number of Cores'", "Cores"));
    coreCount->setDescription(i18nc("@info", "Number of CPU cores across all physical CPUS"));
}

void CpuPlugin::update()
{
    // Parse /proc/stat to get usage values. The format is described at
    // https://www.kernel.org/doc/html/latest/filesystems/proc.html#miscellaneous-kernel-statistics-in-proc-stat
    QFile stat("/proc/stat");
    stat.open(QIODevice::ReadOnly);
    qint64 elapsed = 0;
    if (m_elapsedTimer.isValid()) {
        elapsed = m_elapsedTimer.restart();
    } else {
        m_elapsedTimer.start();
    }
    QByteArray line;
    for (QByteArray line = stat.readLine(); !line.isNull(); line = stat.readLine()) {
        CpuObject *cpu;
        // Total values
        if (line.startsWith("cpu ")) {
            cpu = static_cast<CpuObject*>(m_container->object(QStringLiteral("all")));
        } else if (line.startsWith("cpu")) {
            cpu = static_cast<CpuObject*>(m_container->object(line.left(line.indexOf(' '))));
        } else  {
            continue;
        }
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        std::sscanf(line.data(), "%*s %llu %llu %llu %llu %llu %llu %llu %llu%llu %llu",
            &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
        cpu->setTicks(system + irq + softirq, user + nice , iowait + steal, idle);
    }
}


K_PLUGIN_CLASS_WITH_JSON(CpuPlugin, "metadata.json")
#include "cpu.moc"
