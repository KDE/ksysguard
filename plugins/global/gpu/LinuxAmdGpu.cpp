/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LinuxAmdGpu.h"

#include <libudev.h>

#include <QDir>

#include "SysFsSensor.h"

int ppTableGetMax(const QByteArray &table)
{
    const auto lines = table.split('\n');
    auto line = lines.last();
    return std::atoi(line.mid(line.indexOf(':') + 1).data());
}

int ppTableGetCurrent(const QByteArray &table)
{
    const auto lines = table.split('\n');

    int current = 0;
    for (auto line : lines) {
        if (!line.contains('*')) {
            continue;
        }

        current = std::atoi(line.mid(line.indexOf(':') + 1));
    }

    return current;
}

LinuxAmdGpu::LinuxAmdGpu(const QString& id, const QString& name, udev_device *device)
    : GpuDevice(id, name)
    , m_device(device)
{
    udev_device_ref(m_device);
}

LinuxAmdGpu::~LinuxAmdGpu()
{
    udev_device_unref(m_device);
}

void LinuxAmdGpu::initialize()
{
    // Find temprature sensor paths.
    // Temperature sensors are exposed in the "hwmon" subdirectory of the
    // device, but in a subdirectory with a number appended that differs per
    // device. So search through the hwmon directory for the files that we want.
    QDir hwmonDir(QString::fromLocal8Bit(udev_device_get_syspath(m_device)) % QStringLiteral("/hwmon"));
    const auto entries = hwmonDir.entryList({QStringLiteral("hwmon*")});
    for (auto entry : entries) {
        QString inputPath = entry % QStringLiteral("/temp1_input");
        QString critPath = entry % QStringLiteral("/temp1_crit");
        if (hwmonDir.exists(inputPath) && hwmonDir.exists(critPath)) {
            m_coreTemperatureCurrentPath = QStringLiteral("hwmon/") % inputPath;
            m_coreTemperatureMaxPath = QStringLiteral("hwmon/") % critPath;
            break;
        }
    }

    GpuDevice::initialize();

    m_nameProperty->setValue(QString::fromLocal8Bit(udev_device_get_property_value(m_device, "ID_MODEL_FROM_DATABASE")));

    auto result = udev_device_get_sysattr_value(m_device, "mem_info_vram_total");
    if (result) {
        m_totalVramProperty->setValue(std::atoll(result));
    }

    m_coreFrequencyProperty->setMax(ppTableGetMax(udev_device_get_sysattr_value(m_device, "pp_dpm_sclk")));
    m_memoryFrequencyProperty->setMax(ppTableGetMax(udev_device_get_sysattr_value(m_device, "pp_dpm_mclk")));

    result = udev_device_get_sysattr_value(m_device, m_coreTemperatureMaxPath.toLocal8Bit());
    if (result) {
        m_temperatureProperty->setMax(std::atoi(result) / 1000);
    }
}

void LinuxAmdGpu::update()
{
    for (auto sensor : m_sysFsSensors) {
        sensor->update();
    }
}

void LinuxAmdGpu::makeSensors()
{
    auto devicePath = QString::fromLocal8Bit(udev_device_get_syspath(m_device));

    m_nameProperty = new SensorProperty(QStringLiteral("name"), this);
    m_totalVramProperty = new SensorProperty(QStringLiteral("totalVram"),  this);

    auto sensor = new SysFsSensor(QStringLiteral("usage"), devicePath % QStringLiteral("/gpu_busy_percent"), this);
    m_usageProperty = sensor;
    m_sysFsSensors << sensor;

    sensor = new SysFsSensor(QStringLiteral("usedVram"), devicePath % QStringLiteral("/mem_info_vram_used"), this);
    m_usedVramProperty = sensor;
    m_sysFsSensors << sensor;

    sensor = new SysFsSensor(QStringLiteral("coreFrequency"), devicePath % QStringLiteral("/pp_dpm_sclk"), this);
    sensor->setConvertFunction([](const QByteArray &input) {
        return ppTableGetCurrent(input);
    });
    m_coreFrequencyProperty = sensor;
    m_sysFsSensors << sensor;

    sensor = new SysFsSensor(QStringLiteral("memoryFrequency"), devicePath % QStringLiteral("/pp_dpm_mclk"), this);
    sensor->setConvertFunction([](const QByteArray &input) {
        return ppTableGetCurrent(input);
    });
    m_memoryFrequencyProperty = sensor;
    m_sysFsSensors << sensor;

    sensor = new SysFsSensor(QStringLiteral("temperature"), devicePath % QLatin1Char('/') % m_coreTemperatureCurrentPath, this);
    sensor->setConvertFunction([](const QByteArray &input) {
        auto result = std::atoi(input);
        return result / 1000;
    });
    m_temperatureProperty = sensor;
    m_sysFsSensors << sensor;
}
