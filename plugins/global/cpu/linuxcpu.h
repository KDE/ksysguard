#ifndef LINUXCPU_H
#define LINUXCPU_H

struct sensors_chip_name;
struct sensors_feature;

#include "cpu.h"
#include "usagecomputer.h"

class LinuxCpuObject : public CpuObject
{
public:
    LinuxCpuObject(const QString &id, const QString &name, SensorContainer *parent, double frequency = 0);

    void update(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle);
    void setTemperatureSensor(const sensors_chip_name * const chipName, const sensors_feature * const feature);
private:
    UsageComputer m_usageComputer;
    const sensors_chip_name * m_sensorChipName;
    int m_temperatureSubfeature;
};

class LinuxAllCpusObject : public AllCpusObject {
public:
    using AllCpusObject::AllCpusObject;
    void update(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle);
private:
    UsageComputer m_usageComputer;
};

#endif
