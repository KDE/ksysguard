#ifndef FREEBSDCPU_H
#define FREEBSDCPU_H

#include "cpu.h"
#include "cpuplugin_p.h"
#include "usagecomputer.h"

template <typename T>
class SysctlSensor;

class FreeBsdCpuObject : public CpuObject {
public:
    FreeBsdCpuObject(const QString &id, const QString &name, SensorContainer *parent);
    void update(long system, long user, long idle);
private:
    void makeSensors() override;
    void initialize() override;
    UsageComputer m_usageComputer;
    QVector<SysctlSensor<int>*> m_sysctlSensors;
};

class FreeBsdAllCpusObject : public AllCpusObject {
public:
    using AllCpusObject::AllCpusObject;
    void update(long system, long user, long idle);
private:
    UsageComputer m_usageComputer;
};

class FreeBsdCpuPluginPrivate : public CpuPluginPrivate {
public:
    FreeBsdCpuPluginPrivate(CpuPlugin *q);
    void update() override;
private:
    FreeBsdAllCpusObject *m_allCpus;
    QVector<FreeBsdCpuObject*> m_cpus;
};

#endif
