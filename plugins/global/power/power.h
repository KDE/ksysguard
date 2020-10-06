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

#ifndef POWER_H
#define POWER_H

#include "SensorPlugin.h"

namespace Solid {
class Device;
}

class SensorContainer;

class PowerPlugin : public SensorPlugin {
    Q_OBJECT
public:
    PowerPlugin(QObject *parent, const QVariantList &args);
private:
    SensorContainer *m_container;
};

#endif