/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
    Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2010 David Naylor <naylor.b.david@gmail.com>

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

#include <sys/types.h>
#include <sys/sysctl.h>

#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "Command.h"
#include "Memory.h"
#include "ksysguardd.h"

#define MEM_ACTIVE 0
#define MEM_INACTIVE 1
#define MEM_WIRED 2
#define MEM_CACHED 3
#define MEM_BUFFERED 4
#define MEM_FREE 5
#define MEM_TOTAL 6

static size_t memory_stats[7];

#define SWAP_IN 0
#define SWAP_OUT 1
#define SWAP_USED 2
#define SWAP_FREE 3
#define SWAP_TOTAL 4

static size_t swap_stats[5];
static size_t swap_old[2];

static int pagesize;

static kvm_t *kd;

void initMemory(struct SensorModul* sm)
{
    char *nlistf = NULL;
    char *memf = NULL;
    char buf[_POSIX2_LINE_MAX];

    pagesize = getpagesize();

    if ((kd = kvm_openfiles(nlistf, memf, NULL, O_RDONLY, buf)) == NULL) {
        log_error("kvm_openfiles()");
        return;
    }

    registerMonitor("mem/physical/active", "integer", printMActive, printMActiveInfo, sm);
    registerMonitor("mem/physical/inactive", "integer", printMInactive, printMInactiveInfo, sm);
    registerMonitor("mem/physical/application", "integer", printMApplication, printMApplicationInfo, sm);
    registerMonitor("mem/physical/wired", "integer", printMWired, printMWiredInfo, sm);
    registerMonitor("mem/physical/cached", "integer", printMCached, printMCachedInfo, sm);
    registerMonitor("mem/physical/buf", "integer", printMBuffers, printMBuffersInfo, sm);
    registerMonitor("mem/physical/free", "integer", printMFree, printMFreeInfo, sm);
    registerMonitor("mem/physical/used", "integer", printMUsed, printMUsedInfo, sm);

    registerMonitor("mem/swap/free", "integer", printSwapFree, printSwapFreeInfo, sm);
    registerMonitor("mem/swap/used", "integer", printSwapUsed, printSwapUsedInfo, sm);
    registerMonitor("mem/swap/pageIn", "integer", printSwapIn, printSwapInInfo, sm);
    registerMonitor("mem/swap/pageOut", "integer", printSwapOut, printSwapOutInfo, sm);

    registerLegacyMonitor("cpu/pageIn", "float", printSwapIn, printSwapInInfo, sm);
    registerLegacyMonitor("cpu/pageOut", "float", printSwapOut, printSwapOutInfo, sm);

    swap_old[SWAP_IN] = -1;
    swap_old[SWAP_OUT] = -1;

    updateMemory();
}

void exitMemory(void)
{
    removeMonitor("mem/physical/active");
    removeMonitor("mem/physical/inactive");
    removeMonitor("mem/physical/application");
    removeMonitor("mem/physical/wired");
    removeMonitor("mem/physical/cached");
    removeMonitor("mem/physical/buf");
    removeMonitor("mem/physical/free");
    removeMonitor("mem/physical/used");

    removeMonitor("mem/swap/free");
    removeMonitor("mem/swap/used");
    removeMonitor("mem/swap/pageIn");
    removeMonitor("mem/swap/pageOut");

    removeMonitor("cpu/pageIn");
    removeMonitor("cpu/pageOut");

    kvm_close(kd);
}

int updateMemory(void)
{
    size_t len;
    int swapin, swapout;

#define CONVERT(v)    ((quad_t)(v) * pagesize / 1024)

#define GETSYSCTL(mib, var)                        \
    len = sizeof(var);                        \
    sysctlbyname(mib, &var, &len, NULL, 0);

#define GETPAGESYSCTL(mib, var)                        \
    GETSYSCTL(mib, var)                        \
    var = CONVERT(var);

#define GETMEMSYSCTL(mib, var)                        \
    GETSYSCTL(mib, var)                        \
    var /= 1024;

    /*
     * Memory
     */
    GETPAGESYSCTL("vm.stats.vm.v_active_count", memory_stats[MEM_ACTIVE])
    GETPAGESYSCTL("vm.stats.vm.v_inactive_count", memory_stats[MEM_INACTIVE])
    GETPAGESYSCTL("vm.stats.vm.v_wire_count", memory_stats[MEM_WIRED])
    GETPAGESYSCTL("vm.stats.vm.v_cache_count", memory_stats[MEM_CACHED])
    GETPAGESYSCTL("vm.stats.vm.v_free_count", memory_stats[MEM_FREE])
    GETMEMSYSCTL("vfs.bufspace", memory_stats[MEM_BUFFERED])
    GETMEMSYSCTL("hw.physmem", memory_stats[MEM_TOTAL])

    /*
     * Swap
     */
    GETSYSCTL("vm.stats.vm.v_swappgsin", swapin);
    GETSYSCTL("vm.stats.vm.v_swappgsout", swapout);

    if (swap_old[SWAP_IN] < 0) {
        swap_stats[SWAP_IN] = 0;
        swap_stats[SWAP_OUT] = 0;
    } else {
        swap_stats[SWAP_IN] = CONVERT(swapin - swap_old[SWAP_IN]);
        swap_stats[SWAP_OUT] = CONVERT(swapout - swap_old[SWAP_OUT]);
    }

    /* call CPU heavy swapmode() only for changes */
    if (swap_stats[SWAP_IN] > 0 || swap_stats[SWAP_OUT] > 0 || swap_old[SWAP_IN] < 0) {
        struct kvm_swap swapary;
        if (kvm_getswapinfo(kd, &swapary, 1, 0) < 0 || swapary.ksw_total == 0) {
            int i;
            for (i = 0; i < (sizeof(swap_stats) / sizeof(swap_stats[0])); ++i)
                swap_stats[i] = 0;
        } else {
            swap_stats[SWAP_TOTAL] = CONVERT(swapary.ksw_total);
            swap_stats[SWAP_USED] = CONVERT(swapary.ksw_used);
            swap_stats[SWAP_FREE] = CONVERT(swapary.ksw_total - swapary.ksw_used);
        }
    }

    swap_old[SWAP_IN] = swapin;
    swap_old[SWAP_OUT] = swapout;

    return 0;

#undef CONVERT
#undef GETSYSCTL
#undef GETPAGESYSCTL
#undef GETMEMSYSCTL
}

void printMActive(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_ACTIVE]);
}

void printMActiveInfo(const char* cmd)
{
    fprintf(CurrentClient, "Active Memory\t0\t%lu\tKB\n", memory_stats[MEM_TOTAL]);
}

void printMInactive(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_INACTIVE]);
}

void printMInactiveInfo(const char* cmd)
{
    fprintf(CurrentClient, "Inactive Memory\t0\t%lu\tKB\n", memory_stats[MEM_TOTAL]);
}

void printMApplication(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_ACTIVE] + memory_stats[MEM_INACTIVE]);
}

void printMApplicationInfo(const char* cmd)
{
    fprintf(CurrentClient, "Application (Active and Inactive) Memory\t0\t%ld\tKB\n", memory_stats[MEM_TOTAL]);
}

void printMWired(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_WIRED]);
}

void printMWiredInfo(const char* cmd)
{
    fprintf(CurrentClient, "Wired Memory\t0\t%lu\tKB\n", memory_stats[MEM_TOTAL]);
}

void printMCached(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_CACHED]);
}

void printMCachedInfo(const char* cmd)
{
    fprintf(CurrentClient, "Cached Memory\t0\t%lu\tKB\n", memory_stats[MEM_TOTAL]);
}

void printMBuffers(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_BUFFERED]);
}

void printMBuffersInfo(const char* cmd)
{
    fprintf(CurrentClient, "Buffer Memory\t0\t%lu\tKB\n", memory_stats[MEM_TOTAL]);
}

void printMFree(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_FREE]);
}

void printMFreeInfo(const char* cmd)
{
    fprintf(CurrentClient, "Free Memory\t0\t%lu\tKB\n", memory_stats[MEM_TOTAL]);
}

void printMUsed(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", memory_stats[MEM_TOTAL] - memory_stats[MEM_FREE]);
}

void printMUsedInfo(const char* cmd)
{
    fprintf(CurrentClient, "Used Memory\t0\t%lu\tKB\n", memory_stats[MEM_TOTAL]);
}

void printSwapUsed(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", swap_stats[SWAP_USED]);
}

void printSwapUsedInfo(const char* cmd)
{
    fprintf(CurrentClient, "Used Swap Memory\t0\t%lu\tKB\n", swap_stats[SWAP_TOTAL]);
}

void printSwapFree(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", swap_stats[SWAP_FREE]);
}

void printSwapFreeInfo(const char* cmd)
{
    fprintf(CurrentClient, "Free Swap Memory\t0\t%lu\tKB\n", swap_stats[SWAP_TOTAL]);
}

void printSwapIn(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", swap_stats[SWAP_IN]);
}

void printSwapInInfo(const char* cmd)
{
    fprintf(CurrentClient, "Swapped In Memory\t0\t0\tKB/s\n");
}

void printSwapOut(const char* cmd)
{
    fprintf(CurrentClient, "%lu\n", swap_stats[SWAP_OUT]);
}

void printSwapOutInfo(const char* cmd)
{
    fprintf(CurrentClient, "Swapped Out Memory\t0\t0\tKB/s\n");
}
