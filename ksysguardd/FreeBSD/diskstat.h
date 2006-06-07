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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _diskstat_h_
#define _diskstat_h_

void initDiskStat(struct SensorModul* sm);
void exitDiskStat(void);

int updateDiskStat(void);
void checkDiskStat(void);

void printDiskStat(const char* cmd);
void printDiskStatInfo(const char* cmd);

void printDiskStatUsed(const char* cmd);
void printDiskStatUsedInfo(const char* cmd);
void printDiskStatFree(const char* cmd);
void printDiskStatFreeInfo(const char* cmd);
void printDiskStatPercent(const char* cmd);
void printDiskStatPercentInfo(const char* cmd);

#endif
