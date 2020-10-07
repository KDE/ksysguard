/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>

#include "SensorObject.h"

class GpuDevice : public SensorObject
{
    Q_OBJECT

public:
    GpuDevice(const QString& id, const QString& name);
    ~GpuDevice() override = default;

    virtual void initialize();
    virtual void update();

protected:
    virtual void makeSensors();

    SensorProperty *m_nameProperty;
    SensorProperty *m_usageProperty;
    SensorProperty *m_totalVramProperty;
    SensorProperty *m_usedVramProperty;
    SensorProperty *m_temperatureProperty;
    SensorProperty *m_coreFrequencyProperty;
    SensorProperty *m_memoryFrequencyProperty;
};
