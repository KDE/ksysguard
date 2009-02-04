/*
    KSysGuard, the KDE System Guard

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <config-workspace.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>
#include <signal.h>

#include "../../gui/SignalIDs.h"
#include "Command.h"
#include "ProcessList.h"
#include "ccont.h"
#include "ksysguardd.h"

CONTAINER ProcessList = 0;

#define BUFSIZE 1024

static kvm_t *kd;

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
	char status[16];

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
updateProcess(int pid, struct kinfo_proc2 *p)
{
	static const char * const statuses[] = { "", "idle","run","sleep","stop","zombie","dead","onproc" };
	
	ProcessInfo* ps;
	struct passwd* pwent;
	char **argv, **a;

	if ((ps = findProcessInList(pid)) == 0)
	{
		ps = (ProcessInfo*) malloc(sizeof(ProcessInfo));
		ps->pid = pid;
		ps->centStamp = 0;
		push_ctnr(ProcessList, ps);
		bsort_ctnr(ProcessList, processCmp);
	}

	ps->alive = 1;

	ps->pid       = p->p_pid;
	ps->ppid      = p->p_ppid;
	ps->uid       = p->p_uid;
	ps->gid       = p->p_gid;
	ps->priority  = p->p_priority - 22; /* why 22? */
	ps->niceLevel = p->p_nice - NZERO;

#if 0
	ps->userTime = p->p_rtime_sec*100+p->p_rtime_usec/100;
	ps->sysTime  = 0;
        ps->sysLoad  = 0;
#endif

        /* memory, process name, process uid */
	/* find out user name with process uid */
	pwent = getpwuid(ps->uid);
	strlcpy(ps->userName,pwent&&pwent->pw_name? pwent->pw_name:"????",sizeof(ps->userName));
	ps->userName[sizeof(ps->userName)-1]='\0';

	ps->userLoad = 100.0 * ((double)(p->p_pctcpu) / FSCALE);
	ps->vmSize   = (p->p_vm_tsize +
			p->p_vm_dsize +
			p->p_vm_ssize) * getpagesize();
	ps->vmRss    = p->p_vm_rssize * getpagesize();
	strlcpy(ps->name,p->p_comm ? p->p_comm : "????", sizeof(ps->name));
	strlcpy(ps->status,(p->p_stat<=7)? statuses[p->p_stat]:"????", sizeof(ps->status));

        /* process command line */
	argv = kvm_getargv2(kd, p, sizeof(ps->cmdline));
	ps->cmdline[0] = '\0';
	if ((a = argv) != NULL) {
		while (*a) {
			strlcat(ps->cmdline, *a, sizeof(ps->cmdline));
			a++;
			strlcat(ps->cmdline, " ", sizeof(ps->cmdline));
		}
	} else {
		strcpy(ps->cmdline, "????");
	}

	return (0);
}

static void
cleanupProcessList(void)
{
	ProcessInfo* ps;

	ProcessCount = 0;
	/* All processes that do not have the active flag set are assumed dead
	 * and will be removed from the list. The alive flag is cleared. */
	for (ps = first_ctnr(ProcessList); ps; ps = next_ctnr(ProcessList))
	{
		if (ps->alive)
		{
			/* Process is still alive. Just clear flag. */
			ps->alive = 0;
			ProcessCount++;
		}
		else
		{
			/* Process has probably died. We remove it from the list and
			 * destruct the data structure. */
			free(remove_ctnr(ProcessList));
		}
	}
}

/*
================================ public part ==================================
*/

void
initProcessList(struct SensorModul* sm)
{
	ProcessList = new_ctnr();

	registerMonitor("ps", "table", printProcessList, printProcessListInfo, sm);
	registerMonitor("pscount", "integer", printProcessCount, printProcessCountInfo, sm);

	if (!RunAsDaemon)
	{
		registerCommand("kill", killProcess);
		registerCommand("setpriority", setPriority);
	}

	kd = kvm_open(NULL, NULL, NULL, KVM_NO_FILES, "kvm_open");

	updateProcessList();
}

void
exitProcessList(void)
{
	removeMonitor("ps");
	removeMonitor("pscount");

	if (ProcessList)
		free (ProcessList);

	kvm_close(kd);
}

int
updateProcessList(void)
{
	int len;
	int num;
	struct kinfo_proc2 *p;

	p = kvm_getproc2(kd, KERN_PROC_ALL, 0,
			 sizeof(struct kinfo_proc2), &len);

	for (num = 0; num < len; num++)
		updateProcess(p[num].p_pid, &p[num]);
	cleanupProcessList();

	return (0);
}

void
printProcessListInfo(const char* cmd)
{
	fprintf(CurrentClient, "Name\tPID\tPPID\tUID\tGID\tStatus\tCPU%%\tPrio\tNice\tVmSize\tVmRss\tLogin\tCommand\n");
	fprintf(CurrentClient, "s\td\td\td\td\tS\tf\td\td\tD\tD\ts\ts\n");
}

void
printProcessList(const char* cmd)
{
	ProcessInfo* ps;

	ps = first_ctnr(ProcessList); /* skip 'kernel' entry */
	for (ps = next_ctnr(ProcessList); ps; ps = next_ctnr(ProcessList))
	{
		fprintf(CurrentClient, "%s\t%ld\t%ld\t%ld\t%ld\t%s\t%.2f\t%d\t%d\t%d\t%d\t%s\t%s\n",
			   ps->name, (long)ps->pid, (long)ps->ppid,
			   (long)ps->uid, (long)ps->gid, ps->status,
			   ps->userLoad, ps->priority, ps->niceLevel,
			   ps->vmSize / 1024, ps->vmRss / 1024, ps->userName, ps->cmdline);
	}
}

void
printProcessCount(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", ProcessCount);
}

void
printProcessCountInfo(const char* cmd)
{
	fprintf(CurrentClient, "Number of Processes\t1\t65535\t\n");
}

void
killProcess(const char* cmd)
{
	int sig, pid;

	sscanf(cmd, "%*s %d %d", &pid, &sig);
	switch(sig)
	{
	case MENU_ID_SIGABRT:
		sig = SIGABRT;
		break;
	case MENU_ID_SIGALRM:
		sig = SIGALRM;
		break;
	case MENU_ID_SIGCHLD:
		sig = SIGCHLD;
		break;
	case MENU_ID_SIGCONT:
		sig = SIGCONT;
		break;
	case MENU_ID_SIGFPE:
		sig = SIGFPE;
		break;
	case MENU_ID_SIGHUP:
		sig = SIGHUP;
		break;
	case MENU_ID_SIGILL:
		sig = SIGILL;
		break;
	case MENU_ID_SIGINT:
		sig = SIGINT;
		break;
	case MENU_ID_SIGKILL:
		sig = SIGKILL;
		break;
	case MENU_ID_SIGPIPE:
		sig = SIGPIPE;
		break;
	case MENU_ID_SIGQUIT:
		sig = SIGQUIT;
		break;
	case MENU_ID_SIGSEGV:
		sig = SIGSEGV;
		break;
	case MENU_ID_SIGSTOP:
		sig = SIGSTOP;
		break;
	case MENU_ID_SIGTERM:
		sig = SIGTERM;
		break;
	case MENU_ID_SIGTSTP:
		sig = SIGTSTP;
		break;
	case MENU_ID_SIGTTIN:
		sig = SIGTTIN;
		break;
	case MENU_ID_SIGTTOU:
		sig = SIGTTOU;
		break;
	case MENU_ID_SIGUSR1:
		sig = SIGUSR1;
		break;
	case MENU_ID_SIGUSR2:
		sig = SIGUSR2;
		break;
	}
	if (kill((pid_t) pid, sig))
	{
		switch(errno)
		{
		case EINVAL:
			fprintf(CurrentClient, "4\t%d\n", pid);
			break;
		case ESRCH:
			fprintf(CurrentClient, "3\t%d\n", pid);
			break;
		case EPERM:
			fprintf(CurrentClient, "2\t%d\n", pid);
			break;
		default:
			fprintf(CurrentClient, "1\t%d\n", pid);	/* unknown error */
			break;
		}

	}
	else
		fprintf(CurrentClient, "0\t%d\n", pid);
}

void
setPriority(const char* cmd)
{
	int pid, prio;

	sscanf(cmd, "%*s %d %d", &pid, &prio);
	if (setpriority(PRIO_PROCESS, pid, prio))
	{
		switch(errno)
		{
		case EINVAL:
			fprintf(CurrentClient, "4\n");
			break;
		case ESRCH:
			fprintf(CurrentClient, "3\n");
			break;
		case EPERM:
		case EACCES:
			fprintf(CurrentClient, "2\n");
			break;
		default:
			fprintf(CurrentClient, "1\n");	/* unknown error */
			break;
		}
	}
	else
		fprintf(CurrentClient, "0\n");
}
