/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id$
*/

#include <stdlib.h>
#include <stdio.h>

#include "Command.h"
#include "Memory.h"

static size_t Total = 0;
static size_t MFree = 0;
static size_t Used = 0;
static size_t Buffers = 0;
static size_t Cached = 0;
static size_t STotal = 0;
static size_t SFree = 0;

void
initMemory(void)
{
	registerMonitor("memfree", printMFree, printMFreeInfo);
	registerMonitor("memused", printUsed, printUsedInfo);
	registerMonitor("membuf", printBuffers, printBuffersInfo);
	registerMonitor("memcached", printCached, printCachedInfo);
	registerMonitor("memswap", printSwap, printSwapInfo);
}

void
exitMemory(void)
{
}

int
updateMemory(void)
{
	/*
	 * The amount of total and used memory is read from the /proc/meminfo.
	 * It also contains the information about the swap space.
	 * The 'file' looks like this:
	 *
	 *         total:    used:    free:  shared: buffers:  cached:
	 * Mem:  64593920 60219392  4374528 49426432  6213632 33689600
	 * Swap: 69636096   761856 68874240
	 * MemTotal:     63080 kB
	 * MeMFree:       4272 kB
	 * MemShared:    48268 kB
	 * Buffers:       6068 kB
	 * Cached:       32900 kB
	 * SwapTotal:    68004 kB
	 * SwapFree:     67260 kB
	 */

	FILE* meminfo;

	if ((meminfo = fopen("/proc/meminfo", "r")) == NULL)
	{
		printf("ERROR: Cannot open file \'/proc/meminfo\'!\n"
			   "The kernel needs to be compiled with support\n"
			   "for /proc filesystem enabled!");
		return (-1);
	}
	if (fscanf(meminfo, "%*[^\n]\n") == EOF)
	{
		printf("ERROR: Cannot read memory info file \'/proc/meminfo\'!\n");
		return (-1);
	}
	/* The following works only on systems with 4GB or less. Currently this
	 * is no problem but what happens if Linus changes his mind? */
	fscanf(meminfo, "%*s %d %d %d %*d %d %d\n",
		   &Total, &Used, &MFree, &Buffers, &Cached);
	fscanf(meminfo, "%*s %d %*d %d\n",
		   &STotal, &SFree);

	Total /= 1024;
	MFree /= 1024;
	Used /= 1024;
	Buffers /= 1024;
	Cached /= 1024;

	fclose(meminfo);

	return (1);
}

void
printMFree(const char* cmd)
{
	printf("%d\n", MFree);
}

void
printMFreeInfo(const char* cmd)
{
	printf("Free Memory\t0\t%d\tKB\n", Total);
}

void
printUsed(const char* cmd)
{
	printf("%d\n", Used);
}

void
printUsedInfo(const char* cmd)
{
	printf("Used Memory\t0\t%d\tKB\n", Total);
}

void
printBuffers(const char* cmd)
{
	printf("%d\n", Buffers);
}

void
printBuffersInfo(const char* cmd)
{
	printf("Buffer Memory\t0\t%d\tKB\n", Total);
}

void
printCached(const char* cmd)
{
	printf("%d\n", Cached);
}

void
printCachedInfo(const char* cmd)
{
	printf("Cached Memory\t0\t%d\tKB\n", Total);
}

void
printSwap(const char* cmd)
{
	printf("%d\n", SFree);
}

void
printSwapInfo(const char* cmd)
{
	printf("Swap Memory\t0\t%d\tKB\n", STotal);
}

