/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef KSG_STAT_H
#define KSG_STAT_H

void initStat( struct SensorModul* );
void exitStat( void );

int updateStat( void );

void printCPUUser( const char* );
void printCPUUserInfo( const char* );
void printCPUNice( const char* );
void printCPUNiceInfo( const char* );
void printCPUSys( const char* );
void printCPUSysInfo( const char* );
void printCPUIdle( const char* );
void printCPUIdleInfo( const char* );
void printCPUxUser( const char* );
void printCPUxUserInfo( const char* );
void printCPUxNice( const char* );
void printCPUxNiceInfo( const char* );
void printCPUxSys( const char* );
void printCPUxSysInfo( const char* );
void printCPUxIdle( const char* );
void printCPUxIdleInfo( const char* );
void printDiskTotal( const char* );
void printDiskTotalInfo( const char* );
void printDiskRIO( const char* );
void printDiskRIOInfo( const char* );
void printDiskWIO( const char* );
void printDiskWIOInfo( const char* );
void printDiskRBlk( const char* );
void printDiskRBlkInfo( const char* );
void printDiskWBlk( const char* );
void printDiskWBlkInfo( const char* );
void printPageIn( const char* );
void printPageInInfo( const char* );
void printPageOut( const char* );
void printPageOutInfo( const char* );
void printInterruptx( const char* );
void printInterruptxInfo( const char* );
void printCtxt( const char* );
void printCtxtInfo( const char* );
void printDiskIO( const char* );
void printDiskIOInfo( const char* );

#endif
