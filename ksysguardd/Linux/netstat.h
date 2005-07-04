/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _netstat_h_
#define _netstat_h_

void initNetStat(struct SensorModul* sm);
void exitNetStat(void);

int updateNetStat(void);
int updateNetStatTcpUdpRaw(const char* cmd);
int updateNetStatUnix(void);

void printNetStat(const char* cmd);
void printNetStatInfo(const char* cmd);

void printNetStatTcpUdpRaw(const char *cmd);
void printNetStatTcpUdpRawInfo(const char *cmd);

void printNetStatUnix(const char *cmd);
void printNetStatUnixInfo(const char *cmd);
#endif
