/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	$Id$
*/

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "Dispatcher.h"
#include "Command.h"
#include "ProcessList.h"
#include "Memory.h"
#include "stat.h"
#include "netdev.h"
#include "apm.h"

#define CMDBUFSIZE 128

int QuitApp = 0;

static void
readCommand(char* cmdBuf)
{
	int i;
	int c;

	for (i = 0; i < CMDBUFSIZE - 1 && (c = getchar()) != '\n'; i++)
		cmdBuf[i] = (char) c;
	cmdBuf[i] = '\0';
}

/*
================================ public part =================================
*/

void
exQuit(const char* cmd)
{
	QuitApp = 1;
}
	
int
main(int argc, const char* argv[])
{
	char cmdBuf[CMDBUFSIZE];

	initCommand();
	registerCommand("quit", exQuit);
	initProcessList();
	initMemory();
	initStat();
	initNetDev();
	initApm();
	initDispatcher();

	while (!dispatcherReady())
		;

	printf("ksysguardd %s  (c) 1999, 2000 Chris Schlaeger <cs@kde.org>\n"
		   "This program is part of the KDE Project and licensed under\n"
		   "the GNU GPL version 2. See www.kde.org for details!\n"
		   "ksysguardd> ", VERSION);
	fflush(stdout);
	do
	{
		readCommand(cmdBuf);
		executeCommand(cmdBuf);
		printf("ksysguardd> ");
		fflush(stdout);
	} while (!QuitApp);

	exitApm();
	exitDispatcher();
	exitNetDev();
	exitStat();
	exitMemory();
	exitProcessList();
	exitCommand();

	return (0);
}
