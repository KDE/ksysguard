/*
    KTop, the KDE Task Manager

	Copyright (c) 1999-2000 Hans Petter Bieker<bieker@kde.org>
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
#include <sys/param.h>
#include <sys/user.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>


#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/param.h>

#include "ccont.h"
#include "Command.h"
#include "ProcessList.h"

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
	int userTime, sysTime;
	struct passwd* pwent;
	int mib[4];
	struct kinfo_proc p;
	size_t len;

	if ((ps = findProcessInList(pid)) == 0)
	{
		ps = (ProcessInfo*) malloc(sizeof(ProcessInfo));
		ps->pid = pid;
		ps->centStamp = 0;
		push_ctnr(ProcessList, ps);
		bsort_ctnr(ProcessList, processCmp, 0);
	}

	ps->alive = 1;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = pid;

	len = sizeof (p);
	if (sysctl(mib, 4, &p, &len, NULL, 0) == -1 || !len)
		return -1;

	/* ?? */
        ps->pid = p.kp_proc.p_pid;
        ps->ppid = p.kp_eproc.e_ppid;
        strcpy(ps->name, p.kp_proc.p_comm);
        ps->uid = p.kp_eproc.e_ucred.cr_uid;
        ps->gid = p.kp_eproc.e_pgid;

        /* find out user name with the process uid */
        pwent = getpwuid(ps->uid);
	strcpy(ps->userName, pwent ? pwent->pw_name : "????");
        ps->priority = p.kp_proc.p_priority;
        ps->niceLevel = p.kp_proc.p_nice;

        /* this isn't usertime -- it's total time (??) */
#if __FreeBSD_version >= 300000
        ps->userTime = p.kp_proc.p_runtime / 10000;
#else
        ps->userTime = p.kp_proc.p_rtime.tv_sec*100+p.kp_proc.p_rtime.tv_usec/100
#endif
        ps->sysTime = 0;
        ps->userLoad = p.kp_proc.p_pctcpu / 100;
        ps->sysLoad = 0;

        /* memory */
        ps->vmSize =  (p.kp_eproc.e_vm.vm_tsize +
                    p.kp_eproc.e_vm.vm_dsize +
                    p.kp_eproc.e_vm.vm_ssize) * getpagesize();
        ps->vmRss = p.kp_eproc.e_vm.vm_rssize * getpagesize();

        ps->status = p.kp_proc.p_stat;

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
================================ public part ==================================
*/

void
initProcessList(void)
{
	ProcessList = new_ctnr(CT_DLL);

	registerCommand("ps", printProcessList);
	registerCommand("ps?", printProcessListInfo);
	registerMonitor("pscount", "integer", printProcessCount, printProcessCountInfo);
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
        int mib[3];
        size_t len;
        size_t num;
        struct kinfo_proc *p;


        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_ALL;
        sysctl(mib, 3, NULL, &len, NULL, 0);
	p = malloc(len);
        sysctl(mib, 3, p, &len, NULL, 0);

	for (num = 0; num < len / sizeof(struct kinfo_proc); num++)
		updateProcess(p[num].kp_proc.p_pid);

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
