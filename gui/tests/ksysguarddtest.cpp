#include "ksysguarddtest.h"
#include <QtTest>
#include <Qt>

using namespace KSGRD;
void TestKsysguardd::initTestCase()
{
    QCOMPARE(manager.count(), 0);
    hostConnectionLostSpy = new QSignalSpy(&manager, SIGNAL(hostConnectionLost(const QString &)));
    updateSpy = new QSignalSpy(&manager, SIGNAL(update()));
    bool success = manager.engage("", "", "../../ksysguardd/ksysguardd", -1);
    QVERIFY(success);
    QVERIFY(manager.isConnected(""));
    QCOMPARE(hostConnectionLostSpy->count(), 0);
    QCOMPARE(updateSpy->count(), 0);

    client = new SensorClientTest;
    nextId = 0;
}

void TestKsysguardd::cleanupTestCase()
{
    QCOMPARE(manager.count(), 1);
    manager.disengage("");
    QCOMPARE(manager.count(), 0);
    delete updateSpy;
    delete hostConnectionLostSpy;
    delete client;
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

/** Test the result from ksysguardd for each monitor info and monitor name follows the correct syntax */
void TestKsysguardd::testFormatting_data()
{
    QTest::addColumn<QByteArray>("monitorName");
    QTest::addColumn<QByteArray>("monitorType");
    QTest::addColumn<QByteArray>("monitorInfoName");

    bool success = manager.sendRequest("", "monitors", client, nextId++);
    QVERIFY(success);

    QCOMPARE(hostConnectionLostSpy->count(), 0);
    QCOMPARE(updateSpy->count(), 0);

    int timeout = 50;
    while( !client->haveAnswer && !client->isSensorLost && timeout--)
        QTest::qWait(50);
    QVERIFY(client->haveAnswer);
    QVERIFY(!client->isSensorLost);
    QCOMPARE(client->lastId, nextId-1);

    QList<QByteArray> monitors = client->lastAnswer;
    QVERIFY(!monitors.isEmpty());

    //We now have a list of all the monitors
    foreach(const QByteArray &monitor, monitors) {
        QList<QByteArray> info = monitor.split('\t');
        QCOMPARE(info.count(), 2);
        QByteArray monitorName = info[0];
        QByteArray monitorType = info[1];
        QByteArray monitorInfoName = monitorName;
        monitorInfoName.append('?');
        client->haveAnswer = false;
        QTest::newRow(monitorName) << monitorName << monitorType << monitorInfoName;
    }

}

//Test each monitor for a correctly formatted info and correctly formatted data
void TestKsysguardd::testFormatting()
{
    QFETCH(QByteArray, monitorName);
    QFETCH(QByteArray, monitorType);
    QFETCH(QByteArray, monitorInfoName);

    //Query the monitor for its information
    if(client->haveAnswer || client->isSensorLost)
        return; //Skip rest of tests
   //qDebug() << "Sending request for " << monitorInfoName;
    bool success = manager.sendRequest("", monitorInfoName, client, nextId++);
    QVERIFY(success);
    int timeout = 50;
    while( !client->haveAnswer && !client->isSensorLost && timeout--)
        QTest::qWait(10);
    QVERIFY(client->haveAnswer);
    QVERIFY(!client->isSensorLost);
    QCOMPARE(client->lastId, nextId-1);

    QCOMPARE(updateSpy->count(), 0);
    QCOMPARE(hostConnectionLostSpy->count(), 0);

    //Now check the answer that we got for the monitor information
    if(monitorType == "integer") {
        QCOMPARE(client->lastAnswer.count(), 1);
        QList<QByteArray> answer = client->lastAnswer[0].split('\t');
        QCOMPARE(answer.count(), 4); //Name, minimum value, maximum value, unit
        QVERIFY(!answer[0].isEmpty()); //Name can't be empty
        QVERIFY(!answer[1].isEmpty()); //Minimum value cannot be empty
        QVERIFY(!answer[2].isEmpty()); //Maximum value cannot be empty.  unit can be
        //Make sure minimum and maximum values are numbers (doubles)
        bool isNumber;
        long min = answer[1].toLong(&isNumber); //(note that toLong is in C locale, which is what we want)
        QVERIFY(isNumber);
        long max = answer[2].toLong(&isNumber);
        QVERIFY(isNumber);
        QVERIFY(min <= max);
    } else if(monitorType == "float") {
        QCOMPARE(client->lastAnswer.count(), 1);
        QList<QByteArray> answer = client->lastAnswer[0].split('\t');
        QCOMPARE(answer.count(), 4); //Name, minimum value, maximum value, unit
        QVERIFY(!answer[0].isEmpty()); //Name can't be empty
        QVERIFY(!answer[1].isEmpty()); //Minimum value cannot be empty
        QVERIFY(!answer[2].isEmpty()); //Maximum value cannot be empty.  unit can be
        //Make sure minimum and maximum values are numbers (doubles)
        bool isNumber;
        double min = answer[1].toDouble(&isNumber); //(note that toDouble is in C locale, which is what we want)
        QVERIFY(isNumber);
        double max = answer[2].toDouble(&isNumber);
        QVERIFY(isNumber);
        QVERIFY(min <= max);
    } else if(monitorType == "logfile") {
        QCOMPARE(client->lastAnswer.count(), 1);
        QList<QByteArray> answer = client->lastAnswer[0].split('\t');
        QCOMPARE(answer.count(), 1);
        QCOMPARE(answer[0], QByteArray("LogFile"));
    } else if(monitorType == "listview" || monitorType == "table") {
        //listview is two lines.  The first line is the column headings, the second line is the type of each column
        QCOMPARE(client->lastAnswer.count(), 2);
        QList<QByteArray> columnHeadings = client->lastAnswer[0].split('\t');
        QList<QByteArray> columnTypes = client->lastAnswer[1].split('\t');
        QCOMPARE(columnHeadings.count(), columnTypes.count());
        //column type is well defined
        foreach(const QByteArray &columnType, columnTypes) {
            QCOMPARE(columnType.size(), 1);
            switch(columnType[0]) {
                case 's': //string
                case 'S': //string to translate
                case 'd': //integer
                case 'D': //integer to display localized
                case 'f': //floating point number
                case 'M': //Disk stat - some special case
                    break;
                default:
                    QVERIFY(false);
            }
        }
    } else {
        QVERIFY(false);
    }


    //Now read the actual data for the sensor, and check that it's valid
    client->haveAnswer = false;
    success = manager.sendRequest("", monitorName, client, nextId++);
    QVERIFY(success);
    QTime timer;
    timer.start();
    timeout = 100; //Wait up to 10 seconds
    while( !client->haveAnswer && !client->isSensorLost && timeout--)
        QTest::qWait(100);
    QVERIFY(client->haveAnswer);
    QVERIFY(!client->isSensorLost);
    QCOMPARE(client->lastId, nextId-1);
    if(timer.elapsed() > 200)
        qDebug() << monitorName << "took" << timer.elapsed() << "ms";

    if(monitorType == "integer") {
        QList<QByteArray> answer = client->lastAnswer[0].split('\t');
        QCOMPARE(answer.count(), 1); //Just the number
        QVERIFY(!answer[0].isEmpty()); //Value cannot be empty
        //Make sure the value is valid
        bool isNumber;
        answer[0].toLong(&isNumber); //(note that toLong is in C locale, which is what we want)
        QVERIFY(isNumber);
    } else if(monitorType == "float") {
        QList<QByteArray> answer = client->lastAnswer[0].split('\t');
        QCOMPARE(answer.count(), 1); //Just the number
        QVERIFY(!answer[0].isEmpty()); //Value cannot be empty
        //Make sure the value is valid
        bool isNumber;
        answer[0].toDouble(&isNumber); //(note that toDouble is in C locale, which is what we want)
        QVERIFY(isNumber);
    }

    client->haveAnswer = false;
}
QTEST_MAIN(TestKsysguardd)


