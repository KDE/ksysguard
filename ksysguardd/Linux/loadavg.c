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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "Command.h"
#include "loadavg.h"

static int LoadAvgOK = 0;
static double LoadAvg1, LoadAvg5, LoadAvg15;

#define LOADAVGBUFSIZE 128
static char LoadAvgBuf[LOADAVGBUFSIZE];
static int Dirty = 0;

static void
processLoadAvg(void)
{
	sscanf(LoadAvgBuf, "%lf %lf %lf", &LoadAvg1, &LoadAvg5, &LoadAvg15);
	Dirty = 0;
}

/*
================================ public part =================================
*/

void
initLoadAvg(void)
{
	if (updateLoadAvg() < 0)
	{
		LoadAvgOK = -1;
		return;
	}
	else
		LoadAvgOK = 1;

	registerMonitor("cpu/loadavg1", "float", printLoadAvg1,
					printLoadAvg1Info);
	registerMonitor("cpu/loadavg5", "float", printLoadAvg5,
					printLoadAvg5Info);
	registerMonitor("cpu/loadavg15", "float", printLoadAvg15,
					printLoadAvg15Info);
}

void
exitLoadAvg(void)
{
	LoadAvgOK = -1;
}

int
updateLoadAvg(void)
{
	/* ATTENTION: This function is called from a signal handler! Rules for
	 * signal handlers must be obeyed! */
	size_t n;
	int fd;

	if (LoadAvgOK < 0)
		return (-1);

	if ((fd = open("/proc/loadavg", O_RDONLY)) < 0)
	{
		if (LoadAvgOK != 0)
			perror("ERROR: Cannot open file \'/proc/loadavg\'!");
		return (-1);
	}
	if ((n = read(fd, LoadAvgBuf, LOADAVGBUFSIZE - 1)) == LOADAVGBUFSIZE - 1)
	{
		perror("ERROR: Internal buffer too small to read "
			   "/proc/loadavg!");
		return (-1);
	}
	close(fd);
	LoadAvgBuf[n] = '\0';
	Dirty = 1;

	return (0);
}

void
printLoadAvg1(const char* c)
{
	if (Dirty)
		processLoadAvg();

	printf("%f\n", LoadAvg1);
}

void
printLoadAvg1Info(const char* c)
{
	printf("Load average 1 min\t0\t0\t\n");
}

void
printLoadAvg5(const char* c)
{
	if (Dirty)
		processLoadAvg();

	printf("%f\n", LoadAvg5);
}

void
printLoadAvg5Info(const char* c)
{
	printf("Load average 5 min\t0\t0\t\n");
}

void
printLoadAvg15(const char* c)
{
	if (Dirty)
		processLoadAvg();

	printf("%f\n", LoadAvg15);
}

void
printLoadAvg15Info(const char* c)
{
	printf("Load average 15 min\t0\t0\t\n");
}
