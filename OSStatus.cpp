/*
    KTop, a taskmanager and cpu load monitor
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
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
*/

// $Id$

#include <kapp.h>

#include "OSStatus.h"

OSStatus::OSStatus()
{
	error = false;

	if ((stat = fopen("/proc/stat", "r")) == NULL)
	{
		error = true;
		errMessage = i18n("Cannot open file \'/proc/stat\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return;
	}

    setvbuf(stat, NULL, _IONBF, 0);

	fscanf(stat, "%*s %d %d %d %d",
		   &userTicks, &sysTicks, &niceTicks, &idleTicks);

	/*
	 * Contrary to /proc/stat the /proc/meminfo file cannot be left open.
	 * If done the rewind and read functions will use significant amounts
	 * of system time. Therefore we open and close it each time we have to
	 * access it.
	 */
	FILE* meminfo;
	if ((meminfo = fopen("/proc/meminfo", "r")) == NULL)
	{
		error = true;
		errMessage = i18n("Cannot open file \'/proc/meminfo\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return;
	}
	fclose(meminfo);
}

bool
OSStatus::getCpuLoad(int& user, int& sys, int& nice, int& idle)
{
	int currUserTicks, currSysTicks, currNiceTicks, currIdleTicks;

	rewind(stat);
	fscanf(stat, "%*s %d %d %d %d",
		   &currUserTicks, &currNiceTicks, &currSysTicks, &currIdleTicks);

	int totalTicks = ((currUserTicks - userTicks) +
					  (currSysTicks - sysTicks) +
					  (currNiceTicks - niceTicks) +
					  (currIdleTicks - idleTicks));

	if (totalTicks > 0)
	{
		user = (100 * (currUserTicks - userTicks)) / totalTicks;
		sys = (100 * (currSysTicks - sysTicks)) / totalTicks;
		nice = (100 * (currNiceTicks - niceTicks)) / totalTicks;
		idle = (100 - (user + sys + nice));
	}
	else
		user = sys = nice = idle = 0;

	userTicks = currUserTicks;
	sysTicks = currSysTicks;
	niceTicks = currNiceTicks;
	idleTicks = currIdleTicks;

	return (true);
}

bool
OSStatus::getMemoryInfo(int& total, int& mfree, int& shared, int& buffers,
						int& cached)
{
	FILE* meminfo;

	meminfo = fopen("/proc/meminfo", "r");
	fscanf(meminfo, "%*[^\n]\n");
	fscanf(meminfo, "%*s %d %*d %d %d %d %d\n",
		   &total, &mfree, &shared, &buffers, &cached);
	fclose(meminfo);

	return (true);
}

bool
OSStatus::getSwapInfo(int& stotal, int& sfree)
{
	FILE* meminfo;

	meminfo = fopen("/proc/meminfo", "r");
	fscanf(meminfo, "%*[^\n]\n");
	fscanf(meminfo, "%*[^\n]\n");
	fscanf(meminfo, "%*s %d %*d %d\n",
		   &stotal, &sfree);
	fclose(meminfo);

	return (true);
}
