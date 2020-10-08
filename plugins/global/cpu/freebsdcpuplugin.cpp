#include "freebsdcpuplugin.h"

#include "sysctlsensor.h"

#include <algorithm>
#include <vector>

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/sysctl.h>

#include <KLocalizedString>

#include <SensorContainer.h>

template <typename T>
bool readSysctl(const char *name, T *buffer, size_t size = sizeof(T)) {
    return sysctlbyname(name, buffer, &size, nullptr, 0) != -1;
}

FreeBsdCpuObject::FreeBsdCpuObject(const QString &id, const QString &name, SensorContainer *parent)
    : CpuObject(id, name, parent)
{
}

void FreeBsdCpuObject::makeSensors()
{
    BaseCpuObject::makeSensors();

    const QByteArray prefix = QByteArrayLiteral("dev.cpu.") + id().right(1).toLocal8Bit();
    auto freq = new SysctlSensor<int>(QStringLiteral("frequency"), prefix + QByteArrayLiteral(".freq"), this);
    auto temp = new SysctlSensor<int>(QStringLiteral("temperature"), prefix + QByteArrayLiteral(".freq"), this);
    m_sysctlSensors.append({freq, temp});
    m_frequency = freq;
    m_temperature = temp;
}

void FreeBsdCpuObject::initialize()
{
    CpuObject::initialize();

    const QByteArray prefix = QByteArrayLiteral("dev.cpu.") + id().right(1).toLocal8Bit();
    // For min and max frequency we have to parse the values return by freq_levels because only
    // minimum is exposed as a single value
    size_t size;
    const QByteArray levelsName = prefix + QByteArrayLiteral(".freq_levels");
    // calling sysctl with nullptr writes the needed size to size
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
    const QByteArray tjmax = prefix + QByteArrayLiteral(".coretemp.tjmax");
    int maxTemperature;
    // This is only availabel on Intel (using the coretemp driver)
    if (readSysctl(tjmax.constData(), &maxTemperature)) {
        m_temperature->setMax(maxTemperature);
    }
}

void FreeBsdCpuObject::update(long system, long user, long idle)
{
    if (!isSubscribed()) {
        return;
    }

    // No wait usage on FreeBSD
    m_usageComputer.setTicks(system, user, 0, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_usage->setValue(m_usageComputer.totalUsage);

    for (auto sensor : m_sysctlSensors) {
        sensor->update();
    }
}

void FreeBsdAllCpusObject::update(long system, long user, long idle)
{
    // No wait usage on FreeBSD
    m_usageComputer.setTicks(system, user, 0, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_usage->setValue(m_usageComputer.totalUsage);
}


FreeBsdCpuPluginPrivate::FreeBsdCpuPluginPrivate(CpuPlugin* q)
    : CpuPluginPrivate(q)
{
    // The values used here can be found in smp(4)
    int numCpu;
    readSysctl("hw.ncpu", &numCpu);
    for (int i = 0; i < numCpu; ++i) {
        auto cpu = new FreeBsdCpuObject(QStringLiteral("cpu%1").arg(i), i18nc("@title", "CPU %1", i + 1), m_container);
        cpu->initialize();
        m_cpus.push_back(cpu);
    }
    m_allCpus = new FreeBsdAllCpusObject(m_container);
    m_allCpus->initialize();
}

void FreeBsdCpuPluginPrivate::update()
{
    auto isSubscribed = [] (const SensorObject *o) {return o->isSubscribed();};
    const auto objects = m_container->objects();
    if (std::none_of(objects.cbegin(), objects.cend(), isSubscribed)) {
        return;
    }
    auto updateCpu = [] (auto *cpu, long *cp_time){
        cpu->update(cp_time[CP_SYS] + cp_time[CP_INTR], cp_time[CP_USER] + cp_time[CP_NICE], cp_time[CP_IDLE]);
    };
    unsigned int numCores = m_container->objects().count() - 1;
    std::vector<long> cp_times(numCores * CPUSTATES);
    size_t cpTimesSize = sizeof(long) *  cp_times.size();
    if (readSysctl("kern.cp_times", cp_times.data(), cpTimesSize)) {//, &cpTimesSize, nullptr, 0) != -1) {
        for (unsigned int  i = 0; i < numCores; ++i) {
            auto cpu = m_cpus[i];
            updateCpu(cpu, &cp_times[CPUSTATES * i]);
        }
    }
    // update total values
    long cp_time[CPUSTATES];
    if (readSysctl("kern.cp_time", &cp_time)) {
        updateCpu(m_allCpus, cp_time);
    }
}
