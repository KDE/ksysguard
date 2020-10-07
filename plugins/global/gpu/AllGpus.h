/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "SensorObject.h"

class AggregateSensor;

class AllGpus : public SensorObject
{
    Q_OBJECT

public:
    AllGpus(SensorContainer *parent);

private:
    AggregateSensor *m_usageSensor = nullptr;
    AggregateSensor *m_totalVramSensor = nullptr;
    AggregateSensor *m_usedVramSensor = nullptr;
};
