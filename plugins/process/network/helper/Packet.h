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

#ifndef PACKET_H
#define PACKET_H

#include <chrono>
#include <cstdint>
#include <array>
#include <functional>

#include "TimeStamps.h"

class Packet
{
public:
    enum class NetworkProtocolType {
        Unknown,
        IPv4,
        IPv6,
    };

    enum class TransportProtocolType {
        Unknown,
        Tcp,
        Udp,
    };

    enum class Direction {
        Inbound,
        Outbound,
    };

    struct Address
    {
        std::array<uint32_t, 4> address = { 0 };
        uint32_t port = 0;

        inline bool operator==(const Address &other) const
        {
            return address == other.address
                   && port == other.port;
        }
    };

    Packet();

    Packet(const TimeStamp::MicroSeconds &timeStamp, const uint8_t *data, uint32_t dataLength, uint32_t packetSize);

    ~Packet();

    Packet(const Packet &other) = delete;
    Packet(Packet &&other) = default;

    TimeStamp::MicroSeconds timeStamp() const;
    unsigned int size() const;
    NetworkProtocolType networkProtocol() const;
    TransportProtocolType transportProtocol() const;
    Address sourceAddress() const;
    Address destinationAddress() const;

private:
    void parseIPv4(const uint8_t *data);
    void parseIPv6(const uint8_t *data);
    void parseTransport(uint8_t type, const uint8_t *data);

    TimeStamp::MicroSeconds m_timeStamp;
    unsigned int m_size = 0;

    NetworkProtocolType m_networkProtocol = NetworkProtocolType::Unknown;
    TransportProtocolType m_transportProtocol = TransportProtocolType::Unknown;

    Address m_sourceAddress;
    Address m_destinationAddress;
};

namespace std {
    template <> struct hash<Packet::Address>
    {
        using argument_type = Packet::Address;
        using result_type = std::size_t;
        inline result_type operator()(argument_type const& address) const noexcept {
            return std::hash<uint32_t>{}(address.address[0])
                   ^ (std::hash<uint32_t>{}(address.address[1]) << 1)
                   ^ (std::hash<uint32_t>{}(address.address[2]) << 2)
                   ^ (std::hash<uint32_t>{}(address.address[3]) << 3)
                   ^ (std::hash<uint32_t>{}(address.port) << 4);
        }
    };
}

#endif // PACKET_H
