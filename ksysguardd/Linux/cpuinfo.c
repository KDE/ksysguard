/*
    KTop, the KDE Task Manager
   
	Copyright (c) 2000 Chris Schlaeger <cs@kde.org>
    
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "Command.h"
#include "cpuinfo.h"

static int CpuInfoOK = 0;
static float* Clocks = 0;
static int CPUs = 0;

#define CPUINFOBUFSIZE 8192
static char CpuInfoBuf[CPUINFOBUFSIZE];
static int Dirty = 0;
static int Asleep = 0;
static time_t lastRequest;

static void
processCpuInfo(void)
{
	char format[32];
	char tag[32];
	char value[256];
	char* cibp = CpuInfoBuf;
	int cpuId = 0;

	if (!CpuInfoOK)
		return;

	if (Asleep)
	{
		Asleep = 0;
		updateCpuInfo();
	}

	sprintf(format, "%%%d[^:]: %%%d[^\n]\n", (int) sizeof(tag) - 1,
			(int) sizeof(value) - 1);

	while (sscanf(cibp, format, tag, value) == 2)
	{
		char* p;
		tag[sizeof(tag) - 1] = '\0';
		value[sizeof(value) - 1] = '\0';
		/* remove trailing whitespaces */
		p = tag + strlen(tag) - 1;
		/* remove trailing whitespaces */
		while ((*p == ' ' || *p == '\t') && p > tag)
			*p-- = '\0';

		if (strcmp(tag, "processor") == 0)
		{
			if (sscanf(value, "%d", &cpuId) == 1)
			{
				if (cpuId >= CPUs)
				{
					char cmdName[24];
					if (Clocks)
						free(Clocks);
					CPUs = cpuId + 1;
					Clocks = malloc(CPUs * sizeof(float));
					sprintf(cmdName, "cpu%d/clock", cpuId);
					registerMonitor(cmdName, "float", printCPUxClock,
							printCPUxClockInfo);
				}
			}
		}
		else if (strcmp(tag, "cpu MHz") == 0)
			sscanf(value, "%f", &Clocks[cpuId]);

		/* Move cibp to begining of next line, if there is one. */
		cibp = strchr(cibp, '\n');
		if (cibp)
			cibp++;
		else
			cibp = CpuInfoBuf + strlen(CpuInfoBuf);
	}
	Dirty = 0;
}

/*
================================ public part =================================
*/

void
initCpuInfo(void)
{
	lastRequest = time(0);
	if (updateCpuInfo() < 0)
		return;
	processCpuInfo();
}

void
exitCpuInfo(void)
{
	CpuInfoOK = -1;
}

int
updateCpuInfo(void)
{
	/* ATTENTION: This function is called from a signal handler! Rules for
	 * signal handlers must be obeyed! */
	size_t n;
	int fd;

	if (CpuInfoOK < 0)
		return (-1);

	if (Asleep)
		return (0);

	if (time(0) > lastRequest + 5)
	{
		Asleep = 1;
		return (0);
	}
		
	if ((fd = open("/proc/cpuinfo", O_RDONLY)) < 0)
	{
		if (CpuInfoOK != 0)
			perror("ERROR: Cannot open file \'/proc/cpuinfo\'!");
		CpuInfoOK = -1;
		return (-1);
	}
	if ((n = read(fd, CpuInfoBuf, CPUINFOBUFSIZE - 1)) == CPUINFOBUFSIZE - 1)
	{
		perror("ERROR: Internal buffer too small to read "
			   "/proc/cpuinfo!");
		CpuInfoOK = 0;
		return (-1);
	}
	close(fd);
	CpuInfoOK = 1;
	CpuInfoBuf[n] = '\0';
	Dirty = 1;

	return (0);
}

void
printCPUxClock(const char* cmd)
{
	int id;

	lastRequest = time(0);
	if (Dirty)
		processCpuInfo();

	sscanf(cmd + 3, "%d", &id);
	printf("%f\n", Clocks[id]);
}

void
printCPUxClockInfo(const char* cmd)
{
	int id;

	sscanf(cmd + 3, "%d", &id);
	printf("CPU%d Clock Frequency\t0\t0\tMHz\n", id);
}
