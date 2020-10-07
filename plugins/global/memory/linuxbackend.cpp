
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

#include "linuxbackend.h"

#include <SensorObject.h>
#include <SensorProperty.h>

#include <QFile>

LinuxMemoryBackend::LinuxMemoryBackend(SensorContainer *container)
    : MemoryBackend(container)
{
}

void LinuxMemoryBackend::update()
{
    if (!m_physicalObject->isSubscribed() || ! m_swapObject->isSubscribed()) {
        return;
    }

    QFile meminfo(QStringLiteral("/proc/meminfo"));
    meminfo.open(QIODevice::ReadOnly);
    // The format of the file is as follows:
    // Fieldname:[whitespace]value kB
    // A description of the fields can be found at 
    // https://www.kernel.org/doc/html/latest/filesystems/proc.html#meminfo
    unsigned long long total, free, available, buffer, cache, slab, swapTotal, swapFree;
    for (QByteArray line = meminfo.readLine(); !line.isEmpty(); line = meminfo.readLine()) {
        int colonIndex = line.indexOf(':');
        const auto fields = line.simplified().split(' ');

        const QByteArray name = line.left(colonIndex);
        const unsigned long long value = std::strtoull(line.mid(colonIndex + 1), nullptr, 10) * 1024;
        if (name == "MemTotal") {
            total = value;
        } else if (name == "MemFree") {
            free = value;
        } else if (name == "MemAvailable") {
            available = value;
        } else if (name == "Buffers") {
            buffer = value;
        } else if (name == "Cached") {
            cache = value;
        } else if (name == "Slab") {
            slab = value;
        } else if (name == "SwapTotal") {
            swapTotal = value;
        } else if (name == "SwapFree") {
            swapFree = value;
        }
    }
    m_total->setValue(total);
    m_used->setValue(total - available);
    m_free->setValue(available);
    m_application->setValue(total - free - cache - buffer - slab);
    m_cache->setValue(cache + slab);
    m_buffer->setValue(buffer);
    m_swapTotal->setValue(swapTotal);
    m_swapUsed->setValue(swapTotal - swapFree);
    m_swapFree->setValue(swapFree);
}

