/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LinuxBackend.h"

#include <QDebug>
#include <QDir>
#include <KLocalizedString>

#include <libudev.h>

#include "LinuxAmdGpu.h"
#include "LinuxNvidiaGpu.h"

// Vendor ID strings, as used in sysfs
static const char *amdVendor = "0x1002";
static const char *intelVendor = "0x8086";
static const char *nvidiaVendor = "0x10de";

LinuxBackend::LinuxBackend(QObject *parent)
    : GpuBackend(parent)
{
}

void LinuxBackend::start()
{
    if (!m_udev) {
        m_udev = udev_new();
    }

    auto enumerate = udev_enumerate_new(m_udev);

    udev_enumerate_add_match_property(enumerate, "ID_PCI_CLASS_FROM_DATABASE", "Display controller");
    udev_enumerate_scan_devices(enumerate);

    int gpuCounter = 0;

    auto devices = udev_enumerate_get_list_entry(enumerate);
    for (auto entry = devices; entry; entry = udev_list_entry_get_next(entry)) {
        auto path = udev_list_entry_get_name(entry);
        auto device = udev_device_new_from_syspath(m_udev, path);

        auto vendor = QByteArray(udev_device_get_sysattr_value(device, "vendor"));

        auto gpuId = QStringLiteral("gpu%1").arg(gpuCounter);
        auto gpuName = i18nc("@title %1 is GPU number", "GPU %1", gpuCounter + 1);

        GpuDevice *gpu = nullptr;
        if (vendor == amdVendor) {
            gpu = new LinuxAmdGpu{gpuId, gpuName, device};
        } else if (vendor == nvidiaVendor) {
            gpu = new LinuxNvidiaGpu{gpuCounter, gpuId, gpuName};
        } else {
            qDebug() << "Found unsupported GPU:" << path;
            udev_device_unref(device);
            continue;
        }

        gpuCounter++;
        gpu->initialize();
        m_devices.append(gpu);
        Q_EMIT deviceAdded(gpu);

        udev_device_unref(device);
    }

    udev_enumerate_unref(enumerate);
}

void LinuxBackend::stop()
{
    qDeleteAll(m_devices);
    udev_unref(m_udev);
}

void LinuxBackend::update()
{
    for (auto device : qAsConst(m_devices)) {
        device->update();
    }
}
