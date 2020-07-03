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

class SensorObject;

/**
 * Represents a collection of similar sensors.
 * For example: a SensorContainer could represent all CPUs or represent all disks
 */
class Q_DECL_EXPORT SensorContainer : public QObject
{
    Q_OBJECT
public:
    explicit SensorContainer(const QString &id, const QString &name, SensorPlugin *parent);
    ~SensorContainer();

    /**
     * A computer readable ID of this group of sensors
     */
    QString id() const;
    /**
     * A human readable name, used for selection
     */
    QString name() const;

    QList<SensorObject *> objects();
    SensorObject *object(const QString &id) const;

    /**
     * Add an object to the container.
     *
     * It will be exposed to clients as a subitem of this container.
     *
     * \param object The SensorObject to add.
     */
    void addObject(SensorObject *object);

    /**
     * Remove an object from the container.
     *
     * It will no longer be available to clients.
     *
     * \param object The SensorObject to remove.
     */
    void removeObject(SensorObject *object);

Q_SIGNALS:
    /**
     * Emitted when an object has been added
     */
    void objectAdded(SensorObject *object);
    /**
     * Emitted after an object has been removed
     * it may not be valid at this time
     */
    void objectRemoved(SensorObject *object);

private:
    QString m_id;
    QString m_name;
    QHash<QString, SensorObject *> m_sensorObjects;
    friend class SensorObject;
};
