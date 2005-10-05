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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"

/* Stop <sys/swap.h> from crapping out on 32-bit architectures. */

#if !defined(_LP64) && _FILE_OFFSET_BITS == 64
# undef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS 32
#endif

#include <sys/stat.h>
#include <sys/swap.h>
#include <vm/anon.h>

#ifdef HAVE_KSTAT
#include <kstat.h>
#endif

#include "ksysguardd.h"
#include "Command.h"
#include "Memory.h"

static int Dirty = 1;
static t_memsize totalmem = (t_memsize) 0;
static t_memsize freemem = (t_memsize) 0;
static long totalswap = 0L;
static long freeswap = 0L;
static struct anoninfo am_swap;

/*
 *  this is borrowed from top's m_sunos5 module
 *  used by permission from William LeFebvre
 */
static int pageshift;
static long (*p_pagetok) ();
#define pagetok(size) ((*p_pagetok)(size))

long pagetok_none( long size ) {
	return( size );
}

long pagetok_left( long size ) {
	return( size << pageshift );
}

long pagetok_right( long size ) {
	return( size >> pageshift );
}

void initMemory( struct SensorModul* sm ) {

	long i = sysconf( _SC_PAGESIZE );

	pageshift = 0;
	while( (i >>= 1) > 0 )
		pageshift++;

	/* calculate an amount to shift to K values */
	/* remember that log base 2 of 1024 is 10 (i.e.: 2^10 = 1024) */
	pageshift -= 10;

	/* now determine which pageshift function is appropriate for the 
	result (have to because x << y is undefined for y < 0) */
	if( pageshift > 0 ) {
		/* this is the most likely */
		p_pagetok = pagetok_left;
	} else if( pageshift == 0 ) {
		p_pagetok = pagetok_none;
	} else {
		p_pagetok = pagetok_right;
		pageshift = -pageshift;
	}

#ifdef HAVE_KSTAT
	registerMonitor( "mem/physical/free", "integer",
					printMemFree, printMemFreeInfo, sm );
	registerMonitor( "mem/physical/used", "integer",
					printMemUsed, printMemUsedInfo, sm );
#endif
	registerMonitor( "mem/swap/free", "integer",
					printSwapFree, printSwapFreeInfo, sm );
	registerMonitor( "mem/swap/used", "integer",
					printSwapUsed, printSwapUsedInfo, sm );
}

void exitMemory( void ) {
}

int updateMemory( void ) {

	long			swaptotal;
	long			swapfree;
	long			swapused;
#ifdef HAVE_KSTAT
	kstat_ctl_t		*kctl;
	kstat_t			*ksp;
	kstat_named_t		*kdata;
#endif /* HAVE_KSTAT */
	swaptotal = swapused = swapfree = 0L;

	/*
	 *  Retrieve overall swap information from anonymous memory structure -
	 *  which is the same way "swap -s" retrieves it's statistics.
	 *
	 *  swapctl(SC_LIST, void *arg) does not return what we are looking for.
	 */

	if (swapctl(SC_AINFO, &am_swap) == -1)
		return(0);

	swaptotal = am_swap.ani_max;
	swapused = am_swap.ani_resv;
	swapfree = swaptotal - swapused;

	totalswap = pagetok(swaptotal);
	freeswap = pagetok(swapfree);

#ifdef HAVE_KSTAT
	/*
	 *  get a kstat handle and update the user's kstat chain
	 */
	if( (kctl = kstat_open()) == NULL )
		return( 0 );
	while( kstat_chain_update( kctl ) != 0 )
		;

	totalmem = pagetok( sysconf( _SC_PHYS_PAGES ));

	/*
	 *  traverse the kstat chain to find the appropriate statistics
	 */
	if( (ksp = kstat_lookup( kctl, "unix", 0, "system_pages" )) == NULL )
		return( 0 );
	if( kstat_read( kctl, ksp, NULL ) == -1 )
		return( 0 );

	/*
	 *  lookup the data
	 */
	 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "freemem" );
	 if( kdata != NULL )
	 	freemem = pagetok( kdata->value.ui32 );

	kstat_close( kctl );
#endif /* ! HAVE_KSTAT */

	Dirty = 0;

	return( 0 );
}

void printMemFreeInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Free Memory\t0\t%ld\tKB\n", totalmem );
}

void printMemFree( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", freemem );
}

void printMemUsedInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Used Memory\t0\t%ld\tKB\n", totalmem );
}

void printMemUsed( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", totalmem - freemem );
}

void printSwapFreeInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Free Swap\t0\t%ld\tKB\n", totalswap );
}

void printSwapFree( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", freeswap );
}

void printSwapUsedInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Used Swap\t0\t%ld\tKB\n", totalswap );
}

void printSwapUsed( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", totalswap - freeswap );
}
