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

#ifndef CPU_H
#define CPU_H

#include <SensorObject.h>

class BaseCpuObject : public SensorObject {
public:
protected:
    BaseCpuObject(const QString &id, const QString &name, SensorContainer *parent);

    virtual void initialize();
    virtual void makeSensors();

    SensorProperty *m_usage;
    SensorProperty *m_system;
    SensorProperty *m_user;
    SensorProperty *m_wait;
};

class CpuObject : public BaseCpuObject {
public:
    CpuObject(const QString &id, const QString &name, SensorContainer *parent);

protected:
    void initialize() override;
    void makeSensors() override;

    SensorProperty *m_frequency;
    SensorProperty *m_temperature;
};

class AllCpusObject : public BaseCpuObject {
public:
    AllCpusObject(unsigned int cpuCount, unsigned int coreCount, SensorContainer *parent);

protected:
    void initialize() override;
    void makeSensors() override;

    SensorProperty *m_cpuCount;
    SensorProperty *m_coreCount;
};

#endif
