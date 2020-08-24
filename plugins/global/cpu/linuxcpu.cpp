#include "linuxcpu.h"

#include <QFile>

#include <KLocalizedString>

#include <sensors/sensors.h>

#include <SensorContainer.h>

static double readCpuFreq(const QString &cpuId, const QString &attribute, bool &ok)
{
    QFile file(QStringLiteral("/sys/devices/system/cpu/%1/cpufreq/").arg(cpuId)  + attribute);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        // Remove trailing new line
        return file.readAll().chopped(1).toUInt(&ok) / 1000.0; // CPUFreq reports values in kHZ
    }
    return 0;
}

LinuxCpuObject::LinuxCpuObject(const QString &id, const QString &name, SensorContainer *parent, double frequency)
    : CpuObject(id, name, parent)
{
    if (m_frequency) {
        m_frequency->setValue(frequency);
        bool ok;
        const double max = readCpuFreq(id, "cpuinfo_max_freq", ok);
        if (ok) {
            m_frequency->setMax(max);
        }
        const double min = readCpuFreq(id, "cpuinfo_min_freq", ok);
        if (ok) {
            m_frequency->setMin(min);
        }
    }
}

void LinuxCpuObject::update(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle)
{
    // First calculate usages
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

    // Second update frequencies
    if (!m_frequency) {
        return;
    }
    bool ok = false;
    // First try cpuinfo_cur_freq, it is the frequency the hardware runs at (https://www.kernel.org/doc/html/latest/admin-guide/pm/cpufreq.html)
    int frequency = readCpuFreq(id(), "cpuinfo_cur_freq", ok);
    if (!ok) {
        frequency = readCpuFreq(id(), "scaling_cur_freq", ok);
    }
    if (ok) {
        m_frequency->setValue(frequency);
    } 
    // FIXME Should we fall back to reading /proc/cpuinfo again when the above fails? Could have the 
    // frequency value changed even if the cpu apparently doesn't use CPUFreq?
}

LinuxCpuPluginPrivate::LinuxCpuPluginPrivate(CpuPlugin *q)
    : CpuPluginPrivate(q)
{
    // Parse /proc/cpuinfo
    QFile cpuinfo("/proc/cpuinfo");
    cpuinfo.open(QIODevice::ReadOnly);

    QHash<int, int> numCores;
    for (QByteArray line = cpuinfo.readLine(); !line.isEmpty(); line = cpuinfo.readLine()) {
        int physicalId = -1;
        int processor = -1;
        double frequency = -1;
        // Processors are divided by empty lines
        for (; (physicalId == -1 && processor == -1 && frequency == -1) || line != "\n";  line = cpuinfo.readLine()) {
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
            } else if (field == "cpu MHz") {
                frequency = value.toDouble();
            }
        }
        const QString name = i18nc("@title", "CPU %1 Core %2").arg(physicalId + 1).arg(++numCores[physicalId]);
        new LinuxCpuObject(QStringLiteral("cpu%1").arg(processor), name, m_container, frequency);
    }
    const int cores = m_container->objects().size();
    // Add total usage sensors
    auto total = new LinuxCpuObject(QStringLiteral("all"), i18nc("@title", "All"), m_container, 0);
    auto cpuCount = new SensorProperty(QStringLiteral("cpuCount"), i18nc("@title", "Number of CPUs"), numCores.size(), total);
    cpuCount->setShortName(i18nc("@title, Short fort 'Number of CPUs'", "CPUs"));
    cpuCount->setDescription(i18nc("@info", "Number of physical CPUs installed in the system"));

    auto coreCount = new SensorProperty(QStringLiteral("coreCount"), i18nc("@title", "Number of Cores"), cores, total);
    coreCount->setShortName(i18nc("@title, Short fort 'Number of Cores'", "Cores"));
    coreCount->setDescription(i18nc("@info", "Number of CPU cores across all physical CPUS"));

    addSensors();
}


void LinuxCpuPluginPrivate::addSensors()
{
#ifdef HAVE_SENSORS
    sensors_init(nullptr);
    int number = 0;
    while (sensors_chip_name const *chipName = sensors_get_detected_chips(nullptr, &number)) {
        char name[100];
        sensors_snprintf_chip_name(name, 100, chipName);
        qDebug() << name << chipName->prefix << "Bus(" << chipName->bus.nr << chipName->bus.type << ")" << chipName->addr << chipName->path;
        int featureNumber = 0;
        while (sensors_feature const * feature = sensors_get_features(chipName, &featureNumber)) {
            qDebug() << '\t' << feature->number << sensors_get_label(chipName, feature) << feature->name << feature->type;
            int subfeatureNumber = 0;
            while (sensors_subfeature const * subfeature = sensors_get_all_subfeatures(chipName, feature, &subfeatureNumber)) {
                qDebug() << "\t\t" << subfeature->number << subfeature->name << subfeature->type << subfeature->flags << subfeature->mapping;
            }
        }
    }
#endif
}

void LinuxCpuPluginPrivate::update()
{
    // Parse /proc/stat to get usage values. The format is described at
    // https://www.kernel.org/doc/html/latest/filesystems/proc.html#miscellaneous-kernel-statistics-in-proc-stat
    QFile stat("/proc/stat");
    stat.open(QIODevice::ReadOnly);
    QByteArray line;
    for (QByteArray line = stat.readLine(); !line.isNull(); line = stat.readLine()) {
        LinuxCpuObject *cpu;
        // Total values
        if (line.startsWith("cpu ")) {
            cpu = static_cast<LinuxCpuObject*>(m_container->object(QStringLiteral("all")));
        } else if (line.startsWith("cpu")) {
            cpu = static_cast<LinuxCpuObject*>(m_container->object(line.left(line.indexOf(' '))));
        } else  {
            continue;
        }
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        std::sscanf(line.data(), "%*s %llu %llu %llu %llu %llu %llu %llu %llu%llu %llu",
            &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
        cpu->update(system + irq + softirq, user + nice , iowait + steal, idle);
    }
}
