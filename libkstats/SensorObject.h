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
    explicit SensorObject(const QString &id, const QString &name, SensorContainer *parent);
    explicit SensorObject(const QString &id, SensorContainer *parent);
    ~SensorObject();

    QString id() const;
    QString name() const;
    QString path() const;

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

private:
    QString m_id;
    QString m_name;
    QString m_path; //or keep parent refernce?
    QHash<QString, SensorProperty *> m_sensors;
};