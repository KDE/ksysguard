/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 2001 Tobias Koenig <tokoe82@yahoo.de>
    
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
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ksysguardd.h"
#include "Command.h"
#include "ccont.h"
#include "logfile.h"

static CONTAINER LogFiles = 0;
static unsigned long counter = 1;

typedef struct {
	char name[256];
	FILE* fh;
	unsigned long id;
} LogFileEntry;

/*
================================ public part =================================
*/

void initLogFile(void)
{
	registerCommand("registerlogfile", registerLogFile);
	registerCommand("unregisterlogfile", unregisterLogFile);
	registerMonitor("logfiles", "logfile", printLogFile, printLogFileInfo);

	LogFiles = new_ctnr(CT_DLL);
}

void exitLogFile(void)
{
	if (LogFiles)
		destr_ctnr(LogFiles, free);
}

void printLogFile(const char* cmd)
{
	char filename[256];
	char line[1024];
	unsigned long id;
	int i;
	
	sscanf(cmd, "%*s %256s %lu", filename, &id);

	for (i = 0; i < level_ctnr(LogFiles); i++) {
		LogFileEntry *entry = get_ctnr(LogFiles, i);

		if (!strcmp(entry->name, filename) && (entry->id == id)) {
			while (fgets(line, 1024, entry->fh) != NULL) {
				fprintf(CurrentClient, "%s", line);
			}
		}
	}

	fprintf(CurrentClient, "\n");
}

void printLogFileInfo(const char* cmd)
{
	fprintf(CurrentClient, "LogFile\n");
}

void registerLogFile(const char* cmd)
{
	char filename[256];
	FILE* file;
	LogFileEntry *entry;

	memset(filename, 0, 256);
	sscanf(cmd, "%*s %256s", filename);
	
	if ((file = fopen(filename, "r")) == NULL) {
		print_error("fopen()");
		fprintf(CurrentClient, "0\n");
		return;
	}

	fseek(file, 0, SEEK_END);

	if ((entry = (LogFileEntry *)malloc(sizeof(LogFileEntry))) == NULL) {
		print_error("malloc()");
		fprintf(CurrentClient, "0\n");
		return;
	}

	entry->fh = file;
	strncpy(entry->name, filename, 256);
	entry->id = counter;

	push_ctnr(LogFiles, entry);	

	fprintf(CurrentClient, "%lu\n", counter);

	counter++;
}

void unregisterLogFile(const char* cmd)
{
	char filename[256];
	unsigned long id;
	int i;

	memset(filename, 0, 256);
	sscanf(cmd, "%*s %256s %lu", filename, &id);
	
	for (i = 0; i < level_ctnr(LogFiles); i++) {
		LogFileEntry *entry = get_ctnr(LogFiles, i);

		if (!strcmp(entry->name, filename) && (entry->id == id)) {
			fclose(entry->fh);
			free(remove_ctnr(LogFiles, i));
			fprintf(CurrentClient, "\n");
			return;
		}
	}

	fprintf(CurrentClient, "\n");
}
