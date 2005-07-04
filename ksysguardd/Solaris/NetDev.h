/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>

	Solaris support by Torsten Kasch <tk@Genetik.Uni-Bielefeld.DE>
    
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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _NetDev_h_
#define _NetDev_h_

void initNetDev(struct SensorModul* sm);
void exitNetDev(void);

int updateNetDev(void);

void printIPacketsInfo( const char *cmd );
void printIPackets( const char *cmd );

void printOPacketsInfo( const char *cmd );
void printOPackets( const char *cmd );

void printIErrorsInfo( const char *cmd );
void printIErrors( const char *cmd );

void printOErrorsInfo( const char *cmd );
void printOErrors( const char *cmd );

void printCollisionsInfo( const char *cmd );
void printCollisions( const char *cmd );

void printMultiXmitsInfo( const char *cmd );
void printMultiXmits( const char *cmd );

void printMultiRecvsInfo( const char *cmd );
void printMultiRecvs( const char *cmd );

void printBcastXmitsInfo( const char *cmd );
void printBcastXmits( const char *cmd );

void printBcastRecvsInfo( const char *cmd );
void printBcastRecvs( const char *cmd );

#endif /* _NetDev_h */
