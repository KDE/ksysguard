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
#include <sys/time.h>
#include <signal.h>

#include "Dispatcher.h"
#include "Command.h"
#include "ProcessList.h"
#include "Memory.h"
#include "stat.h"
#include "netdev.h"

/* This variable will be set to 1 as soon as the first interrupt (SIGALRM)
 * has been received. */
static volatile int DispatcherReady = 0;

/*
 * signalHandler()
 * Some signals have to be caught, because they require special treatment.
 */
static void 
signalHandler(int sig)
{
	/* restore the trap table */
	if (signal(sig, signalHandler) == SIG_ERR)
	{
		perror("signalHandler");
		exit(1);
	}

	switch (sig)
    {
    case SIGINT:
		break;
	case SIGALRM:
		updateProcessList();
		updateMemory();
		updateStat();
		updateNetDev();
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
}

static void
startTimer(long sec)
{
	struct itimerval dum;
	struct itimerval tv = { { sec, 0 }, { sec, 0 } };

	setitimer(ITIMER_REAL, &tv, &dum);
}

/*
================================ public part =================================
*/

void
initDispatcher(void)
{
	signal(SIGALRM, signalHandler);
	startTimer(TIMERINTERVAL);
}

void
exitDispatcher(void)
{
	/* It would be good programming style to restore the signal table and
	 * the timer here. */
}

int
dispatcherReady(void)
{
	return (DispatcherReady);
}
