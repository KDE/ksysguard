#include "signalplotterbenchmark.h"
#include "../../../libs/ksysguard/signalplotter/ksignalplotter.h"

#include <qtest_kde.h>
#include <QTest>
#include <QtGui>
#include <limits>

void BenchmarkSignalPlotter::init()
{
    s = new KSignalPlotter;
}
void BenchmarkSignalPlotter::cleanup()
{
    delete s;
}

void BenchmarkSignalPlotter::addData()
{
    s->addBeam(Qt::blue);
}
QTEST_KDEMAIN(BenchmarkSignalPlotter, GUI)

