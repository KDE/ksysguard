/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
	
	Irix support by Carsten Kroll <ckroll@pinnaclesys.com>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h>

#include "cpu.h"
#include "Command.h"
#include "ksysguardd.h"

#define CPUSTATES 6

long percentages(int cnt, int *out, long *new, long *old, long *diffs);

static int nCPUs=0;


long cp_time[CPUSTATES];
long cp_old[CPUSTATES];
long cp_diff[CPUSTATES];
int  cpu_states[CPUSTATES];

struct cpu_info{
	long cp_time[CPUSTATES];
	long cp_old[CPUSTATES];
	long cp_diff[CPUSTATES];
	int  cpu_states[CPUSTATES];
};

static struct cpu_info *g_ci;

/* returns the requested cpu number starting at 0*/
int getID(const char *cmd){
	int id;
	sscanf(cmd + 7, "%d", &id);
	return id-1;
}

void
initCpuInfo(struct SensorModul* sm)
{
	char mname[50];
	int i;
	if (sysmp(MP_NPROCS,&nCPUs) < 0) nCPUs=0;
	nCPUs++;
	g_ci = malloc(sizeof(struct cpu_info) * nCPUs);
	memset(g_ci,0,sizeof(struct cpu_info) * nCPUs);

	registerMonitor("cpu/user", "integer", printCPUUser,
		printCPUUserInfo, sm);
	registerMonitor("cpu/sys",  "integer", printCPUSys,
		printCPUSysInfo, sm);
	registerMonitor("cpu/idle", "integer", printCPUIdle,
		printCPUIdleInfo, sm);

	if (nCPUs > 1) for (i=0;i<nCPUs;i++){
		/* indidividual CPU load */
		sprintf(mname,"cpu/cpu%d/user",i+1);
		registerMonitor(mname, "integer", printCPUxUser,
				printCPUUserInfo, sm);
		sprintf(mname,"cpu/cpu%d/sys",i+1);
		registerMonitor(mname, "integer", printCPUxSys,
				printCPUSysInfo, sm);
		sprintf(mname,"cpu/cpu%d/idle",i+1);
		registerMonitor(mname, "integer", printCPUxIdle,
				printCPUIdleInfo, sm);
	}

	updateCpuInfo();
}

void
exitCpuInfo(void)
{
	free(g_ci);
}

int
updateCpuInfo(void)
{
	struct sysinfo si;
	int rv=0;
	int i;
	/* overall summary */
	if (sysmp(MP_SAGET,MPSA_SINFO,&si,sizeof(struct sysinfo)) >=0){
		cp_time[CPU_IDLE]  =si.cpu[CPU_IDLE];
		cp_time[CPU_USER]  =si.cpu[CPU_USER];
		cp_time[CPU_KERNEL]=si.cpu[CPU_KERNEL];
		cp_time[CPU_SXBRK] =si.cpu[CPU_SXBRK];
		cp_time[CPU_INTR]  =si.cpu[CPU_INTR];
		cp_time[CPU_WAIT]  =si.cpu[CPU_WAIT];
		percentages(CPUSTATES,cpu_states,cp_time,cp_old,cp_diff);
	}
	/* individual CPU statistics*/
	if (nCPUs > 1) for (i=0;i<nCPUs;i++){
		if (sysmp(MP_SAGET1,MPSA_SINFO,&si,sizeof(struct sysinfo),i) >=0){
			g_ci[i].cp_time[CPU_IDLE]  =si.cpu[CPU_IDLE];
			g_ci[i].cp_time[CPU_USER]  =si.cpu[CPU_USER];
			g_ci[i].cp_time[CPU_KERNEL]=si.cpu[CPU_KERNEL];
			g_ci[i].cp_time[CPU_SXBRK] =si.cpu[CPU_SXBRK];
			g_ci[i].cp_time[CPU_INTR]  =si.cpu[CPU_INTR];
			g_ci[i].cp_time[CPU_WAIT]  =si.cpu[CPU_WAIT];
			percentages(CPUSTATES, g_ci[i].cpu_states, g_ci[i].cp_time, g_ci[i].cp_old,g_ci[i].cp_diff);
		}else{
			rv =-1;
		}
	}
	return (rv);
}

void
printCPUUser(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", cpu_states[CPU_USER]/10);
}

void
printCPUUserInfo(const char* cmd)
{
	fprintf(CurrentClient, "CPU User Load\t0\t100\t%%\n");
}

void
printCPUSys(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", cpu_states[CPU_KERNEL]/10);
}

void
printCPUSysInfo(const char* cmd)
{
	fprintf(CurrentClient, "CPU System Load\t0\t100\t%%\n");
}

void
printCPUIdle(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", cpu_states[CPU_IDLE]/10);
}

void
printCPUIdleInfo(const char* cmd)
{
	fprintf(CurrentClient, "CPU Idle Load\t0\t100\t%%\n");
}
/* same as above but for individual CPUs */
void
printCPUxUser(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", g_ci[getID(cmd)].cpu_states[CPU_USER]/10);
}

void
printCPUxSys(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", g_ci[getID(cmd)].cpu_states[CPU_KERNEL]/10);
}

void
printCPUxIdle(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", g_ci[getID(cmd)].cpu_states[CPU_IDLE]/10);
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
 *	between array "old" and "new", putting the percentages in "out".
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
