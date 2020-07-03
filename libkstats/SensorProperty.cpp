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

#include "SensorProperty.h"
#include "SensorObject.h"

SensorProperty::SensorProperty(const QString &id, SensorObject *parent)
    : SensorProperty(id, QString(), parent)
{
}

SensorProperty::SensorProperty(const QString &id, const QString &name, SensorObject *parent)
    : SensorProperty(id, name, QVariant(), parent)
{
}

SensorProperty::SensorProperty(const QString &id, const QString &name, const QVariant &initalValue, SensorObject *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_id(id)
{
    setName(name);
    if (initalValue.isValid()) {
        setValue(initalValue);
    }
    parent->addProperty(this);
}

SensorProperty::~SensorProperty()
{
}

SensorInfo SensorProperty::info() const
{
    return m_info;
}

QString SensorProperty::id() const
{
    return m_id;
}

QString SensorProperty::path() const
{
    return m_parent->path() % QLatin1Char('/') % m_id;
}

void SensorProperty::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    m_info.name = m_prefix.isEmpty() ? m_name : m_prefix % QLatin1Char(' ') % m_name;
    emit sensorInfoChanged();
}

void SensorProperty::setShortName(const QString &name)
{
    if (m_info.shortName == name) {
        return;
    }

    m_info.shortName = name;
    emit sensorInfoChanged();
}

void SensorProperty::setPrefix(const QString &prefix)
{
    if (m_prefix == prefix) {
        return;
    }

    m_prefix = prefix;
    m_info.name = prefix.isEmpty() ? m_name : prefix % QLatin1Char(' ') % m_name;
    emit sensorInfoChanged();
}

void SensorProperty::setDescription(const QString &description)
{
    if (m_info.description == description) {
        return;
    }

    m_info.description = description;
    emit sensorInfoChanged();
}

void SensorProperty::setMin(qreal min)
{
    if (qFuzzyCompare(m_info.min, min)) {
        return;
    }

    m_info.min = min;
    emit sensorInfoChanged();
}

void SensorProperty::setMax(qreal max)
{
    if (qFuzzyCompare(m_info.max, max)) {
        return;
    }

    m_info.max = max;
    emit sensorInfoChanged();
}

void SensorProperty::setMax(SensorProperty *other)
{
    setMax(other->value().toReal());
    if (isSubscribed()) {
        other->subscribe();
    }
    connect(this, &SensorProperty::subscribedChanged, this, [this, other](bool isSubscribed) {
        if (isSubscribed) {
            other->subscribe();
            setMax(other->value().toReal());
        } else {
            other->unsubscribe();
        }
    });
    connect(other, &SensorProperty::valueChanged, this, [this, other]() {
        setMax(other->value().toReal());
    });
}

void SensorProperty::setUnit(KSysGuard::utils::Unit unit)
{
    if (m_info.unit == unit) {
        return;
    }

    m_info.unit = unit;
    emit sensorInfoChanged();
}

void SensorProperty::setVariantType(QVariant::Type type)
{
    if (m_info.variantType == type) {
        return;
    }

    m_info.variantType = type;
    emit sensorInfoChanged();
}

bool SensorProperty::isSubscribed() const
{
    return m_subscribers > 0;
}

void SensorProperty::subscribe()
{
    m_subscribers++;
    if (m_subscribers == 1) {
        emit subscribedChanged(true);
    }
}

void SensorProperty::unsubscribe()
{
    m_subscribers--;
    if (m_subscribers == 0) {
        emit subscribedChanged(false);
    }
}

QVariant SensorProperty::value() const
{
    return m_value;
}

void SensorProperty::setValue(const QVariant &value)
{
    m_value = value;
    emit valueChanged();
}
