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

#include "cpu.h"
#include "cpuplugin_p.h"

class LinuxCpuObject : public CpuObject
{
public:
    LinuxCpuObject(const QString &id, const QString &name, SensorContainer *parent, double frequency);

    void update(unsigned long long system, unsigned long long  user, unsigned long long wait, unsigned long long idle);
private:
    unsigned long long m_totalTicks;
    unsigned long long m_systemTicks;
    unsigned long long m_userTicks;
    unsigned long long m_waitTicks;
};

class LinuxCpuPluginPrivate : public CpuPluginPrivate {
public:
    LinuxCpuPluginPrivate(CpuPlugin *q);
    void update() override;
private:
    void addSensors();
};

#endif
