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

#ifndef KSG_DISKSTAT_H
#define KSG_DISKSTAT_H

void initDiskStat( struct SensorModul* );
void exitDiskStat( void );

int updateDiskStat( void );
void checkDiskStat( void );

void printDiskStat( const char* );
void printDiskStatInfo( const char* );

void printDiskStatUsed( const char* );
void printDiskStatUsedInfo( const char* );
void printDiskStatFree( const char* );
void printDiskStatFreeInfo( const char* );
void printDiskStatPercent( const char* );
void printDiskStatPercentInfo( const char* );
void printDiskStatTotal( const char* );
void printDiskStatTotalInfo( const char* );

#endif
