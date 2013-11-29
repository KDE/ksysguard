#include "ksysguarddtest.h"
#include <QtTest>
#include <Qt>

KSGRD::SensorAgent *agent;

Q_DECLARE_METATYPE(KSGRD::SensorAgent *)

using namespace KSGRD;
void TestKsysguardd::initTestCase()
{
    qRegisterMetaType<KSGRD::SensorAgent*>();
    QCOMPARE(manager.count(), 0);
    hostConnectionLostSpy = new QSignalSpy(&manager, SIGNAL(hostConnectionLost(QString)));
    updateSpy = new QSignalSpy(&manager, SIGNAL(update()));
    hostAddedSpy = new QSignalSpy(&manager, SIGNAL(hostAdded(KSGRD::SensorAgent*,QString)));
    bool success = manager.engage("", "", "../../ksysguardd/ksysguardd", -1);
    QCOMPARE(hostAddedSpy->count(), 1);
    QVERIFY(success);
    QVERIFY(manager.isConnected(""));
    QCOMPARE(hostConnectionLostSpy->count(), 0);
    QCOMPARE(updateSpy->count(), 0);
    client = new SensorClientTest;
    nextId = 0;
}
void TestKsysguardd::init()
{
}
void TestKsysguardd::cleanup()
{
}

void TestKsysguardd::cleanupTestCase()
{
    QCOMPARE(hostAddedSpy->count(), 1);
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

    int id = nextId++;
    bool success = manager.sendRequest("", "monitors", client, id);
    QVERIFY(success);

    QCOMPARE(hostConnectionLostSpy->count(), 0);
    QCOMPARE(updateSpy->count(), 0);

    int timeout = 300; // Wait up to 30 seconds
    while( !client->haveAnswer && !client->isSensorLost && timeout--)
        QTest::qWait(100);
    QVERIFY(client->haveAnswer);
    QVERIFY(!client->isSensorLost);
    QVERIFY(!client->answers[id].isSensorLost);
    QCOMPARE(client->answers[id].id, id);

    QList<QByteArray> monitors = client->answers[id].answer;
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
        QTest::newRow(monitorName.constData()) << monitorName << monitorType << monitorInfoName;
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
    int id = nextId++;
    bool success = manager.sendRequest("", monitorInfoName, client, id);
    QVERIFY(success);
    int timeout = 50;
    while( !client->haveAnswer && !client->isSensorLost && timeout--)
        QTest::qWait(10);
    QVERIFY(client->haveAnswer);
    QVERIFY(!client->isSensorLost);
    QVERIFY(!client->answers[id].isSensorLost);
    QCOMPARE(client->answers[id].id, id);

    QCOMPARE(updateSpy->count(), 0);
    QCOMPARE(hostConnectionLostSpy->count(), 0);

    QList<QByteArray> columnHeadings;
    QList<QByteArray> columnTypes;

    //Now check the answer that we got for the monitor information
    if(monitorType == "integer") {
        QCOMPARE(client->answers[id].answer.count(), 1);
        QList<QByteArray> answer = client->answers[id].answer[0].split('\t');
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
        QCOMPARE(client->answers[id].answer.count(), 1);
        QList<QByteArray> answer = client->answers[id].answer[0].split('\t');
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
        QCOMPARE(client->answers[id].answer.count(), 1);
        QList<QByteArray> answer = client->answers[id].answer[0].split('\t');
        QCOMPARE(answer.count(), 1);
        QCOMPARE(answer[0], QByteArray("LogFile"));
    } else if(monitorType == "listview" || monitorType == "table") {
        //listview is two lines.  The first line is the column headings, the second line is the type of each column
        QCOMPARE(client->answers[id].answer.count(), 2);
        columnHeadings = client->answers[id].answer[0].split('\t');
        columnTypes = client->answers[id].answer[1].split('\t');
        QCOMPARE(columnHeadings.count(), columnTypes.count());
        //column type is well defined
        foreach(const QByteArray &columnType, columnTypes) {
            switch(columnType[0]) {
                case 's': //string
                case 'S': //string to translate
                case 'd': //integer
                case 'D': //integer to display localized
                case 'f': //floating point number
                case 'M': //Disk stat - some special case
                case '%': //Disk stat - some special case
                    QCOMPARE(columnType.size(), 1);
                    break;
                case 'K': //Disk stat - some special case
                    QCOMPARE(columnType.size(), 2);
                    QCOMPARE(columnType, QByteArray("KB"));
                    break;
                default:
                    QVERIFY(false);
            }
        }
    } else if(monitorType == "string") {
        // TODO Check for something in the answer?
    } else {
        QVERIFY(false);
    }


    //Now read the actual data for the sensor, and check that it's valid
    client->haveAnswer = false;
    id = nextId++;
    success = manager.sendRequest("", monitorName, client, id);
    QVERIFY(success);
    QTime timer;
    timer.start();
    timeout = 300; //Wait up to 30 seconds
    while( !client->haveAnswer && !client->isSensorLost && timeout--)
        QTest::qWait(100);
    QVERIFY(client->haveAnswer);
    QVERIFY(!client->isSensorLost);
    QCOMPARE(client->answers[id].id, id);
    if(timer.elapsed() > 200)
        qDebug() << monitorName << "took" << timer.elapsed() << "ms";

    if(monitorType == "integer") {
        QList<QByteArray> answer = client->answers[id].answer[0].split('\t');
        QCOMPARE(answer.count(), 1); //Just the number
        QVERIFY(!answer[0].isEmpty()); //Value cannot be empty
        //Make sure the value is valid
        bool isNumber;
        answer[0].toLong(&isNumber); //(note that toLong is in C locale, which is what we want)
        QVERIFY(isNumber);
    } else if(monitorType == "float") {
        QList<QByteArray> answer = client->answers[id].answer[0].split('\t');
        QCOMPARE(answer.count(), 1); //Just the number
        QVERIFY(!answer[0].isEmpty()); //Value cannot be empty
        //Make sure the value is valid
        bool isNumber;
        answer[0].toDouble(&isNumber); //(note that toDouble is in C locale, which is what we want)
        QVERIFY(isNumber);
    } else if(monitorType == "listview" || monitorType == "table") {
        foreach(const QByteArray &row, client->answers[id].answer) {
            QList<QByteArray> rowData = row.split('\t');
            QCOMPARE(rowData.count(), columnHeadings.count());
            for(int column = 0; column < columnHeadings.count(); column++) {
                switch(columnTypes[column][0]) {
                case 's': //string
                case 'S': //string to translate
                    break;
                case 'd': //integer
                case 'D': { //integer to display localized
                    bool isNumber;
                    rowData[column].toLong(&isNumber);
                    QVERIFY2(isNumber, (QString("Row data was ") +  row).toLatin1().constData());
                }
                case 'f': { //floating point number
                    bool isNumber;
                    rowData[column].toDouble(&isNumber);
                    QVERIFY(isNumber);
                }
                case 'M': //Disk stat - some special case
                    break;
                }
            }
        }
    }
    client->haveAnswer = false;
}

void TestKsysguardd::testQueueing()
{
    //Send lots of commands to ksysguardd, and check that they all return the right answer, in order

    delete client; //Start with a new client
    client = new SensorClientTest;

    const int N = 1000;
    for(int i = 0; i < N; i++) {
        bool success = manager.sendRequest("", "ps", client, i);
        QVERIFY(success);
    }

    int timeout = 300; //Wait up to 30 seconds for a single answer
    int lastCount = 0;
    while( client->answers.count() != N  && !client->isSensorLost && timeout--) {
        if(client->answers.count() != lastCount) {
            lastCount = client->answers.count();
            timeout = 300; //reset timeout as we get new answers
        }
        QTest::qWait(100);
    }
    QCOMPARE(client->answers.count(), N);

    for(int i = 0; i < N; i++) {
        QCOMPARE(client->answers[i].id,i);
    }

}
QTEST_MAIN(TestKsysguardd)


