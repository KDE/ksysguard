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

#ifndef KSG_NETDEV_H
#define KSG_NETDEV_H

void initNetDev( struct SensorModul* );
void exitNetDev( void );

int updateNetDev( void );
void checkNetDev( void );

void printNetDevRecBytes( const char* );
void printNetDevRecBytesInfo( const char* );
void printNetDevSentBytes( const char* );
void printNetDevSentBytesInfo( const char* );

#endif
