/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
        
        Irix support by Carsten Kroll <ckroll@pinnaclesys.com>
    
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/swap.h>
#include <sys/sysmp.h>

#include "config.h"

#include "ksysguardd.h"
#include "Command.h"
#include "Memory.h"

static int Dirty = 1;
static t_memsize totalmem = (t_memsize) 0;
static t_memsize freemem = (t_memsize) 0;
static unsigned long totalswap = 0L,vswap = 0L;
static unsigned long freeswap  = 0L,bufmem = 0L ;

void initMemory( struct SensorModul* sm ) {

	registerMonitor( "mem/physical/free", "integer",
					printMemFree, printMemFreeInfo, sm );
	registerMonitor( "mem/physical/used", "integer",
					printMemUsed, printMemUsedInfo, sm );
	registerMonitor( "mem/swap/free", "integer",
					printSwapFree, printSwapFreeInfo, sm );
	registerMonitor( "mem/swap/used", "integer",
					printSwapUsed, printSwapUsedInfo, sm );
}

void exitMemory( void ) {
}

int updateMemory( void ) {
	struct statfs sf;
	off_t val;
	int pagesize = getpagesize();
	struct rminfo rmi;
	if( sysmp(MP_SAGET, MPSA_RMINFO, &rmi, sizeof(rmi)) == -1 )
		return( -1 );
	totalmem  = rmi.physmem*pagesize/1024; // total physical memory (without swaps)
	freemem   = rmi.freemem*pagesize/1024; // total free physical memory (without swaps)
	bufmem    = rmi.bufmem *pagesize/1024;

	statfs ("/proc", &sf,sizeof(sf),0);

	swapctl(SC_GETSWAPVIRT,&val);
	vswap = val >> 1;
	swapctl(SC_GETSWAPTOT,&val);
	totalswap = val >> 1;
	swapctl(SC_GETFREESWAP,&val);
	freeswap = val >> 1;

	Dirty = 1;

	return( 0 );
}

void printMemFreeInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Free Memory\t0\t%ld\tKB\n", freemem );
}

void printMemFree( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", freemem );
}

void printMemUsedInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Used Memory\t0\t%ld\tKB\n", totalmem - freemem );
}

void printMemUsed( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", totalmem - freemem );
}

void printSwapFreeInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Free Swap\t0\t%ld\tKB\n", freeswap );
}

void printSwapFree( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", freeswap );
}
void printSwapUsedInfo( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "Used Swap\t0\t%ld\tKB\n", totalswap - freeswap );
}

void printSwapUsed( const char *cmd ) {
	if( Dirty )
		updateMemory();
	fprintf(CurrentClient, "%ld\n", totalswap - freeswap );
}
