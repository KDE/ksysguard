
#include <QtTest>
#include <Qt>

#include <QObject>
#include <QProcess>
#include "../ksgrd/SensorManager.h"
#include "../ksgrd/SensorAgent.h"
#include "../ksgrd/SensorClient.h"
#include <QDebug>
class SensorClientTest;

class TestKsysguardd : public QObject
{
    Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testSetup();
        void testFormatting_data();
        void testFormatting();
    private:
        KSGRD::SensorManager manager;
        SensorClientTest *client;
        QSignalSpy *hostConnectionLostSpy;
        QSignalSpy *updateSpy;
        int nextId;
};

struct SensorClientTest : public KSGRD::SensorClient
{
    SensorClientTest() {
        lastId = -1;
        isSensorLost = false;
        haveAnswer = false;
        sensorLostId = -1;
    }
    virtual void answerReceived( int id, const QList<QByteArray>& answer ) {
       lastId = id;
       lastAnswer = answer;
       QVERIFY(!haveAnswer);
       haveAnswer = true;
    }
    virtual void sensorLost(int id)
    {
        sensorLostId = id;
        QVERIFY(!haveAnswer);
        QVERIFY(!isSensorLost);
        isSensorLost = true;
    }
    int lastId;
    QList<QByteArray> lastAnswer;
    bool isSensorLost;
    bool haveAnswer;
    bool sensorLostId;
};
