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
#include <ctype.h>

#include "stat.h"
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

typedef struct
{
	unsigned long delta;
	unsigned long old;
} DiskLoadSample;

typedef struct
{
	/* 5 types of samples are taken:
       total, rio, wio, rBlk, wBlk */
	DiskLoadSample s[5];
} DiskLoadInfo;
	
#define STATBUFSIZE 2048
static char StatBuf[STATBUFSIZE];
static int Dirty = 0;

static CPULoadInfo CPULoad;
static CPULoadInfo* SMPLoad = 0;
static unsigned CPUCount = 0;
static DiskLoadInfo* DiskLoad = 0;
static unsigned DiskCount = 0;
static unsigned long PageIn = 0;
static unsigned long OldPageIn = 0;
static unsigned long PageOut = 0;
static unsigned long OldPageOut = 0;
static unsigned long Ctxt = 0;
static unsigned long OldCtxt = 0;

static int initStatDisk(char* tag, char* buf, char* label, char* shortLabel,
						int index, cmdExecutor ex, cmdExecutor iq);
static void updateCPULoad(const char* line, CPULoadInfo* load);
static int processDisk(char* tag, char* buf, char* label, int index);
static void processStat(void);

static int
initStatDisk(char* tag, char* buf, char* label, char* shortLabel, int index,
			 cmdExecutor ex, cmdExecutor iq)
{
	char sensorName[128];

	if (strcmp(label, tag) == 0)
	{
		int i;
		buf = buf + strlen(label) + 1;
		
		for (i = 0; i < DiskCount; ++i)
		{
			sscanf(buf, "%lu", &DiskLoad[i].s[index].old);
			while (*buf && isblank(*buf++))
				;
			while (*buf && isdigit(*buf++))
				;
			sprintf(sensorName, "disk/disk%d/%s", i, shortLabel);
			registerMonitor(sensorName, "integer", ex, iq);

		}
		return (1);
	}

	return (0);
}

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

static int
processDisk(char* tag, char* buf, char* label, int index)
{
	if (strcmp(label, tag) == 0)
	{
		unsigned long val;
		int i;
		buf = buf + strlen(label) + 1;
		
		for (i = 0; i < DiskCount; ++i)
		{
			sscanf(buf, "%lu", &val);
			while (*buf && isblank(*buf++))
				;
			while (*buf && isdigit(*buf++))
				;
			DiskLoad[i].s[index].delta = val - DiskLoad[i].s[index].old;
			DiskLoad[i].s[index].old = val;
		}
		return (1);
	}

	return (0);
}

static void
processStat(void)
{
	char format[32];
	char tagFormat[16];
	char buf[1024];
	char tag[32];
	char* statBufP = StatBuf;

	sprintf(format, "%%%d[^\n]\n", (int) sizeof(buf) - 1);
	sprintf(tagFormat, "%%%ds", (int) sizeof(tag) - 1);

	while (sscanf(statBufP, format, buf) == 1)
	{
		buf[sizeof(buf) - 1] = '\0';
		statBufP += strlen(buf) + 1;	/* move statBufP to next line */
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
		else if (processDisk(tag, buf, "disk", 0))
			;
		else if (processDisk(tag, buf, "disk_rio", 1))
			;
		else if (processDisk(tag, buf, "disk_wio", 2))
			;
		else if (processDisk(tag, buf, "disk_rblk", 3))
			;
		else if (processDisk(tag, buf, "disk_wblk", 4))
			;
		else if (strcmp("page", tag) == 0)
		{
			unsigned long v1, v2;
			sscanf(buf + 5, "%lu %lu", &v1, &v2);
			PageIn = v1 - OldPageIn;
			OldPageIn = v1;
			PageOut = v2 - OldPageOut;
			OldPageOut = v2;
		}
		else if (strcmp("ctxt", tag) == 0)
		{
			unsigned long val;
			sscanf(buf + 5, "%lu", &val);
			Ctxt = val - OldCtxt;
			OldCtxt = val;
		}
	}

	Dirty = 0;
}

/*
================================ public part =================================
*/

void
initStat(void)
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

	sprintf(format, "%%%d[^\n]\n", (int) sizeof(buf) - 1);

	while (fscanf(stat, format, buf) == 1)
	{
		char tag[32];
		char tagFormat[16];
		
		buf[sizeof(buf) - 1] = '\0';
		sprintf(tagFormat, "%%%ds", (int) sizeof(tag) - 1);
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
			unsigned long val;
			char* b = buf + 5;

			/* Count the number of registered disks */
			for (DiskCount = 0; *b && sscanf(b, "%lu", &val) == 1; DiskCount++)
			{
				while (*b && isblank(*b++))
					;
				while (*b && isdigit(*b++))
					;
			}
			if (DiskCount > 0)
				DiskLoad = (DiskLoadInfo*) malloc(sizeof(DiskLoadInfo)
												  * DiskCount);
			initStatDisk(tag, buf, "disk", "disk", 0, printDiskTotal,
						 printDiskTotalInfo);
		}
		else if (initStatDisk(tag, buf, "disk_rio", "rio", 1, printDiskRIO,
							  printDiskRIOInfo))
			;
		else if (initStatDisk(tag, buf, "disk_wio", "wio", 2, printDiskWIO,
							  printDiskWIOInfo))
			;
		else if (initStatDisk(tag, buf, "disk_rblk", "rblk", 3, printDiskRBlk,
							  printDiskRBlkInfo))
			;
		else if (initStatDisk(tag, buf, "disk_wblk", "wblk", 4, printDiskWBlk,
							  printDiskWBlkInfo))
			;
		else if (strcmp("page", tag) == 0)
		{
			sscanf(buf + 5, "%lu %lu", &OldPageIn, &OldPageOut);
			registerMonitor("cpu/pageIn", "integer", printPageIn,
							printPageInInfo);
			registerMonitor("cpu/pageOut", "integer", printPageOut,
							printPageOutInfo);
		}
		else if (strcmp("ctxt", tag) == 0)
		{
			sscanf(buf + 5, "%lu", &OldCtxt);
			registerMonitor("cpu/context", "integer", printCtxt,
							printCtxtInfo);
		}
	}
	fclose(stat);

	/* Call processStat to eliminate initial peek values. */
	processStat();

	if (CPUCount > 0)
		SMPLoad = (CPULoadInfo*) malloc(sizeof(CPULoadInfo) * CPUCount);
}

void
exitStat(void)
{
	if (DiskLoad)
		free(DiskLoad);

	if (SMPLoad)
	{
		free(SMPLoad);
		SMPLoad = 0;
	}
}

int
updateStat(void)
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
printDiskTotal(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 9, "%d", &id);
	printf("%lu\n", DiskLoad[id].s[0].delta / TIMERINTERVAL);
}

void
printDiskTotalInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 9, "%d", &id);
	printf("Disk%d Total Load\t0\t0\tkBytes/s\n", id);
}

void
printDiskRIO(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 9, "%d", &id);
	printf("%lu\n", DiskLoad[id].s[1].delta / TIMERINTERVAL);
}

void
printDiskRIOInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 9, "%d", &id);
	printf("Disk%d Read\t0\t0\tkBytes/s\n", id);
}

void
printDiskWIO(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 9, "%d", &id);
	printf("%lu\n", DiskLoad[id].s[2].delta / TIMERINTERVAL);
}

void
printDiskWIOInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 9, "%d", &id);
	printf("Disk%d Write\t0\t0\tkBytes/s\n", id);
}

void
printDiskRBlk(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 9, "%d", &id);
	/* a block is 512 bytes or 1/2 kBytes */
	printf("%lu\n", DiskLoad[id].s[3].delta / TIMERINTERVAL * 2);
}

void
printDiskRBlkInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 9, "%d", &id);
	printf("Disk%d Read Data\t0\t0\tkBytes/s\n", id);
}

void
printDiskWBlk(const char* cmd)
{
	int id;

	if (Dirty)
		processStat();
	sscanf(cmd + 9, "%d", &id);
	/* a block is 512 bytes or 1/2 kBytes */
	printf("%lu\n", DiskLoad[id].s[4].delta / TIMERINTERVAL * 2);
}

void
printDiskWBlkInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 9, "%d", &id);
	printf("Disk%d Write Data\t0\t0\tkBytes/s\n", id);
}

void
printPageIn(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%lu\n", PageIn / TIMERINTERVAL);
}

void
printPageInInfo(const char* cmd)
{
	printf("Paged in Pages\t0\t0\t1/s\n");
}

void
printPageOut(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%lu\n", PageOut / TIMERINTERVAL);
}

void
printPageOutInfo(const char* cmd)
{
	printf("Paged out Pages\t0\t0\t1/s\n");
}

void
printCtxt(const char* cmd)
{
	if (Dirty)
		processStat();
	printf("%lu\n", Ctxt / TIMERINTERVAL);
}

void
printCtxtInfo(const char* cmd)
{
	printf("Context switches\t0\t0\t1/s\n");
}
