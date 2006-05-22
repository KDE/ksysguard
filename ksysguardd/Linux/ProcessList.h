/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2000 Chris Schlaeger <cs@kde.org>

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

#ifndef KSG_PROCESSLIST_H
#define KSG_PROCESSLIST_H

#include "config-ksysguardd.h" /*For HAVE_XRES*/

void initProcessList( struct SensorModul* );
void exitProcessList( void );

int updateProcessList( void );

void printProcessList( const char* );
void printProcessListInfo( const char* );
void printProcessCount( const char* );
void printProcessCountInfo( const char* );

#ifdef HAVE_XRES
void printXresListInfo( const char *);
void printXresList( const char *);
#endif

void killProcess( const char* );
void setPriority( const char* );

#endif
