/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include <memory>
#include <thread>
#include <atomic>
#include <unordered_map>

#include "TimeStamps.h"
#include "Packet.h"

class Capture;
class ConnectionMapping;
class Packet;

class Accumulator
{

public:
    using InboundOutboundData = std::pair<int, int>;
    using PidDataCounterHash = std::unordered_map<int, InboundOutboundData>;

    Accumulator(std::shared_ptr<Capture> capture, std::shared_ptr<ConnectionMapping> mapping);

    PidDataCounterHash data();

    void stop();

private:
    void addData(Packet::Direction direction, const Packet &packet, int pid);
    void loop();

    std::shared_ptr<Capture> m_capture;
    std::shared_ptr<ConnectionMapping> m_mapping;

    std::thread m_thread;
    std::atomic_bool m_running;

    PidDataCounterHash m_data;
};

#endif // ACCUMULATOR_H
