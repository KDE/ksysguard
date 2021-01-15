/*
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AggregateSensor.h"

#include "SensorContainer.h"
#include <QTimer>

// Add two QVariants together.
//
// This will try to add two QVariants together based on the type of first. This
// will try to convert first and second to a common type, add them, then convert
// back to the type of first.
//
// If any conversion fails or there is no way to add two types, first will be
// returned.
QVariant addVariants(const QVariant &first, const QVariant &second)
{
    auto result = QVariant {};

    bool convertFirst = false;
    bool convertSecond = false;

    auto type = first.type();
    switch (static_cast<QMetaType::Type>(type)) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
        result = first.toLongLong(&convertFirst) + second.toLongLong(&convertSecond);
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        result = first.toULongLong(&convertFirst) + second.toULongLong(&convertSecond);
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        result = first.toDouble(&convertFirst) + second.toDouble(&convertSecond);
        break;
    default:
        return first;
    }

    if (!convertFirst || !convertSecond) {
        return first;
    }

    if (!result.convert(type)) {
        return first;
    }

    return result;
}

AggregateSensor::AggregateSensor(SensorObject *provider, const QString &id, const QString &name)
    : SensorProperty(id, name, provider)
    , m_subsystem(qobject_cast<SensorContainer *>(provider->parent()))
{
    m_aggregateFunction = addVariants;
    connect(m_subsystem, &SensorContainer::objectAdded, this, &AggregateSensor::updateSensors);
    connect(m_subsystem, &SensorContainer::objectRemoved, this, &AggregateSensor::updateSensors);
}

AggregateSensor::~AggregateSensor()
{
}

QRegularExpression AggregateSensor::matchSensors() const
{
    return m_matchObjects;
}

QVariant AggregateSensor::value() const
{
    if (m_sensors.isEmpty()) {
        return QVariant();
    }

    auto it = m_sensors.constBegin();
    while (!it.value() && it != m_sensors.constEnd()) {
        it++;
    }

    if (it == m_sensors.constEnd()) {
        return QVariant{};
    }

    QVariant result = it.value()->value();
    it++;
    for (; it != m_sensors.constEnd(); it++) {
        if (it.value()) {
            result = m_aggregateFunction(result, it.value()->value());
        }
    }
    return result;
}

void AggregateSensor::subscribe()
{
    bool wasSubscribed = SensorProperty::isSubscribed();
    SensorProperty::subscribe();
    if (!wasSubscribed && isSubscribed()) {
        for (auto sensor : qAsConst(m_sensors)) {
            if (sensor) {
                sensor->subscribe();
            }
        }
    }
}

void AggregateSensor::unsubscribe()
{
    bool wasSubscribed = SensorProperty::isSubscribed();
    SensorProperty::unsubscribe();
    if (wasSubscribed && !isSubscribed()) {
        for (auto sensor : qAsConst(m_sensors)) {
            if (sensor) {
                sensor->unsubscribe();
            }
        }
    }
}

void AggregateSensor::setMatchSensors(const QRegularExpression &objectIds, const QString &propertyName)
{
    if (objectIds == m_matchObjects && propertyName == m_matchProperty) {
        return;
    }

    m_matchProperty = propertyName;
    m_matchObjects = objectIds;
    updateSensors();
}

std::function<QVariant(QVariant, QVariant)> AggregateSensor::aggregateFunction() const
{
    return m_aggregateFunction;
}

void AggregateSensor::setAggregateFunction(const std::function<QVariant(QVariant, QVariant)> &newAggregateFunction)
{
    m_aggregateFunction = newAggregateFunction;
}

void AggregateSensor::addSensor(SensorProperty *sensor)
{
    if (!sensor || sensor->path() == path() || m_sensors.contains(sensor->path())) {
        return;
    }

    if (isSubscribed()) {
        sensor->subscribe();
    }

    connect(sensor, &SensorProperty::valueChanged, this, [this, sensor]() {
        sensorDataChanged(sensor);
    });
    m_sensors.insert(sensor->path(), sensor);
}

void AggregateSensor::removeSensor(const QString &sensorPath)
{
    auto sensor = m_sensors.take(sensorPath);
    sensor->disconnect(this);
    if (isSubscribed()) {
        sensor->unsubscribe();
    }
}

int AggregateSensor::matchCount() const
{
    return m_sensors.size();
}

void AggregateSensor::updateSensors()
{
    if (!m_matchObjects.isValid()) {
        return;
    }
    for (auto obj : m_subsystem->objects()) {
        if (m_matchObjects.match(obj->id()).hasMatch()) {
            auto sensor = obj->sensor(m_matchProperty);
            if (sensor) {
                addSensor(sensor);
            }
        }
    }

    auto itr = m_sensors.begin();
    while (itr != m_sensors.end()) {
        if (!itr.value()) {
            itr = m_sensors.erase(itr);
        } else {
            ++itr;
        }
    }

    delayedEmitDataChanged();
}

void AggregateSensor::sensorDataChanged(SensorProperty *sensor)
{
    Q_UNUSED(sensor)
    delayedEmitDataChanged();
}

void AggregateSensor::delayedEmitDataChanged()
{
    if (!m_dataChangeQueued) {
        m_dataChangeQueued = true;
        QTimer::singleShot(m_dataCompressionDuration, [this]() {
            Q_EMIT valueChanged();
            m_dataChangeQueued = false;
        });
    }
}

PercentageSensor::PercentageSensor(SensorObject *provider, const QString &id, const QString &name)
    : SensorProperty(id, name, provider)
{
    setUnit(KSysGuard::UnitPercent);
    setMax(100);
}

PercentageSensor::~PercentageSensor()
{
}

void PercentageSensor::setBaseSensor(SensorProperty *property)
{
    m_sensor = property;
    connect(property, &SensorProperty::valueChanged, this, &PercentageSensor::valueChanged);
    connect(property, &SensorProperty::sensorInfoChanged, this, &PercentageSensor::valueChanged);
}

QVariant PercentageSensor::value() const
{
    if (!m_sensor) {
        return QVariant();
    }
    QVariant value = m_sensor->value();
    if (!value.isValid()) {
        return QVariant();
    }
    return (value.toReal() / m_sensor->info().max) * 100.0;
}

void PercentageSensor::subscribe()
{
    m_sensor->subscribe();
}

void PercentageSensor::unsubscribe()
{
    m_sensor->unsubscribe();
}
