/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CPU.h"
#include "Command.h"

typedef struct
{
	/* A CPU can be loaded with user processes, reniced processes and
	 * system processes. Unused processing time is called idle load.
	 * These variable store the percentage of each load type. */
	int userLoad;
	int niceLoad;
	int sysLoad;
	int idleLoad;

	/* To calculate the loads we need to remember the tick values for each
	 * load type. */
	unsigned long userTicks;
	unsigned long niceTicks;
	unsigned long sysTicks;
	unsigned long idleTicks;
} CPULoadInfo;

#define STATBUFSIZE 2048
static char StatBuf[STATBUFSIZE];
static int Dirty = 0;

static CPULoadInfo CPULoad;
static CPULoadInfo* SMPLoad = 0;
static unsigned CPUCount = 0;
static unsigned long Disk = 0;
static unsigned long OldDisk = 0;
static unsigned long DiskRIO = 0;
static unsigned long OldDiskRIO = 0;
static unsigned long DiskWIO = 0;
static unsigned long OldDiskWIO = 0;

static void
updateCPULoad(const char* line, CPULoadInfo* load)
{
	unsigned long currUserTicks, currSysTicks, currNiceTicks, currIdleTicks;
	unsigned long totalTicks;

	sscanf(line, "%*s %lu %lu %lu %lu", &currUserTicks, &currNiceTicks,
		   &currSysTicks, &currIdleTicks);

	totalTicks = ((currUserTicks - load->userTicks) +
				  (currSysTicks - load->sysTicks) +
				  (currNiceTicks - load->niceTicks) +
				  (currIdleTicks - load->idleTicks));

	if (totalTicks > 10)
	{
		load->userLoad = (100 * (currUserTicks - load->userTicks))
			/ totalTicks;
		load->sysLoad = (100 * (currSysTicks - load->sysTicks))
			/ totalTicks;
		load->niceLoad = (100 * (currNiceTicks - load->niceTicks))
			/ totalTicks;
		load->idleLoad = (100 - (load->userLoad + load->sysLoad
								 + load->niceLoad));
	}
	else
		load->userLoad = load->sysLoad = load->niceLoad = load->idleLoad = 0;

	load->userTicks = currUserTicks;
	load->sysTicks = currSysTicks;
	load->niceTicks = currNiceTicks;
	load->idleTicks = currIdleTicks;
}

static void
processStat(void)
{
	char format[32];
	char tagFormat[16];
	char buf[1024];
	char tag[32];
	char* statBufP = StatBuf;

	sprintf(format, "%%%d[^\n]\n", sizeof(buf) - 1);
	sprintf(tagFormat, "%%%ds", sizeof(tag) - 1);

	while (sscanf(statBufP, format, buf) == 1)
	{
		buf[sizeof(buf) - 1] = '\0';
		statBufP += strlen(buf) + 1;	// move statBufP to next line
		sscanf(buf, tagFormat, tag);

		if (strcmp("cpu", tag) == 0)
		{
			/* Total CPU load */
			updateCPULoad(buf, &CPULoad);
		}
		else if (strncmp("cpu", tag, 3) == 0)
		{
			/* Load for each SMP CPU */
			int id;
			sscanf(tag + 3, "%d", &id);
			updateCPULoad(buf, &SMPLoad[id]);
		}
		else if (strcmp("disk", tag) == 0)
		{
			unsigned long val;
			sscanf(buf + 5, "%lu", &val);
			Disk = val - OldDisk;
			OldDisk = val;
		}
		else if (strcmp("disk_rio", tag) == 0)
		{
			unsigned long val;
			sscanf(buf + 9, "%lu", &val);
			DiskRIO = val - OldDiskRIO;
			OldDiskRIO = val;
		}
		else if (strcmp("disk_wio", tag) == 0)
		{
			unsigned long val;
			sscanf(buf + 9, "%lu", &val);
			DiskWIO = val - OldDiskWIO;
			OldDiskWIO = val;
		}
	}

	Dirty = 0;
}

/*
================================ public part =================================
*/

void
initCPU(void)
{
	/* The CPU load is calculated from the values in /proc/stat. The cpu
	 * entry contains 4 counters. These counters count the number of ticks
	 * the system has spend on user processes, system processes, nice
	 * processes and idle time.
	 *
	 * SMP systems will have cpu1 to cpuN lines right after the cpu info. The
	 * format is identical to cpu and reports the information for each cpu.
	 * Linux kernels <= 2.0 do not provide this information!
	 *
	 * The /proc/stat file looks like this:
	 *
	 * cpu  1586 4 808 36274
	 * disk 7797 0 0 0
	 * disk_rio 6889 0 0 0
	 * disk_wio 908 0 0 0
	 * disk_rblk 13775 0 0 0
	 * disk_wblk 1816 0 0 0
	 * page 27575 1330
	 * swap 1 0
	 * intr 50444 38672 2557 0 0 0 0 2 0 2 0 0 3 1429 1 7778 0
	 * ctxt 54155
	 * btime 917379184
	 * processes 347 
	 */

	char format[32];
	char buf[1024];
	FILE* stat = 0;

	if ((stat = fopen("/proc/stat", "r")) == NULL)
	{
		fprintf(stderr, "ERROR: Cannot open file \'/proc/stat\'!\n"
				"The kernel needs to be compiled with support\n"
				"for /proc filesystem enabled!");
		return;
	}

	/* Use unbuffered input for /proc/stat file. */
    setvbuf(stat, NULL, _IONBF, 0);

	sprintf(format, "%%%d[^\n]\n", sizeof(buf) - 1);

	while (fscanf(stat, format, buf) == 1)
	{
		char tag[32];
		char tagFormat[16];
		
		buf[sizeof(buf) - 1] = '\0';
		sprintf(tagFormat, "%%%ds", sizeof(tag) - 1);
		sscanf(buf, tagFormat, tag);

		if (strcmp("cpu", tag) == 0)
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
		}
		else if (strncmp("cpu", tag, 3) == 0)
		{
			char cmdName[16];
			/* Load for each SMP CPU */
			int id;

			sscanf(tag + 3, "%d", &id);
			CPUCount++;
			sprintf(cmdName, "cpu%d/user", id);
			registerMonitor(cmdName, "integer", printCPUxUser,
							printCPUxUserInfo);
			sprintf(cmdName, "cpu%d/nice", id);
			registerMonitor(cmdName, "integer", printCPUxNice,
							printCPUxNiceInfo);
			sprintf(cmdName, "cpu%d/sys", id);
			registerMonitor(cmdName, "integer", printCPUxSys,
							printCPUxSysInfo);
			sprintf(cmdName, "cpu%d/idle", id);
			registerMonitor(cmdName, "integer", printCPUxIdle,
							printCPUxIdleInfo);
		}
		else if (strcmp("disk", tag) == 0)
		{
			registerMonitor("disk/load", "integer", printDiskLoad,
							printDiskLoadInfo);
		}
		else if (strcmp("disk_rio", tag) == 0)
		{
			registerMonitor("disk/rio", "integer", printDiskRIO,
							printDiskRIOInfo);
		}
		else if (strcmp("disk_wio", tag) == 0)
		{
			registerMonitor("disk/wio", "integer", printDiskWIO,
							printDiskWIOInfo);
		}
	}
	fclose(stat);

	if (CPUCount > 0)
		SMPLoad = (CPULoadInfo*) malloc(sizeof(CPULoadInfo) * CPUCount);
}

void
exitCPU(void)
{
	if (SMPLoad)
	{
		free(SMPLoad);
		SMPLoad = 0;
	}
}

int
updateCPU(void)
{
	size_t n;
	FILE* stat;

	if ((stat = fopen("/proc/stat", "r")) == NULL)
	{
		fprintf(stderr, "ERROR: Cannot open file \'/proc/stat\'!\n"
				"The kernel needs to be compiled with support\n"
				"for /proc filesystem enabled!");
		return (-1);
	}
	n = fread(StatBuf, 1, STATBUFSIZE - 1, stat);
	fclose(stat);
	StatBuf[n] = '\0';
	Dirty = 1;

	return (0);
}

void
printCPUUser(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%d\n", CPULoad.userLoad);
}

void 
printCPUUserInfo(const char* cmd)
{
	printf("CPU User Load\t0\t100\t%%\n");
}

void
printCPUNice(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%d\n", CPULoad.niceLoad);
}

void 
printCPUNiceInfo(const char* cmd)
{
	printf("CPU Nice Load\t0\t100\t%%\n");
}

void
printCPUSys(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%d\n", CPULoad.sysLoad);
}

void 
printCPUSysInfo(const char* cmd)
{
	printf("CPU System Load\t0\t100\t%%\n");
}

void
printCPUIdle(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%d\n", CPULoad.idleLoad);
}

void 
printCPUIdleInfo(const char* cmd)
{
	printf("CPU Idle Load\t0\t100\t%%\n");
}

void
printCPUxUser(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 3, "%d", &id);
	printf("%d\n", SMPLoad[id].userLoad);
}

void 
printCPUxUserInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 3, "%d", &id);
	printf("CPU%d User Load\t0\t100\t%%\n", id);
}

void
printCPUxNice(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 3, "%d", &id);
	printf("%d\n", SMPLoad[id].niceLoad);
}

void 
printCPUxNiceInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 3, "%d", &id);
	printf("CPU%d Nice Load\t0\t100\t%%\n", id);
}

void
printCPUxSys(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 3, "%d", &id);
	printf("%d\n", SMPLoad[id].sysLoad);
}

void 
printCPUxSysInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 3, "%d", &id);
	printf("CPU%d System Load\t0\t100\t%%\n", id);
}

void
printCPUxIdle(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 3, "%d", &id);
	printf("%d\n", SMPLoad[id].idleLoad);
}

void 
printCPUxIdleInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 3, "%d", &id);
	printf("CPU%d Idle Load\t0\t100\t%%\n", id);
}

void
printDiskLoad(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%lu\n", Disk);
}

void
printDiskLoadInfo(const char* cmd)
{
	printf("Disk Load\t0\t0\tkBytes/s\n");
}

void
printDiskRIO(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%lu\n", DiskRIO);
}

void
printDiskRIOInfo(const char* cmd)
{
	printf("Disk Read\t0\t100\tkBytes/s\n");
}

void
printDiskWIO(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%lu\n", DiskWIO);
}

void
printDiskWIOInfo(const char* cmd)
{
	printf("Disk Write\t0\t100\tkBytes/s\n");
}
