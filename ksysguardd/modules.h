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
#endif /* OSTYPE_Linux */

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
#endif /* OSTYPE_FreeBSD */

#ifdef OSTYPE_Solaris
#include "LoadAvg.h"
#include "Memory.h"
#include "NetDev.h"
#include "ProcessList.h"
#endif /* OSTYPE_Solaris */

#ifdef OSTYPE_Irix
#include "LoadAvg.h"
#include "Memory.h"
#include "NetDev.h"
#include "ProcessList.h"
#include "cpu.h"
#endif /* OSTYPE_Irix */

#ifdef OSTYPE_Tru64
#include "LoadAvg.h"
#include "Memory.h"
#include "NetDev.h"
#endif /* OSTYPE_Tru64 */

struct ModulListEntry {
	char *configName;
	void (*initCommand)(void);
	void (*exitCommand)(void);
	int (*updateCommand)(void);
	void (*checkCommand)(void);
};

typedef void (*VVFunc)(void);
#define NULLVVFUNC ((VVFunc) 0)
typedef int (*IVFunc)(void);
#define NULLIVFUNC ((IVFunc) 0)

struct ModulListEntry ModulList[] = {
#ifdef OSTYPE_Linux
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, 
		NULLVVFUNC },
	{ "Memory", 
		initMemory, 
		exitMemory, 
		updateMemory, 
		NULLVVFUNC },
	{ "Stat", initStat, exitStat, updateStat, NULLVVFUNC },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC },
	{ "NetStat", initNetStat, exitNetStat, NULLIVFUNC, NULLVVFUNC },
	{ "Apm", initApm, exitApm, updateApm, NULLVVFUNC },
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC },
	{ "LmSensors", initLmSensors, exitLmSensors, NULLIVFUNC, NULLVVFUNC },
	{ "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat },
	{ "LogFile", initLogFile, exitLogFile, NULLIVFUNC, NULLVVFUNC }
};
#define NUM_MODULES 11
#endif /* OSTYPE_Linux */

#ifdef OSTYPE_FreeBSD
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC },
	{ "Apm", initApm, exitApm, updateApm, NULLVVFUNC },
	{ "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC },
	{ "LogFile", initLogFile, exitLogFile, NULLVVFUNC, NULLVVFUNC },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, checkNetDev },
};
#define NUM_MODULES 8
#endif /* OSTYPE_FreeBSD */

#ifdef OSTYPE_Solaris
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC },
};
#define NUM_MODULES 4
#endif /* OSTYPE_Solaris */

#ifdef OSTYPE_Irix
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC },
};
#define NUM_MODULES 5
#endif /* OSTYPE_Irix */

#ifdef OSTYPE_Tru64
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC },
};
#define NUM_MODULES 3
#endif /* OSTYPE_Tru64 */

#endif /* modules_h */
