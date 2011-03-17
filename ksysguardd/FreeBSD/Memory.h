/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _memory_h_
#define _memory_h_

void initMemory(struct SensorModul* sm);
void exitMemory(void);

int updateMemory(void);

void printMActive(const char* cmd);
void printMActiveInfo(const char* cmd);
void printMInactive(const char* cmd);
void printMInactiveInfo(const char* cmd);
void printMApplication(const char* cmd);
void printMApplicationInfo(const char* cmd);
void printMWired(const char* cmd);
void printMWiredInfo(const char* cmd);
void printMCached(const char* cmd);
void printMCachedInfo(const char* cmd);
void printMBuffers(const char* cmd);
void printMBuffersInfo(const char* cmd);
void printMFree(const char* cmd);
void printMFreeInfo(const char* cmd);
void printMUsed(const char* cmd);
void printMUsedInfo(const char* cmd);

void printSwapUsed(const char* cmd);
void printSwapUsedInfo(const char* cmd);
void printSwapFree(const char* cmd);
void printSwapFreeInfo(const char* cmd);
void printSwapIn(const char* cmd);
void printSwapInInfo(const char* cmd);
void printSwapOut(const char* cmd);
void printSwapOutInfo(const char* cmd);

#endif
