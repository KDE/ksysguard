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

#include "memory.h"

#include "backend.h"
#if defined Q_OS_LINUX
#include "linuxbackend.h"
#elif defined Q_OS_FREEBSD
#include "freebsdbackend.h"
#endif

#include <SensorContainer.h>

#include <KLocalizedString>
#include <KPluginFactory>

#include <QtGlobal>

MemoryPlugin::MemoryPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    auto container = new SensorContainer(QStringLiteral("memory"), i18nc("@title", "Memory"), this);
#if defined Q_OS_LINUX
    m_backend = std::make_unique<LinuxMemoryBackend>(container);
#elif defined Q_OS_FREEBSD
    m_backend = std::make_unique<FreeBsdMemoryBackend>(container);
#endif
    m_backend->initSensors();
}

MemoryPlugin::~MemoryPlugin() = default;

void MemoryPlugin::update()
{
    m_backend->update();
}

K_PLUGIN_CLASS_WITH_JSON(MemoryPlugin, "metadata.json")
#include "memory.moc"
