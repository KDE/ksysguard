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

#include "SensorObject.h"

#include "SensorContainer.h"
#include "SensorPlugin.h"

SensorObject::SensorObject(const QString &id, SensorContainer *parent)
    : SensorObject(id, QString(), parent)
{
}

SensorObject::SensorObject(const QString &id, const QString &name, SensorContainer *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_id(id)
    , m_name(name)
{
    if (parent) {
    parent->addObject(this);
    }
}

SensorObject::~SensorObject()
{
}

QString SensorObject::id() const
{
    return m_id;
}

QString SensorObject::name() const
{
    return m_name;
}

QString SensorObject::path() const
{
    if (!m_parent) {
        return QString{};
    }

    return m_parent->id() % QLatin1Char('/') % m_id;
}

void SensorObject::setParentContainer(SensorContainer* parent)
{
    m_parent = parent;
}

QList<SensorProperty *> SensorObject::sensors() const
{
    return m_sensors.values();
}

SensorProperty *SensorObject::sensor(const QString &sensorId) const
{
    return m_sensors.value(sensorId);
}

void SensorObject::addProperty(SensorProperty *property)
{
    m_sensors[property->id()] = property;

    connect(property, &SensorProperty::subscribedChanged, this, [=]() {
        uint count = std::count_if(m_sensors.constBegin(), m_sensors.constEnd(), [](const SensorProperty *prop) {
            return prop->isSubscribed();
        });
        if (count == 1) {
            emit subscribedChanged(true);
        } else if (count == 0) {
            emit subscribedChanged(false);
        }
    });
}

bool SensorObject::isSubscribed() const
{
    return std::any_of(m_sensors.constBegin(), m_sensors.constEnd(), [](const SensorProperty *prop) {
        return prop->isSubscribed();
    });
}
