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

#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>

#include <kapp.h>

#include "OSProcessList.h"
#include "TimeStampList.h"

TimeStampList* OldTStamps = NULL;
TimeStampList* NewTStamps = NULL;

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

OSProcess::OSProcess(const char* pidStr)
{
	char buffer[1024];
	FILE* fd;

	error = false;

	QString buf;
	buf.sprintf("/proc/%s/status", pidStr);
	if((fd = fopen(buf, "r")) == 0)
	{
		error = true;
		errMessage.sprintf(i18n("Cannot open %s!\n"), buf.data());
		return;
	}

	fscanf(fd, "%s %s", buffer, name);
	fscanf(fd, "%s %c %*s", buffer, &status);
	fscanf(fd, "%*s %*d");
	fscanf(fd, "%*s %*d");
	fscanf(fd, "%*s %d %*d %*d %*d", (int*) &uid);
	fscanf(fd, "%*s %d %*d %*d %*d", (int*) &gid);
	fscanf(fd, "%s %d %*s\n", buffer, &vm_size);
	if (strcmp(buffer, "VmSize:"))
		vm_size = 0;
	fscanf(fd, "%s %d %*s\n", buffer, &vm_lock);
	if (strcmp(buffer, "VmLck:"))
		vm_lock = 0;
	fscanf(fd, "%s %d %*s\n", buffer, &vm_rss);
	if (strcmp(buffer, "VmRSS:"))
		vm_rss = 0;
	fscanf(fd, "%s %d %*s\n", buffer, &vm_data);
	if (strcmp(buffer, "VmData:"))
		vm_data = 0;
	fscanf(fd, "%s %d %*s\n", buffer, &vm_stack);
	if (strcmp(buffer, "VmStk:"))
		vm_stack = 0;
	fscanf(fd, "%s %d %*s\n", buffer, &vm_exe);
	if (strcmp(buffer, "VmExe:"))
		vm_exe = 0;
	fscanf(fd, "%s %d %*s\n", buffer, &vm_lib);
	if (strcmp(buffer, "VmLib:"))
		vm_lib = 0;
	fclose(fd);

    buf.sprintf("/proc/%s/stat", pidStr);
	if ((fd = fopen(buf, "r")) == 0)
	{
		error = true;
		errMessage.sprintf(i18n("Cannot open %s!\n"), buf.data());
		return;
	}
    
	fscanf(fd, "%d %*s %c %d %d %*d %d %*d %*d %*d %*d %*d %*d %d %d",
		   (int*) &pid, &status, (int*) &ppid, (int*) &gid, &ttyNo,
		   &userTime, &sysTime);
	fclose(fd);

	switch (status)
	{
	case 'R':
		statusTxt = i18n("Run");
		break;
	case 'S':
		statusTxt = i18n("Sleep");
		break;
	case 'D': 
		statusTxt = i18n("Disk");
		break;
	case 'Z': 
		statusTxt = i18n("Zombie");
		break;
	case 'T': 
		statusTxt = i18n("Stop");
		break;
	case 'W': 
		statusTxt = i18n("Swap");
		break;
	default:
		statusTxt = i18n("????");
		break;
	}

	TimeStamp* ts = new TimeStamp(pid, userTime, sysTime);
	NewTStamps->inSort(ts);

	if (OldTStamps->find(ts) >= 0)
	{
		int lastCentStamp = OldTStamps->current()->getCentStamp();
		int lastUserTime = OldTStamps->current()->getUserTime();
		int lastSysTime = OldTStamps->current()->getSysTime();

		int timeDiff = ts->getCentStamp() - lastCentStamp;
		int userDiff = userTime - lastUserTime;
		int sysDiff =  sysTime - lastSysTime;

		if (timeDiff > 0)
		{
			userLoad = ((double) userDiff / timeDiff) * 100.0;
			sysLoad = ((double) sysDiff / timeDiff) * 100.0;
		}
		else
			sysLoad = userLoad = 0.0;
	}
	else
		sysLoad = userLoad = 0.0;

	// find out user name with the process uid
	struct passwd* pwent = getpwuid(uid);
	userName = pwent ? pwent->pw_name : "????";
}

OSProcessList::OSProcessList()
{
	error = false;

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

	NewTStamps = new TimeStampList;
	// If there is no old list yet, we create an empty one.
	if (!OldTStamps)
		OldTStamps = new TimeStampList;

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
			OSProcess* ps = new OSProcess(entry->d_name);
			if (!ps || !ps->ok())
			{
				error = true;
				if (ps)
					errMessage = ps->getErrMessage();
				else
					errMessage = i18n("Cannot read status of processes "
									  "from /proc/* directories!\n");
				delete ps;
				return (false);
			}

			// insert process into sorted list
			inSort(ps);
		}
	}
	closedir(dir);

	// make new list old one and discard the really old one
	delete OldTStamps;
	OldTStamps = NewTStamps;
	NewTStamps = NULL;

	return (true);
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

	case SORTBY_STATUS:
		return (cmp(item1->getStatus(), item2->getStatus()));

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
