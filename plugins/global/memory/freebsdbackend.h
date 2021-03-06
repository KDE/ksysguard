/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3 of
    the license or (at your option) at any later version that is
    accepted by the membership of KDE e.V. (or its successor
    approved by the membership of KDE e.V.), which shall act as a
    proxy as defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
*/

#ifndef FREEBSDBACKEND_H
#define FREEBSDBACKEND_H

#include "backend.h"

#include <QVector>

#include <kvm.h> // can't forward declare typedefed kvm_t

class SensorProperty;

class FreeBsdMemoryBackend : public MemoryBackend {
public:
    FreeBsdMemoryBackend(SensorContainer *container);
    void update() override;
private:
    void makeSensors() override;
    unsigned long long pagesToBytes(uint32_t pages);

    unsigned int m_pageSize;
    kvm_t *m_kd;
    QVector<SensorProperty*> m_sysctlSensors;
};

#endif
