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

	$Id$
*/

#ifndef modules_h
#define modules_h

#include "Command.h"
#include "conf.h"
#include "ksysguardd.h"

#ifdef OSTYPE_Linux
#include "Memory.h"
#include "ProcessList.h"
#include "apm.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "lmsensors.h"
#include "loadavg.h"
#include "logfile.h"
#include "netdev.h"
#include "netstat.h"
#include "stat.h"
#endif // OSTYPE_Linux

#ifdef OSTYPE_FreeBSD
#include <grp.h>
#include "CPU.h"
#include "Memory.h"
#include "ProcessList.h"
#include "apm.h"
#include "diskstat.h"
#include "loadavg.h"
#include "logfile.h"
#include "netdev.h"
#endif // OSTYPE_FreeBSD

#ifdef OSTYPE_Solaris
#include "LoadAvg.h"
#include "Memory.h"
#include "NetDev.h"
#include "ProcessList.h"
#endif // OSTYPE_Solaris

struct ModulListEntry {
	char *configName;
	void (*initCommand)(void);
	void (*exitCommand)(void);
	void (*updateCommand)(void);
	void (*checkCommand)(void);
};

struct ModulListEntry ModulList[] = {
#ifdef OSTYPE_Linux
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULL },
	{ "Memory", initMemory, exitMemory, updateMemory, NULL },
	{ "Stat", initStat, exitStat, updateStat, NULL },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULL },
	{ "NetStat", initNetStat, exitNetStat, NULL, NULL },
	{ "Apm", initApm, exitApm, updateApm, NULL },
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULL },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULL },
	{ "LmSensors", initLmSensors, exitLmSensors, NULL, NULL },
	{ "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat },
	{ "LogFile", initLogFile, exitLogFile, NULL, NULL }
};
#define NUM_MODULES 11
#endif // OSTYPE_Linux

#ifdef OSTYPE_FreeBSD
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULL },
	{ "Memory", initMemory, exitMemory, updateMemory, NULL },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULL },
	{ "Apm", initApm, exitApm, updateApm, NULL },
	{ "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULL },
	{ "LogFile", initLogFile, exitLogFile, NULL, NULL },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, checkNetDev },
};
#define NUM_MODULES 8
#endif // OSTYPE_FreeBSD

#ifdef OSTYPE_Solaris
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULL },
	{ "Memory", initMemory, exitMemory, updateMemory, NULL },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULL },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULL },
};
#define NUM_MODULES 4
#endif // OSTYPE_Solaris

#endif // modules_h
