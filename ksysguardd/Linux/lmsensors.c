/*
    KSysGuard, the KDE System Guard
	   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

//#define HAVE_SENSORS_SENSORS_H

#ifdef HAVE_SENSORS_SENSORS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sensors/sensors.h>

#include <ccont.h>

#include "Command.h"
#include "ksysguardd.h"
#include "lmsensors.h"

typedef struct
{
	char* fullName;
	const sensors_chip_name* scn;
	const sensors_feature_data* sfd;
} LMSENSOR;

static CONTAINER LmSensors;
static int LmSensorsOk = -1;

static int 
sensorCmp(void* s1, void* s2)
{
	return (strcmp(((LMSENSOR*) s1)->fullName,
				   ((LMSENSOR*) s2)->fullName));
}

static LMSENSOR*
findMatchingSensor(const char* name)
{
	long index;
	LMSENSOR key;
	LMSENSOR* s;

	key.fullName = (char*) malloc(strlen(name) + 1);
	strcpy(key.fullName, name);
	if ((index = search_ctnr(LmSensors, sensorCmp, &key)) < 0)
	{
		free(key.fullName);
		return (0);
	}
	free(key.fullName);
	s = get_ctnr(LmSensors, index);

	return (s);
}

void
initLmSensors(void)
{
	const sensors_chip_name* scn;
	int nr = 0;

	FILE* input;
	if ((input = fopen("/etc/sensors.conf", "r")) == NULL)
	{
		LmSensorsOk = -1;
		return;
	}

	if (sensors_init(input))
	{
		LmSensorsOk = -1;
		fclose(input);
		return;
	}
	fclose(input);

	LmSensors = new_ctnr(CT_SLL);
	while ((scn = sensors_get_detected_chips(&nr)) != NULL)
	{
		int nr1, nr2;
		const sensors_feature_data* sfd;
		nr1 = nr2 = 0;
		while ((sfd = sensors_get_all_features(*scn, &nr1, &nr2)) != 0)
		{
			if (sfd->mapping == SENSORS_NO_MAPPING)
			{
				LMSENSOR* p;
				char* label;
				char* s;

				sensors_get_label(*scn, sfd->number, &label);
				p = (LMSENSOR*) malloc(sizeof(LMSENSOR));

				p->fullName = (char*) malloc(strlen("lmsensors/") +
											 strlen(scn->prefix) + 1 +
											 strlen(label) + 1);
				sprintf(p->fullName, "lmsensors/%s/%s", scn->prefix, label);

				/* Make sure that name contains only propper characters. */
				for (s = p->fullName; *s; s++)
					if (*s == ' ')
						*s = '_';

				p->scn = scn;
				p->sfd = sfd;
				if (search_ctnr(LmSensors, sensorCmp, p) < 0)
				{
					push_ctnr(LmSensors, p);
					registerMonitor(p->fullName, "float", printLmSensor,
									printLmSensorInfo);
				}
				else
				{
					free(p->fullName);
					free(p);
				}
			}
		}
	}
	bsort_ctnr(LmSensors, sensorCmp, 0);
}

void
exitLmSensors(void)
{
	destr_ctnr(LmSensors, free);
}

void
printLmSensor(const char* cmd)
{
	double value;
	LMSENSOR* s;

	if ((s = findMatchingSensor(cmd)) == 0)
	{
		/* should never happen */
		fprintf(CurrentClient, "0\n");
		return;
	}

	sensors_get_feature(*(s->scn), s->sfd->number, &value);
	fprintf(CurrentClient, "%f\n", value);
}

void
printLmSensorInfo(const char* cmd)
{
	LMSENSOR* s;

	if ((s = findMatchingSensor(cmd)) == 0)
	{
		/* should never happen */
		fprintf(CurrentClient, "0\n");
		return;
	}
	/* TODO: print real name here */
	fprintf(CurrentClient, "Sensor Info\t0\t0\t\n");
}

#else /* HAVE_SENSORS_SENSORS_H */

/* dummy version for systems that have no lmsensors support */

void
initLmSensors(void)
{
}

void
exitLmSensors(void)
{
}

#endif
