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

#include <sys/dkstat.h>
#include <devstat.h>
#include <fcntl.h>
#include <kvm.h>
#include <nlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "CPU.h"
#include "Command.h"
#include "ksysguardd.h"

long percentages(int cnt, int *out, long *new, long *old, long *diffs);

struct nlist my_nlist[] = {
	{"_cp_time"},
	{ 0 }
};

kvm_t *kd;

unsigned long cp_time_offset;

long cp_time[CPUSTATES];
long cp_old[CPUSTATES];
long cp_diff[CPUSTATES];
int cpu_states[CPUSTATES];

void
initCPU(void)
{
	/* Total CPU load */
	registerMonitor("cpu/user", "integer", printCPUUser,
			printCPUUserInfo);
	registerMonitor("cpu/nice", "integer", printCPUNice,
			printCPUNiceInfo);
	registerMonitor("cpu/sys", "integer", printCPUSys,
			printCPUSysInfo);
	registerMonitor("cpu/idle", "integer", printCPUIdle,
			printCPUIdleInfo);
	kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open");
	kvm_nlist(kd, my_nlist);
	cp_time_offset = my_nlist[0].n_value;

	updateCPU();
}

void
exitCPU(void)
{
	kvm_close(kd);
}

int
updateCPU(void)
{
        kvm_read(kd, cp_time_offset, (char *)cp_time, sizeof(cp_time));
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

long percentages(cnt, out, new, old, diffs)

int cnt;
int *out;
register long *new;
register long *old;
long *diffs;

{
    register int i;
    register long change;
    register long total_change;
    register long *dp;
    long half_total;

    /* initialization */
    total_change = 0;
    dp = diffs;

    /* calculate changes for each state and the overall change */
    for (i = 0; i < cnt; i++)
    {
	if ((change = *new - *old) < 0)
	{
	    /* this only happens when the counter wraps */
	    change = (int)
		((unsigned long)*new-(unsigned long)*old);
	}
	total_change += (*dp++ = change);
	*old++ = *new++;
    }

    /* avoid divide by zero potential */
    if (total_change == 0)
    {
	total_change = 1;
    }

    /* calculate percentages based on overall change, rounding up */
    half_total = total_change / 2l;

    /* Do not divide by 0. Causes Floating point exception */
    if(total_change) {
        for (i = 0; i < cnt; i++)
        {
          *out++ = (int)((*diffs++ * 1000 + half_total) / total_change);
        }
    }

    /* return the total in case the caller wants to use it */
    return(total_change);
}
