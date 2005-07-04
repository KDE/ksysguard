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

#ifndef KSG_I8K_H
#define KSG_I8K_H

void initI8k( struct SensorModul* );
void exitI8k( void );

int updateI8k( void );

void printI8kCPUTemperature( const char* );
void printI8kCPUTemperatureInfo( const char* );
void printI8kFan0Speed( const char* );
void printI8kFan0SpeedInfo( const char* );
void printI8kFan1Speed( const char* );
void printI8kFan1SpeedInfo( const char* );

#endif
