 /*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "Dispatcher.h"
#include "Command.h"
#include "ProcessList.h"
#include "Memory.h"
#include "stat.h"
#include "netdev.h"
#include "apm.h"
#include "cpuinfo.h"
#include "loadavg.h"

/* Special version of perror for use in signal handler functions. */
#define perror(a) write(STDERR_FILENO, (a), strlen(a))

/* This variable will be set to 1 as soon as the first interrupt (SIGALRM)
 * has been received. */
static volatile int DispatcherReady = 0;

static struct sigaction Action, OldAction;

/*
 * signalHandler()
 * Some signals have to be caught, because they require special treatment.
 */
static void 
signalHandler(int sig)
{
	int errnoSave = errno;

	switch (sig)
    {
    case SIGINT:
		break;
	case SIGALRM:
		updateMemory();
		updateStat();
		updateNetDev();
		updateApm();
		updateCpuInfo();
		updateLoadAvg();
		DispatcherReady = 1;
		break;
    case SIGQUIT:
		perror("SIGQUIT received\n");
		break;
    case SIGTERM:
		perror("SIGTERM received\n");
		break;
	default:
		break;
    }
	errno = errnoSave;
}

static void
startTimer(long sec)
{
	struct itimerval dum;
	struct itimerval tv;
	tv.it_interval.tv_sec = sec;
	tv.it_interval.tv_usec = 0;
	tv.it_value.tv_sec = sec;
	tv.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &tv, &dum);
}

/*
================================ public part =================================
*/

void
initDispatcher(void)
{
	Action.sa_handler = signalHandler;
	sigemptyset(&Action.sa_mask);
	sigaddset(&Action.sa_mask, SIGALRM);
	/* make sure that interrupted system calls are restarted. */
	Action.sa_flags = SA_RESTART;
	sigaction(SIGALRM, &Action, &OldAction);

	startTimer(TIMERINTERVAL);
}

void
exitDispatcher(void)
{
	/* restore signal handler */
	sigaction(SIGALRM, &OldAction, 0);
}

int
dispatcherReady(void)
{
	return (DispatcherReady);
}
