/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
