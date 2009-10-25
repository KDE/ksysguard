
#include <QtTest>
#include <Qt>

class KSignalPlotter;
class TestSignalPlotter : public QObject
{
    Q_OBJECT
    private slots:
        void init();
        void cleanup();

        void testAddRemoveBeams();
        void testAddRemoveBeamsWithData();
        void testReorderBeams();
        void testReorderBeamsWithData();
        void testMaximumRange();
        void testNegativeMinimumRange();
        void testSetBeamColor();
        void testSetUnit();
        void testGettersSetters();
        void testAddingData();
        void testNonZeroRange();
        void testNonZeroRange2();
        void testNiceRangeCalculation_data();
        void testNiceRangeCalculation();
    private:
        KSignalPlotter *s;
};
