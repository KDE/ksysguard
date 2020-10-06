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
#ifndef USAGECOMPUTER_H
#define USAGECOMPUTER_H

// Helper class to calculate usage percentage values from ticks
class UsageComputer {
public:
    void setTicks(unsigned long long system, unsigned long long  user, unsigned long long wait, unsigned long long idle);
    double totalUsage = 0;
    double systemUsage = 0;
    double userUsage = 0;
    double waitUsage = 0;
private:
    unsigned long long m_totalTicks = 0;
    unsigned long long m_systemTicks = 0;
    unsigned long long m_userTicks = 0;
    unsigned long long m_waitTicks = 0;
};

#endif
