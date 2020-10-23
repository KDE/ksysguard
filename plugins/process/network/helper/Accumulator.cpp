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

#include "Accumulator.h"

#include "Capture.h"
#include "ConnectionMapping.h"

using namespace std::chrono_literals;

Accumulator::Accumulator(std::shared_ptr<Capture> capture, std::shared_ptr<ConnectionMapping> mapping)
{
    m_capture = capture;
    m_mapping = mapping;

    m_running = true;
    m_thread = std::thread { &Accumulator::loop, this };
}

Accumulator::PidDataCounterHash Accumulator::data()
{
    std::lock_guard<std::mutex> lock{m_mutex};

    auto tmp = m_data;

    auto toErase = std::vector<int>{};
    for (auto &entry : m_data) {
        if (entry.second.first == 0 && entry.second.second == 0) {
            toErase.push_back(entry.first);
        } else {
            entry.second.first = 0;
            entry.second.second = 0;
        }
    }

    std::for_each(toErase.cbegin(), toErase.cend(), [this](int pid) { m_data.erase(pid); });

    return tmp;
}

void Accumulator::stop()
{
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Accumulator::loop()
{
    while (m_running) {
        auto packet = m_capture->nextPacket();

        auto result = m_mapping->pidForPacket(packet);
        if (result.pid == 0)
            continue;

        addData(result.direction, packet, result.pid);
    }
}

void Accumulator::addData(Packet::Direction direction, const Packet &packet, int pid)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    auto itr = m_data.find(pid);
    if (itr == m_data.end()) {
        m_data.emplace(pid, InboundOutboundData{0, 0});
    }

    if (direction == Packet::Direction::Inbound) {
        m_data[pid].first += packet.size();
    } else {
        m_data[pid].second += packet.size();
    };
}
