/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

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

#include "Command.h"
#include "ksysguardd.h"
#include "loadavg.h"

static double LoadAvg[3];

/*
================================ public part =================================
*/

void
initLoadAvg(struct SensorModul* sm)
{
	if (updateLoadAvg() < 0)
		return;

	registerMonitor("cpu/system/loadavg1", "float", printLoadAvg1, printLoadAvg1Info, sm);
	registerMonitor("cpu/system/loadavg5", "float", printLoadAvg5, printLoadAvg5Info, sm);
	registerMonitor("cpu/system/loadavg15", "float", printLoadAvg15, printLoadAvg15Info, sm);

	/* Monitor names changed from kde3 => kde4. Remain compatible with legacy requests when possible. */
	registerLegacyMonitor("cpu/loadavg1", "float", printLoadAvg1, printLoadAvg1Info, sm);
	registerLegacyMonitor("cpu/loadavg5", "float", printLoadAvg5, printLoadAvg5Info, sm);
	registerLegacyMonitor("cpu/loadavg15", "float", printLoadAvg15, printLoadAvg15Info, sm);
}

void
exitLoadAvg(void)
{
	removeMonitor("cpu/system/loadavg1");
	removeMonitor("cpu/system/loadavg5");
	removeMonitor("cpu/system/loadavg15");

	/* These were registered as legacy monitors */
	removeMonitor("cpu/loadavg1");
	removeMonitor("cpu/loadavg5");
	removeMonitor("cpu/loadavg15");
}

int
updateLoadAvg(void)
{
	return getloadavg(LoadAvg, 3);
}

void
printLoadAvg1(const char* c)
{
	fprintf(CurrentClient, "%f\n", LoadAvg[0]);
}

void
printLoadAvg1Info(const char* c)
{
	fprintf(CurrentClient, "Load average 1 min\t0\t0\t\n");
}

void
printLoadAvg5(const char* c)
{
	fprintf(CurrentClient, "%f\n", LoadAvg[1]);
}

void
printLoadAvg5Info(const char* c)
{
	fprintf(CurrentClient, "Load average 5 min\t0\t0\t\n");
}

void
printLoadAvg15(const char* c)
{
	fprintf(CurrentClient, "%f\n", LoadAvg[2]);
}

void
printLoadAvg15Info(const char* c)
{
	fprintf(CurrentClient, "Load average 15 min\t0\t0\t\n");
}
