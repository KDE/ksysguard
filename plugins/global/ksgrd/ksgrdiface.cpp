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
#include <kpluginfactory.h>
#include <ksgrd/SensorManager.h>

#include <QEventLoop>
#include <QTimer>

// TODO instantiate multiple instances with args for which host to use

KSGRDIface::KSGRDIface(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    auto registerSubsystem = [this](const QString &id) {
        m_subsystems[id] = new SensorContainer(id, id, this); // FIXME name resolve
    };
    registerSubsystem("acpi");
    registerSubsystem("cpu");
    registerSubsystem("disk");
    registerSubsystem("lmsensors");
    registerSubsystem("mem");
    registerSubsystem("network");
    registerSubsystem("partitions");
    registerSubsystem("uptime");

    KSGRD::SensorMgr = new KSGRD::SensorManager(this);
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
    if (Q_UNLIKELY(!subsystem)) {
        qDebug() << "could not find subsystem" << subsystemId;
        return;
    }
    auto sensorObject = subsystem->object(objectId);
    if (!sensorObject) {
        sensorObject = new SensorObject(objectId, objectId, subsystem); // FIXME i18n name for object id?
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

    for (const QByteArray &sens : answer) {
        const QString sensStr { QString::fromUtf8(sens) };
        const QVector<QStringRef> newSensorInfo = sensStr.splitRef('\t');
        if (newSensorInfo.count() < 2) {
            continue;
        }
        auto type = newSensorInfo.at(1);
        if (type == QLatin1String("logfile")) {
            continue; // logfile data type not currently supported
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

KSysGuard::utils::Unit KSGRDIface::unitFromString(const QString &unitString) const
{
    using namespace KSysGuard;
    static const QHash<QString, utils::Unit> map({ { "%", utils::UnitPercent },
        { "1/s", utils::UnitRate },
        { "°C", utils::UnitCelsius },
        { "dBm", utils::UnitDecibelMilliWatts },
        { "KB", utils::UnitKiloByte },
        { "KB/s", utils::UnitKiloByteRate },
        { "MHz", utils::UnitMegaHertz },
        { "port", utils::UnitNone },
        { "rpm", utils::UnitRpm },
        { "s", utils::UnitTime },
        { "V", utils::UnitVolt } });
    return map.value(unitString, utils::UnitNone);
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
    auto networkAll = new SensorObject("all", i18n("All"), m_subsystems["network"]);

    auto sensor = new AggregateSensor(networkAll, "receivedDataRate", i18n("Received Data Rate"));
    sensor->setShortName(i18n("Down"));
    sensor->setMatchSensors(QRegularExpression("[^/]*/receiver"), QStringLiteral("data"));
    sensor->setDescription(i18n("The rate at which data is received on all interfaces."));
    sensor->setUnit(KSysGuard::utils::UnitKiloByteRate);

    sensor = new AggregateSensor(networkAll, "totalReceivedData", i18n("Total Received Data"));
    sensor->setShortName(i18n("Total Down"));
    sensor->setMatchSensors(QRegularExpression("[^/]*/receiver"), QStringLiteral("dataTotal"));
    sensor->setDescription(i18n("The total amount of data received on all interfaces."));
    sensor->setUnit(KSysGuard::utils::UnitKiloByte);

    sensor = new AggregateSensor(networkAll, "sentDataRate", i18n("Sent Data Rate"));
    sensor->setShortName(i18n("Up"));
    sensor->setMatchSensors(QRegularExpression("[^/]*/transmitter"), QStringLiteral("data"));
    sensor->setDescription(i18n("The rate at which data is sent on all interfaces."));
    sensor->setUnit(KSysGuard::utils::UnitKiloByteRate);

    sensor = new AggregateSensor(networkAll, "totalSentData", i18n("Total Sent Data"));
    sensor->setShortName(i18n("Total Up"));
    sensor->setMatchSensors(QRegularExpression("[^/]*/transmitter"), QStringLiteral("dataTotal"));
    sensor->setDescription(i18n("The total amount of data sent on all interfaces."));
    sensor->setUnit(KSysGuard::utils::UnitKiloByte);

    auto diskAll = new SensorObject("all", i18n("all"), m_subsystems["disk"]);
    sensor = new AggregateSensor(diskAll, "read", i18n("Disk Read Accesses"));
    sensor->setShortName(i18n("Read"));
    // TODO: This regex is not exhaustive as it doesn't consider things that aren't treated as sdX devices.
    //       However, we do not simply want to match disk/* as that would include duplicate devices.
    sensor->setMatchSensors(QRegularExpression("^sd[a-z]+[0-9]+_[^/]*/Rate$"), QStringLiteral("rblk"));
    sensor->setDescription(i18n("Read accesses across all disk devices"));

    sensor = new AggregateSensor(diskAll, "write", i18n("Disk Write Accesses"));
    sensor->setShortName(i18n("Write"));
    // TODO: See above.
    sensor->setMatchSensors(QRegularExpression("^sd[a-z]+[0-9]+_[^/]*/Rate$"), QStringLiteral("wblk"));
    sensor->setDescription(i18n("Write accesses across all disk devices"));

    auto memPhysical = m_subsystems["mem"]->object("physical");
    Q_ASSERT(memPhysical);
    if (!memPhysical) {
        return;
    }

    PercentageSensor *appLevel = new PercentageSensor(memPhysical, "applicationlevel", i18n("Application Memory Percentage"));
    appLevel->setShortName(i18n("Application"));
    appLevel->setBaseSensor(memPhysical->sensor("application"));
    appLevel->setDescription(i18n("Percentage of memory taken by applications."));

    PercentageSensor *bufLevel = new PercentageSensor(memPhysical, "buflevel", i18n("Buffer Memory Percentage"));
    bufLevel->setShortName(i18n("Buffer"));
    bufLevel->setBaseSensor(memPhysical->sensor("buf"));
    bufLevel->setDescription(i18n("Percentage of memory taken by the buffer."));

    PercentageSensor *cacheLevel = new PercentageSensor(memPhysical, "cachelevel", i18n("Cache Memory Percentage"));
    cacheLevel->setShortName(i18n("Cache"));
    cacheLevel->setBaseSensor(memPhysical->sensor("cached"));
    cacheLevel->setDescription(i18n("Percentage of memory taken by the cache."));

    PercentageSensor *freeLevel = new PercentageSensor(memPhysical, "freelevel", i18n("Free Memory Percentage"));
    freeLevel->setShortName(i18n("Cache"));
    freeLevel->setBaseSensor(memPhysical->sensor("free"));
    freeLevel->setDescription(i18n("Percentage of free memory."));

    PercentageSensor *usedLevel = new PercentageSensor(memPhysical, "usedlevel", i18n("Used Memory Percentage"));
    usedLevel->setShortName(i18n("Used"));
    usedLevel->setBaseSensor(memPhysical->sensor("used"));
    usedLevel->setDescription(i18n("Percentage of used memory."));

    PercentageSensor *availableLevel = new PercentageSensor(memPhysical, "availablelevel", i18n("Available Memory Percentage"));
    availableLevel->setShortName(i18n("Available"));
    availableLevel->setBaseSensor(memPhysical->sensor("available"));
    availableLevel->setDescription(i18n("Percentage of used memory."));

    PercentageSensor *allocatedLevel = new PercentageSensor(memPhysical, "allocatedlevel", i18n("Allocated Memory Percentage"));
    allocatedLevel->setShortName(i18n("Used"));
    allocatedLevel->setBaseSensor(memPhysical->sensor("allocated"));
    allocatedLevel->setDescription(i18n("Percentage of used memory."));
}

QString KSGRDIface::shortNameFor(const QString &key)
{
    // TODO: This is pretty ugly, but it is really hard to add this information to ksysguardd.
    // So for now, we just map sensor ids to short names and return that.

    static QHash<QString, QString> shortNames = {
        { QStringLiteral("cpu/system/TotalLoad"), i18n("Usage") },
        { QStringLiteral("mem/physical/used"), i18n("Total Used") },
        { QStringLiteral("mem/physical/cached"), i18n("Cached") },
        { QStringLiteral("mem/physical/free"), i18n("Free") },
        { QStringLiteral("mem/physical/available"), i18n("Avalable") },
        { QStringLiteral("mem/physical/application"), i18n("Application") },
        { QStringLiteral("mem/physical/buf"), i18n("Buffer") },
        { QStringLiteral("cpu/system/processors"), i18n("Processors") },
        { QStringLiteral("cpu/system/cores"), i18n("Cores") },
    };

    return shortNames.value(key, QString {});
}

K_PLUGIN_FACTORY(KSGRDPluginFactory, registerPlugin<KSGRDIface>();)

#include "ksgrdiface.moc"
