#include "linuxcpu.h"

#include <QFile>

#include <KLocalizedString>

#ifdef HAVE_SENSORS
#include <sensors/sensors.h>
#endif

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
    , m_sensorChipName{nullptr}
    , m_temperatureSubfeature{-1}
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

void LinuxCpuObject::setTemperatureSensor(const sensors_chip_name * const chipName, const sensors_feature * const feature)
{
#ifdef HAVE_SENSORS
    m_sensorChipName = chipName;
    const sensors_subfeature * const temperature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_TEMP_INPUT);
    if (temperature) {
        m_temperatureSubfeature = temperature->number;
    }
    // Typically temp_emergency > temp_crit > temp_max, but not every processor has them  
    // see https://www.kernel.org/doc/html/latest/hwmon/sysfs-interface.html
    double value;
    for (auto subfeatureType : {SENSORS_SUBFEATURE_TEMP_EMERGENCY, SENSORS_SUBFEATURE_TEMP_CRIT, SENSORS_SUBFEATURE_TEMP_MAX}) {
        const sensors_subfeature * const subfeature = sensors_get_subfeature(chipName, feature, subfeatureType);
        if (subfeature && sensors_get_value(chipName, subfeature->number, &value) == 0) {
            m_temperature->setMax(value);
            break;
        }
    }
#endif
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

    // Third update temperature
#ifdef HAVE_SENSORS
    if (m_sensorChipName && m_temperatureSubfeature != -1) {
        double value;
        if (sensors_get_value(m_sensorChipName, m_temperatureSubfeature, &value) == 0) {
            m_temperature->setValue(value);
        }
    }
#endif
}

LinuxCpuPluginPrivate::LinuxCpuPluginPrivate(CpuPlugin *q)
    : CpuPluginPrivate(q)
{
    // Parse /proc/cpuinfo
    QFile cpuinfo("/proc/cpuinfo");
    cpuinfo.open(QIODevice::ReadOnly);

    QHash<int, int> numCores;
    for (QByteArray line = cpuinfo.readLine(); !line.isEmpty(); line = cpuinfo.readLine()) {
        unsigned int processor, physicalId, coreId;
        double frequency = 0;
        // Processors are divided by empty lines
        for (; line != "\n";  line = cpuinfo.readLine()) {
            // we are interested in processor number as identifier for /proc/stat, physical id (the
            // cpu the core belongs to) and the number of the core. However with hyperthreading
            // multiple entries will have the same combination of physical id and core id. So we just
            // count up the core number. For mapping temperature both ids are still needed nonetheless.
            const int delim = line.indexOf(":");
            const QByteArray field = line.left(delim).trimmed();
            const QByteArray value = line.mid(delim + 1).trimmed();
            if (field == "processor") {
                processor = value.toInt();
            } else if (field == "physical id") {
                physicalId = value.toInt();
            } else if (field == "core id") {
                coreId = value.toInt();
            } else if (field == "cpu MHz") {
                frequency = value.toDouble();
            }
        }
        const QString name = i18nc("@title", "CPU %1 Core %2", physicalId + 1, ++numCores[physicalId]);
        auto cpu = new LinuxCpuObject(QStringLiteral("cpu%1").arg(processor), name, m_container, frequency);
        m_cpusBySystemIds.insert({physicalId, coreId}, cpu);
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
        auto values = line.split(' ');
        unsigned long long user = values[1].toULongLong();
        unsigned long long nice = values[2].toULongLong();
        unsigned long long system = values[3].toULongLong();
        unsigned long long idle = values[4].toULongLong();
        unsigned long long iowait = values[5].toULongLong();
        unsigned long long irq = values[6].toULongLong();
        unsigned long long softirq = values[7].toULongLong();
        unsigned long long steal = values[8].toULongLong();
        cpu->update(system + irq + softirq, user + nice , iowait + steal, idle);
    }
}


void LinuxCpuPluginPrivate::addSensors()
{
#ifdef HAVE_SENSORS
    sensors_init(nullptr);
    int number = 0;
    while (const sensors_chip_name * const chipName = sensors_get_detected_chips(nullptr, &number)) {
        char name[100];
        sensors_snprintf_chip_name(name, 100, chipName);
        if (qstrcmp(chipName->prefix, "coretemp") == 0) {
            addSensorsIntel(chipName);
        } else if (qstrcmp(chipName->prefix, "k10temp") == 0) {
            addSensorsAmd(chipName);
        }
    }
#endif
}

// Documentation: https://www.kernel.org/doc/html/latest/hwmon/coretemp.html
void LinuxCpuPluginPrivate::addSensorsIntel(const sensors_chip_name * const chipName)
{
#ifdef HAVE_SENSORS
    int featureNumber = 0;
    QHash<unsigned int,  sensors_feature const *> coreFeatures;
    int physicalId = -1;
    while (sensors_feature const * feature = sensors_get_features(chipName, &featureNumber)) {
        if (feature->type != SENSORS_FEATURE_TEMP) {
            continue;
        }
        char * sensorLabel = sensors_get_label(chipName, feature);
        unsigned int coreId;
        // First try to see if it's a core temperature because we should have more of those
        if (std::sscanf(sensorLabel, "Core %d", &coreId) != 0) {
            coreFeatures.insert(coreId, feature);
        } else {
            std::sscanf(sensorLabel, "Package id %d", &physicalId);
        }
        free(sensorLabel);
    }
    if (physicalId == -1) {
        return;
    }
    for (auto feature = coreFeatures.cbegin(); feature != coreFeatures.cend(); ++feature) {
        if (m_cpusBySystemIds.contains({physicalId, feature.key()})) {
            // When the cpu has hyperthreading we display multiple cores for each physical core.
            // Naturally they share the same temperature sensor and have the same coreId.
            auto cpu_range = m_cpusBySystemIds.equal_range({physicalId, feature.key()});
            for (auto cpu_it = cpu_range.first; cpu_it != cpu_range.second; ++cpu_it) {
                (*cpu_it)->setTemperatureSensor(chipName, feature.value());
            }
        }
    }
#endif
}

void LinuxCpuPluginPrivate::addSensorsAmd(const sensors_chip_name * const chipName)
{
    // All Processors should have the Tctl pseudo temperature as temp1. Newer ones have the real die
    // temperature Tdie as temp2. Some of those have temperatures for each core complex die (CCD) as
    // temp3-6 or temp3-10 depending on the number of CCDS.
    // https://www.kernel.org/doc/html/latest/hwmon/k10temp.html
    int featureNumber = 0;
    sensors_feature const * tctl = nullptr;
    sensors_feature const * tdie = nullptr;
    sensors_feature const * tccd[8] = {nullptr};
    while (sensors_feature const * feature = sensors_get_features(chipName, &featureNumber)) {
        const QByteArray name (feature->name);
        if (feature->type != SENSORS_FEATURE_TEMP || !name.startsWith("temp")) {
            continue;
        }
        // For temps 1 and 2 we can't just go by the number because in  kernels older than 5.7 they
        // are the wrong way around, so we have to compare labels.
        // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b02c6857389da66b09e447103bdb247ccd182456
        char * label = sensors_get_label(chipName, feature);
        if (qstrcmp(label, "Tctl") == 0) {
            tctl = feature;
        }
        else if (qstrcmp(label, "Tdie") == 0) {
            tdie = feature;
        } else {
            tccd[name.mid(4).toUInt()] = feature;
        }
        free(label);
    }
    // TODO How to map CCD temperatures to cores?

    auto setSingleSensor = [this, chipName] (const sensors_feature * const feature) {
        for (auto &cpu : m_cpusBySystemIds) {
            cpu->setTemperatureSensor(chipName, feature);
        }
    };
    if (tdie) {
        setSingleSensor(tdie);
    } else if (tctl) {
        setSingleSensor(tctl);
    }
}

