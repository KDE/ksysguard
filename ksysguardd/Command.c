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

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "Command.h"
#include "ccont.h"
#include "ksysguardd.h"

typedef struct
{
	char* command;
	cmdExecutor ex;
	char* type;
	int isMonitor;
	struct SensorModul* sm;
} Command;

static CONTAINER CommandList;
static sigset_t SignalSet;

void _Command(void* v);

void 
_Command(void* v)
{
	if (v) {
		Command* c = v;
		if (c->command)
			free (c->command);
		if (c->type)
			free (c->type);
		free (v);
	}
}

/*
================================ public part =================================
*/

int ReconfigureFlag = 0;
int CheckSetupFlag = 0;

void
print_error(const char *fmt, ...)
{
	char errmsg[1024];
	va_list az;
	
	va_start(az, fmt);
	vsnprintf(errmsg, sizeof(errmsg)-1, fmt, az);
        errmsg[sizeof(errmsg)-1] = '\0';
	va_end(az);

	if (CurrentClient)
		fprintf(CurrentClient, "\033%s\033", errmsg);
}

void
log_error(const char *fmt, ...)
{
	char errmsg[1024];
	va_list az;
	
	va_start(az, fmt);
	vsnprintf(errmsg, sizeof(errmsg)-1, fmt, az);
        errmsg[sizeof(errmsg)-1] = '\0';
	va_end(az);

	openlog("ksysguardd", LOG_PID, LOG_DAEMON);
	syslog(LOG_ERR, "%s", errmsg);
	closelog();
}

void
initCommand(void)
{
	CommandList = new_ctnr();
	sigemptyset(&SignalSet);
	sigaddset(&SignalSet, SIGALRM);

	registerCommand("monitors", printMonitors);
	registerCommand("test", printTest);

	if (RunAsDaemon == 0)
		registerCommand("quit", exQuit);
}

void
exitCommand(void)
{
	destr_ctnr(CommandList, _Command);
}

void 
registerCommand(const char* command, cmdExecutor ex)
{
	Command* cmd = (Command*) malloc(sizeof(Command));
	cmd->command = (char*) malloc(strlen(command) + 1);
	strcpy(cmd->command, command);
	cmd->type = 0;
	cmd->ex = ex;
	cmd->isMonitor = 0;
	push_ctnr(CommandList, cmd);
	ReconfigureFlag = 1;
}

void
removeCommand(const char* command)
{
	Command* cmd;

	for (cmd = first_ctnr(CommandList); cmd; cmd = next_ctnr(CommandList))
	{
		if (strcmp(cmd->command, command) == 0)
		{
			remove_ctnr(CommandList);
			if (cmd->command)
				free(cmd->command);
			if (cmd->type)
				free(cmd->type);
			free(cmd);
		}
	}
	ReconfigureFlag = 1;
}

void
registerMonitor(const char* command, const char* type, cmdExecutor ex,
				cmdExecutor iq, struct SensorModul* sm)
{
	/* Monitors are similar to regular commands except that every monitor
	 * registers two commands. The first is the value request command and
	 * the second is the info request command. The info request command is
	 * identical to the value request but with an '?' appended. The value
	 * command prints a single value. The info request command prints
	 * a description of the monitor, the mininum value, the maximum value
	 * and the unit. */
	Command* cmd = (Command*) malloc(sizeof(Command));
	cmd->command = (char*) malloc(strlen(command) + 1);
	strcpy(cmd->command, command);
	cmd->ex = ex;
	cmd->type = (char*) malloc(strlen(type) + 1);
	strcpy(cmd->type, type);
	cmd->isMonitor = 1;
	cmd->sm = sm;
	push_ctnr(CommandList, cmd);

	cmd = (Command*) malloc(sizeof(Command));
	cmd->command = (char*) malloc(strlen(command) + 2);
	strcpy(cmd->command, command);
	cmd->command[strlen(command)] = '?';
	cmd->command[strlen(command) + 1] = '\0';
	cmd->ex = iq;
	cmd->isMonitor = 0;
	cmd->sm = sm;
	cmd->type = 0;
	push_ctnr(CommandList, cmd);
}

void
removeMonitor(const char* command)
{
	char* buf;

	removeCommand(command);
	buf = (char*) malloc(strlen(command) + 2);
	strcpy(buf, command);
	strcat(buf, "?");
	removeCommand(buf);
	free(buf);
}

void
executeCommand(const char* command)
{
	int i;
	char tokenFormat[64];
	char token[64];

	sprintf(tokenFormat, "%%%ds", (int) sizeof(token) - 1);
	sscanf(command, tokenFormat, token);

	for (i = 0; i < level_ctnr(CommandList); i++)
	{
		Command* cmd = (Command*) get_ctnr(CommandList, i);
		if (strcmp(cmd->command, token) == 0)
		{
			if (cmd->isMonitor)
			{
				if ((time(NULL) - cmd->sm->time) > UPDATEINTERVAL) {
					cmd->sm->time = time(NULL);
					
					if (cmd->sm->updateCommand != NULL) /* should never happen */
						cmd->sm->updateCommand();
				}
			}

			(*(cmd->ex))(command);

			if (ReconfigureFlag)
			{
				ReconfigureFlag = 0;
				print_error("RECONFIGURE\n");
			}

			fflush(CurrentClient);
			return;
		}
	}

	if (CurrentClient) {
		fprintf(CurrentClient, "UNKNOWN COMMAND\n");
		fflush(CurrentClient);
	}
}

void
printMonitors(const char *c)
{
	int i;
	ReconfigureFlag = 0;

	(void) c;
	for (i = 0; i < level_ctnr(CommandList); i++)
	{
		Command* cmd = (Command*) get_ctnr(CommandList, i);

		if (cmd->isMonitor)
			fprintf(CurrentClient, "%s\t%s\n", cmd->command, cmd->type);
	}
	fflush(CurrentClient);
}

void
printTest(const char* c)
{
	int i;

	for (i = 0; i < level_ctnr(CommandList); i++)
	{
		Command* cmd = (Command*) get_ctnr(CommandList, i);

		if (strcmp(cmd->command, c + strlen("test ")) == 0)
		{
			fprintf(CurrentClient, "1\n");
			fflush(CurrentClient);
			return;
		}
	}
	fprintf(CurrentClient, "0\n");
	fflush(CurrentClient);
}

void
exQuit(const char* cmd)
{
	(void) cmd;
	QuitApp = 1;
}
