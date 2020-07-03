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

#include <QObject>

#include "SensorPlugin.h"
#include "SensorProperty.h"

class SensorContainer;
class SensorObject;

/**
 * Represents a physical or virtual object for example
 * A CPU core, or a disk
 */
class Q_DECL_EXPORT SensorObject : public QObject
{
    Q_OBJECT
public:
    explicit SensorObject(const QString &id, const QString &name, SensorContainer *parent = nullptr);
    explicit SensorObject(const QString &id, SensorContainer *parent = nullptr);
    ~SensorObject();

    QString id() const;
    QString path() const;
    QString name() const;

    void setName(const QString &newName);
    void setParentContainer(SensorContainer *parent);

    QList<SensorProperty *> sensors() const;
    SensorProperty *sensor(const QString &sensorId) const;

    void addProperty(SensorProperty *property);

    bool isSubscribed() const;

Q_SIGNALS:
    /**
     * Emitted when a client subscribes to one or more of the underlying properties of this object
     */
    void subscribedChanged(bool);

    /**
     * Emitted just before deletion
     * The object is still valid at this point
     */
    void aboutToBeRemoved();

    /**
     * Emitted whenever the object's name changes.
     */
    void nameChanged();

private:
    SensorContainer *m_parent = nullptr;
    QString m_id;
    QString m_name;
    QHash<QString, SensorProperty *> m_sensors;
};
