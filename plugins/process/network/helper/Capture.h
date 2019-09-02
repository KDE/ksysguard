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

#ifndef CAPTURE_H
#define CAPTURE_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <atomic>

class pcap;
class Packet;

class Capture
{
public:
    Capture(const std::string &interface = std::string{});
    ~Capture();

    bool start();
    void stop();
    std::string lastError() const;
    void reportStatistics();
    Packet nextPacket();

    void handlePacket(const struct pcap_pkthdr *header, const uint8_t *data);

private:
    void loop();
    bool checkError(int result);

    std::string m_interface;
    std::string m_error;
    std::atomic_bool m_active;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::deque<Packet> m_queue;

    int m_packetCount = 0;

    pcap *m_pcap;
};

#endif // CAPTURE_H
