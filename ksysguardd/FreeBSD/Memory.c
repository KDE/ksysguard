/*
    KSysGuard, the KDE System Guard

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	$Id$
*/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>

#include <vm/vm_param.h>

#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Command.h"
#include "Memory.h"
#include "ksysguardd.h"

static size_t Total = 0;
static size_t MFree = 0;
static size_t Used = 0;
static size_t Buffers = 0;
static size_t Cached = 0;
static size_t Application = 0;
static size_t STotal = 0;
static size_t SFree = 0;
static size_t SUsed = 0;
static kvm_t *kd;

void
initMemory(struct SensorModul* sm)
{
	char *nlistf = NULL;
	char *memf = NULL;
	char buf[_POSIX2_LINE_MAX];
	
	if ((kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, buf)) == NULL) {
		log_error("kvm_openfiles()");
		return;
	}

        registerMonitor("mem/physical/free", "integer", printMFree, printMFreeInfo, sm);
	registerMonitor("mem/physical/used", "integer", printUsed, printUsedInfo, sm);
	registerMonitor("mem/physical/buf", "integer", printBuffers, printBuffersInfo, sm);
	registerMonitor("mem/physical/cached", "integer", printCached, printCachedInfo, sm);
	registerMonitor("mem/physical/application", "integer", printApplication, printApplicationInfo, sm);
	registerMonitor("mem/swap/free", "integer", printSwapFree, printSwapFreeInfo, sm);
	registerMonitor("mem/swap/used", "integer", printSwapUsed, printSwapUsedInfo, sm);
}

void
exitMemory(void)
{
	kvm_close(kd);
}

int
updateMemory(void)
{
	size_t len;
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
        Cached *= pagesize / 1024;

	len = sizeof (MFree);
	if ((sysctlbyname("vm.stats.vm.v_free_count", &MFree, &len, NULL, 0) == -1) || !len)
		MFree = 0; /* Doesn't work under FreeBSD v2.2.x */
	MFree *= pagesize / 1024;

	Used = Total - MFree;
	Application = Used - Buffers - Cached;

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
printApplication(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Application);
}

void
printApplicationInfo(const char* cmd)
{
	fprintf(CurrentClient, "Application Memory\t0\t%ld\tKB\n", Total);
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
