/*
    KSysGuard, the KDE System Guard

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

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/dkstat.h>
#include <sys/sched.h>         /* CPUSTATES */
#include <fcntl.h>
#include <nlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "CPU.h"
#include "Command.h"
#include "ksysguardd.h"

void percentages(int, int *, u_int64_t *, u_int64_t *, u_int64_t *);

u_int64_t cp_time[CPUSTATES];
u_int64_t cp_old[CPUSTATES];
u_int64_t cp_diff[CPUSTATES];
int cpu_states[CPUSTATES];

void
initCpuInfo(struct SensorModul* sm)
{
	/* Total CPU load */
	registerMonitor("cpu/system/user", "integer", printCPUUser, printCPUUserInfo, sm);
	registerMonitor("cpu/system/nice", "integer", printCPUNice, printCPUNiceInfo, sm);
	registerMonitor("cpu/system/sys", "integer", printCPUSys, printCPUSysInfo, sm);
	registerMonitor("cpu/system/idle", "integer", printCPUIdle, printCPUIdleInfo, sm);
	
	/* Monitor names changed from kde3 => kde4. Remain compatible with legacy requests when possible. */
	registerLegacyMonitor("cpu/user", "integer", printCPUUser, printCPUUserInfo, sm);
	registerLegacyMonitor("cpu/nice", "integer", printCPUNice, printCPUNiceInfo, sm);
	registerLegacyMonitor("cpu/sys", "integer", printCPUSys, printCPUSysInfo, sm);
	registerLegacyMonitor("cpu/idle", "integer", printCPUIdle, printCPUIdleInfo, sm);
	
	updateCpuInfo();
}

void
exitCpuInfo(void)
{
	removeMonitor("cpu/system/user");
	removeMonitor("cpu/system/nice");
	removeMonitor("cpu/system/sys");
	removeMonitor("cpu/system/idle");

	/* These were registered as legacy monitors */
	removeMonitor("cpu/user");
	removeMonitor("cpu/nice");
	removeMonitor("cpu/sys");
	removeMonitor("cpu/idle");
}

int
updateCpuInfo(void)
{
	int mib[2];
	size_t size;

	mib[0] = CTL_KERN;
        mib[1] = KERN_CP_TIME;
	size = sizeof(cp_time[0]) * CPUSTATES;
	sysctl(mib, 2, cp_time, &size, NULL, 0);
        percentages(CPUSTATES, cpu_states, cp_time, cp_old, cp_diff);
	return (0);
}

void
printCPUUser(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", cpu_states[CP_USER]/10);
}

void
printCPUUserInfo(const char* cmd)
{
	fprintf(CurrentClient, "CPU User Load\t0\t100\t%%\n");
}

void
printCPUNice(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", cpu_states[CP_NICE]/10);
}

void
printCPUNiceInfo(const char* cmd)
{
	fprintf(CurrentClient, "CPU Nice Load\t0\t100\t%%\n");
}

void
printCPUSys(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", cpu_states[CP_SYS]/10);
}

void
printCPUSysInfo(const char* cmd)
{
	fprintf(CurrentClient, "CPU System Load\t0\t100\t%%\n");
}

void
printCPUIdle(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", cpu_states[CP_IDLE]/10);
}

void
printCPUIdleInfo(const char* cmd)
{
	fprintf(CurrentClient, "CPU Idle Load\t0\t100\t%%\n");
}


/* The part ripped from top... */
/*
 *  Top users/processes display for Unix
 *  Version 3
 *
 *  This program may be freely redistributed,
 *  but this entire comment MUST remain intact.
 *
 *  Copyright (c) 1984, 1989, William LeFebvre, Rice University
 *  Copyright (c) 1989, 1990, 1992, William LeFebvre, Northwestern University
 */

/*
 *  percentages(cnt, out, new, old, diffs) - calculate percentage change
 *	between array "old" and "new", putting the percentages i "out".
 *	"cnt" is size of each array and "diffs" is used for scratch space.
 *	The array "old" is updated on each call.
 *	The routine assumes modulo arithmetic.  This function is especially
 *	useful on BSD mchines for calculating cpu state percentages.
 */

void percentages(cnt, out, new, old, diffs)

int cnt;
int *out;
u_int64_t *new;
u_int64_t *old;
u_int64_t *diffs;

{
    int i;
    u_int64_t change;
    u_int64_t total_change;
    u_int64_t *dp;
    u_int64_t half_total;

    /* initialization */
    total_change = 0;
    dp = diffs;

    /* calculate changes for each state and the overall change */
    for (i = 0; i < cnt; i++)
    {
        /*
	 * Don't worry about wrapping - even at hz=1GHz, a
	 * u_int64_t will last at least 544 years.
	 */
        change = *new - *old;
	total_change += (*dp++ = change);
	*old++ = *new++;
    }

    /* avoid divide by zero potential */
    if (total_change == 0)
    {
	total_change = 1;
    }

    /* calculate percentages based on overall change, rounding up */
    half_total = total_change / 2;
    for (i = 0; i < cnt; i++)
    {
        *out++ = (int)((*diffs++ * 1000 + half_total) / total_change);
    }
}
