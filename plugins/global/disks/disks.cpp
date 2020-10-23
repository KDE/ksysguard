/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>
    Copyright (c) 2020 Harald Sitter <sitter@kde.org>

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

#include "disks.h"

#ifdef Q_OS_FREEBSD
#include <devstat.h>
#include <libgeom.h>
#endif

#include <QCoreApplication>
#include <QUrl>

#include <KIO/FileSystemFreeSpaceJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/Predicate>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

#include <AggregateSensor.h>
#include <SensorContainer.h>
#include <SensorObject.h>

class VolumeObject : public SensorObject {
public:
    VolumeObject(const Solid::Device &device, SensorContainer *parent);
    void update();
    void setBytes(quint64 read, quint64 written, qint64 elapsedTime);

    const QString udi;
private:
    static QString idHelper(const Solid::Device &device);

    SensorProperty *m_name;
    SensorProperty *m_total;
    SensorProperty *m_used;
    SensorProperty *m_free;
    SensorProperty *m_readRate;
    SensorProperty *m_writeRate;
    quint64 m_bytesRead;
    quint64 m_bytesWritten;
    const QString m_mountPoint;
};

QString VolumeObject::idHelper(const Solid::Device &device)
{
    auto volume = device.as<Solid::StorageVolume>();
    return volume->uuid().isEmpty() ? volume->label() : volume->uuid();
}


VolumeObject::VolumeObject(const Solid::Device &device, SensorContainer* parent)
    : SensorObject(idHelper(device), device.displayName(),  parent)
    , udi(device.udi())
    , m_mountPoint(device.as<Solid::StorageAccess>()->filePath())
{
    auto volume = device.as<Solid::StorageVolume>();

    m_name = new SensorProperty("name", i18nc("@title", "Name"), device.displayName(), this);
    m_name->setShortName(i18nc("@title", "Name"));
    m_name->setVariantType(QVariant::String);

    m_total = new SensorProperty("total", i18nc("@title", "Total Space"), volume->size(), this);
    m_total->setPrefix(name());
    m_total->setShortName(i18nc("@title Short for 'Total Space'", "Total"));
    m_total->setUnit(KSysGuard::UnitByte);
    m_total->setVariantType(QVariant::ULongLong);

    m_used = new SensorProperty("used", i18nc("@title", "Used Space"), this);
    m_used->setPrefix(name());
    m_used->setShortName(i18nc("@title Short for 'Used Space'", "Used"));
    m_used->setUnit(KSysGuard::UnitByte);
    m_used->setVariantType(QVariant::ULongLong);
    m_used->setMax(volume->size());

    m_free = new SensorProperty("free", i18nc("@title", "Free Space"), this);
    m_free->setPrefix(name());
    m_free->setShortName(i18nc("@title Short for 'Free Space'", "Free"));
    m_free->setUnit(KSysGuard::UnitByte);
    m_free->setVariantType(QVariant::ULongLong);
    m_free->setMax(volume->size());

    m_readRate = new SensorProperty("read", i18nc("@title", "Read Rate"), this);
    m_readRate->setPrefix(name());
    m_readRate->setShortName(i18nc("@title Short for 'Read Rate'", "Read"));
    m_readRate->setUnit(KSysGuard::UnitByteRate);
    m_readRate->setVariantType(QVariant::Double);

    m_writeRate = new SensorProperty("write", i18nc("@title", "Write Rate"), this);
    m_writeRate->setPrefix(name());
    m_writeRate->setShortName(i18nc("@title Short for 'Write Rate'", "Write"));
    m_writeRate->setUnit(KSysGuard::UnitByteRate);
    m_writeRate->setVariantType(QVariant::Double);

    auto usedPercent = new PercentageSensor(this, "usedPercent", i18nc("@title", "Percentage Used"));
    usedPercent->setPrefix(name());
    usedPercent->setBaseSensor(m_used);

    auto freePercent = new PercentageSensor(this, "freePercent", i18nc("@title", "Percentage Free"));
    freePercent->setPrefix(name());
    freePercent->setBaseSensor(m_free);
}

void VolumeObject::update()
{
    auto job = KIO::fileSystemFreeSpace(QUrl::fromLocalFile(m_mountPoint));
    connect(job, &KIO::FileSystemFreeSpaceJob::result, this, [this] (KJob *job, KIO::filesize_t size, KIO::filesize_t available) {
        if (!job->error()) {
            m_total->setValue(size);
            m_free->setValue(available);
            m_free->setMax(size);
            m_used->setValue(size - available);
            m_used->setMax(size);
        }
    });
}

void VolumeObject::setBytes(quint64 read, quint64 written, qint64 elapsed)
{
    if (elapsed != 0) {
        double seconds = elapsed / 1000.0;
        m_readRate->setValue((read - m_bytesRead) / seconds);
        m_writeRate->setValue((written - m_bytesWritten) / seconds);
    }
    m_bytesRead = read;
    m_bytesWritten = written;
}

DisksPlugin::DisksPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    auto container = new SensorContainer("disk", i18n("Disks"), this);
    auto storageAccesses = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);
    for (const auto &storageAccess : storageAccesses) {
       addDevice(storageAccess);
    }
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, [this] (const QString &udi) {
            addDevice(Solid::Device(udi));
    });
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, [this, container] (const QString &udi) {
        Solid::Device device(udi);
        if (device.isDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
            auto it = std::find_if(m_volumesByDevice.begin(), m_volumesByDevice.end(), [&udi] (VolumeObject *volume) {
                return volume->udi == udi;
            });
            if (it != m_volumesByDevice.end()) {
                m_volumesByDevice.erase(it);
                container->removeObject(*it);
            }
        }
    });
    addAggregateSensors();
#ifdef Q_OS_FREEBSD
    geom_stats_open();
#endif
}

DisksPlugin::~DisksPlugin()
{
#ifdef Q_OS_FREEBSD
    geom_stats_close();
#endif
}

void DisksPlugin::addDevice(const Solid::Device& device)
{
    auto container = containers()[0];
    const auto volume = device.as<Solid::StorageVolume>();
    auto access = device.as<Solid::StorageAccess>();
    if (!access || !volume || volume->isIgnored()) {
        return;
    }

    Solid::Device drive = device;
    while (drive.isValid() && !drive.is<Solid::StorageDrive>()) {
        drive = drive.parent();
    }
    if (!drive.isValid()) {
        return;
    }

    if (drive.as<Solid::StorageDrive>()->driveType() != Solid::StorageDrive::HardDisk) {
        return;
    }

    if (access->filePath() != QString()) {
        auto block = device.as<Solid::Block>();
        m_volumesByDevice.insert(block->device(), new VolumeObject(device, container));
    }
    connect(access, &Solid::StorageAccess::accessibilityChanged, this, [this, container] (bool accessible, const QString &udi) {
        if (accessible) {
            Solid::Device device(udi);
            auto block = device.as<Solid::Block>();
            m_volumesByDevice.insert(block->device(), new VolumeObject(device, container));
        } else {
            auto it = std::find_if(m_volumesByDevice.begin(), m_volumesByDevice.end(), [&udi] (VolumeObject *disk) {
                return disk->udi == udi;
            });
            if (it != m_volumesByDevice.end()) {
                m_volumesByDevice.erase(it);
                container->removeObject(*it);
            }
        }
    });
}


void DisksPlugin::addAggregateSensors()
{
    auto container = containers()[0];
    auto allDisks = new SensorObject("all", i18nc("@title", "All Disks"), container);

    auto total = new AggregateSensor(allDisks, "total", i18nc("@title", "Total Space"));
    total->setShortName(i18nc("@title Short for 'Total Space'", "Total"));
    total->setUnit(KSysGuard::UnitByte);
    total->setVariantType(QVariant::ULongLong);
    total->setMatchSensors(QRegularExpression("^(?!all).*$"), "total");

    auto free = new AggregateSensor(allDisks, "free", i18nc("@title", "Free Space"));
    free->setShortName(i18nc("@title Short for 'Free Space'", "Free"));
    free->setUnit(KSysGuard::UnitByte);
    free->setVariantType(QVariant::ULongLong);
    free->setMax(total->value().toULongLong());
    free->setMatchSensors(QRegularExpression("^(?!all).*$"), "free");

    auto used = new AggregateSensor(allDisks, "used", i18nc("@title", "Used Space"));
    used->setShortName(i18nc("@title Short for 'Used Space'", "Used"));
    used->setUnit(KSysGuard::UnitByte);
    used->setVariantType(QVariant::ULongLong);
    used->setMax(total->value().toULongLong());
    used->setMatchSensors(QRegularExpression("^(?!all).*$"), "used");

    auto readRate = new AggregateSensor(allDisks, "readRate", i18nc("@title", "Read Rate"));
    readRate->setShortName(i18nc("@title Short for 'Read Rate'", "Read"));
    readRate->setUnit(KSysGuard::UnitByteRate);
    readRate->setVariantType(QVariant::Double);
    readRate->setMatchSensors(QRegularExpression("^(?!all).*$"), "read");

    auto writeRate = new AggregateSensor(allDisks, "writeRate", i18nc("@title", "Write Rate"));
    writeRate->setShortName(i18nc("@title Short for 'Write Rate'", "Write"));
    writeRate->setUnit(KSysGuard::UnitByteRate);
    writeRate->setVariantType(QVariant::Double);
    writeRate->setMatchSensors(QRegularExpression("^(?!all).*$"), "write");

    auto freePercent = new PercentageSensor(allDisks, "freePercent", i18nc("@title", "Percentage Free"));
    freePercent->setShortName(i18nc("@title, Short for `Percentage Free", "Free"));
    freePercent->setBaseSensor(free);

    auto usedPercent = new PercentageSensor(allDisks, "usedPercent", i18nc("@title", "Percentage Used"));
    usedPercent->setShortName(i18nc("@title, Short for `Percentage Used", "Used"));
    usedPercent->setBaseSensor(used);

    connect(total, &SensorProperty::valueChanged, this, [total, free, used] () {
        free->setMax(total->value().toULongLong());
        used->setMax(total->value().toULongLong());
    });
}

void DisksPlugin::update()
{
    for (auto volume : m_volumesByDevice) {
        volume->update();
    }
    qint64 elapsed = 0;
    if (m_elapsedTimer.isValid()) {
        elapsed = m_elapsedTimer.restart();
    } else {
        m_elapsedTimer.start();
    }
#if defined Q_OS_LINUX
    QFile diskstats("/proc/diskstats");
    if (!diskstats.exists()) {
        return;
    }
    diskstats.open(QIODevice::ReadOnly | QIODevice::Text);
    /* procfs-diskstats (See https://www.kernel.org/doc/Documentation/ABI/testing/procfs-diskstats)
    The /proc/diskstats file displays the I/O statistics
    of block devices. Each line contains the following 14
    fields:
    1 - major number
    2 - minor mumber
    3 - device name
    4 - reads completed successfully
    5 - reads merged
    6 - sectors read
    7 - time spent reading (ms)
    8 - writes completed
    9 - writes merged
    10 - sectors written
    [...]
    */
    for (QByteArray line = diskstats.readLine(); !line.isNull(); line = diskstats.readLine()) {
        QList<QByteArray> fields = line.simplified().split(' ');
        const QString device = QStringLiteral("/dev/%1").arg(QString::fromLatin1(fields[2]));
        if (m_volumesByDevice.contains(device)) {
            // A sector as reported in diskstats is 512 Bytes, see https://stackoverflow.com/a/38136179
            m_volumesByDevice[device]->setBytes(fields[6].toULongLong() * 512, fields[10].toULongLong() * 512, elapsed);
        }
    }
#elif defined Q_OS_FREEBSD
    std::unique_ptr<void, decltype(&geom_stats_snapshot_free)> stats(geom_stats_snapshot_get(), geom_stats_snapshot_free);
    gmesh mesh;
    geom_gettree(&mesh);
    while (devstat *dstat = geom_stats_snapshot_next(stats.get())) {
        gident *id = geom_lookupid(&mesh, dstat->id);
        if (id && id->lg_what == gident::ISPROVIDER) {
            auto provider = static_cast<gprovider*>(id->lg_ptr);
            const QString device = QStringLiteral("/dev/%1").arg(QString::fromLatin1(provider->lg_name));
            if (m_volumesByDevice.contains(device)) {
                uint64_t bytesRead, bytesWritten;
                devstat_compute_statistics(dstat, nullptr, 0, DSM_TOTAL_BYTES_READ, &bytesRead, DSM_TOTAL_BYTES_WRITE, &bytesWritten, DSM_NONE);
                m_volumesByDevice[device]->setBytes(bytesRead, bytesWritten, elapsed);
            }
        }
    }
    geom_deletetree(&mesh);
#endif
}

K_PLUGIN_CLASS_WITH_JSON(DisksPlugin, "metadata.json")
#include "disks.moc"
