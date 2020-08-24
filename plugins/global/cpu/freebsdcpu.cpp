#include "freebsdcpu.h"

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/sysctl.h>

#include <KLocalizedString>

#include <SensorContainer.h>

FreeBsdCpuObject::FreeBsdCpuObject(const QString &id, const QString &name, SensorContainer *parent)
    : CpuObject(id, name, parent)
{
    // For min and max frequency we have to parse the values return by freq_levels because only
    // minimum is exposed as a single value
    size_t size;
    const QByteArray levelsName = QByteArrayLiteral("dev.cpu.") + id.right(1).toLocal8Bit() + QByteArrayLiteral(".freq_levels");
    if (sysctlbyname(levelsName, nullptr, &size, nullptr, 0) != -1) {
        QByteArray freqLevels(size, Qt::Uninitialized);
        if (sysctlbyname(levelsName, freqLevels.data(), &size, nullptr, 0) != -1) {
            // The format is a list of pairs "frequency/power", see https://svnweb.freebsd.org/base/head/sys/kern/kern_cpu.c?revision=360464&view=markup#l1019
            const QList<QByteArray> levels = freqLevels.split(' ');
            int min = INT_MAX;
            int max = 0;
            for (const auto &level : levels) {
                const int frequency = level.left(level.indexOf('/')).toInt();
                min = std::min(frequency, min);
                max = std::max(frequency, max);
            }
            // value are already in MHz  see cpufreq(4)
            m_frequency->setMin(min);
            m_frequency->setMax(max);
        }
    }
}

// No wait usage on FreeBSD
void FreeBsdCpuObject::update(long system, long user, long idle)
{
    long totalTicks = system + user + idle;
    long totalDiff = totalTicks - m_totalTicks;
    auto percentage =  [totalDiff] (long tickDiff) {
        // according to the documentation some counters can go backwards in some circumstances
        return tickDiff > 0 ? 100.0 *  tickDiff / totalDiff : 0;
    };
    
    m_system->setValue(percentage(system - m_systemTicks));
    m_user->setValue(percentage(user - m_userTicks));
    m_usage->setValue(percentage((system + user) - (m_systemTicks + m_userTicks)));
    
    m_systemTicks = system;
    m_userTicks = user;
    m_totalTicks = totalTicks;
    
    int frequency;
    size_t frequencySize = sizeof(frequency);
    const QByteArray name = QByteArrayLiteral("dev.cpu.") + id().right(1).toLocal8Bit() + QByteArrayLiteral(".freq");
    if (sysctlbyname(name.constData(), &frequency, &frequencySize, nullptr, 0) != -1) {
        // value is already in MHz
        m_frequency->setValue(frequency);
    }
}


FreeBsdCpuPluginPrivate::FreeBsdCpuPluginPrivate(CpuPlugin* q)
    : CpuPluginPrivate(q)
{
    // The values used here can be found in smp(4)
    unsigned int numCpu;
    size_t unsignedIntSize = sizeof(numCpu);
    sysctl((int[]){CTL_HW, HW_NCPU}, 2, &numCpu, &unsignedIntSize, nullptr, 0);
    for (unsigned int i = 0; i < numCpu; ++i) {
        new FreeBsdCpuObject(QStringLiteral("cpu%1").arg(i), i18nc("@title", "CPU %1", i + 1), m_container);
    }
    // TODO parse smp sysctl kern.sched.topology_spec xml to be able show names like "CPU 1 Core 2"

    // Add total usage sensors
    auto total = new FreeBsdCpuObject(QStringLiteral("all"), i18nc("@title", "All"), m_container);
    auto cpuCount = new SensorProperty(QStringLiteral("cpuCount"), i18nc("@title", "Number of CPUs"), numCpu, total);
    cpuCount->setShortName(i18nc("@title, Short fort 'Number of CPUs'", "CPUs"));
    cpuCount->setDescription(i18nc("@info", "Number of physical CPUs installed in the system"));

    auto coreCount = new SensorProperty(QStringLiteral("coreCount"), i18nc("@title", "Number of Cores"), numCpu, total);
    coreCount->setShortName(i18nc("@title, Short fort 'Number of Cores'", "Cores"));
    coreCount->setDescription(i18nc("@info", "Number of CPU cores across all physical CPUS"));
}

void FreeBsdCpuPluginPrivate::update()
{
    auto updateCpu = [] (FreeBsdCpuObject *cpu, long *cp_time){
        cpu->update(cp_time[CP_SYS + CP_INTR], cp_time[CP_USER] + cp_time[CP_NICE], cp_time[CP_IDLE]);
    };
    unsigned int numCores = m_container->objects().count() - 1;
    std::vector<long> cp_times(numCores * CPUSTATES);
    size_t cpTimesSize = sizeof(long) * numCores * CPUSTATES;
    if (sysctlbyname("kern.cp_times", &cp_times[0], &cpTimesSize, nullptr, 0) != -1) {
        for (unsigned int  i = 0; i < numCores; ++i) {
            auto cpu = static_cast<FreeBsdCpuObject*>(m_container->object(QStringLiteral("cpu%1").arg(i)));
            updateCpu(cpu, &cp_times[CPUSTATES * i]);
        }
    }
    // update total values
    long cp_time[CPUSTATES];
    size_t cpTimeSize = sizeof(long) * CPUSTATES;
    if (sysctlbyname("kern.cp_times", cp_time, &cpTimeSize, nullptr, 0) != -1) {
        updateCpu(static_cast<FreeBsdCpuObject*>(m_container->object(QStringLiteral("all"))), cp_time);
    }
}
