
#include <QtTest>
#include <Qt>

class KSignalPlotter;
class TestSignalPlotter : public QObject
{
    Q_OBJECT
    private slots:
        void init();
        void cleanup();

        void testReorderBeams();
        void testReorderBeamsWithData();
    private:
        KSignalPlotter *s;
};
