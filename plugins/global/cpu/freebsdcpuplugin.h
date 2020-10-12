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
    void initialize() override;
private:
    void makeSensors() override;
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
