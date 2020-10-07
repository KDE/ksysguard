/*
    Copyright (c) 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

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

#include "GpuPlugin.h"

#include <KPluginFactory>
#include <KLocalizedString>

#include <SensorContainer.h>

#include "GpuDevice.h"
#include "LinuxBackend.h"
#include "AllGpus.h"

class GpuPlugin::Private
{
public:
    std::unique_ptr<SensorContainer> container;
    std::unique_ptr<GpuBackend> backend;

    AllGpus *allGpus = nullptr;
};

GpuPlugin::GpuPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
    , d(std::make_unique<Private>())
{
    d->container = std::make_unique<SensorContainer>(QStringLiteral("gpu"), i18nc("@title", "GPU"), this);

    d->allGpus = new AllGpus(d->container.get());

#ifdef Q_OS_LINUX
    d->backend = std::make_unique<LinuxBackend>();
#endif

    if (d->backend) {
        connect(d->backend.get(), &GpuBackend::deviceAdded, this, [this](GpuDevice* device) {
            d->container->addObject(device);
        });
        connect(d->backend.get(), &GpuBackend::deviceRemoved, this, [this](GpuDevice* device) {
            d->container->removeObject(device);
        });
        d->backend->start();
    }
}

GpuPlugin::~GpuPlugin()
{
    d->container.reset();
    if (d->backend) {
        d->backend->stop();
    }
}

void GpuPlugin::update()
{
    if (d->backend) {
        d->backend->update();
    }
}

K_PLUGIN_CLASS_WITH_JSON(GpuPlugin, "metadata.json")

#include "GpuPlugin.moc"
