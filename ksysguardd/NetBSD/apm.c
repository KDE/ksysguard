/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>
    
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


#include <fcntl.h>
#include <machine/apmvar.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

#include "Command.h"
#include "apm.h"
#include "ksysguardd.h"

static int ApmFD, BattFill, BattTime;

#define APMDEV "/dev/apm"

/*
================================ public part =================================
*/

void
initApm(struct SensorModul* sm)
{
	if ((ApmFD = open(APMDEV, O_RDONLY)) < 0)
		return;

	if (updateApm() < 0)
		return;

	registerMonitor("apm/batterycharge", "integer", printApmBatFill,
					printApmBatFillInfo, sm);
	registerMonitor("apm/remainingtime", "integer", printApmBatTime,
					printApmBatTimeInfo, sm);
}

void
exitApm(void)
{
	removeMonitor("apm/batterycharge");
	removeMonitor("apm/remainingtime");

	close(ApmFD);
}

int
updateApm(void)
{
  int retval;
  struct apm_power_info info;
  retval = ioctl(ApmFD, APM_IOC_GETPOWER, &info);

  BattFill = info.battery_life;
  BattTime = info.minutes_left;

  return retval;
}

void
printApmBatFill(const char* c)
{
	fprintf(CurrentClient, "%d\n", BattFill);
}

void
printApmBatFillInfo(const char* c)
{
	fprintf(CurrentClient, "Battery charge\t0\t100\t%%\n");
}

void
printApmBatTime(const char* c)
{
	fprintf(CurrentClient, "%d\n", BattTime);
}

void
printApmBatTimeInfo(const char* c)
{
	fprintf(CurrentClient, "Remaining battery time\t0\t0\tmin\n");
}

