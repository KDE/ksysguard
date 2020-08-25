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

class CpuObject : public SensorObject {
public:
    CpuObject(const QString &id, const QString &name, SensorContainer *parent);

//     const int physicalId; // NOTE The combination of these two ids is not necessarily unique with hyperthreading
//     const int coreId;
protected: 
    SensorProperty *m_usage;
    SensorProperty *m_system;
    SensorProperty *m_user;
    SensorProperty *m_wait;
    SensorProperty *m_frequency;
    SensorProperty *m_temperature;
};

#endif
