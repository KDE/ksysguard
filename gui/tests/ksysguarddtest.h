
#include <QtTest>
#include <Qt>

#include <QObject>
#include <QProcess>
#include "../ksgrd/SensorManager.h"
#include "../ksgrd/SensorAgent.h"
class TestKsysguardd : public QObject
{
    Q_OBJECT
    private slots:
        void init();
        void cleanup();

        void testSetup();
        void testFormatting();
    private:
        KSGRD::SensorManager manager;
};
