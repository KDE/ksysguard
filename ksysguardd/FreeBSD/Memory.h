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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _memory_h_
#define _memory_h_

void initMemory(struct SensorModul* sm);
void exitMemory(void);

int updateMemory(void);

void printMFree(const char* cmd);
void printMFreeInfo(const char* cmd);
void printUsed(const char* cmd);
void printUsedInfo(const char* cmd);
void printBuffers(const char* cmd);
void printBuffersInfo(const char* cmd);
void printCached(const char* cmd);
void printCachedInfo(const char* cmd);
void printApplication(const char* cmd);
void printApplicationInfo(const char* cmd);
void printSwapUsed(const char* cmd);
void printSwapUsedInfo(const char* cmd);
void printSwapFree(const char* cmd);
void printSwapFreeInfo(const char* cmd);

#endif
