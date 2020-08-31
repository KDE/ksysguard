/*
 * This file is part of KSysGuard.
 * Copyright 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright 2020 David Redondo <kde@david-redondo.de>
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

#include "ConnectionMapping.h"

#include <fstream>
#include <iostream>

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/inet_diag.h>
#include <linux/sock_diag.h>

#include <netlink/msg.h>
#include <netlink/netlink.h>

using namespace std::string_literals;

// Convert /proc/net/tcp's mangled big-endian notation to a host-endian int32'
uint32_t tcpToInt(const std::string &part)
{
    uint32_t result = 0;
    result |= std::stoi(part.substr(0, 2), 0, 16) << 24;
    result |= std::stoi(part.substr(2, 2), 0, 16) << 16;
    result |= std::stoi(part.substr(4, 2), 0, 16) << 8;
    result |= std::stoi(part.substr(6, 2), 0, 16) << 0;
    return result;
}

int parseInetDiagMesg(struct nl_msg *msg, void *arg) 
{
    auto self = static_cast<ConnectionMapping*>(arg);
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    auto inetDiagMsg = static_cast<inet_diag_msg*>(nlmsg_data(nlh));
    Packet::Address localAddress;
    if (inetDiagMsg->idiag_family == AF_INET) {
        // I expected to need ntohl calls here and bewlow for src but comparing to values gathered
        // by parsing proc they are not needed and even produce wrong results
        localAddress.address[3] = inetDiagMsg->id.idiag_src[0];
    } else if (inetDiagMsg->id.idiag_src[0] == 0 && inetDiagMsg->id.idiag_src[1] == 0 &&  inetDiagMsg->id.idiag_src[2] == 0xffff0000) {
         // Some applications (like Steam) use ipv6 sockets with ipv4.
        // This results in ipv4 addresses that end up in the tcp6 file.
        // They seem to start with 0000000000000000FFFF0000, so if we
        // detect that, assume it is ipv4-over-ipv6.
        localAddress.address[3] = inetDiagMsg->id.idiag_src[3];

    } else {
        std::memcpy(localAddress.address.data(), inetDiagMsg->id.idiag_src, sizeof(localAddress.address));
    }
    localAddress.port = ntohs(inetDiagMsg->id.idiag_sport);
    self->m_localToINode[localAddress] = inetDiagMsg->idiag_inode;
    self->m_inodes.insert(inetDiagMsg->idiag_inode);
    return NL_OK;
}

ConnectionMapping::ConnectionMapping()
    : m_socket(nl_socket_alloc(), nl_socket_free)
{
    m_socketFileMatch =
        // Format of /proc/net/tcp is:
        //  sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
        //   0: 017AA8C0:0035 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 31896 ...
        // Where local_address is a hex representation of the IP Address and port, in big endian notation.
        // Since we care only about local address, local port and inode we ignore the middle 70 characters.
        std::regex("\\s*\\d+: (?:(\\w{8})|(\\w{32})):([A-F0-9]{4}) (.{94}|.{70}) (\\d+) .*", std::regex::ECMAScript | std::regex::optimize);

    nl_connect(m_socket.get(), NETLINK_SOCK_DIAG);
    nl_socket_modify_cb(m_socket.get(), NL_CB_VALID, NL_CB_CUSTOM, &parseInetDiagMesg, this);
    getSocketInfo();
}

ConnectionMapping::PacketResult ConnectionMapping::pidForPacket(const Packet &packet)
{
    PacketResult result;

    auto sourceInode = m_localToINode.find(packet.sourceAddress());
    auto destInode = m_localToINode.find(packet.destinationAddress());

    if (sourceInode == m_localToINode.end() && destInode == m_localToINode.end()) {
        getSocketInfo();

        sourceInode = m_localToINode.find(packet.sourceAddress());
        destInode = m_localToINode.find(packet.destinationAddress());

        if (sourceInode == m_localToINode.end() && destInode == m_localToINode.end()) {
            return result;
        }
    }

    auto inode = m_localToINode.end();
    if (sourceInode != m_localToINode.end()) {
        result.direction = Packet::Direction::Outbound;
        inode = sourceInode;
    } else {
        result.direction = Packet::Direction::Inbound;
        inode = destInode;
    }

    auto pid = m_inodeToPid.find((*inode).second);
    if (pid == m_inodeToPid.end()) {
        result.pid = -1;
    } else {
        result.pid = (*pid).second;
    }
    return result;
}

void ConnectionMapping::getSocketInfo()
{
    auto oldInodes = m_inodes;
    m_inodes.clear();
    m_localToINode.clear();

    if (!dumpSockets()) {
        // There was some error with sock_diag
        parseSockets();
    }
    if (m_inodes != oldInodes) {
        parsePid();
    }
}

bool ConnectionMapping::dumpSockets()
{
    for (auto family : {AF_INET, AF_INET6}) {
        for (auto protocol : {IPPROTO_TCP, IPPROTO_UDP}) {
            if (!dumpSockets(family, protocol)) {
                return false;
            }
        }
    }
    return true;
}

bool ConnectionMapping::dumpSockets(int inet_family, int protocol)
{
    inet_diag_req_v2 inet_request;
    inet_request.id = {};
    inet_request.sdiag_family = inet_family;
    inet_request.sdiag_protocol = protocol;
    inet_request.idiag_states = -1;
    if (nl_send_simple(m_socket.get(), SOCK_DIAG_BY_FAMILY, NLM_F_DUMP | NLM_F_REQUEST, &inet_request, sizeof(inet_diag_req_v2)) < 0) {
        return false;
    }
    if (nl_recvmsgs_default(m_socket.get()) != 0) {
        return false;
    }
    return true;
}


void ConnectionMapping::parseSockets()
{
    parseSocketFile("/proc/net/tcp");
    parseSocketFile("/proc/net/udp");
    parseSocketFile("/proc/net/tcp6");
    parseSocketFile("/proc/net/udp6");
}

void ConnectionMapping::parsePid()
{
    std::unordered_set<int> pids;

    auto dir = opendir("/proc");
    dirent *entry = nullptr;
    while ((entry = readdir(dir)))
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9')
            pids.insert(std::stoi(entry->d_name));
    closedir(dir);

    char buffer[100] = { "\0" };
    m_inodeToPid.clear();
    for (auto pid : pids) {
        auto fdPath = "/proc/%/fd"s.replace(6, 1, std::to_string(pid));
        auto dir = opendir(fdPath.data());
        if (dir == NULL) {
            continue;
        }

        dirent *fd = nullptr;
        while ((fd = readdir(dir))) {
            memset(buffer, 0, 100);
            readlinkat(dirfd(dir), fd->d_name, buffer, 99);
            auto target = std::string(buffer);
            if (target.find("socket:") == std::string::npos)
                continue;

            auto inode = std::stoi(target.substr(8));
            m_inodeToPid.insert(std::make_pair(inode, pid));
        }

        closedir(dir);
    }
}

void ConnectionMapping::parseSocketFile(const char *fileName)
{
    std::ifstream file { fileName };
    if (!file.is_open())
        return;

    std::string data;
    while (std::getline(file, data)) {
        std::smatch match;
        if (!std::regex_match(data, match, m_socketFileMatch)) {
            continue;
        }

        Packet::Address localAddress;
        if (!match.str(1).empty()) {
            localAddress.address[3] = tcpToInt(match.str(1));
        } else {
            auto ipv6 = match.str(2);
            if (ipv6.compare(0, 24, "0000000000000000FFFF0000") == 0) {
                // Some applications (like Steam) use ipv6 sockets with ipv4.
                // This results in ipv4 addresses that end up in the tcp6 file.
                // They seem to start with 0000000000000000FFFF0000, so if we
                // detect that, assume it is ipv4-over-ipv6.
                localAddress.address[3] = tcpToInt(ipv6.substr(24,8));
            } else {
                localAddress.address[0] = tcpToInt(ipv6.substr(0, 8));
                localAddress.address[1] = tcpToInt(ipv6.substr(8, 8));
                localAddress.address[2] = tcpToInt(ipv6.substr(16, 8));
                localAddress.address[3] = tcpToInt(ipv6.substr(24, 8));
            }
        }

        localAddress.port = std::stoi(match.str(3), 0, 16);
        auto inode = std::stoi(match.str(5));
        m_localToINode.insert(std::make_pair(localAddress, inode));
        m_inodes.insert(inode);
    }
}
