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

#pragma once

#include "types.h"
#include <QObject>

class SensorObject;

/**
 * Represents a given value source with attached metadata
 * For example, current load for a given CPU core, or a disk capacity
 */
class Q_DECL_EXPORT SensorProperty : public QObject
{
    Q_OBJECT
public:
    explicit SensorProperty(const QString &id, SensorObject *parent);
    explicit SensorProperty(const QString &id, const QString &name, SensorObject *parent);
    explicit SensorProperty(const QString &id, const QString &name, const QVariant &initalValue, SensorObject *parent);

    ~SensorProperty() override;

    SensorInfo info() const;

    /**
     * A computer readable ID of the property
     */
    QString id() const;

    /**
     * A deduced path based on the concatenated ID of ourselves + parent IDs
     */
    QString path() const;
    /**
     * A human reabable translated name of the property
     */
    void setName(const QString &name);
    void setShortName(const QString &name);
    void setPrefix(const QString &name);

    void setDescription(const QString &description);
    /**
     * Sets a hint describing the minimum value this value can be.
     * Values are not clipped, it is a hint for graphs.
     * When not relevant, leave unset
     */
    void setMin(qreal min);
    /**
     * Sets a hint describing the maximum value this value can be.
     * Values are not clipped, it is a hint for graphs.
     * When not relevant, leave unset
     */
    void setMax(qreal max);
    /**
     * Shorthand for setting the maximum value to that of another property
     * For example to mark the usedSpace of a disk to be the same as the disk capacity
     */
    void setMax(SensorProperty *other);
    void setUnit(KSysGuard::Unit unit);
    void setVariantType(QVariant::Type type);

    bool isSubscribed() const;

    /**
     * Called when a client requests to get continual updates from this property.
     */
    virtual void subscribe();
    /**
     * Called when a client disconnects or no longer wants updates for this property.
     */
    virtual void unsubscribe();
    /**
     * Returns the last value set for this property
     */
    virtual QVariant value() const;
    /**
     * Update the stored value for this property
     */
    void setValue(const QVariant &value);

    /**
     * Updates the value of this property if possible. The default implementation does nothing.
     */
    virtual void update() {}

Q_SIGNALS:
    /**
     * Emitted when the value changes
     * Clients should emit this manually if they implement value() themselves
     */
    void valueChanged();
    /**
     * Emitted when the metadata of a sensor changes.
     * min/max etc.
     */
    void sensorInfoChanged();
    /**
     * Emitted when we have our first subscription, or all subscriptions are gone
     */
    void subscribedChanged(bool);

private:
    SensorObject *m_parent = nullptr;
    SensorInfo m_info;
    QString m_id;
    QString m_name;
    QString m_prefix;
    QVariant m_value;
    int m_subscribers = 0;
};
