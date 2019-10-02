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

#include "Packet.h"

#include <sys/types.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <pcap/pcap.h>
#include <pcap/sll.h>

uint32_t u8Tou32(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
{
    return uint32_t(first) << 24 | uint32_t(second) << 16 | uint32_t(third) << 8 | uint32_t(fourth);
}

Packet::Packet()
{
}

Packet::Packet(const TimeStamp::MicroSeconds &timeStamp, const uint8_t *data, uint32_t dataLength, uint32_t packetSize)
{
    m_timeStamp = timeStamp;
    m_size = packetSize;

    const sll_header *header = reinterpret_cast<const sll_header *>(data);
    switch (ntohs(header->sll_protocol)) {
    case ETHERTYPE_IP:
        m_networkProtocol = NetworkProtocolType::IPv4;
        if (sizeof(sll_header) <= dataLength) {
            parseIPv4(data + sizeof(sll_header), dataLength - sizeof(sll_header));
        }
        break;
    case ETHERTYPE_IPV6:
        m_networkProtocol = NetworkProtocolType::IPv6;
        if (sizeof(sll_header) <= dataLength) {
            parseIPv6(data + sizeof(sll_header), dataLength - sizeof(sll_header));
        }
        break;
    default:
        m_networkProtocol = NetworkProtocolType::Unknown;
        break;
    }
}

Packet::~Packet()
{
}

unsigned int Packet::size() const
{
    return m_size;
}

TimeStamp::MicroSeconds Packet::timeStamp() const
{
    return m_timeStamp;
}

Packet::NetworkProtocolType Packet::networkProtocol() const
{
    return m_networkProtocol;
}

Packet::TransportProtocolType Packet::transportProtocol() const
{
    return m_transportProtocol;
}

Packet::Address Packet::sourceAddress() const
{
    return m_sourceAddress;
}

Packet::Address Packet::destinationAddress() const
{
    return m_destinationAddress;
}

void Packet::parseIPv4(const uint8_t *data, int32_t dataLength)
{
    if (dataLength < int32_t(sizeof(ip))) {
        return;
    }

    const ip *header = reinterpret_cast<const ip *>(data);

    m_sourceAddress.address[3] = header->ip_src.s_addr;
    m_destinationAddress.address[3] = header->ip_dst.s_addr;

    parseTransport(header->ip_p, data + sizeof(ip), dataLength - sizeof(ip));
}

void Packet::parseIPv6(const uint8_t *data, int32_t dataLength)
{
    if (dataLength < int32_t(sizeof(ip6_hdr))) {
        return;
    }

    const ip6_hdr *header = reinterpret_cast<const ip6_hdr *>(data);

    m_sourceAddress.address = {
        u8Tou32(header->ip6_src.s6_addr[0], header->ip6_src.s6_addr[1], header->ip6_src.s6_addr[2], header->ip6_src.s6_addr[3]),
        u8Tou32(header->ip6_src.s6_addr[4], header->ip6_src.s6_addr[5], header->ip6_src.s6_addr[6], header->ip6_src.s6_addr[7]),
        u8Tou32(header->ip6_src.s6_addr[8], header->ip6_src.s6_addr[9], header->ip6_src.s6_addr[10], header->ip6_src.s6_addr[11]),
        u8Tou32(header->ip6_src.s6_addr[12], header->ip6_src.s6_addr[13], header->ip6_src.s6_addr[14], header->ip6_src.s6_addr[15])
    };
    m_destinationAddress.address = {
        u8Tou32(header->ip6_dst.s6_addr[0], header->ip6_dst.s6_addr[1], header->ip6_dst.s6_addr[2], header->ip6_dst.s6_addr[3]),
        u8Tou32(header->ip6_dst.s6_addr[4], header->ip6_dst.s6_addr[5], header->ip6_dst.s6_addr[6], header->ip6_dst.s6_addr[7]),
        u8Tou32(header->ip6_dst.s6_addr[8], header->ip6_dst.s6_addr[9], header->ip6_dst.s6_addr[10], header->ip6_dst.s6_addr[11]),
        u8Tou32(header->ip6_dst.s6_addr[12], header->ip6_dst.s6_addr[13], header->ip6_dst.s6_addr[14], header->ip6_dst.s6_addr[15])
    };

    parseTransport(header->ip6_nxt, data + sizeof(ip6_hdr), dataLength - sizeof(ip6_hdr));
}

void Packet::parseTransport(uint8_t type, const uint8_t *data, int32_t dataLength)
{
    switch (type) {
    case IPPROTO_TCP: {
        m_transportProtocol = TransportProtocolType::Tcp;
        if (dataLength >= int32_t(sizeof(tcphdr))) {
            const tcphdr *tcpHeader = reinterpret_cast<const tcphdr *>(data);
            m_sourceAddress.port = ntohs(tcpHeader->th_sport);
            m_destinationAddress.port = ntohs(tcpHeader->th_dport);
        }
        break;
    }
    case IPPROTO_UDP: {
        m_transportProtocol = TransportProtocolType::Udp;
        if (dataLength >= int32_t(sizeof(udphdr))) {
            const udphdr *udpHeader = reinterpret_cast<const udphdr *>(data);
            m_sourceAddress.port = ntohs(udpHeader->uh_sport);
            m_destinationAddress.port = ntohs(udpHeader->uh_dport);
        }
        break;
    }
    default:
        m_transportProtocol = TransportProtocolType::Unknown;
        break;
    }
}
