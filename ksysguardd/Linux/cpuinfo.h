/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2000-2001 Chris Schlaeger <cs@kde.org>

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

#ifndef KSG_CPUINFO_H
#define KSG_CPUINFO_H

void initCpuInfo( struct SensorModul* );
void exitCpuInfo( void );

int updateCpuInfo( void );

void printCPUxClock( const char* );
void printCPUxClockInfo( const char* );

void printCPUClock( const char* );
void printCPUClockInfo( const char* );

void printNumCpus( const char* cmd );
void printNumCpusInfo( const char* cmd );

void printNumCores( const char* cmd );
void printNumCoresInfo( const char* cmd );

#endif
