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
 * For general porting information please look at the file OSStatus.cpp!
 *
 * To keep this file readable and maintainable please keep the number of
 * #ifdef _PLATFORM_ as low as possible. Ideally you dont have to make any
 * platform specific changes in the header files. Please do not add any new
 * features. This is planned for KTop version after 1.0.0!
 */

#include <config.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>       
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>

#include <kapp.h>

#include "OSProcessList.h"

#ifdef _PLATFORM_A

// nothing here yet

#elif linux

// Code for Linux 2.x

OSProcessList::OSProcessList()
{
	error = false;

	lastTStamps = NULL;

	sortCriteria = SORTBY_PID;
	setAutoDelete(true);

	/*
	 * Here we make sure that the kernel has been compiled with /proc
	 * support enabled. If not we generate an error message and set the
	 * error variable to true.
	 */
	DIR* dir;
	if ((dir = opendir("/proc")) == NULL)
	{
		error = true;
		errMessage = i18n("Cannot open directory \'/proc\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return;
	}
	closedir(dir);
}


bool
OSProcessList::update(void)
{
	// delete old process list
	clear();

	// read in current process list via the /proc filesystem entry
	DIR* dir;
	struct dirent* entry;

	TimeStampList* newTStamps = new TimeStampList;
	// If there is no old list yet, we create an empty one.
	if (!lastTStamps)
		lastTStamps = new TimeStampList;

	if ((dir = opendir("/proc")) == NULL)
	{
		error = true;
		errMessage = i18n("Cannot open directory \'/proc\'!\n"
						  "The kernel needs to be compiled with support\n"
						  "for /proc filesystem enabled!");
		return (false);
	}
	while ((entry = readdir(dir))) 
	{
		if (isdigit(entry->d_name[0]))
		{
			OSProcess* ps = new OSProcess(entry->d_name, lastTStamps,
										  newTStamps);
			if (!ps || !ps->ok())
			{
				error = true;
				if (ps)
					errMessage = ps->getErrMessage();
				else
					errMessage = i18n("Cannot read status of processes"
									  "from /proc/... directories!\n");
				delete ps;
				return (false);
			}

			// insert process into sorted list
			inSort(ps);
		}
	}
	closedir(dir);

	// make new list old one and discard the really old one
	delete lastTStamps;
	lastTStamps = newTStamps;

	return (true);
}

bool 
OSProcessList::hasName(void) const
{
	return (true);
}

bool 
OSProcessList::hasUid(void) const
{
	return (true);
}

bool 
OSProcessList::hasUserTime(void) const
{
	return (true);
}

bool
OSProcessList::hasSysTime(void) const
{
	return (true);
}

bool 
OSProcessList::hasUserLoad(void) const
{
	return (true);
}

bool
OSProcessList::hasSysLoad(void) const
{
	return (true);
}

bool
OSProcessList::hasStatus(void) const
{
	return (true);
}

bool
OSProcessList::hasPriority(void) const
{
	return (true);
}

bool 
OSProcessList::hasVmSize(void) const
{
	return (true);
}

bool
OSProcessList::hasVmRss(void) const
{
	return (false);
}

bool 
OSProcessList::hasVmLib(void) const
{
	return (false);
}

#elif __FreeBSD__
/* Port to FreeBSD by Hans Petter Bieker <zerium@webindex.no>.
 *
 * Copyright 1999 Hans Petter Bieker <zerium@webindex.no>.
 */

#include <stdlib.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>

OSProcessList::OSProcessList()
{
	error = false;

	lastTStamps = NULL;

	sortCriteria = SORTBY_PID;
	setAutoDelete(true);
}


bool
OSProcessList::update(void)
{
	// delete old process list
	clear();

	int mib[3];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;

	size_t len;
	sysctl(mib, 3, NULL, &len, NULL, 0);

	struct kinfo_proc *p = (struct kinfo_proc *)malloc(len);
	sysctl(mib, 3, p, &len, NULL, 0);

	size_t num;
	for (num = 0; num < len / sizeof(struct kinfo_proc); num++) {
		OSProcess* ps = new OSProcess((char *)&p[num], 0, 0);

		if (!ps || !ps->ok())
		{
			error = true;
			if (ps)
				errMessage = ps->getErrMessage();
			else
				errMessage = i18n("Cannot read status of processes\n");
			delete ps;
			return (false);
		}

		// insert process into sorted list
		inSort(ps);
	}
	
	// make new list old one and discard the really old one
	free(p);

	return (true);
}

bool 
OSProcessList::hasName(void) const
{
	return (true);
}

bool 
OSProcessList::hasUid(void) const
{
	return (true);
}

bool 
OSProcessList::hasUserTime(void) const
{
	return (true);
}

bool
OSProcessList::hasSysTime(void) const
{
	return (true);
}

bool 
OSProcessList::hasUserLoad(void) const
{
	return (true);
}

bool
OSProcessList::hasSysLoad(void) const
{
	return (true);
}

bool
OSProcessList::hasStatus(void) const
{
	return (true);
}

bool
OSProcessList::hasPriority(void) const
{
	return (true);
}

bool 
OSProcessList::hasVmSize(void) const
{
	return (true);
}

bool
OSProcessList::hasVmRss(void) const
{
	return (true);
}

bool 
OSProcessList::hasVmLib(void) const
{
	// FreeBSD doesn't support this!??!
	return (false);
}


#else

OSProcessList::OSProcessList()
{

  error = true;
  errMessage = i18n("Your OS is not supported yet.  Sorry");
  return;
}

bool
OSProcessList::update(void)
{
	return (false);
}

bool 
OSProcessList::hasName(void) const
{
	return (false);
}

bool 
OSProcessList::hasUid(void) const
{
	return (false);
}

bool 
OSProcessList::hasUserTime(void) const
{
	return (false);
}

bool
OSProcessList::hasSysTime(void) const
{
	return (false);
}

bool 
OSProcessList::hasUserLoad(void) const
{
	return (false);
}

bool
OSProcessList::hasSysLoad(void) const
{
	return (false);
}

bool
OSProcessList::hasStatus(void) const
{
	return (false);
}

bool
OSProcessList::hasPriority(void) const
{
	return (false);
}

bool 
OSProcessList::hasVmSize(void) const
{
	return (false);
}

bool
OSProcessList::hasVmRss(void) const
{
	return (false);
}

bool 
OSProcessList::hasVmLib(void) const
{
	return (false);
}

#endif


/*
 * The following code should be platform independant.
 */

inline int cmp(int a, int b)
{
	return (a < b ? -1 : (a > b ? 1 : 0));
}

inline int cmp(unsigned int a, unsigned int b)
{
	return (a < b ? -1 : (a > b ? 1 : 0));
}

inline int cmp(long a, long b)
{
	return (a < b ? -1 : (a > b ? 1 : 0));
}

inline int cmp(double a, double b)
{
	return (a < b ? -1 : (a > b ? 1 : 0));
}

int
OSProcessList::compareItems(GCI it1, GCI it2)
{
	OSProcess* item1 = (OSProcess*) it1;
	OSProcess* item2 = (OSProcess*) it2;

	/*
	 * Since the sorting criteria for the process list can vary, we have to
	 * use this rather complex function to compare two processes. Some keys
	 * use descending sorting order which is reflected by the '* -1'.
	 */
	switch (sortCriteria)
	{
	case SORTBY_PID:
		return (cmp(item1->getPid(), item2->getPid()));

	case SORTBY_PPID:
		return (cmp(item1->getPpid(), item2->getPpid()));

	case SORTBY_UID:
		return (cmp(item1->getUid(), item2->getUid()));

	case SORTBY_USERNAME:
		return (strcmp(item1->getUserName(), item2->getUserName()));

	case SORTBY_NAME:
		return (strcmp(item1->getName(), item2->getName()));

	case SORTBY_TIME:
		return (cmp(item1->getUserTime() + item1->getSysTime(),
					item2->getUserTime() + item2->getSysTime()) * -1);

	case SORTBY_PRIORITY:
		return (cmp(item1->getPriority(), item2->getPriority()));

	case SORTBY_STATUS:
		return (strcmp(item1->getStatusTxt(), item2->getStatusTxt()));

	case SORTBY_VMSIZE:
		return (cmp(item1->getVm_size(), item2->getVm_size()) * -1);

	case SORTBY_VMRSS:
		return (cmp(item1->getVm_rss(), item2->getVm_rss()) * -1);

	case SORTBY_VMLIB:
		return (cmp(item1->getVm_lib(), item2->getVm_lib()) * -1);

	case SORTBY_CPU:
	default:
		return (cmp(item1->getUserLoad() + item1->getSysLoad(),
					item2->getUserLoad() + item2->getSysLoad()) * -1);
	}

	return (0);
}
