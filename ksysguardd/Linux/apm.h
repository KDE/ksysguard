/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_APM_H
#define KSG_APM_H

void initApm( struct SensorModul* );
void exitApm( void );

int updateApm( void );

void printApmBatFill( const char* );
void printApmBatFillInfo( const char* );
void printApmBatTime( const char* );
void printApmBatTimeInfo( const char* );

#endif
