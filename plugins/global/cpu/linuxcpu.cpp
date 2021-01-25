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

#include "linuxcpu.h"

#include <QFile>

#ifdef HAVE_SENSORS
#include <sensors/sensors.h>
#endif

static double readCpuFreq(const QString &cpuId, const QString &attribute, bool &ok)
{
    ok = false;
    QFile file(QStringLiteral("/sys/devices/system/cpu/%1/cpufreq/").arg(cpuId)  + attribute);
    bool open = file.open(QIODevice::ReadOnly);
    if (open) {
        // Remove trailing new line
        return file.readAll().chopped(1).toUInt(&ok) / 1000.0; // CPUFreq reports values in kHZ
    }
    return 0;
}

TemperatureSensor::TemperatureSensor(const QString& id, SensorObject* parent)
    : SensorProperty(id, parent)
    , m_sensorChipName{nullptr}
    , m_temperatureSubfeature{-1}
{
}

void TemperatureSensor::setFeature(const sensors_chip_name *const chipName, const sensors_feature *const feature)
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
            setMax(value);
            break;
        }
    }
#endif
}

void TemperatureSensor::update()
{
#ifdef HAVE_SENSORS
    if (m_sensorChipName && m_temperatureSubfeature != -1) {
        double value;
        if (sensors_get_value(m_sensorChipName, m_temperatureSubfeature, &value) == 0) {
            setValue(value);
        }
    }
#endif
}

LinuxCpuObject::LinuxCpuObject(const QString &id, const QString &name, SensorContainer *parent)
    : CpuObject(id, name, parent)

{
}

void LinuxCpuObject::initialize(double initialFrequency)
{
    CpuObject::initialize();
    m_frequency->setValue(initialFrequency);
    bool ok;
    const double max = readCpuFreq(id(), "cpuinfo_max_freq", ok);
    if (ok) {
        m_frequency->setMax(max);
    }
    const double min = readCpuFreq(id(), "cpuinfo_min_freq", ok);
    if (ok) {
        m_frequency->setMin(min);
    }
}

void LinuxCpuObject::makeSensors()
{
    BaseCpuObject::makeSensors();
    m_frequency = new SensorProperty(QStringLiteral("frequency"), this);
    m_temperatureSensor = new TemperatureSensor(QStringLiteral("temperature"), this);
    m_temperature = m_temperatureSensor;
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
    m_temperatureSensor->update();
}

void LinuxAllCpusObject::update(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle) {
    m_usageComputer.setTicks(system, user, wait, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_wait->setValue(m_usageComputer.waitUsage);
    m_usage->setValue(m_usageComputer.totalUsage);
}

TemperatureSensor *LinuxCpuObject::temperatureSensor()
{
    return m_temperatureSensor;
}


