/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
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

#include <stdlib.h>
#include <dirent.h>

#include <kapp.h>

#include "OSStatus.h"

// uncomment this line to fake SMP reporting on non SMP systems
// #define FAKE_SMP 1

#ifdef linux

// Code for Linux 2.x

OSStatus::OSStatus()
{
	error = false;
	cpuCount = -1;

	if ((stat = fopen("/proc/stat", "r")) == NULL)
	{
		error = true;
		errMessage = i18n("Cannot open file \'/proc/stat\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return;
	}

    setvbuf(stat, NULL, _IONBF, 0);

	if (!readCpuInfo("cpu", &userTicks, &sysTicks, &niceTicks,
					 &idleTicks))
		return;

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

	userTicksX = sysTicksX = niceTicksX = idleTicksX = 0;
}

OSStatus::~OSStatus()
{
	delete userTicksX;
	delete sysTicksX;
	delete niceTicksX;
	delete idleTicksX;

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
	 * SMP systems will have cpu1 to cpuN lines right after the cpu info. The
	 * format is identical to cpu and reports the information for each cpu.
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

	if (!readCpuInfo("cpu", &currUserTicks, &currSysTicks,
					 &currNiceTicks, &currIdleTicks))
		return (false);
		
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

int
OSStatus::getCpuCount(void)
{
	/*
	 * The number of CPUs is only determined once. Initially the cpuCount
	 * is set to -1. The first call of this function then checks for the
	 * availability of cpu load information from /proc/stat.
	 */
	if (cpuCount < 0)
	{
		int dum;
		QString cpuName;
		for (cpuCount = 1; ; cpuCount++)
		{
			cpuName.sprintf("cpu%d", cpuCount);
			if (!readCpuInfo(cpuName, &dum, &dum, &dum, &dum))
			{
				/*
				 * Clear the error flag since this situation was caused on
				 * purpose.
				 */
				error = false;
				errMessage = "";

				if (cpuCount > 1)
				{
					/*
					 * We only need the TicksX arrays on SMP systems. So we
					 * don't allocate them on single CPU systems.
					 */
					userTicksX = new int[cpuCount];
					sysTicksX = new int[cpuCount];
					niceTicksX = new int[cpuCount];
					idleTicksX = new int[cpuCount];
				}
				return (cpuCount);
			}
		}
	}

	return (cpuCount);
}

bool
OSStatus::getCpuXLoad(int cpu, int& user, int& sys, int& nice, int& idle)
{
	int currUserTicks, currSysTicks, currNiceTicks, currIdleTicks;

	QString cpuName;
	cpuName.sprintf("cpu%d", cpu);

	if (!readCpuInfo(cpuName, &currUserTicks, &currSysTicks,
					 &currNiceTicks, &currIdleTicks))
		return (false);

	int totalTicks = ((currUserTicks - userTicksX[cpu]) +
					  (currSysTicks - sysTicksX[cpu]) +
					  (currNiceTicks - niceTicksX[cpu]) +
					  (currIdleTicks - idleTicksX[cpu]));

	if (totalTicks > 0)
	{
		user = (100 * (currUserTicks - userTicksX[cpu])) / totalTicks;
		sys = (100 * (currSysTicks - sysTicksX[cpu])) / totalTicks;
		nice = (100 * (currNiceTicks - niceTicksX[cpu])) / totalTicks;
		idle = (100 - (user + sys + nice));
	}
	else
		user = sys = nice = idle = 0;

	userTicksX[cpu] = currUserTicks;
	sysTicksX[cpu] = currSysTicks;
	niceTicksX[cpu] = currNiceTicks;
	idleTicksX[cpu] = currIdleTicks;

	return (true);
}

bool
OSStatus::getMemoryInfo(int& total, int& mfree, int& used, int& buffers,
						int& cached)
{
	/*
	 * The amount of total and used memory is read from the /proc/meminfo.
	 * It also contains the information about the swap space.
	 * The 'file' looks like this:
	 *
	 *         total:    used:    free:  shared: buffers:  cached:
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
	/*
	 * The following works only on systems with 4GB or less. Currently this
	 * is no problem but what happens if Linus changes his mind?
	 */
	fscanf(meminfo, "%*s %d %d %d %*d %d %d\n",
		   &total, &used, &mfree, &buffers, &cached);

	total /= 1024;
	mfree /= 1024;
	used /= 1024;
	buffers /= 1024;
	cached /= 1024;

	fclose(meminfo);

	return (true);
}

bool
OSStatus::readCpuInfo(const char* cpu, int* u, int* s, int* n, int* i)
{
#ifdef FAKE_SMP
	/*
	 * This code redirects inquieries for cpu0 and cpu1 to cpu to fake a
	 * 2 processor SMP system for test purposes.
	 */
	char cpuFake[10] = "cpu";
	if (strcmp(cpu, "cpu0") != 0 && strcmp(cpu, "cpu1") != 0)
		strcpy(cpuFake, cpu);
#define cpu cpuFake
#endif
	char tag[32];

	rewind(stat);

	do
	{
		if (fscanf(stat, "%32s %d %d %d %d", tag, u, n, s, i) != 5)
		{
			error = true;
			errMessage.sprintf(i18n("Cannot read info for %s from file "
									"\'/proc/stat\'!\n"
									"The kernel needs to be compiled with "
									"support\n"
									"for /proc filesystem enabled!"), cpu);
			return (false);
		}
	} while (strcmp(tag, cpu));

#ifdef FAKE_SMP
#undef cpu
#endif

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

	stotal /= 1024;
	sfree /= 1024;

	return (true);
}

int 
isProcDir(const struct dirent* dir)
{
	return (atoi(dir->d_name));
}

int
OSStatus::getNoProcesses(void)
{
	int processes;
	struct dirent** namelist;


	processes = scandir("/proc", &namelist, isProcDir, alphasort);

	for (int i = 0; i < processes; i++)
		free(namelist[i]);
	free(namelist);

	return (processes);
}

#elif __FreeBSD__
/* Port to FreeBSD by Hans Petter Bieker <zerium@webindex.no>.
 *
 * Copyright 1999 Hans Petter Bieker <zerium@webindex.no>.
 */

#include <stdlib.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <sys/vmmeter.h>
#include <sys/user.h>
#include <unistd.h>

OSStatus::OSStatus()
{
	cpuCount = -1;
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

	return (true);
}

int
OSStatus::getCpuCount(void)
{
	if (cpuCount < 0)
	{
		size_t len = sizeof (cpuCount);
		if (sysctlbyname("hw.ncpu", &cpuCount, &len, NULL, 0) == -1 || !len)
			cpuCount = 1; // default to 1;
	}

	return (cpuCount);
}

bool
OSStatus::getCpuXLoad(int, int& user, int& sys, int& nice, int& idle)
{
	// FIXME
	// SMP support not yet implemented!
	return (getCpuLoad(user, sys, nice, idle));
}

bool
OSStatus::readCpuInfo(const char*, int*, int*, int*, int*)
{
	// FIXME
	return (false);
}

bool
OSStatus::getMemoryInfo(int& total, int& mfree, int& used, int& buffers,
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

	// total
	sysctl(mib, 2, &total, &len, NULL, 0);
	total /= 1024;

	// mfree
	mfree = p.t_free * getpagesize() / 1024;

	// used
	used = p.t_arm * getpagesize() / 1024;

	// buffers
	len = sizeof (buffers);
	if ((sysctlbyname("vfs.bufspace", &buffers, &len, NULL, 0) == -1) || !len)
		buffers = 0; // Doesn't work under FreeBSD v2.2.x

	// cached
	len = sizeof (cached);
	if ((sysctlbyname("vm.stats.vm.v_cache_count", &cached, &len, NULL, 0) == -1) || !len)
		cached = 0; // Doesn't work under FreeBSD v2.2.x
	cached *= getpagesize() / 1024;

	

	return (true);
}

bool
OSStatus::getSwapInfo(int& stotal, int& sfree)
{
	FILE *file;
	char buf[256];

	/* Q&D hack for swap display. Borrowed from xsysinfo-1.4 */
	if ((file = popen("/usr/sbin/pstat -ks", "r")) == NULL) {
		stotal = sfree = 0;
	return (true);
	}

	fgets(buf, sizeof(buf), file);
	fgets(buf, sizeof(buf), file);
	fgets(buf, sizeof(buf), file);
	fgets(buf, sizeof(buf), file);
	pclose(file);

	char *total_str, *free_str;
	strtok(buf, " ");
	total_str = strtok(NULL, " ");
	strtok(NULL, " ");
	free_str = strtok(NULL, " ");

	stotal = atoi(total_str);
	sfree = atoi(free_str);

	return (true);
}

int
OSStatus::getNoProcesses(void)
{
	int mib[3];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;

	size_t len;
	sysctl(mib, 3, NULL, &len, NULL, 0);

	return (len / sizeof (struct kinfo_proc));
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

int
OSStatus::getCpuCount(void)
{
	return (1);
}

bool
OSStatus::getCpuXLoad(int, int& user, int& sys, int& nice, int& idle)
{
	return (getCpuLoad(user, sys, nice, idle));
}

bool
OSStatus::readCpuInfo(const char*, int*, int*, int*, int*)
{
	return (false);
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

int
OSStatus::getNoProcesses(void)
{
	return (0);
}

#endif
