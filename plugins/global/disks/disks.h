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

#ifndef DISKS_H
#define DISKS_H

#include <QObject>
#include <QElapsedTimer>

#include "SensorPlugin.h"

namespace Solid {
    class Device;
    class StorageVolume;
}

class VolumeObject;

class DisksPlugin : public SensorPlugin

{
    Q_OBJECT
public:
    DisksPlugin(QObject *parent, const QVariantList &args);
    QString providerName() const override
    {
        return QStringLiteral("disks");
    }
    ~DisksPlugin() override;

    void update() override;


private:
    void addDevice(const Solid::Device &device);
    void addAggregateSensors();

    QHash<QString, VolumeObject*> m_volumesByDevice;
    QElapsedTimer m_elapsedTimer;
};

#endif
