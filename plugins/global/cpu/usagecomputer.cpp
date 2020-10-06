/*
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "usagecomputer.h"

void UsageComputer::setTicks(unsigned long long system, unsigned long long user, unsigned long long wait, unsigned long long idle)
{
    unsigned long long totalTicks = system + user + wait + idle;
    unsigned long long totalDiff = totalTicks - m_totalTicks;

    auto percentage =  [totalDiff] (unsigned long long tickDiff) {
        // according to the documentation some counters can go backwards in some circumstances
        return tickDiff > 0 ? 100.0 *  tickDiff / totalDiff : 0;
    };

    systemUsage = percentage(system - m_systemTicks);
    userUsage = percentage(user - m_userTicks);
    waitUsage = percentage(wait - m_waitTicks);
    totalUsage = percentage((system + user + wait) - (m_systemTicks + m_userTicks + m_waitTicks));

    m_totalTicks = totalTicks;
    m_systemTicks = system;
    m_userTicks = user;
    m_waitTicks = wait;
}
