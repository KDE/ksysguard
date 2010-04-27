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

/* Stop <sys/swap.h> from crapping out on 32-bit architectures. */

#if !defined(_LP64) && _FILE_OFFSET_BITS == 64
# undef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS 32
#endif

#include <sys/stat.h>
#include <sys/swap.h>
#include <vm/anon.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

#ifdef HAVE_KSTAT
#include <kstat.h>

static uint64_t swap_resv = 0;
static uint64_t swap_free = 0;
static uint64_t swap_avail = 0;
static uint64_t swap_alloc = 0;
#endif

#include "ksysguardd.h"
#include "Command.h"
#include "Memory.h"

static t_memsize totalmem = (t_memsize) 0;
static t_memsize freemem = (t_memsize) 0;
static unsigned long usedswap = 0L;
static unsigned long freeswap = 0L;
static struct anoninfo am_swap;
static struct timeval lastSampling;

/*
 *  this is borrowed from top's m_sunos5 module
 *  used by permission from William LeFebvre
 */
static int pageshift;
static unsigned long (*p_pagetok) ();
#define pagetok(size) ((*p_pagetok)(size))

unsigned long
pagetok_none( unsigned long size ) {
	return( size );
}

unsigned long
pagetok_left( unsigned long size ) {
	return( size << pageshift );
}

unsigned long
pagetok_right( unsigned long size ) {
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
	registerMonitor( "mem/physical/application", "integer",
					printMemUsed, printMemUsedInfo, sm );
#endif
	registerMonitor( "mem/swap/free", "integer",
					printSwapFree, printSwapFreeInfo, sm );
	registerMonitor( "mem/swap/used", "integer",
					printSwapUsed, printSwapUsedInfo, sm );

	gettimeofday(&lastSampling, 0);
	updateMemory();
	updateSwap(0);
}

void exitMemory( void ) {
}

int updateMemory( void ) {
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

	totalmem = pagetok( sysconf( _SC_PHYS_PAGES ));

	/*
	 *  traverse the kstat chain to find the appropriate statistics
	 */
	if( (ksp = kstat_lookup( kctl, "unix", 0, "system_pages" )) == NULL ) {
		goto done;
	}

	if( kstat_read( kctl, ksp, NULL ) == -1 ) {
		goto done;
	}

	/*
	 *  lookup the data
	 */
	 kdata = (kstat_named_t *) kstat_data_lookup( ksp, "freemem" );
	 if( kdata != NULL )
		freemem = pagetok( kdata->value.ui32 );

done:
	kstat_close( kctl );
#endif /* ! HAVE_KSTAT */

	return( 0 );
}

int updateSwap( int upd ) {
	long			swaptotal;
	long			swapfree;
	long			swapused;
#ifdef HAVE_KSTAT
	kstat_ctl_t		*kctl;
	kstat_t			*ksp;
	struct timeval 		sampling;
	vminfo_t		vmi;

	uint64_t _swap_resv = 0;
	uint64_t _swap_free = 0;
	uint64_t _swap_avail = 0;
	uint64_t _swap_alloc = 0;

#endif /* HAVE_KSTAT */
	swaptotal = swapused = swapfree = 0L;

#ifndef HAVE_KSTAT
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

	usedswap = pagetok(swapused);
	freeswap = pagetok(swapfree);

#else /* HAVE_KSTAT */
	/*
	 *  get a kstat handle and update the user's kstat chain
	 */
	if( (kctl = kstat_open()) == NULL )
		return( 0 );
	while( kstat_chain_update( kctl ) != 0 )
		;

	totalmem = pagetok( sysconf( _SC_PHYS_PAGES ));

	if( (ksp = kstat_lookup( kctl, "unix", -1, "vminfo" )) == NULL ) {
		goto done;
	}

	if( kstat_read( kctl, ksp, &vmi ) == -1 ) {
                goto done;
        }

	_swap_resv = vmi.swap_resv - swap_resv;
	if (_swap_resv)
		swap_resv = vmi.swap_resv;

	_swap_free = vmi.swap_free - swap_free;
	if (_swap_free)
		swap_free = vmi.swap_free;

	_swap_avail = vmi.swap_avail - swap_avail;
	if (_swap_avail)
		swap_avail = vmi.swap_avail;

	_swap_alloc = vmi.swap_alloc - swap_alloc;
	if (_swap_alloc)
		swap_alloc = vmi.swap_alloc;

	if (upd) {
		long timeInterval;

		gettimeofday(&sampling, 0);
		timeInterval = sampling.tv_sec - lastSampling.tv_sec +
		    ( sampling.tv_usec - lastSampling.tv_usec ) / 1000000.0;
		lastSampling = sampling;
		timeInterval = timeInterval > 0 ? timeInterval : 1;

		_swap_resv = _swap_resv / timeInterval;
		_swap_free = _swap_free / timeInterval;
		_swap_avail = _swap_avail / timeInterval;
		_swap_alloc = _swap_alloc / timeInterval;

		if (_swap_alloc)
			usedswap = pagetok((unsigned long)(_swap_alloc + _swap_free - _swap_avail));

		/*
		 * Assume minfree = totalmem / 8, i.e. it has not been tuned
		 */
		if (_swap_avail)
			freeswap = pagetok((unsigned long)(_swap_avail + totalmem / 8));
	}
done:
	kstat_close( kctl );
#endif /* ! HAVE_KSTAT */

	return( 0 );
}
void printMemFreeInfo( const char *cmd ) {
	fprintf(CurrentClient, "Free Memory\t0\t%lu\tKB\n", totalmem );
}

void printMemFree( const char *cmd ) {
	updateMemory();
	fprintf(CurrentClient, "%lu\n", freemem );
}

void printMemUsedInfo( const char *cmd ) {
	fprintf(CurrentClient, "Used Memory\t0\t%lu\tKB\n", totalmem );
}

void printMemUsed( const char *cmd ) {
	updateMemory();
	fprintf(CurrentClient, "%lu\n", totalmem - freemem );
}

void printSwapFreeInfo( const char *cmd ) {
	fprintf(CurrentClient, "Free Swap\t0\t%lu\tKB\n", freeswap );
}

void printSwapFree( const char *cmd ) {
	updateSwap(1);
	fprintf(CurrentClient, "%lu\n", freeswap );
}

void printSwapUsedInfo( const char *cmd ) {
	fprintf(CurrentClient, "Used Swap\t0\t%lu\tKB\n", usedswap );
}

void printSwapUsed( const char *cmd ) {
	updateSwap(1);
	fprintf(CurrentClient, "%lu\n", usedswap );
}
