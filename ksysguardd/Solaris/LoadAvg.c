/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

	Solaris support by Torsten Kasch <tk@Genetik.Uni-Bielefeld.DE>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/swap.h>

#include "config.h"

#ifdef HAVE_KSTAT
#include <kstat.h>
#endif

#include "ksysguardd.h"
#include "Command.h"
#include "LoadAvg.h"

double loadavg1 = 0.0;
double loadavg5 = 0.0;
double loadavg15 = 0.0;

void initLoadAvg( struct SensorModul* sm ) {
#ifdef HAVE_KSTAT
	registerMonitor( "cpu/loadavg1", "float",
					printLoadAvg1, printLoadAvg1Info, sm );
	registerMonitor( "cpu/loadavg5", "float",
					printLoadAvg5, printLoadAvg5Info, sm );
	registerMonitor( "cpu/loadavg15", "float",
					printLoadAvg15, printLoadAvg15Info, sm );
#endif
}

void exitLoadAvg( void ) {
}

int updateLoadAvg( void ) {

#ifdef HAVE_KSTAT
	kstat_ctl_t		*kctl;
	kstat_t			*ksp;
	kstat_named_t		*kdata;

	/*
	 *  get a kstat handle and update the user's kstat chain
	 */
	if( (kctl = kstat_open()) == NULL )
		return( 0 );
	while( kstat_chain_update( kctl ) != 0 )
		;

	/*
	 *  traverse the kstat chain to find the appropriate statistics
	 */
	if( (ksp = kstat_lookup( kctl, "unix", 0, "system_misc" )) == NULL )
		return( 0 );
	if( kstat_read( kctl, ksp, NULL ) == -1 )
		return( 0 );

	/*
	 *  lookup the data
	 */
	 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "avenrun_1min" );
	 if( kdata != NULL )
	 	loadavg1 = LOAD( kdata->value.ui32 );
	 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "avenrun_5min" );
	 if( kdata != NULL )
	 	loadavg5 = LOAD( kdata->value.ui32 );
	 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "avenrun_15min" );
	 if( kdata != NULL )
	 	loadavg15 = LOAD( kdata->value.ui32 );

	kstat_close( kctl );
#endif /* ! HAVE_KSTAT */

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
