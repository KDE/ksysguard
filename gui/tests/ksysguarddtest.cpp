#include "ksysguarddtest.h"
#include <QtTest>
#include <Qt>

void TestKsysguardd::init()
{
    QCOMPARE(manager.count(), 0);
    bool success = manager.engage("", "", "../../ksysguardd/ksysguardd", -1);
    QVERIFY(success);
    QVERIFY(manager.isConnected(""));
}

void TestKsysguardd::cleanup()
{
    QCOMPARE(manager.count(), 1);
    manager.disengage("");
    QCOMPARE(manager.count(), 0);
}

void TestKsysguardd::testSetup()
{
    QCOMPARE(manager.count(), 1);
    QString shell;
    QString command;
    int port;
    bool success = manager.hostInfo("", shell, command, port );
    QCOMPARE(success, true);
    QCOMPARE(shell, QString(""));
    QCOMPARE(command, QString("../../ksysguardd/ksysguardd"));
    QCOMPARE(port, -1);

    success = manager.hostInfo("nonexistant host", shell, command, port );
    QCOMPARE(success, false);
}

void TestKsysguardd::testFormatting()
{
}
QTEST_MAIN(TestKsysguardd)


