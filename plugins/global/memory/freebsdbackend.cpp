/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3 of
    the license or (at your option) at any later version that is
    accepted by the membership of KDE e.V. (or its successor
    approved by the membership of KDE e.V.), which shall act as a
    proxy as defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
*/

#include "freebsdbackend.h"

#include <SensorObject.h>
#include <SensorProperty.h>
#include <SysctlSensor.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>

template <typename T>
bool readSysctl(const char *name, T *buffer, size_t size = sizeof(T))
{
    return sysctlbyname(name, buffer, &size, nullptr, 0) != -1;
}

FreeBsdMemoryBackend::FreeBsdMemoryBackend(SensorContainer* container)
    : MemoryBackend(container)
    , m_pageSize(getpagesize())
    , m_kd(nullptr)
{
    char errorBuffer[_POSIX2_LINE_MAX];
    // Get a descriptor for accesing kernel memory which we need for querying swap info. /dev/null
    // indicates that we do not want to directly access kernel memory.
    m_kd = kvm_openfiles(nullptr, "/dev/null", nullptr, O_RDONLY, errorBuffer);
    if (!m_kd) {
        qWarning() << errorBuffer;
    }
}

void FreeBsdMemoryBackend::makeSensors()
{
    auto totalSensor = new SysctlSensor<unsigned long long>(QStringLiteral("total"), "hw.physmem",  m_physicalObject);
    m_total = totalSensor;
    m_sysctlSensors.append(totalSensor);

    m_used = new SensorProperty(QStringLiteral("used"), m_physicalObject);
    m_application = new SensorProperty(QStringLiteral("application"), m_physicalObject);

    auto capturedPagesToBytes = [this] (auto pages) {return pagesToBytes(pages);};

    auto freeSensor = new SysctlSensor<uint32_t>(QStringLiteral("free"), "vm.stats.vm.v_free_count", m_physicalObject);
    freeSensor->setConversionFunction(capturedPagesToBytes);
    m_free = freeSensor;
    m_sysctlSensors.push_back(freeSensor);

    auto cacheSensor = new SysctlSensor<uint32_t>(QStringLiteral("cache"),"vm.v_cache_count", m_physicalObject);
    cacheSensor->setConversionFunction(capturedPagesToBytes);
    m_cache = cacheSensor;
    m_sysctlSensors.push_back(cacheSensor);

    auto bufferSensor = new SysctlSensor<uint64_t>(QStringLiteral("buffer"), "vfs.bufspace", m_physicalObject);
    m_buffer = bufferSensor;
    m_sysctlSensors.push_back(bufferSensor);

    m_swapTotal = new SensorProperty(QStringLiteral("total"), m_swapObject);
    m_swapUsed = new SensorProperty(QStringLiteral("used"), m_swapObject);
    m_swapFree = new SensorProperty(QStringLiteral("free"), m_swapObject);

}

unsigned long long FreeBsdMemoryBackend::pagesToBytes(uint32_t pages)
{
    return m_pageSize * pages;
}

void FreeBsdMemoryBackend::update()
{

    kvm_swap swapInfo;
    // Calling it with just one swapInfo gets us the totals, passing 0 as the last argument is mandatory
    if (m_swapObject->isSubscribed() && m_kd && (kvm_getswapinfo(m_kd, &swapInfo, 1, 0) != -1)) {
        m_swapTotal->setValue(pagesToBytes(swapInfo.ksw_total));
        m_swapUsed->setValue(pagesToBytes(swapInfo.ksw_used));
        m_swapFree->setValue(pagesToBytes(swapInfo.ksw_total - swapInfo.ksw_used));
    }

    for (const auto sysctlSensor : m_sysctlSensors) {
        sysctlSensor->update();
    }

    m_used->setValue(m_total->value().toULongLong() - m_free->value().toULongLong());

    uint32_t activePages = 0;
    uint32_t inactivePages = 0;
    if (readSysctl("vm.stats.vm.v_active_count", &activePages) && readSysctl("vm.stats.vm.v_inactive_count", &inactivePages)) {
        m_application->setValue(pagesToBytes(activePages + inactivePages));
    }
}


