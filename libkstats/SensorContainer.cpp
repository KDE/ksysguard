/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>

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

#include "SensorContainer.h"

#include "SensorObject.h"
#include "SensorPlugin.h"

SensorContainer::SensorContainer(const QString &id, const QString &name, SensorPlugin *parent)
    : QObject(parent)
    , m_id(id)
    , m_name(name)
{
    parent->addContainer(this);
}

SensorContainer::~SensorContainer()
{
}

QString SensorContainer::id() const
{
    return m_id;
}

QString SensorContainer::name() const
{
    return m_name;
}

QList<SensorObject *> SensorContainer::objects()
{
    return m_sensorObjects.values();
}

SensorObject *SensorContainer::object(const QString &id) const
{
    return m_sensorObjects.value(id);
}

void SensorContainer::addObject(SensorObject *object)
{
    const QString id = object->id();
    Q_ASSERT(!m_sensorObjects.contains(id));
    m_sensorObjects[id] = object;
    Q_EMIT objectAdded(object);

    connect(object, &SensorObject::aboutToBeRemoved, this, [this, object]() {
        removeObject(object);
    });
}

void SensorContainer::removeObject(SensorObject *object)
{
    if (!m_sensorObjects.contains(object->id())) {
        return;
    }

    m_sensorObjects.remove(object->id());
    Q_EMIT objectRemoved(object);
}
