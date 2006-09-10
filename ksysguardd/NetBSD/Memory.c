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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
/* Everything post 1.5.x uses uvm/uvm_* includes */
#if __NetBSD_Version__ >= 105010000
#include <uvm/uvm_param.h>
#else
#include <vm/vm_param.h>
#endif

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

#define ARRLEN(X) (sizeof(X)/sizeof(X[0]))
  long pagesize; /* using a long promotes the arithmetic */
  size_t len;
  
  {
    static int mib[]={ CTL_HW, HW_PHYSMEM };
    
    len = sizeof(Total);
    sysctl(mib, ARRLEN(mib), &Total, &len, NULL, 0);
    Total >>= 10;
  }
 
  {
    struct uvmexp x;
    static int mib[] = { CTL_VM, VM_UVMEXP };
    
    len = sizeof(x);
    STotal = SUsed = SFree = -1;
    pagesize = 1;
    if (-1 < sysctl(mib, ARRLEN(mib), &x, &len, NULL, 0)) {
      pagesize = x.pagesize;
      STotal = (pagesize*x.swpages) >> 10;
      SUsed = (pagesize*x.swpginuse) >> 10;
      SFree = STotal - SUsed;
    }
  }
 
  /* can't find NetBSD file system buffer info */
  Buffers = -1;
 
  /* NetBSD doesn't know about vm.stats */
  Cached = -1;
  
  {
    static int mib[]={ CTL_VM, VM_METER };
    struct vmtotal x;
    
    len = sizeof(x);
    MFree = Used = -1;
    if (sysctl(mib, ARRLEN(mib), &x, &len, NULL, 0) > -1) {
      MFree = (x.t_free * pagesize) >> 10;
      Used = (x.t_rm * pagesize) >> 10;
    }
  }
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
