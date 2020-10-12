/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

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
