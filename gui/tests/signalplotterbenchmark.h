
#include <QtTest>
#include <Qt>

class KSignalPlotter;
class BenchmarkSignalPlotter : public QObject
{
    Q_OBJECT
    private slots:
        void init();
        void cleanup();

        void addData();
    private:
        KSignalPlotter *s;
};
