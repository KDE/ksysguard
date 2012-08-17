/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2011 David Naylor <naylor.b.david@gmail.com>
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

#include "ProcessList.h"

#include <ctype.h>
#include <fcntl.h>
#include <paths.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <time.h>
#include <unistd.h>

#include "Command.h"
#include "../../gui/SignalIDs.h"

#define KILL_COMMAND "kill"
#define SETPRIORITY_COMMAND "setpriority"

#define PROCBUF 1024
#define STATEBUF 12
#define NAMEBUF 24
#define UNAMEBUF 12
#define ARGBUF 256
#define PWDBUF 16
#define NAMELEN 128

#define MONITORBUF 20

static struct kinfo_proc proc_buf[PROCBUF], prev_list[PROCBUF];
static int nproc, prev_nproc, sorted_proc[PROCBUF], prev_sorted[PROCBUF];

static struct timespec last_update;
static float scale;

static int pagesize, smpmode;

static pid_t lastpid = 0, procspawn;

static struct {
    uid_t uid;
    char name[NAMELEN];
} pwd_cache[PWDBUF];
static int pwd_size = 0, pwd_hit = 0, pwd_last = 0;

static char *const statuses[] = { "", "IDLE", "RUN", "SLEEP", "STOP", "ZOMBIE", "WAIT", "LOCK" };
static pid_t statcnt[8];
static char (*cpunames)[8] = NULL;

static int cmp_pid(const void *, const void *);
static char *getname(const uid_t);

void initProcessList(struct SensorModul *sm) {
    char name[MONITORBUF];
    size_t len, stat, chr, chr_size;

    if (!RunAsDaemon) {
        registerCommand(KILL_COMMAND, killProcess);
        registerCommand(SETPRIORITY_COMMAND, setPriority);
    }

    len = sizeof(int);
    if (sysctlbyname("kern.smp.active", &smpmode, &len, NULL, 0))
        smpmode = 0;
    else {
        int cpus = 0;
        len = sizeof(int);
        sysctlbyname("kern.smp.cpus", &cpus, &len, NULL, 0);
        cpunames = malloc(8 * sizeof(char) * cpus);
        while (cpus--)
            snprintf(cpunames[cpus], 8, "CPU%d", cpus);
    }

    pagesize = getpagesize() / 1024;

    registerMonitor("processes/ps", "table", printProcessList, printProcessListInfo, sm);
    registerMonitor("processes/lastpid", "integer", printLastPID, printLastPIDInfo, sm);
    registerMonitor("processes/procspawn", "integer", printProcSpawn, printProcSpawnInfo, sm);

    strcpy(name, "processes/ps");
    registerMonitor("processes/pscount", "integer", printProcessCount, printProcessCountInfo, sm);
    for (stat = 1; stat < 8; ++stat) {
        chr_size = strlcpy(name + 12, statuses[stat], MONITORBUF - 12);
        for (chr = 0; chr < chr_size; ++chr)
            name[12 + chr] = tolower(name[12 + chr]);
        registerMonitor(name, "integer", printProcessxCount, printProcessxCountInfo, sm);
    }

    registerLegacyMonitor("ps", "table", printProcessList, printProcessListInfo, sm);
    registerLegacyMonitor("processes/pscount", "integer", printProcessCount, printProcessCountInfo, sm);

    nproc = 0;
    updateProcessList();
}

void exitProcessList(void) {
    char name[MONITORBUF];
    size_t stat, chr, chr_size;

    removeCommand(KILL_COMMAND);
    removeCommand(SETPRIORITY_COMMAND);

    removeMonitor("processes/ps");
    removeMonitor("processes/lastpid");
    removeMonitor("processes/procspawn");

    strcpy(name, "processes/ps");
    removeMonitor("processes/pscount");
    for (stat = 1; stat < 8; ++stat) {
        chr_size = strlcpy(name + 12, statuses[stat], MONITORBUF - 12);
        for (chr = 0; chr < chr_size; ++chr)
            name[12 + chr] = tolower(name[12 + chr]);
        removeMonitor(name);
    }

    removeMonitor("ps");
    removeMonitor("pscount");

    free(cpunames);
    cpunames = NULL;
}

int updateProcessList(void) {
    int proc;
    int mib[3];
    pid_t prevpid = lastpid, pid;
    size_t len;
    struct timespec update;

    memcpy(prev_list, proc_buf, sizeof(struct kinfo_proc) * nproc);
    memcpy(prev_sorted, sorted_proc, sizeof(int) * nproc);
    prev_nproc = nproc;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PROC;
    len = PROCBUF * sizeof(struct kinfo_proc);
    clock_gettime(CLOCK_MONOTONIC, &update);
    sysctl(mib, 3, proc_buf, &len, NULL, 0);
    nproc = len / sizeof(struct kinfo_proc);

    len = sizeof(lastpid);
    sysctlbyname("kern.lastpid", &lastpid, &len, NULL, 0);

    if (nproc > PROCBUF)
        nproc = PROCBUF;
    for (proc = 0; proc < nproc; ++proc)
        sorted_proc[proc] = proc;
    qsort(sorted_proc, nproc, sizeof(int), cmp_pid);

    bzero(statcnt, sizeof(statcnt));
    if (lastpid >= prevpid) {
        procspawn = lastpid - prevpid;
        for (proc = 0; proc < prev_nproc; ++proc) {
            pid = prev_list[prev_sorted[proc]].ki_pid;
            if (prevpid < pid && pid <= lastpid)
                --procspawn;
            ++statcnt[prev_list[prev_sorted[proc]].ki_stat];
        }
    } else {
        procspawn = prevpid - lastpid + 1;
        for (proc = 0; proc < prev_nproc; ++proc) {
            pid = prev_list[prev_sorted[proc]].ki_pid;
            if (pid <= lastpid || pid > prevpid)
                --procspawn;
            ++statcnt[prev_list[prev_sorted[proc]].ki_stat];
        }
    }

    scale = (update.tv_sec - last_update.tv_sec) + (update.tv_nsec - last_update.tv_nsec) / 1000000000.0;
    last_update = update;

    return (0);
}

void printProcessList(const char* cmd)
{
    int proc, prev_proc;
    float load;
    int mib[4];
    char *name, *uname, *state, *arg_fix;
    char buf[STATEBUF + 1], buf2[UNAMEBUF], buf3[NAMEBUF], args[ARGBUF];
    struct kinfo_proc *ps, *last_ps;
    size_t len;

    buf[STATEBUF] = '\0';
    buf3[0] = '[';
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ARGS;

    for (prev_proc = 0, proc = 0; proc < nproc; ++proc) {
        ps = &proc_buf[sorted_proc[proc]];

        mib[3] = ps->ki_pid;
        len = ARGBUF;
        sysctl(mib, 4, args, &len, 0, 0);
        if (!len)
            args[0] = '\0';
        else {
            arg_fix = args;
            while ((arg_fix += strlen(arg_fix)) < args + len - 1)
                *arg_fix = '*';
        }

        if (args[0] == '\0' && (ps->ki_flag & P_SYSTEM || ps->ki_args == NULL)) {
            int cpy;
            cpy = strlcpy(buf3 + 1, ps->ki_comm, NAMEBUF - 1);
            if (cpy > NAMEBUF - 2)
                cpy = NAMEBUF - 2;
            buf3[cpy + 1] = ']';
            buf3[cpy + 2] = '\0';
            name = buf3;
            /* TODO: should kernel processes be displayed? */
            /* continue; */
        } else if (ps->ki_comm != NULL)
            name = ps->ki_comm;
        else
            name = "????";

        switch (ps->ki_stat) {
            case SRUN:
                if (smpmode && ps->ki_oncpu != 0xff)
                    state = cpunames[ps->ki_oncpu];
                else
                    state = statuses[2];
                break;
            case SSLEEP:
                if (ps->ki_wmesg != NULL) {
                    state = ps->ki_wmesg;
                    break;
                }

            case SLOCK:
                if (ps->ki_kiflag & KI_LOCKBLOCK) {
                    snprintf(buf, STATEBUF, "*%s", ps->ki_lockname);
                    state = buf;
                    break;
                }

            case SIDL:
            case SSTOP:
            case SZOMB:
            case SWAIT:
                state = statuses[(int)ps->ki_stat];
                break;

            default:
                snprintf(buf, STATEBUF, "?%d", ps->ki_stat);
                state = buf;
        }

        uname = getname(ps->ki_ruid);
        if (uname[0] == '\0') {
            snprintf(buf2, UNAMEBUF, "%d", ps->ki_ruid);
            uname = buf2;
        }

        for (;;) {
            if (prev_proc >= prev_nproc) {
                last_ps = NULL;
                break;
            }
            last_ps = &prev_list[prev_sorted[prev_proc]];
            if (last_ps->ki_pid == ps->ki_pid &&
                last_ps->ki_start.tv_sec == ps->ki_start.tv_sec &&
                last_ps->ki_start.tv_usec == ps->ki_start.tv_usec)
                break;
            else if (last_ps->ki_pid > ps->ki_pid) {
                last_ps = NULL;
                break;
            }
            ++prev_proc;
        }

        if (last_ps != NULL)
            load = (ps->ki_runtime - last_ps->ki_runtime) / 1000000.0 / scale;
        else
            load = ps->ki_runtime / 1000000.0 / scale;

        if (!ps->ki_pid)
            /* XXX: TODO: add support for displaying kernel process */
            continue;

        fprintf(CurrentClient, "%s\t%ld\t%ld\t%ld\t%ld\t%s\t%.2f\t%.2f\t%d\t%ld\t%ld\t%s\t%s\n",
               name, (long)ps->ki_pid, (long)ps->ki_ppid,
               (long)ps->ki_uid, (long)ps->ki_pgid, state,
               ps->ki_runtime / 1000000.0, load, ps->ki_nice,
               ps->ki_size / 1024, ps->ki_rssize * pagesize, uname, args);
    }
}

void printProcessListInfo(const char* cmd)
{
    fprintf(CurrentClient, "Name\tPID\tPPID\tUID\tGID\tStatus\tUser%%\tSystem%%\tNice\tVmSize\tVmRss\tLogin\tCommand\n");
    fprintf(CurrentClient, "s\td\td\td\td\tS\tf\tf\td\tD\tD\ts\ts\n");
}

void printProcessCount(const char *cmd) {
    fprintf(CurrentClient, "%d\n", nproc);
}

void printProcessCountInfo(const char *cmd) {
    fprintf(CurrentClient, "Number of Processes\t0\t0\t\n");
}

void printProcessxCount(const char *cmd) {
    int idx;

    for (idx = 1; idx < 7; ++idx)
        if (strncasecmp(cmd + 12, statuses[idx], strlen(cmd + 12) - 1) == 0)
            break;

    fprintf(CurrentClient, "%d\n", statcnt[idx]);
}

void printProcessxCountInfo(const char *cmd) {
    int idx;
    static char *const statnames[] = {"", "Idle", "Running", "Sleeping", "Stopped", "Zombie", "Waiting", "Locked" };

    for (idx = 1; idx < 7; ++idx) {
        if (strncasecmp(cmd + 12, statuses[idx], strlen(cmd + 12) - 1) == 0)
            break;
    }

    fprintf(CurrentClient, "%s Processes\t0\t0\t\n", statnames[idx]);
}

void printProcSpawn(const char *cmd) {
    fprintf(CurrentClient, "%u\n", procspawn);
}

void printProcSpawnInfo(const char *cmd) {
    fprintf(CurrentClient, "Number of processes spawned\t0\t0\t1/s\n");
}

void printLastPID(const char *cmd) {
    fprintf(CurrentClient, "%u\n", lastpid);
}

void printLastPIDInfo(const char *cmd) {
    fprintf(CurrentClient, "Last used Process ID\t1\t65535\t\n");
}

void killProcess(const char *cmd)
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
            fprintf(CurrentClient, "1\t%d\n", pid);    /* unknown error */
            break;
        }

    }
    else
        fprintf(CurrentClient, "0\t%d\n", pid);
}

void setPriority(const char *cmd)
{
    int pid, prio;

    sscanf(cmd, "%*s %d %d", &pid, &prio);
    if (setpriority(PRIO_PROCESS, pid, prio))
    {
        switch(errno)
        {
        case EINVAL:
            fprintf(CurrentClient, "4\t%d\t%d\n", pid, prio);
            break;
        case ESRCH:
            fprintf(CurrentClient, "3\t%d\t%d\n", pid, prio);
            break;
        case EPERM:
        case EACCES:
            fprintf(CurrentClient, "2\t%d\t%d\n", pid, prio);
            break;
        default:
            fprintf(CurrentClient, "1\t%d\t%d\n", pid, prio);    /* unknown error */
            break;
        }
    }
    else
        fprintf(CurrentClient, "0\n");
}

int cmp_pid(const void *first_idx, const void *last_idx) {
    struct kinfo_proc *first = &proc_buf[*(int *)first_idx];
    struct kinfo_proc *last = &proc_buf[*(int *)last_idx];

    if (first->ki_pid < last->ki_pid)
        return -1;
    else if (first->ki_pid > last->ki_pid)
        return 1;
    else
        return 0;
}

char *getname(const uid_t uid) {
    int idx;
    struct passwd *pw;

    for (idx = 0; idx < pwd_size; ++idx) {
        if (pwd_cache[pwd_hit].uid == uid)
            return pwd_cache[pwd_hit].name;
        pwd_hit = (pwd_hit + 1) % pwd_size;
    }

    if (pwd_size < PWDBUF)
        pwd_last = pwd_size++;
    else
        pwd_last = (pwd_last + 1) % PWDBUF;
    pwd_hit = pwd_last;

    pwd_cache[pwd_hit].uid = uid;
    pw = getpwuid(uid);
    if (pw == NULL)
        pwd_cache[pwd_hit].name[0] = '\0';
    else
        strlcpy(pwd_cache[pwd_hit].name, pw->pw_name, NAMELEN);

    return (pwd_cache[pwd_hit].name);
}
