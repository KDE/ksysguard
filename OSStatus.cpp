/*
    KTop, the KDE Taskmanager
   
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

/*
 * ATTENTION: PORTING INFORMATION!
 * 
 * If you plan to port KTop to a new platform please follow these instructions.
 * They are designed to bring KTop to a maximum number of instructions without
 * sacrifying the ability to improve it further by adding new features.
 * The operation system abstraction classes were introduced to isolate OS
 * specific code in a very small number of modules. These modules are called
 * OSStatus, OSProcess and OSProcessList. OSStatus can be used to retrieve
 * information about the current system status while OSProcessList provides
 * a list of running processes.
 *
 * If you add support for a new platform try to make as little changes as
 * possible. Idealy you only need to change the two OS*.cpp files. Please
 * use a single #ifdef PLATFORM to add your code so that the files will look
 * like this:
 *
 * #ifdef _PLATFORM_A_
 * // Code for Platform A
 * OSStatus::OSStatus()
 * {
 * ...
 * OSStatus::getSwapInfo()
 * {
 * ...
 * }
 * #else
 * #ifdef _PLATFORM_B_
 * // Code for Platform B
 * OSStatus::OSStatus()
 * {
 * ...
 * OSStatus::getSwapInfo()
 * {
 * ...
 * }
 * #else
 * // generic code
 * OSStatus::OSStatus()
 * {
 * ...
 * OSStatus::getSwapInfo()
 * {
 * ...
 * }
 * #endif
 *
 * Please do _not_ put any #ifdef _PLATFORM_X_ in the other modules. Since
 * these things make the code ugly and difficult to maintain I will not
 * allow them unless you have _really_ good reasons for doing so.
 */

#include <kapp.h>

#include "OSStatus.h"

#ifdef linux

// Code for Linux 2.x

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

	if (fscanf(stat, "%*s %d %d %d %d",
			   &userTicks, &sysTicks, &niceTicks, &idleTicks) != 4)
	{
		error = true;
		errMessage = i18n("Cannot read file \'/proc/stat\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return;
	}

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

OSStatus::~OSStatus()
{
	if (stat)
		fclose(stat);
}

bool
OSStatus::getCpuLoad(int& user, int& sys, int& nice, int& idle)
{
	/*
	 * The CPU load is calculated from the values in /proc/stat. The cpu
	 * entry contains 4 counters. These counters count the number of ticks
	 * the system has spend on user processes, system processes, nice
	 * processes and idle time.
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

	int currUserTicks, currSysTicks, currNiceTicks, currIdleTicks;

	rewind(stat);
	if (fscanf(stat, "%*s %d %d %d %d",
			   &currUserTicks, &currNiceTicks,
			   &currSysTicks, &currIdleTicks) != 4)
	{
		error = true;
		errMessage = i18n("Cannot read file \'/proc/stat\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return (false);
	}

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
	/*
	 * The amount of total and used memory is read from the /proc/meminfo.
	 * It also contains the information about the swap space.
	 * The 'file' looks like this:
	 *
	 * total:    used:    free:  shared: buffers:  cached:
	 * Mem:  64593920 60219392  4374528 49426432  6213632 33689600
	 * Swap: 69636096   761856 68874240
	 * MemTotal:     63080 kB
	 * MemFree:       4272 kB
	 * MemShared:    48268 kB
	 * Buffers:       6068 kB
	 * Cached:       32900 kB
	 * SwapTotal:    68004 kB
	 * SwapFree:     67260 kB
	 */

	FILE* meminfo;

	if ((meminfo = fopen("/proc/meminfo", "r")) == NULL)
	{
		error = true;
		errMessage = i18n("Cannot open file \'/proc/meminfo\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return (false);
	}
	if (fscanf(meminfo, "%*[^\n]\n") == EOF)
	{
		error = true;
		errMessage = i18n("Cannot read memory info file \'/proc/meminfo\'!\n");
		return (false);
	}
	fscanf(meminfo, "%*s %d %*d %d %d %d %d\n",
		   &total, &mfree, &shared, &buffers, &cached);
	fclose(meminfo);

	return (true);
}

bool
OSStatus::getSwapInfo(int& stotal, int& sfree)
{
	FILE* meminfo;

	if ((meminfo = fopen("/proc/meminfo", "r")) == NULL)
	{
		error = true;
		errMessage = i18n("Cannot open file \'/proc/meminfo\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return (false);
	}
	if (fscanf(meminfo, "%*[^\n]\n") == EOF)
	{
		error = true;
		errMessage = i18n("Cannot read swap info from \'/proc/meminfo\'!\n");
		return (false);
	}
	fscanf(meminfo, "%*[^\n]\n");
	fscanf(meminfo, "%*s %d %*d %d\n",
		   &stotal, &sfree);
	fclose(meminfo);

	return (true);
}

#elif __FreeBSD__
/* Port to FreeBSD by Hans Petter Bieker <zerium@webindex.no>.
 *
 * Copyright 1999 Hans Petter Bieker <zerium@webindex.no>.
 */

#include <sys/param.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <sys/vmmeter.h>
#include <unistd.h>

OSStatus::OSStatus()
{
	error = false;
}

OSStatus::~OSStatus ()
{
}

bool
OSStatus::getCpuLoad(int& user, int& sys, int& nice, int& idle)
{
	user = 0; // FIXME
	sys = 0; // FIXME
	nice = 0; // FIXME

	idle = 100 - (user + sys + nice);

	return (true);
}

bool
OSStatus::getMemoryInfo(int& total, int& mfree, int& shared, int& buffers,
						int& cached)
{
	int mib[2];
	mib[0] = CTL_VM;
	mib[1] = VM_METER;

	size_t len = sizeof (vmtotal);
	struct vmtotal p;
	sysctl(mib, 2, &p, &len, NULL, 0);

	mib[0] = CTL_HW;
	mib[1] = HW_PHYSMEM;
	len = sizeof (total);

	sysctl(mib, 2, &total, &len, NULL, 0);
	mfree = p.t_free * getpagesize();
	shared = p.t_rmshr * getpagesize();
	buffers = 0; // FIXME
	cached = 0; // FIXME

	return (true);
}

bool
OSStatus::getSwapInfo(int& stotal, int& sfree)
{
	stotal = 0; // FIXME
	sfree = 0; // FIXME

	return (true);
}

#else

OSStatus::OSStatus()
{
	error = true;
	errMessage = i18n("Your system is not currently supported.\n"
			  "Sorry");
	return;
}

OSStatus::~OSStatus ()
{
}

bool
OSStatus::getCpuLoad(int &, int &, int &, int &)
{
	error = true;
	errMessage = i18n("Your system is not currently supported.\n"
			  "Sorry");
	return false;
}

bool
OSStatus::getMemoryInfo(int &, int &, int &, int &, int &)
{
	error = true;
	errMessage = i18n("Your system is not currently supported.\n"
			  "Sorry");
	return false;
}

bool
OSStatus::getSwapInfo(int &, int &)
{
	error = true;
	errMessage = i18n("Your system is not currently supported.\n"
			  "Sorry");
	return false;
}
#endif
