/********************************************************************
Copyright 2020 David Edmundson <davidedmundson@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include <QtTest>

#include "../ksysguarddaemon.h"

#include <SensorContainer.h>
#include <SensorObject.h>
#include <SensorPlugin.h>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusMetaType>

#include "kstatsiface.h"
#include <QDebug>

class TestPlugin : public SensorPlugin
{
public:
    TestPlugin(QObject *parent)
        : SensorPlugin(parent, {})
    {
        m_testContainer = new SensorContainer("testContainer", "Test Container", this);
        m_testObject = new SensorObject("testObject", "Test Object", m_testContainer);
        m_property1 = new SensorProperty("property1", m_testObject);
        m_property1->setMin(0);
        m_property1->setMax(100);
        m_property1->setShortName("Some Sensor 1");
        m_property1->setName("Some Sensor Name 1");

        m_property2 = new SensorProperty("property2", m_testObject);
    }
    QString providerName() const override
    {
        return "testPlugin";
    }
    void update() override
    {
        m_updateCount++;
    }
    SensorContainer *m_testContainer;
    SensorObject *m_testObject;
    SensorProperty *m_property1;
    SensorProperty *m_property2;
    int m_updateCount = 0;
};

class KStatsTest : public KSysGuardDaemon
{
    Q_OBJECT
public:
    KStatsTest();

protected:
    void loadProviders() override;
private Q_SLOTS:
    void init();
    void findById();
    void update();
    void subscription();
    void changes();
    void dbusApi();

private:
    TestPlugin *m_testPlugin = nullptr;
};

KStatsTest::KStatsTest()
{
    qDBusRegisterMetaType<SensorData>();
    qDBusRegisterMetaType<SensorInfo>();
    qDBusRegisterMetaType<SensorDataList>();
    qDBusRegisterMetaType<QHash<QString, SensorInfo>>();
    qDBusRegisterMetaType<QStringList>();
}

void KStatsTest::loadProviders()
{
    m_testPlugin = new TestPlugin(this);
    registerProvider(m_testPlugin);
}

void KStatsTest::init()
{
    KSysGuardDaemon::init();
}

void KStatsTest::findById()
{
    QVERIFY(findSensor("testContainer/testObject/property1"));
    QVERIFY(findSensor("testContainer/testObject/property2"));
    QVERIFY(!findSensor("testContainer/asdfasdfasfs/property1"));
}

void KStatsTest::update()
{
    QCOMPARE(m_testPlugin->m_updateCount, 0);
    sendFrame();
    QCOMPARE(m_testPlugin->m_updateCount, 1);
}

void KStatsTest::subscription()
{
    QSignalSpy property1Subscribed(m_testPlugin->m_property1, &SensorProperty::subscribedChanged);
    QSignalSpy property2Subscribed(m_testPlugin->m_property2, &SensorProperty::subscribedChanged);
    QSignalSpy objectSubscribed(m_testPlugin->m_testObject, &SensorObject::subscribedChanged);

    m_testPlugin->m_property1->subscribe();
    QCOMPARE(property1Subscribed.count(), 1);
    QCOMPARE(objectSubscribed.count(), 1);

    m_testPlugin->m_property1->subscribe();
    QCOMPARE(property1Subscribed.count(), 1);
    QCOMPARE(objectSubscribed.count(), 1);

    m_testPlugin->m_property2->subscribe();
    QCOMPARE(objectSubscribed.count(), 1);

    m_testPlugin->m_property1->unsubscribe();
    QCOMPARE(property1Subscribed.count(), 1);
    m_testPlugin->m_property1->unsubscribe();
    QCOMPARE(property1Subscribed.count(), 2);
}

void KStatsTest::changes()
{
    QSignalSpy property1Changed(m_testPlugin->m_property1, &SensorProperty::valueChanged);
    m_testPlugin->m_property1->setValue(14);
    QCOMPARE(property1Changed.count(), 1);
    QCOMPARE(m_testPlugin->m_property1->value(), QVariant(14));
}

void KStatsTest::dbusApi()
{
    OrgKdeKSysGuardDaemonInterface iface("org.kde.ksystemstats",
        "/",
        QDBusConnection::sessionBus(),
        this);
    // list all objects
    auto pendingSensors = iface.allSensors();
    pendingSensors.waitForFinished();
    auto sensors = pendingSensors.value();
    QVERIFY(sensors.count() == 2);

    // test metadata
    QCOMPARE(sensors["testContainer/testObject/property1"].name, "Some Sensor Name 1");

    // query value
    m_testPlugin->m_property1->setValue(100);

    auto pendingValues = iface.sensorData({ "testContainer/testObject/property1" });
    pendingValues.waitForFinished();
    QCOMPARE(pendingValues.value().first().sensorProperty, "testContainer/testObject/property1");
    QCOMPARE(pendingValues.value().first().payload.toInt(), 100);

    // change updates
    QSignalSpy changesSpy(&iface, &OrgKdeKSysGuardDaemonInterface::newSensorData);

    iface.subscribe({ "testContainer/testObject/property1" });

    sendFrame();
    // a frame with no changes, does nothing
    QVERIFY(!changesSpy.wait(20));

    m_testPlugin->m_property1->setValue(101);
    // an update does nothing till it gets a frame, in order to batch
    QVERIFY(!changesSpy.wait(20));
    sendFrame();

    QVERIFY(changesSpy.wait(20));
    QCOMPARE(changesSpy.first().first().value<SensorDataList>().first().sensorProperty, "testContainer/testObject/property1");
    QCOMPARE(changesSpy.first().first().value<SensorDataList>().first().payload, QVariant(101));

    // we're not subscribed to property 2 so if that updates we should not get anything
    m_testPlugin->m_property2->setValue(102);
    sendFrame();
    QVERIFY(!changesSpy.wait(20));
}

QTEST_GUILESS_MAIN(KStatsTest)

#include "main.moc"
