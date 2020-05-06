/*
 * Copyright 2019 Arjen Hiemstra <ahiemsta@heimr.nl>
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

#ifndef AGGREGATESENSOR_H
#define AGGREGATESENSOR_H

#include <functional>

#include <QRegularExpression>
#include <QVariant>
#include <QVector>

#include "SensorObject.h"
#include "SensorPlugin.h"
#include "SensorProperty.h"

/**
 * @todo write docs
 */
class Q_DECL_EXPORT AggregateSensor : public SensorProperty
{
    Q_OBJECT

public:
    AggregateSensor(SensorObject *provider, const QString &id, const QString &name);
    ~AggregateSensor();

    QVariant value() const override;
    void subscribe() override;
    void unsubscribe() override;

    QRegularExpression matchSensors() const;
    void setMatchSensors(const QRegularExpression &objectMatch, const QString &propertyId);
    std::function<QVariant(QVariant, QVariant)> aggregateFunction() const;
    void setAggregateFunction(const std::function<QVariant(QVariant, QVariant)> &function);

    void addSensor(SensorProperty *sensor);
    void removeSensor(const QString &sensorPath);

private:
    void updateSensors();
    void sensorDataChanged(SensorProperty *sensor);
    void delayedEmitDataChanged();

    QRegularExpression m_matchObjects;
    QString m_matchProperty;
    QHash<QString, SensorProperty *> m_sensors;
    bool m_dataChangeQueued = false;
    int m_dataCompressionDuration = 100;
    SensorContainer *m_subsystem;

    std::function<QVariant(QVariant, QVariant)> m_aggregateFunction;
};

class Q_DECL_EXPORT PercentageSensor : public SensorProperty
{
    Q_OBJECT
public:
    PercentageSensor(SensorObject *provider, const QString &id, const QString &name);
    ~PercentageSensor() override;

    QVariant value() const override;
    void subscribe() override;
    void unsubscribe() override;

    void setBaseSensor(SensorProperty *sensor);

private:
    SensorProperty *m_sensor;
};

#endif // AGGREGATESENSOR_H
