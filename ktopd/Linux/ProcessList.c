/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
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

	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/user.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>

#include "ccont.h"
#include "Command.h"
#include "ProcessList.h"

#ifndef PAGE_SIZE /* This happens on SPARC */
#include <asm/page.h>
#endif

CONTAINER ProcessList = 0;

#define BUFSIZE 1024

typedef struct
{
	/* This flag is set for all found processes at the beginning of the
	 * process list update. Processes that do not have this flag set will
	 * be assumed dead and removed from the list. The flag is cleared after
	 * each list update. */
	int alive;

	/* the process ID */
	pid_t pid;

	/* the parent process ID */
	pid_t ppid;

	/* the real user ID */
	uid_t uid;

	/* the real group ID */
	gid_t gid;

	/* a character description of the process status */
    char status;

	/* the number of the tty the process owns */
	int ttyNo;

	/*
	 * The nice level. The range should be -20 to 20. I'm not sure
	 * whether this is true for all platforms.
	 */
	int niceLevel;

	/*
	 * The scheduling priority.
	 */
	int priority;

	/*
	 * The total amount of memory the process uses. This includes shared and
	 * swapped memory.
	 */
	unsigned int vmSize;

	/*
	 * The amount of physical memory the process currently uses.
	 */
	unsigned int vmRss;

	/*
	 * The amount of memory (shared/swapped/etc) the process shares with
	 * other processes.
	 */
	unsigned int vmLib;

	/*
	 * The number of 1/100 of a second the process has spend in user space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
	unsigned int userTime;

	/*
	 * The number of 1/100 of a second the process has spend in system space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
	unsigned int sysTime;

	/* system time as multime of 100ms */
	int centStamp;

	/* the current CPU load (in %) from user space */
	double userLoad;

	/* the current CPU load (in %) from system space */
	double sysLoad;

	/* the name of the process */
	char name[64];

	/* the command used to start the process */
	char cmdline[256];

	/* the login name of the user that owns this process */
	char userName[32];
} ProcessInfo;

static unsigned ProcessCount;

static int 
processCmp(void* p1, void* p2)
{
	return (((ProcessInfo*) p1)->pid - ((ProcessInfo*) p2)->pid);
}

static ProcessInfo*
findProcessInList(int pid)
{
	ProcessInfo key;
	long index;

	key.pid = pid;
	if ((index = search_ctnr(ProcessList, processCmp, &key)) < 0)
		return (0);

	return (get_ctnr(ProcessList, index));
}

static int
updateProcess(int pid)
{
	ProcessInfo* ps;
	FILE* fd;
	char buf[BUFSIZE];
	int userTime, sysTime;
	struct passwd* pwent;

	if ((ps = findProcessInList(pid)) == 0)
	{
		ps = (ProcessInfo*) malloc(sizeof(ProcessInfo));
		ps->pid = pid;
		ps->centStamp = 0;
		push_ctnr(ProcessList, ps);
		bsort_ctnr(ProcessList, processCmp, 0);
	}

	ps->alive = 1;
	snprintf(buf, BUFSIZE - 1, "/proc/%d/status", pid);
	if((fd = fopen(buf, "r")) == 0)
	{
		printf("Error: Cannot open %s!\n", buf);
		return (-1);
	}

	fscanf(fd, "%*s %63s", ps->name);
	fscanf(fd, "%*s %*c %*s");
	fscanf(fd, "%*s %*d");
	fscanf(fd, "%*s %*d");
	fscanf(fd, "%*s %d %*d %*d %*d", (int*) &ps->uid);
	fscanf(fd, "%*s %*d %*d %*d %*d");
	fscanf(fd, "%*s %*d %*d %*d %*d");
	fscanf(fd, "%*s %*d %*s");	/* VmSize */
	fscanf(fd, "%*s %*d %*s");	/* VmLck */
	fscanf(fd, "%*s %*d %*s");	/* VmRSS */
	fscanf(fd, "%*s %*d %*s");	/* VmData */
	fscanf(fd, "%*s %*d %*s");	/* VmStk */
	fscanf(fd, "%*s %*d %*s");	/* VmExe */
	fscanf(fd, "%8s %d %*s", buf, &ps->vmLib);	/* VmLib */
	buf[7] = '\0';
	if (strcmp(buf, "VmLib:") != 0)
		ps->vmLib = 0;
	else
		ps->vmLib *= 1024;

	fclose(fd);

    snprintf(buf, BUFSIZE - 1, "/proc/%d/stat", pid);
	buf[BUFSIZE - 1] = '\0';
	if ((fd = fopen(buf, "r")) == 0)
	{
		sprintf("Cannot open %s!\n", buf);
		return (-1);
	}

	fscanf(fd, "%*d %*s %c %d %d %*d %d %*d %*u %*u %*u %*u %*u %d %d"
		   "%*d %*d %*d %d %*u %*u %*d %u %u",
		   &ps->status, (int*) &ps->ppid, (int*) &ps->gid, &ps->ttyNo,
		   &userTime, &sysTime, &ps->niceLevel, &ps->vmSize,
		   &ps->vmRss);

	ps->vmRss = (ps->vmRss + 3) * PAGE_SIZE;

	if (ps->centStamp > 0)
	{
		int newCentStamp;
		int timeDiff, userDiff, sysDiff;
		struct timeval tv;

		gettimeofday(&tv, 0);
		newCentStamp = tv.tv_sec * 100 + tv.tv_usec / 10000;

		timeDiff = newCentStamp - ps->centStamp;
		userDiff = userTime - ps->userTime;
		sysDiff =  sysTime - ps->sysTime;

		if (timeDiff > 0)
		{
			ps->userLoad = ((double) userDiff / timeDiff) * 100.0;
			ps->sysLoad = ((double) sysDiff / timeDiff) * 100.0;
		}
		else
			ps->sysLoad = ps->userLoad = 0.0;

		ps->centStamp = newCentStamp;
		ps->userTime = userTime;
		ps->sysTime = sysTime;
	}
	else
		ps->sysLoad = ps->userLoad = 0.0;

	fclose(fd);

	snprintf(buf, BUFSIZE - 1, "/proc/%d/cmdline", pid);
	if ((fd = fopen(buf, "r")) == 0)
	{
		sprintf(buf, "Cannot open %s!\n", buf);
		return (-1);
	}
	ps->cmdline[0] = '\0';
	sprintf(buf, "%%%d[^\n]", sizeof(ps->cmdline) - 1);
	fscanf(fd, buf, ps->cmdline);
	ps->cmdline[sizeof(ps->cmdline) - 1] = '\0';
	fclose(fd);

	/* find out user name with the process uid */
	pwent = getpwuid(ps->uid);
	if (pwent)
	{
		strncpy(ps->userName, pwent->pw_name, sizeof(ps->userName) - 1);
		ps->userName[sizeof(ps->userName) - 1] = '\0';
	}

	return (0);
}

static void
cleanupProcessList(void)
{
	int i;

	ProcessCount = 0;
	/* All processes that do not have the active flag set are assumed dead
	 * and will be removed from the list. The alive flag is cleared. */
	for (i = 0; i < level_ctnr(ProcessList); i++)
	{
		ProcessInfo* ps = get_ctnr(ProcessList, i);
		if (ps->alive)
		{
			/* Process is still alive. Just clear flag. */
			ps->alive = 0;
			ProcessCount++;
		}
		else
		{
			/* Process has probably died. We remove it from the list and
			 * destruct the data structure. i needs to be decremented so
			 * that after i++ the next list element will be inspected. */
			free(remove_ctnr(ProcessList, i--));
		}
	}
}

/*
================================ public part =================================
*/

void
initProcessList(void)
{
	ProcessList = new_ctnr(CT_DLL);

	registerMonitor("pscount", "integer", printProcessCount,
					printProcessCountInfo);
	registerMonitor("ps", "table", printProcessList, printProcessListInfo);
}

void
exitProcessList(void)
{
	if (ProcessList)
		free (ProcessList);
}

int
updateProcessList(void)
{
	DIR* dir;
	struct dirent* entry;

	/* read in current process list via the /proc filesystem entry */

	if ((dir = opendir("/proc")) == NULL)
	{
		printf("ERROR: Cannot open directory \'/proc\'!\n"
			   "The kernel needs to be compiled with support\n"
			   "for /proc filesystem enabled!");
		return (-1);
	}
	while ((entry = readdir(dir))) 
	{
		if (isdigit(entry->d_name[0]))
		{
			int pid;

			pid = atoi(entry->d_name);
			updateProcess(pid);
		}
	}
	closedir(dir);

	cleanupProcessList();

	return (0);
}

void
printProcessListInfo(const char* cmd)
{
	printf("Name\tPID\tPPID\tStatus\tNice\tVmSize\tVmRss\tVmLib\tUser"
		   "\tSystem\tUser\tCommand\n");
	printf("s\td\td\ts\td\td\td\td\tf\tf\ts\ts\n");
}

void
printProcessList(const char* cmd)
{
	int i;

	for (i = 0; i < level_ctnr(ProcessList); i++)
	{
		ProcessInfo* ps = get_ctnr(ProcessList, i);

		printf("%s\t%d\t%d\t%c\t%d\t%d\t%d\t%d\t%3.2f%%\t%3.2f%%\t%s\t%s\n",
			   ps->name, (int) ps->pid, (int) ps->ppid, ps->status,
			   ps->niceLevel, ps->vmSize, ps->vmRss, ps->vmLib,
			   ps->userLoad, ps->sysLoad, ps->userName, ps->cmdline);
	}
}

void
printProcessCount(const char* cmd)
{
	printf("%d\n", ProcessCount);
}

void
printProcessCountInfo(const char* cmd)
{
	printf("Number of Processes\t1\t65535\t\n");
}
