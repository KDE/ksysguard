/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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

#ifndef KSG_MEMORY_H
#define KSG_MEMORY_H

void initMemory( struct SensorModul* );
void exitMemory( void );

int updateMemory( void );

void printMFree( const char* );
void printMFreeInfo( const char* );
void printUsed( const char* );
void printUsedInfo( const char* );
void printAppl( const char* );
void printApplInfo( const char* );
void printBuffers( const char* );
void printBuffersInfo( const char* );
void printCached( const char* );
void printCachedInfo( const char* );
void printSwapUsed( const char* );
void printSwapUsedInfo( const char* );
void printSwapFree( const char* );
void printSwapFreeInfo( const char* );

#endif
