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

#include <stdlib.h>
#include <stdio.h>

#include "ccont.h"
#include "Command.h"

typedef struct
{
	char* command;
	cmdExecutor ex;
        char *type;
	int isMonitor;
} Command;

static CONTAINER CommandList;

/*
================================ public part ==================================
*/

void
initCommand(void)
{
	CommandList = new_ctnr(CT_SLL);

	registerCommand("monitors", printMonitors);
}

void
exitCommand(void)
{
	destr_ctnr(CommandList, (DESTR_FUNC) NIL);
}

void
registerCommand(const char* command, cmdExecutor ex)
{
	Command* cmd = (Command*) malloc(sizeof(Command));
	cmd->command = (char*) malloc(strlen(command) + 1);
	strcpy(cmd->command, command);
	cmd->ex = ex;
	cmd->isMonitor = 0;
	push_ctnr(CommandList, cmd);
}

void
registerMonitor(const char* command, const char *type, cmdExecutor ex, cmdExecutor iq)
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
	cmd->isMonitor = 1;
	cmd->type = (char *) malloc(strlen(type) + 1);
	strcpy(cmd->type, type);
	push_ctnr(CommandList, cmd);

	cmd = (Command*) malloc(sizeof(Command));
	cmd->command = (char*) malloc(strlen(command) + 2);
	strcpy(cmd->command, command);
	cmd->command[strlen(command)] = '?';
	cmd->ex = iq;
	cmd->isMonitor = 0;
	cmd->type = 0;
	push_ctnr(CommandList, cmd);
}

void
executeCommand(const char* command)
{
	int i;
	char tokenFormat[32];
	char token[32];

	sprintf(tokenFormat, "%%%ds", sizeof(token) - 1);
	sscanf(command, tokenFormat, token);

	for (i = 0; i < level_ctnr(CommandList); i++)
	{
		Command* cmd = (Command*) get_ctnr(CommandList, i);
		if (strcmp(cmd->command, token) == 0)
		{
			(*(cmd->ex))(command);
			return;
		}
	}
	printf("Unkown command\n");
}

void
printMonitors(const char* cmd)
{
	int i;

	for (i = 0; i < level_ctnr(CommandList); i++)
	{
		Command* cmd = (Command*) get_ctnr(CommandList, i);

		if (cmd->isMonitor)
			printf("%s\t%s\n", cmd->command, cmd->type);
	}
}
