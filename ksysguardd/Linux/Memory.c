/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "Command.h"
#include "Memory.h"
#include "ksysguardd.h"

#define MEMINFOBUFSIZE 1024
static char MemInfoBuf[MEMINFOBUFSIZE];
static int Dirty = 1;

static unsigned long Total = 0;
static unsigned long MFree = 0;
static unsigned long Appl = 0;
static unsigned long Used = 0;
static unsigned long Buffers = 0;
static unsigned long Cached = 0;
static unsigned long STotal = 0;
static unsigned long SFree = 0;
static unsigned long SUsed = 0;

static void
processMemInfo()
{
	sscanf(MemInfoBuf, "%*[^\n]\n"
		   "%*s %ld %ld %ld %*d %ld %ld\n"
		   "%*s %ld %ld %ld\n", 
		   &Total, &Used, &MFree, &Buffers, &Cached,
		   &STotal, &SUsed, &SFree);

	Appl = (Used - (Buffers + Cached)) / 1024;
	Total /= 1024;
	MFree /= 1024;
	Used /= 1024;
	Buffers /= 1024;
	Cached /= 1024;
	STotal /= 1024;
	SFree /= 1024;
	SUsed /= 1024;

	Dirty = 0;
}

/*
================================ public part =================================
*/

void
initMemory(struct SensorModul* sm)
{
	/* Make sure that /proc/meminfo exists and is readable. If not we do
	 * not register any monitors for memory. */
	if (updateMemory() < 0)
		return;

	registerMonitor("mem/physical/free", "integer", printMFree,
					printMFreeInfo, sm);
	registerMonitor("mem/physical/used", "integer", printUsed, printUsedInfo, sm);
	registerMonitor("mem/physical/application", "integer", printAppl,
					printApplInfo, sm);
	registerMonitor("mem/physical/buf", "integer", printBuffers,
					printBuffersInfo, sm);
	registerMonitor("mem/physical/cached", "integer", printCached,
					printCachedInfo, sm);
	registerMonitor("mem/swap/used", "integer", printSwapUsed,
					printSwapUsedInfo, sm);
	registerMonitor("mem/swap/free", "integer", printSwapFree,
					printSwapFreeInfo, sm);
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

	int fd;
	size_t n;

	if ((fd = open("/proc/meminfo", O_RDONLY)) < 0)
	{
		print_error("Cannot open \'/proc/meminfo\'!\n"
			   "The kernel needs to be compiled with support\n"
			   "for /proc filesystem enabled!\n");
		return (-1);
	}
	if ((n = read(fd, MemInfoBuf, MEMINFOBUFSIZE - 1)) ==
		MEMINFOBUFSIZE - 1)
	{
		log_error("Internal buffer too small to read \'/proc/mem\'");
		return (-1);
	}

	close(fd);
	MemInfoBuf[n] = '\0';
	Dirty = 1;

	return (0);
}

void
printMFree(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "%ld\n", MFree);
}

void
printMFreeInfo(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "Free Memory\t0\t%ld\tKB\n", Total);
}

void
printUsed(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "%ld\n", Used);
}

void
printUsedInfo(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "Used Memory\t0\t%ld\tKB\n", Total);
}

void
printAppl(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "%ld\n", Appl);
}

void
printApplInfo(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "Application Memory\t0\t%ld\tKB\n", Total);
}

void
printBuffers(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "%ld\n", Buffers);
}

void
printBuffersInfo(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "Buffer Memory\t0\t%ld\tKB\n", Total);
}

void
printCached(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "%ld\n", Cached);
}

void
printCachedInfo(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "Cached Memory\t0\t%ld\tKB\n", Total);
}

void
printSwapUsed(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "%ld\n", SUsed);
}

void
printSwapUsedInfo(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "Used Swap Memory\t0\t%ld\tKB\n", STotal);
}

void
printSwapFree(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "%ld\n", SFree);
}

void
printSwapFreeInfo(const char* cmd)
{
	if (Dirty)
		processMemInfo();
	fprintf(CurrentClient, "Free Swap Memory\t0\t%ld\tKB\n", STotal);
}
