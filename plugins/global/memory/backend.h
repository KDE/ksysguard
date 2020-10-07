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

#ifndef BACKEND_H
#define BACKEND_H

class SensorContainer;
class SensorObject;
class SensorProperty;

class MemoryBackend {
public:
    MemoryBackend(SensorContainer *container);
    virtual ~MemoryBackend() = default;

    void initSensors();
    virtual void update() = 0;
protected:
    virtual void makeSensors();

    SensorProperty *m_total;
    SensorProperty *m_used;
    SensorProperty *m_free;
    SensorProperty *m_application;
    SensorProperty *m_cache;
    SensorProperty *m_buffer;
    SensorProperty *m_swapTotal;
    SensorProperty *m_swapUsed;
    SensorProperty *m_swapFree;
    SensorObject *m_physicalObject;
    SensorObject *m_swapObject;
};

#endif
