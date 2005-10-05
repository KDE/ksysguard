/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

	Irix support by Carsten Kroll <CKroll@pinnaclesys.com>
    
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/swap.h>

#include "config.h"

#include "ksysguardd.h"
#include "Command.h"
#include "LoadAvg.h"

double loadavg1 = 0.0;
double loadavg5 = 0.0;
double loadavg15 = 0.0;

void initLoadAvg(struct SensorModul* sm ) {
	registerMonitor( "cpu/loadavg1", "float",
					printLoadAvg1, printLoadAvg1Info, sm );
	registerMonitor( "cpu/loadavg5", "float",
					printLoadAvg5, printLoadAvg5Info, sm );
	registerMonitor( "cpu/loadavg15", "float",
					printLoadAvg15, printLoadAvg15Info, sm );
}

void exitLoadAvg( void ) {
}

int updateLoadAvg( void ) {

	return( 0 );
}

void printLoadAvg1Info( const char *cmd ) {
	fprintf(CurrentClient, "avnrun 1min\t0\t0\n" );
}

void printLoadAvg1( const char *cmd ) {
	fprintf(CurrentClient, "%f\n", loadavg1 );
}

void printLoadAvg5Info( const char *cmd ) {
	fprintf(CurrentClient, "avnrun 5min\t0\t0\n" );
}

void printLoadAvg5( const char *cmd ) {
	fprintf(CurrentClient, "%f\n", loadavg5 );
}

void printLoadAvg15Info( const char *cmd ) {
	fprintf(CurrentClient, "avnrun 15min\t0\t0\n" );
}

void printLoadAvg15( const char *cmd ) {
	fprintf(CurrentClient, "%f\n", loadavg15 );
}
