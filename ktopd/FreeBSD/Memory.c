/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
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

#include <sys/param.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <sys/vmmeter.h>
#include <string.h>
#include <unistd.h>

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
        int mib[2];
        size_t len;
        struct vmtotal p;
        FILE *file;
        char buf[256];


        mib[0] = CTL_HW;
        mib[1] = HW_PHYSMEM;
        len = sizeof (Total);
        sysctl(mib, 2, &Total, &len, NULL, 0);
        Total /= 1024;


        /* Q&D hack for swap display. Borrowed from xsysinfo-1.4 */
        if ((file = popen("/usr/sbin/pstat -ks", "r")) == NULL)
        {
                STotal = SFree = 0;
        }
	else
	{
		char *total_str, *free_str;

		fgets(buf, sizeof(buf), file);
		fgets(buf, sizeof(buf), file);
		fgets(buf, sizeof(buf), file);
		fgets(buf, sizeof(buf), file);
		pclose(file);

		strtok(buf, " ");
		total_str = strtok(NULL, " ");
		strtok(NULL, " ");
		free_str = strtok(NULL, " ");

		STotal = atoi(total_str);
		SFree = atoi(free_str);
	}


        len = sizeof (Buffers);
        if ((sysctlbyname("vfs.bufspace", &Buffers, &len, NULL, 0) == -1) || !len)
                Buffers = 0; /* Doesn't work under FreeBSD v2.2.x */
        Buffers /= 1024;


        len = sizeof (Cached);
        if ((sysctlbyname("vm.stats.vm.v_cache_count", &Cached, &len, NULL, 0) == -1) || !len)
                Cached = 0; /* Doesn't work under FreeBSD v2.2.x */
        Cached *= getpagesize() / 1024;


	/* initializes the pointer to the vmmeter struct */
	mib[0] = CTL_VM;
	mib[1] = VM_METER;
	len = sizeof (p);
	sysctl(mib, 2, &p, &len, NULL, 0);
        MFree = p.t_free * getpagesize() / 1024;
        Used = p.t_arm * getpagesize() / 1024 + Buffers + Cached;

	return 0;
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
