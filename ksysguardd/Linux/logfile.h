/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef KSG_LOGFILE_H
#define KSG_LOGFILE_H

void initLogFile( struct SensorModul* );
void exitLogFile( void );

void printLogFile( const char* );
void printLogFileInfo( const char* );

void registerLogFile( const char* );
void unregisterLogFile( const char* );

/* debug command */
void printRegistered( const char* );

#endif
