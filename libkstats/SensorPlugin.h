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

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QVariant>

#include <QDBusArgument>

#include "types.h"

class SensorPlugin;
class SensorContainer;

/**
 * Base class for plugins 
 */
class Q_DECL_EXPORT SensorPlugin : public QObject
{
    Q_OBJECT
public:
    SensorPlugin(QObject *parent, const QVariantList &args);
    ~SensorPlugin() = default;

    /**
      A list of all containers provided by this plugin
     */
    QList<SensorContainer *> containers() const;

    SensorContainer *container(const QString &id) const;

    /**
     * @brief providerName
     * @returns a non-user facing name of the plugin base
     */
    virtual QString providerName() const;

    /**
     * @brief
     * A hook called before an update will be sent to the user
     */
    virtual void update();

    /**
     * Registers an object as being available for stat retrieval.
     */
    void addContainer(SensorContainer *container);

private:
    QList<SensorContainer *> m_containers;
};
