#include "signalplottertest.h"
#include "../SensorDisplayLib/SignalPlotter.h"

#include <qtest_kde.h>
#include <QtTest>
#include <QtGui>

void TestSignalPlotter::init()
{
    s = new KSignalPlotter;
}
void TestSignalPlotter::cleanup()
{
    delete s;
}

void TestSignalPlotter::testAddRemoveBeams()
{
    //Just try various variations of adding and removing beams
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::red);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    s->removeBeam(1);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::blue);

    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
}
void TestSignalPlotter::testAddRemoveBeamsWithData()
{
    //Just try various variations of adding and removing beams,
    //this time with data as well
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);

    QCOMPARE(s->lastValue(0), 0.0); //unset, so should default to 0
    QCOMPARE(s->lastValue(1), 0.0); //unset, so should default to 0
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    s->addSample(QList<double>() << 1.0 << 2.0);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::red);
    QCOMPARE(s->lastValue(0), 2.0);

    s->removeBeam(0);
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    s->addSample(QList<double>() << 1.0 << 2.0);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    s->removeBeam(1);
    QCOMPARE(s->numBeams(), 1);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QCOMPARE(s->lastValue(0), 1.0);

    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 0.0); //unset, so should default to 0
}

void TestSignalPlotter::testReorderBeams()
{
    QCOMPARE(s->numBeams(), 0);
    QList<int> newOrder;
    s->reorderBeams(newOrder); // do nothing
    QCOMPARE(s->numBeams(), 0);

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    newOrder << 0 << 1; //nothing changed
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    newOrder.clear();
    newOrder << 1 << 0; //reverse them
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::red);
    QVERIFY(s->beamColor(1) == Qt::blue);

    //reverse them back again
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);

    //switch them yet again
    s->reorderBeams(newOrder);

    //Add a third beam
    s->addBeam(Qt::green);
    QCOMPARE(s->numBeams(), 3);
    QVERIFY(s->beamColor(0) == Qt::red);
    QVERIFY(s->beamColor(1) == Qt::blue);
    QVERIFY(s->beamColor(2) == Qt::green);

    newOrder.clear();
    newOrder << 2 << 0 << 1;
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 3);
    QVERIFY(s->beamColor(0) == Qt::green);
    QVERIFY(s->beamColor(1) == Qt::red);
    QVERIFY(s->beamColor(2) == Qt::blue);
}
void TestSignalPlotter::testReorderBeamsWithData()
{
    QCOMPARE(s->numBeams(), 0);
    QList<int> newOrder;

    s->addBeam(Qt::blue);
    s->addBeam(Qt::red);
    QCOMPARE(s->numBeams(), 2);
    QCOMPARE(s->lastValue(0), 0.0);
    QCOMPARE(s->lastValue(1), 0.0);
    //Add some data
    QList<double> data;
    data << 1.0 << 2.0;
    s->addSample(data);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    newOrder << 0 << 1; //nothing changed
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    newOrder.clear();
    newOrder << 1 << 0; //reverse them
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QCOMPARE(s->lastValue(0), 2.0);
    QCOMPARE(s->lastValue(1), 1.0);

    //reverse them back again
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 2);
    QCOMPARE(s->lastValue(0), 1.0);
    QCOMPARE(s->lastValue(1), 2.0);

    //switch them yet again
    s->reorderBeams(newOrder);

    //Add a third beam
    s->addBeam(Qt::green);
    QCOMPARE(s->numBeams(), 3);
    QCOMPARE(s->lastValue(0), 2.0);
    QCOMPARE(s->lastValue(1), 1.0);
    QCOMPARE(s->lastValue(2), 0.0);

    newOrder.clear();
    newOrder << 2 << 0 << 1;
    s->reorderBeams(newOrder);
    QCOMPARE(s->numBeams(), 3);
    QCOMPARE(s->lastValue(0), 0.0);
    QCOMPARE(s->lastValue(1), 2.0);
    QCOMPARE(s->lastValue(2), 1.0);
}
void TestSignalPlotter::testMaximumRange()
{
    QCOMPARE(s->maximumValue(), 0.0);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 0.0);
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);
    QCOMPARE(s->useAutoRange(), true);

    s->addBeam(Qt::blue);
    //Nothing should have changed yet
    QCOMPARE(s->maximumValue(), 0.0);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 0.0);
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);

    QList<double> data;
    data << 1.1;
    s->addSample(data);

    QCOMPARE(s->maximumValue(), 0.0);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1.25); //It gets rounded up
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);

    s->setMaximumValue(1.0);
    QCOMPARE(s->maximumValue(), 1.0);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1.25); //Current value is still larger
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);

    s->setMaximumValue(1.4);
    QCOMPARE(s->maximumValue(), 1.4);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1.5); //given maximum range is now the larger value
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);


    s->addBeam(Qt::red);
    //nothing changed by adding a beam
    QCOMPARE(s->maximumValue(), 1.4);
    QCOMPARE(s->minimumValue(), 0.0);
    QCOMPARE(s->currentMaximumRangeValue(), 1.5); //given maximum range hasn't changed
    QCOMPARE(s->currentMinimumRangeValue(), 0.0);
}

void TestSignalPlotter::testNegativeMinimumRange()
{
    s->setMinimumValue(-1000);
    s->setMaximumValue(4000);
    QCOMPARE(s->minimumValue(), -1000.0);
    QCOMPARE(s->maximumValue(),  4000.0);
    QCOMPARE(s->currentMinimumRangeValue(), -1000.0);
    QCOMPARE(s->currentMaximumRangeValue(), 4000.0);

    s->setScaleDownBy(1024);
    QCOMPARE(s->minimumValue(), -1000.0);
    QCOMPARE(s->maximumValue(),  4000.0);
    QCOMPARE(s->currentMinimumRangeValue(), -1000.0);
    QCOMPARE(s->currentMaximumRangeValue(), 4120.0);

    QCOMPARE(s->valueAsString(4096,1), QString("4.0"));
    QCOMPARE(s->valueAsString(-4096,1), QString("-4.0"));
}
void TestSignalPlotter::testSetBeamColor() {
    s->addBeam(Qt::red);
    s->setBeamColor(0, Qt::blue);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QCOMPARE(s->numBeams(), 1);

    s->addBeam(Qt::red);
    QVERIFY(s->beamColor(0) == Qt::blue);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->numBeams(), 2);

    s->setBeamColor(0, Qt::green);
    QVERIFY(s->beamColor(0) == Qt::green);
    QVERIFY(s->beamColor(1) == Qt::red);
    QCOMPARE(s->numBeams(), 2);

    s->setBeamColor(1, Qt::blue);
    QVERIFY(s->beamColor(0) == Qt::green);
    QVERIFY(s->beamColor(1) == Qt::blue);

    s->removeBeam(0);
    QVERIFY(s->beamColor(0) == Qt::blue);
    s->setBeamColor(0, Qt::red);
    QVERIFY(s->beamColor(0) == Qt::red);
}

void TestSignalPlotter::testSetUnit() {
    //Test default
    QCOMPARE(s->valueAsString(3e20,1), QString("3e+20"));
    QCOMPARE(s->valueAsString(-3e20,1), QString("-3e+20"));

    s->setUnit(ki18ncp("Units", "%1 second", "%1 seconds") );

    QCOMPARE(s->valueAsString(3e20,1), QString("3e+20 seconds"));
    QCOMPARE(s->valueAsString(-3e20,1), QString("-3e+20 seconds"));
    QCOMPARE(s->valueAsString(3.4,1), QString("3.4 seconds"));
    QCOMPARE(s->valueAsString(-3.4,1), QString("-3.4 seconds"));
    QCOMPARE(s->valueAsString(1), QString("1.0 seconds"));
    QCOMPARE(s->valueAsString(-1), QString("-1.0 seconds"));
    QCOMPARE(s->valueAsString(1,0), QString("1 second"));
    QCOMPARE(s->valueAsString(-1,0), QString("-1 second"));

    //now switch to minutes
    s->setScaleDownBy(60);
    s->setUnit(ki18ncp("Units", "%1 minute", "%1 minutes") );
    QCOMPARE(s->valueAsString(3.4), QString("0.06 minutes"));
    QCOMPARE(s->valueAsString(-3.4), QString("-0.06 minutes"));
    QCOMPARE(s->valueAsString(60), QString("1.0 minutes"));
    QCOMPARE(s->valueAsString(-60), QString("-1.0 minutes"));
    QCOMPARE(s->valueAsString(60,0), QString("1 minute"));
    QCOMPARE(s->valueAsString(-60,0), QString("-1 minute"));
}

void TestSignalPlotter::testGettersSetters() {
    //basic test of all the getters and setters and default values
    KLocalizedString string = ki18ncp("Units", "%1 second", "%1 seconds");
    s->setUnit( string );
    QVERIFY( s->unit().toString() == string.toString() );
    s->setMaximumValue(3);
    s->setMinimumValue(-3);
    QCOMPARE(s->maximumValue(), 3.0);
    QCOMPARE(s->minimumValue(), -3.0);

    s->changeRange(-2,2);
    QCOMPARE(s->maximumValue(), 2.0);
    QCOMPARE(s->minimumValue(), -2.0);

    s->setMinimumValue(-3);
    QCOMPARE(s->useAutoRange(), true); //default
    s->setUseAutoRange(false);
    QCOMPARE(s->useAutoRange(), false);

    QCOMPARE(s->thinFrame(), true); //default
    s->setThinFrame(false);
    QCOMPARE(s->thinFrame(), false);

    QCOMPARE(s->scaleDownBy(), 1.0); //default
    s->setScaleDownBy(1.2);
    QCOMPARE(s->scaleDownBy(), 1.2);
    s->setScaleDownBy(0.5);
    QCOMPARE(s->scaleDownBy(), 0.5);

    QCOMPARE(s->horizontalScale(), 1); //default
    s->setHorizontalScale(2);
    QCOMPARE(s->horizontalScale(), 2);

    QCOMPARE(s->showHorizontalLines(), true); //default
    s->setShowHorizontalLines(false);
    QCOMPARE(s->showHorizontalLines(), false);

    QCOMPARE(s->showVerticalLines(), false); //default
    s->setShowVerticalLines(true);
    QCOMPARE(s->showVerticalLines(), true);

    QCOMPARE(s->verticalLinesScroll(), true); //default
    s->setVerticalLinesScroll(false);
    QCOMPARE(s->verticalLinesScroll(), false);

    QCOMPARE(s->verticalLinesDistance(), (uint)30); //default
    s->setVerticalLinesDistance(1);
    QCOMPARE(s->verticalLinesDistance(), (uint)1);

    QCOMPARE(s->showAxis(), true); //default
    s->setShowAxis(false);
    QCOMPARE(s->showAxis(), false);

    QCOMPARE(s->maxAxisTextWidth(), 0); //default
    s->setMaxAxisTextWidth(30);
    QCOMPARE(s->maxAxisTextWidth(), 30);
    s->setMaxAxisTextWidth(0);
    QCOMPARE(s->maxAxisTextWidth(), 0);

    QCOMPARE(s->smoothGraph(), true); //default
    s->setSmoothGraph(false);
    QCOMPARE(s->smoothGraph(), false);

    QCOMPARE(s->stackGraph(), false); //default
    s->setStackGraph(true);
    QCOMPARE(s->stackGraph(), true);

    QCOMPARE(s->fillOpacity(), 20); //default
    s->setFillOpacity(255);
    QCOMPARE(s->fillOpacity(), 255);
    s->setFillOpacity(0);
    QCOMPARE(s->fillOpacity(), 0);


}

QTEST_KDEMAIN(TestSignalPlotter, GUI)

