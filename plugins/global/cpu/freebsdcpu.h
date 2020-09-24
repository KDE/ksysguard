#ifndef FREEBSDCPU_H
#define FREEBSDCPU_H

#include "cpu.h"
#include "cpuplugin_p.h"

class FreeBsdCpuObject : public CpuObject {
public:
    FreeBsdCpuObject(const QString &id, const QString &name, SensorContainer *parent);
    void update(long system, long user, long idle);
private:
    long m_totalTicks;
    long m_systemTicks;
    long m_userTicks;
};

class FreeBsdCpuPluginPrivate : public CpuPluginPrivate {
public:
    FreeBsdCpuPluginPrivate(CpuPlugin *q);
    void update() override;
private:
    AllCpusObject<FreeBsdCpuObject> *m_allCpusObject;
};

#endif
