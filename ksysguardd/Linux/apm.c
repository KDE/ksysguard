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

#include "Command.h"
#include "apm.h"

static int ApmOK = 0;
static int BatFill, BatTime;

void
initApm(void)
{
	if (updateApm() < 0)
		ApmOK = -1;
	else
		ApmOK = 1;

	registerMonitor("apm/batterycharge", "integer", printApmBatFill,
					printApmBatFillInfo);
	registerMonitor("apm/remainingtime", "integer", printApmBatTime,
					printApmBatTimeInfo);
}

void
exitApm(void)
{
	ApmOK = -1;
}

int
updateApm(void)
{
	FILE* apm;

	if (ApmOK < 0)
		return (-1);

	if ((apm = fopen("/proc/apm", "r")) == NULL)
		return (-1);

	if (fscanf(apm, "%*f %*f %*x %*x %*x %*x %d%% %d min",
			   &BatFill, &BatTime) != 2)
		return (-1);

	fclose(apm);

	return (0);
}

void
printApmBatFill(const char* c)
{
	printf("%d\n", BatFill);
}

void
printApmBatFillInfo(const char* c)
{
	printf("Battery charge\t0\t100\t%%\n");
}

void
printApmBatTime(const char* c)
{
	printf("%d\n", BatTime);
}

void
printApmBatTimeInfo(const char* c)
{
	printf("Remaining battery time\t0\t0\tmin\n");
}
