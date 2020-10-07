#include "linuxcpu.h"

#include <QFile>

#ifdef HAVE_SENSORS
#include <sensors/sensors.h>
#endif

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
    if (!isSubscribed()) {
        return;
    }

    // First update usages
    m_usageComputer.setTicks(system, user, wait, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_wait->setValue(m_usageComputer.waitUsage);
    m_usage->setValue(m_usageComputer.totalUsage);

    // Second try to get current frequency
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

void LinuxAllCpusObject::update(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle) {
    m_usageComputer.setTicks(system, user, wait, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_wait->setValue(m_usageComputer.waitUsage);
    m_usage->setValue(m_usageComputer.totalUsage);
}


