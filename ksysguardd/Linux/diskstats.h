/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2006 Greg Martyn <greg.martyn@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 or later of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_DISKSTATS_H
#define KSG_DISKSTATS_H

void initDiskstats( struct SensorModul* );
void exitDiskstats( void );
int updateDiskstats( void );

void processDiskstats( void );

void print26DiskTotal( const char* );
void print26DiskTotalInfo( const char* );
void print26DiskRIO( const char* );
void print26DiskRIOInfo( const char* );
void print26DiskWIO( const char* );
void print26DiskWIOInfo( const char* );
void print26DiskRBlk( const char* );
void print26DiskRBlkInfo( const char* );
void print26DiskWBlk( const char* );
void print26DiskWBlkInfo( const char* );
void print26DiskIO( const char* );
void print26DiskIOInfo( const char* );


#endif
