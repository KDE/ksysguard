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
static size_t Active = 0;
static size_t Inactive = 0;
static size_t Wired = 0;
static size_t Execpages = 0;
static size_t Filepages = 0;
static size_t STotal = 0;
static size_t SFree = 0;
static size_t SUsed = 0;

void
initMemory(struct SensorModul* sm)
{
        registerMonitor("mem/physical/free", "integer", printMFree, printMFreeInfo, sm);
	registerMonitor("mem/physical/used", "integer", printUsed, printUsedInfo, sm);
	registerMonitor("mem/physical/active", "integer", printActive, printActiveInfo, sm);
	registerMonitor("mem/physical/inactive", "integer", printInactive, printInactiveInfo, sm);
	registerMonitor("mem/physical/wired", "integer", printWired, printWiredInfo, sm);
	registerMonitor("mem/physical/execpages", "integer", printExecpages, printExecpagesInfo, sm);
	registerMonitor("mem/physical/filepages", "integer", printFilepages, printFilepagesInfo, sm);
	registerMonitor("mem/swap/free", "integer", printSwapFree, printSwapFreeInfo, sm);
	registerMonitor("mem/swap/used", "integer", printSwapUsed, printSwapUsedInfo, sm);
}

void
exitMemory(void)
{
}

int
updateMemory(void)
{

#define ARRLEN(X) (sizeof(X)/sizeof(X[0]))
  size_t len;
  
  {
    static int mib[]={ CTL_HW, HW_PHYSMEM };
    
    len = sizeof(Total);
    sysctl(mib, ARRLEN(mib), &Total, &len, NULL, 0);
    Total >>= 10;
  }
 
  {
    struct uvmexp_sysctl x;
    static int mib[] = { CTL_VM, VM_UVMEXP2 };
    
    len = sizeof(x);
    STotal = SUsed = SFree = -1;
    Active = Inactive = Wired = Execpages = Filepages = MFree = Used = -1;
    if (-1 < sysctl(mib, ARRLEN(mib), &x, &len, NULL, 0)) {
      STotal = (x.pagesize*x.swpages) >> 10;
      SUsed = (x.pagesize*x.swpginuse) >> 10;
      SFree = STotal - SUsed;
      MFree = (x.free * x.pagesize) >> 10;
      Active = (x.active * x.pagesize) >> 10;
      Inactive = (x.inactive * x.pagesize) >> 10;
      Wired = (x.wired * x.pagesize) >> 10;
      Execpages = (x.execpages * x.pagesize) >> 10;
      Filepages = (x.filepages * x.pagesize) >> 10;
      Used = Total - MFree;
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
printActive(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Active);
}

void
printActiveInfo(const char* cmd)
{
	fprintf(CurrentClient, "Active Memory\t0\t%d\tKB\n", Total);
}

void
printInactive(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Inactive);
}

void
printInactiveInfo(const char* cmd)
{
	fprintf(CurrentClient, "Inactive Memory\t0\t%d\tKB\n", Total);
}

void
printWired(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Wired);
}

void
printWiredInfo(const char* cmd)
{
	fprintf(CurrentClient, "Wired Memory\t0\t%d\tKB\n", Total);
}

void
printExecpages(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Execpages);
}

void
printExecpagesInfo(const char* cmd)
{
	fprintf(CurrentClient, "Exec Pages\t0\t%d\tKB\n", Total);
}

void
printFilepages(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", Filepages);
}

void
printFilepagesInfo(const char* cmd)
{
	fprintf(CurrentClient, "File Pages\t0\t%d\tKB\n", Total);
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
