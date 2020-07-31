/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>

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

#include "ksgrdiface.h"

#include "AggregateSensor.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <ksgrd/SensorManager.h>

#include <QEventLoop>
#include <QTimer>

// TODO instantiate multiple instances with args for which host to use

KSGRDIface::KSGRDIface(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    KSGRD::SensorMgr = new KSGRD::SensorManager(this);

    auto registerSubsystem = [this](const QString &id) {
        m_subsystems[id] = new SensorContainer(id, KSGRD::SensorMgr->translateSensorPath(id), this);
    };
    registerSubsystem("acpi");
    registerSubsystem("cpu");
    registerSubsystem("disk");
    registerSubsystem("lmsensors");
    registerSubsystem("mem");
    registerSubsystem("partitions");
    registerSubsystem("uptime");
    registerSubsystem("system");

    KSGRD::SensorMgr->engage(QStringLiteral("localhost"), QLatin1String(""), QStringLiteral("ksysguardd"));
    connect(KSGRD::SensorMgr, &KSGRD::SensorManager::update, this, &KSGRDIface::updateMonitorsList);
    updateMonitorsList();

    // block for sensors to be loaded, but with a guard in case one fails to process
    // a non issue when we port things to be in-process

    QEventLoop e;
    auto t = new QTimer(&e);
    t->setInterval(2000);
    t->start();
    connect(t, &QTimer::timeout, &e, [&e, this]() {
        e.quit();
    });
    connect(this, &KSGRDIface::sensorAdded, &e, [&e, this] {
        if (m_sensors.count() == m_sensorIds.count()) {
            e.quit();
        }
    });
    e.exec();
    addAggregateSensors();
}

KSGRDIface::~KSGRDIface()
{
}

void KSGRDIface::subscribe(const QString &sensorPath)
{
    if (!m_subscribedSensors.contains(sensorPath)) {
        m_subscribedSensors << sensorPath;

        const int index = m_sensorIds.indexOf(sensorPath);

        if (index != -1) {
            m_waitingFor++;
            KSGRD::SensorMgr->sendRequest(QStringLiteral("localhost"), sensorPath, (KSGRD::SensorClient *)this, index);
            KSGRD::SensorMgr->sendRequest(QStringLiteral("localhost"), QStringLiteral("%1?").arg(sensorPath), (KSGRD::SensorClient *)this, -(index + 2));
        }
    }
}

void KSGRDIface::unsubscribe(const QString &sensorPath)
{
    m_subscribedSensors.removeAll(sensorPath);
}

void KSGRDIface::updateMonitorsList()
{
    m_sensorIds.clear();
    KSGRD::SensorMgr->sendRequest(QStringLiteral("localhost"), QStringLiteral("monitors"), (KSGRD::SensorClient *)this, -1);
}

void KSGRDIface::onSensorMetaDataRetrieved(int id, const QList<QByteArray> &answer)
{
    if (answer.isEmpty() || id > m_sensorIds.count()) {
        qDebug() << "sensor info answer was empty, (" << answer.isEmpty() << ") or sensors does not exist to us ";
        return;
    }

    const QStringList newSensorInfo = QString::fromUtf8(answer[0]).split('\t');

    if (newSensorInfo.count() < 4) {
        qDebug() << "bad sensor info, only" << newSensorInfo.count()
                 << "entries, and we were expecting 4. Answer was " << answer;
        return;
    }

    const QString key = m_sensorIds.value(id);

    const QString &sensorName = newSensorInfo[0];
    const QString &min = newSensorInfo[1];
    const QString &max = newSensorInfo[2];
    const QString &unit = newSensorInfo[3];

    int subsystemIndex = key.indexOf('/');
    int propertyIndex = key.lastIndexOf('/');

    const QString subsystemId = key.left(subsystemIndex);
    const QString objectId = key.mid(subsystemIndex + 1, propertyIndex - (subsystemIndex + 1));
    const QString propertyId = key.mid(propertyIndex + 1);

    auto subsystem = m_subsystems[subsystemId];
    if (!subsystem) {
        return;
    }
    auto sensorObject = subsystem->object(objectId);
    if (!sensorObject) {
        auto name = KSGRD::SensorMgr->translateSensorPath(objectId);
        sensorObject = new SensorObject(objectId, name, subsystem);
    }

    auto sensor = m_sensors.value(key, nullptr);
    if (!sensor) {
        sensor = new SensorProperty(propertyId, sensorObject);
    }

    sensor->setName(sensorName);
    sensor->setShortName(shortNameFor(key));
    sensor->setMin(min.toDouble());
    sensor->setMax(max.toDouble());
    sensor->setUnit(unitFromString(unit));

    if (m_sensors.contains(key)) {
        return;
    }

    auto type = m_pendingTypes.take(key);
    if (type == QLatin1String("float")) {
        sensor->setVariantType(QVariant::Double);
    } else {
        sensor->setVariantType(QVariant::Int);
    }
    connect(sensor, &SensorProperty::subscribedChanged, this, [this, sensor](bool subscribed) {
        if (subscribed) {
            subscribe(sensor->path());
        } else {
            unsubscribe(sensor->path());
        }
    });
    m_sensors[key] = sensor;

    emit sensorAdded();
}

void KSGRDIface::onSensorListRetrieved(const QList<QByteArray> &answer)
{
    QSet<QString> sensors;
    int count = 0;

    // A list of data types not currently supported
    static const QStringList excludedTypes = {
        QStringLiteral("logfile"),
        QStringLiteral("listview"),
        QStringLiteral("table")
    };

    for (const QByteArray &sens : answer) {
        const QString sensStr { QString::fromUtf8(sens) };
        const QVector<QStringRef> newSensorInfo = sensStr.splitRef('\t');
        if (newSensorInfo.count() < 2) {
            continue;
        }
        auto type = newSensorInfo.at(1);
        if (excludedTypes.contains(type)) {
            continue;
        }

        const QString newSensor = newSensorInfo[0].toString();
        sensors.insert(newSensor);
        m_sensorIds.append(newSensor);

        m_pendingTypes.insert(newSensor, type.toString());
        //we don't emit sensorAdded yet, instead wait for the meta info to be fetched
        KSGRD::SensorMgr->sendRequest(QStringLiteral("localhost"), QStringLiteral("%1?").arg(newSensor), (KSGRD::SensorClient *)this, -(count + 2));
        ++count;
    }

    // look for removed sensors
    // FIXME?
    foreach (const QString &sensor, m_sensorIds) {
        if (!sensors.contains(sensor)) {
            m_sensorIds.removeOne(sensor);
            m_sensors.remove(sensor);
        }
    }
}

void KSGRDIface::onSensorUpdated(int id, const QList<QByteArray> &answer)
{
    m_waitingFor--;

    if (answer.isEmpty() || id > m_sensorIds.count()) {
        return;
    }

    const QString sensorName = m_sensorIds.at(id);
    if (sensorName.isEmpty()) {
        return;
    }
    QString reply;
    if (!answer.isEmpty()) {
        reply = QString::fromUtf8(answer[0]);
    }
    auto sensor = m_sensors[sensorName];
    if (sensor) {
        if (sensor->info().variantType == QVariant::Double) {
            bool rc;
            double value = reply.toDouble(&rc);
            if (rc) {
                sensor->setValue(value);
            }
        } else if (sensor->info().variantType == QVariant::Int) {
            bool rc;
            int value = reply.toInt(&rc);
            if (rc) {
                sensor->setValue(value);
            }
        } else {
            sensor->setValue(reply);
        }
    }
}

KSysGuard::Unit KSGRDIface::unitFromString(const QString &unitString) const
{
    using namespace KSysGuard;
    static const QHash<QString, Unit> map({ { "%", UnitPercent },
        { "1/s", UnitRate },
        { "°C", UnitCelsius },
        { "dBm", UnitDecibelMilliWatts },
        { "KB", UnitKiloByte },
        { "KB/s", UnitKiloByteRate },
        { "MHz", UnitMegaHertz },
        { "port", UnitNone },
        { "rpm", UnitRpm },
        { "s", UnitTime },
        { "V", UnitVolt } });
    return map.value(unitString, UnitNone);
}

void KSGRDIface::update()
{
    for (int i = 0; i < m_subscribedSensors.count(); i++) {
        auto sensorName = m_subscribedSensors.at(i);

        int index = m_sensorIds.indexOf(sensorName);
        if (index < 0) {
            return;
        }
        m_waitingFor++;
        KSGRD::SensorMgr->sendRequest(QStringLiteral("localhost"), sensorName, (KSGRD::SensorClient *)this, index);
    }
}

void KSGRDIface::sensorLost(int)
{
    m_waitingFor--;
}

void KSGRDIface::answerReceived(int id, const QList<QByteArray> &answer)
{
    //this is the response to "sensorName?"
    if (id < -1) {
        onSensorMetaDataRetrieved(-id - 2, answer);
        return;
    }

    //response to "monitors"
    if (id == -1) {
        onSensorListRetrieved(answer);
        return;
    }
    onSensorUpdated(id, answer);
}

void KSGRDIface::addAggregateSensors()
{
    auto diskAll = new SensorObject("all", i18nc("@title All Disks", "All"), m_subsystems["disk"]);
    auto sensor = new AggregateSensor(diskAll, "read", i18nc("@title", "Disk Read Accesses"));
    sensor->setShortName(i18nc("@title Disk Read Accesses", "Read"));
    // TODO: This regex is not exhaustive as it doesn't consider things that aren't treated as sdX devices.
    //       However, we do not simply want to match disk/* as that would include duplicate devices.
    sensor->setMatchSensors(QRegularExpression("^sd[a-z]+[0-9]+_[^/]*/Rate$"), QStringLiteral("rblk"));
    sensor->setDescription(i18nc("@info", "Read accesses across all disk devices"));

    sensor = new AggregateSensor(diskAll, "write", i18nc("@title", "Disk Write Accesses"));
    sensor->setShortName(i18nc("@title Disk Write Accesses", "Write"));
    // TODO: See above.
    sensor->setMatchSensors(QRegularExpression("^sd[a-z]+[0-9]+_[^/]*/Rate$"), QStringLiteral("wblk"));
    sensor->setDescription(i18nc("@info", "Write accesses across all disk devices"));

    auto memPhysical = m_subsystems["mem"]->object("physical");
    Q_ASSERT(memPhysical);
    if (!memPhysical) {
        return;
    }

    PercentageSensor *appLevel = new PercentageSensor(memPhysical, "applicationlevel", i18nc("@title", "Application Memory Percentage"));
    appLevel->setShortName(i18nc("@title Application Memory Percentage", "Application"));
    appLevel->setBaseSensor(memPhysical->sensor("application"));
    appLevel->setDescription(i18nc("@info", "Percentage of memory taken by applications."));

    PercentageSensor *bufLevel = new PercentageSensor(memPhysical, "buflevel", i18nc("@title", "Buffer Memory Percentage"));
    bufLevel->setShortName(i18nc("@title Buffer Memory Percentage", "Buffer"));
    bufLevel->setBaseSensor(memPhysical->sensor("buf"));
    bufLevel->setDescription(i18nc("@info", "Percentage of memory taken by the buffer."));

    PercentageSensor *cacheLevel = new PercentageSensor(memPhysical, "cachelevel", i18nc("@title", "Cache Memory Percentage"));
    cacheLevel->setShortName(i18nc("@title Cache Memory Percentage", "Cache"));
    cacheLevel->setBaseSensor(memPhysical->sensor("cached"));
    cacheLevel->setDescription(i18nc("@info", "Percentage of memory taken by the cache."));

    PercentageSensor *freeLevel = new PercentageSensor(memPhysical, "freelevel", i18nc("@title", "Free Memory Percentage"));
    freeLevel->setShortName(i18nc("@title Free Memory Percentage", "Free"));
    freeLevel->setBaseSensor(memPhysical->sensor("free"));
    freeLevel->setDescription(i18nc("@info", "Percentage of free memory."));

    PercentageSensor *usedLevel = new PercentageSensor(memPhysical, "usedlevel", i18nc("@title", "Used Memory Percentage"));
    usedLevel->setShortName(i18nc("@title Used Memory Percentage", "Used"));
    usedLevel->setBaseSensor(memPhysical->sensor("used"));
    usedLevel->setDescription(i18nc("@info", "Percentage of used memory."));

    PercentageSensor *availableLevel = new PercentageSensor(memPhysical, "availablelevel", i18nc("@title", "Available Memory Percentage"));
    availableLevel->setShortName(i18nc("@title Available Memory Percentage", "Available"));
    availableLevel->setBaseSensor(memPhysical->sensor("available"));
    availableLevel->setDescription(i18nc("@info", "Percentage of available memory."));

    PercentageSensor *allocatedLevel = new PercentageSensor(memPhysical, "allocatedlevel", i18nc("@title", "Allocated Memory Percentage"));
    allocatedLevel->setShortName(i18nc("@title Allocated Memory Percentage", "Allocated"));
    allocatedLevel->setBaseSensor(memPhysical->sensor("allocated"));
    allocatedLevel->setDescription(i18nc("@info", "Percentage of allocated memory."));
}

QString KSGRDIface::shortNameFor(const QString &key)
{
    // TODO: This is pretty ugly, but it is really hard to add this information to ksysguardd.
    // So for now, we just map sensor ids to short names and return that.

    static QHash<QString, QString> shortNames = {
        { QStringLiteral("cpu/system/TotalLoad"), i18nc("@title Total CPU Usage", "Usage") },
        { QStringLiteral("mem/physical/used"), i18nc("@title Total Memory Usage", "Total Used") },
        { QStringLiteral("mem/physical/cached"), i18nc("@title Cached Memory Usage", "Cached") },
        { QStringLiteral("mem/physical/free"), i18nc("@title Free Memory Amount", "Free") },
        { QStringLiteral("mem/physical/available"), i18nc("@title Available Memory Amount", "Available") },
        { QStringLiteral("mem/physical/application"), i18nc("@title Application Memory Usage", "Application") },
        { QStringLiteral("mem/physical/buf"), i18nc("@title Buffer Memory Usage", "Buffer") },
        { QStringLiteral("cpu/system/processors"), i18nc("@title Number of Processors", "Processors") },
        { QStringLiteral("cpu/system/cores"), i18nc("@title Number of Cores", "Cores") },
    };

    return shortNames.value(key, QString {});
}

K_PLUGIN_CLASS_WITH_JSON(KSGRDIface, "metadata.json")

#include "ksgrdiface.moc"
