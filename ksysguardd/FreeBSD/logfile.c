/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Command.h"
#include "ccont.h"
#include "conf.h"
#include "ksysguardd.h"
#include "logfile.h"

static CONTAINER LogFiles = 0;
static unsigned long counter = 1;

typedef struct {
	char name[256];
	FILE* fh;
	unsigned long id;
} LogFileEntry;

extern CONTAINER LogFileList;

/*
================================ public part =================================
*/

void initLogFile(struct SensorModul* sm)
{
	char monitor[1024];
	ConfigLogFile *entry;

	registerCommand("logfile_register", registerLogFile);
	registerCommand("logfile_unregister", unregisterLogFile);
	registerCommand("logfile_registered", printRegistered);

	for (entry = first_ctnr(LogFileList); entry; entry = next_ctnr(LogFileList))
	{
		FILE* fp;

		/* register the log file if we can actually read the file. */
		if ((fp = fopen(entry->path, "r")) != NULL)
		{
			fclose(fp);
			snprintf(monitor, 1024, "logfiles/%s", entry->name);
			registerMonitor(monitor, "logfile", printLogFile,
							printLogFileInfo, sm);
		}
	}

	LogFiles = new_ctnr();
}

void exitLogFile(void)
{
	destr_ctnr(LogFiles, free);
}

void printLogFile(const char* cmd)
{
	char line[1024];
	unsigned long id;
	int i;
	char ch;
	LogFileEntry *entry;

	sscanf(cmd, "%*s %lu", &id);

	for (entry = first_ctnr(LogFiles); entry; entry = next_ctnr(LogFiles)) {
		if (entry->id == id) {
			while (fgets(line, sizeof(line), entry->fh) != NULL) {
				fprintf(CurrentClient, "%s", line);
			}
			clearerr(entry->fh);
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
	char name[257];
	FILE* file;
	LogFileEntry *entry;
	ConfigLogFile *conf;

	memset(name, 0, sizeof(name));
	sscanf(cmd, "%*s %256s", name);
	
	for (conf = first_ctnr(LogFileList); conf; conf = next_ctnr(LogFileList)) {
		if (!strcmp(conf->name, name)) {
			if ((file = fopen(conf->path, "r")) == NULL) {
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
			strlcpy(entry->name, conf->name, sizeof(entry->name));
			entry->id = counter;

			push_ctnr(LogFiles, entry);	

			fprintf(CurrentClient, "%lu\n", counter);
			counter++;

			return;
		}
	}

	fprintf(CurrentClient, "0\n");
}

void unregisterLogFile(const char* cmd)
{
	unsigned long id;
	LogFileEntry *entry;

	sscanf(cmd, "%*s %lu", &id);
	
	for (entry = first_ctnr(LogFiles); entry; entry = next_ctnr(LogFiles)) {
		if (entry->id == id) {
			fclose(entry->fh);
			free(remove_ctnr(LogFiles));
			fprintf(CurrentClient, "\n");
			return;
		}
	}

	fprintf(CurrentClient, "\n");
}

void printRegistered(const char* cmd)
{
	LogFileEntry *entry;

	for (entry = first_ctnr(LogFiles); entry; entry = next_ctnr(LogFiles))
		fprintf(CurrentClient, "%s:%lu\n", entry->name, entry->id);

	fprintf(CurrentClient, "\n");
}
