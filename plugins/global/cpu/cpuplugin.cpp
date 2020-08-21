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

#include "cpuplugin.h"
#include "cpuplugin_p.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <SensorContainer.h>

#include "linuxcpu.h"

CpuPluginPrivate::CpuPluginPrivate(CpuPlugin *q)
    : m_container(new SensorContainer(QStringLiteral("cpu"), i18n("CPUs"), q))
{
}

CpuPlugin::CpuPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
#ifdef Q_OS_LINUX
    , d(new LinuxCpuPluginPrivate(this))
#else
    , d(new CpuPluginPrivate(this))
#endif
{
}

CpuPlugin::~CpuPlugin() = default;

void CpuPlugin::update()
{
    d->update();
}

K_PLUGIN_CLASS_WITH_JSON(CpuPlugin, "metadata.json")

#include "cpuplugin.moc"
