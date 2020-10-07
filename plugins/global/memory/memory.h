/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3 of
    the license or (at your option) at any later version that is
    accepted by the membership of KDE e.V. (or its successor
    approved by the membership of KDE e.V.), which shall act as a
    proxy as defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "SensorPlugin.h"

#include <QObject>

#include <memory>

class MemoryBackend;

class MemoryPlugin : public SensorPlugin
{
    Q_OBJECT
public:
    MemoryPlugin(QObject *parent, const QVariantList &args);
    ~MemoryPlugin();

    QString providerName() const override
    {
        return QStringLiteral("memory");
    }
    void update() override;
private:
    std::unique_ptr<MemoryBackend> m_backend;
};

#endif
