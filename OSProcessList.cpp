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

inline int cmp(double a, double b)
{
	return (a < b ? -1 : (a > b ? 1 : 0));
}

bool
OSProcess::loadInfo(const char* pidStr)
{
	char buffer[1024], temp[128];
	FILE* fd;
	int u1, u2, u3, u4;

	sprintf(buffer, "/proc/%s/status", pidStr);
	if((fd = fopen(buffer, "r")) == 0)
		return (false);

	fscanf(fd, "%s %s", buffer, name);
	fscanf(fd, "%s %c %s", buffer, &status, temp);
	switch (status)
	{
	case 'R':
		strcpy(statusTxt,"Run");
		break;
	case 'S':
		strcpy(statusTxt,"Sleep");
		break;
	case 'D': 
		strcpy(statusTxt,"Disk");
		break;
	case 'Z': 
		strcpy(statusTxt,"Zombie");
		break;
	case 'T': 
		strcpy(statusTxt,"Stop");
		break;
	case 'W': 
		strcpy(statusTxt,"Swap");
		break;
	default:
		strcpy(statusTxt,"????");
		break;
	}
	fscanf(fd, "%s %d", buffer, &pid);
	fscanf(fd, "%s %d", buffer, &ppid);
	fscanf(fd, "%s %d %d %d %d", buffer, &u1, &u2, &u3, &u4);
	uid = u1;
	fscanf(fd, "%s %d %d %d %d", buffer, &u1, &u2, &u3, &u4);
	gid = u1;
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

    sprintf(buffer, "/proc/%s/stat", pidStr);
	if ((fd = fopen(buffer, "r")) == 0)
		return (0);
    
	fscanf(fd, "%*s %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d",
		   &userTime, &sysTime);
	fclose(fd);

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

	return (true);
}

void
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

	dir = opendir("/proc");
	while ((entry = readdir(dir))) 
	{
		OSProcess* ps = new OSProcess();
		if (isdigit(entry->d_name[0]) && ps->loadInfo(entry->d_name))
		{
			inSort(ps);
		}
		else
			delete ps;
	}
	closedir(dir);

	// make new list old one and discard the really old one
	delete OldTStamps;
	OldTStamps = NewTStamps;
	NewTStamps = NULL;
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
		return cmp(item1->getPid(), item2->getPid());

	case SORTBY_UID:
		return cmp(item1->getUid(), item2->getUid());

	case SORTBY_NAME:
		return strcmp(item1->getName(), item2->getName());

	case SORTBY_TIME:
		return (cmp(item1->getUserTime() + item1->getSysTime(),
					item2->getUserTime() + item2->getSysTime()) * -1);

	case SORTBY_STATUS:
		return cmp(item1->getStatus(), item2->getStatus());

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
