/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

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

#ifndef CPUPLUGIN_H
#define CPUPLUGIN_H

#include <SensorPlugin.h>

class CpuPluginPrivate;

class CpuPlugin : public SensorPlugin
{
    Q_OBJECT
public:
    CpuPlugin(QObject *parent, const QVariantList &args);
    ~CpuPlugin() override;

    QString providerName() const override
    {
        return QStringLiteral("cpu");
    }
    void update() override;

private: 
   std::unique_ptr<CpuPluginPrivate> d;
};

#endif
