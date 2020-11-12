/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <memory>

#include "SensorPlugin.h"

class GpuPlugin : public SensorPlugin
{
    Q_OBJECT
public:
    GpuPlugin(QObject *parent, const QVariantList &args);
    ~GpuPlugin();

    QString providerName() const override
    {
        return QStringLiteral("gpu");
    }

    void update() override;

private:
    class Private;
    std::unique_ptr<Private> d;
};
