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

#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/vmmeter.h>
#include <unistd.h>
#include <vm/vm_param.h>

#include "Command.h"
#include "Memory.h"
#include "ksysguardd.h"

static size_t Total = 0;
static size_t MFree = 0;
static size_t Used = 0;
static size_t Buffers = 0;
static size_t Cached = 0;
static size_t STotal = 0;
static size_t SFree = 0;
static size_t SUsed = 0;
static kvm_t *kd;

void
initMemory(void)
{
	char *nlistf = NULL;
	char *memf = NULL;
	char buf[_POSIX2_LINE_MAX];
	
	if ((kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, buf)) == NULL) {
		log_error("kvm_openfiles()");
		return;
	}

        registerMonitor("mem/physical/free", "integer", printMFree, printMFreeInfo);
	registerMonitor("mem/physical/used", "integer", printUsed, printUsedInfo);
	registerMonitor("mem/physical/buf", "integer", printBuffers, printBuffersInfo);
	registerMonitor("mem/physical/cached", "integer", printCached, printCachedInfo);
	registerMonitor("mem/swap/free", "integer", printSwapFree, printSwapFreeInfo);
	registerMonitor("mem/swap/used", "integer", printSwapUsed, printSwapUsedInfo);
}

void
exitMemory(void)
{
	kvm_close(kd);
}

int
updateMemory(void)
{
	int mib[2];
	size_t len;
	struct vmtotal p;
	FILE *file;
	char buf[256];
	struct kvm_swap kswap[16];
	int i, swap_count, hlen, pagesize = getpagesize();
	long blocksize;

        len = sizeof (Total);
        sysctlbyname("hw.physmem", &Total, &len, NULL, 0);
        Total /= 1024;

	/* Borrowed from pstat */ 
	swap_count = kvm_getswapinfo(kd, kswap, 16, SWIF_DEV_PREFIX);
	getbsize(&hlen, &blocksize);

#define CONVERT(v) ((int)((quad_t)(v) * pagesize / blocksize))

	if (swap_count > 0) {
		STotal = CONVERT(kswap[0].ksw_total);
		SUsed = CONVERT(kswap[0].ksw_used);
		SFree = CONVERT(kswap[0].ksw_total - kswap[0].ksw_used);
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
	len = sizeof (p);
	sysctlbyname("vm.vmmeter", &p, &len, NULL, 0);
        MFree = p.t_free * getpagesize() / 1024;
        Used = p.t_arm * getpagesize() / 1024 + Buffers + Cached;

	return 0;
}

void
printMFree(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", MFree);
}

void
printMFreeInfo(const char* cmd)
{
	fprintf(CurrentClient, "Free Memory\t0\t%d\tKB\n", Total);
}

void
printUsed(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Used);
}

void
printUsedInfo(const char* cmd)
{
	fprintf(CurrentClient, "Used Memory\t0\t%d\tKB\n", Total);
}

void
printBuffers(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Buffers);
}

void
printBuffersInfo(const char* cmd)
{
	fprintf(CurrentClient, "Buffer Memory\t0\t%d\tKB\n", Total);
}

void
printCached(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Cached);
}

void
printCachedInfo(const char* cmd)
{
	fprintf(CurrentClient, "Cached Memory\t0\t%d\tKB\n", Total);
}

void
printSwapUsed(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", SUsed);
}

void
printSwapUsedInfo(const char* cmd)
{
	fprintf(CurrentClient, "Used Swap Memory\t0\t%d\tKB\n", STotal);
}

void
printSwapFree(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", SFree);
}

void
printSwapFreeInfo(const char* cmd)
{
	fprintf(CurrentClient, "Free Swap Memory\t0\t%d\tKB\n", STotal);
}
