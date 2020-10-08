#ifndef LINUXCPU_H
#define LINUXCPU_H

struct sensors_chip_name;
struct sensors_feature;

#include "cpu.h"
#include "usagecomputer.h"


class TemperatureSensor : public SensorProperty {
public:
    TemperatureSensor(const QString &id, SensorObject *parent);
    void setFeature(const sensors_chip_name * const chipName, const sensors_feature * const feature);
    void update();
private:
    const sensors_chip_name * m_sensorChipName;
    int m_temperatureSubfeature;
};

class LinuxCpuObject : public CpuObject
{
public:
    LinuxCpuObject(const QString &id, const QString &name, SensorContainer *parent);

    void update(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle);
    TemperatureSensor* temperatureSensor();
    void initialize(double initialFrequency);
private:
    void initialize() override {};
    void makeSensors() override;
    UsageComputer m_usageComputer;
    TemperatureSensor *m_temperatureSensor;
};

class LinuxAllCpusObject : public AllCpusObject {
public:
    using AllCpusObject::AllCpusObject;
    void update(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle);
private:
    UsageComputer m_usageComputer;
};

#endif
