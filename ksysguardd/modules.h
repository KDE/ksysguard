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
#include "i8k.h"
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
#ifdef __i386__
	#include "apm.h"
#endif
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

#ifdef OSTYPE_OpenBSD
#include "cpu.h"
#include "memory.h"
#endif /* OSTYPE_OpenBSD */

typedef void (*VSFunc)(struct SensorModul*);
#define NULLVSFUNC ((VSFunc) 0)
typedef void (*VVFunc)(void);
#define NULLVVFUNC ((VVFunc) 0)
typedef int (*IVFunc)(void);
#define NULLIVFUNC ((IVFunc) 0)
#define NULLTIME ((time_t) 0)

struct SensorModul SensorModulList[] = {
#ifdef OSTYPE_Linux
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, 
		NULLVVFUNC, 0, NULLTIME },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
	{ "Stat", initStat, exitStat, updateStat, NULLVVFUNC, 0, NULLTIME },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
	{ "NetStat", initNetStat, exitNetStat, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
	{ "Apm", initApm, exitApm, updateApm, NULLVVFUNC, 0, NULLTIME },
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
	{ "LmSensors", initLmSensors, exitLmSensors, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
	{ "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat, 0, NULLTIME },
	{ "LogFile", initLogFile, exitLogFile, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
	{ "DellLaptop", initI8k, exitI8k, updateI8k, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Linux */

#ifdef OSTYPE_FreeBSD
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#ifdef __i386__
	{ "Apm", initApm, exitApm, updateApm, NULLVVFUNC, 0, NULLTIME },
#endif
	{ "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat, 0, NULLTIME },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
	{ "LogFile", initLogFile, exitLogFile, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, checkNetDev, 0, NULLTIME },
#endif /* OSTYPE_FreeBSD */

#ifdef OSTYPE_Solaris
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Solaris */

#ifdef OSTYPE_Irix
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
	{ "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Irix */

#ifdef OSTYPE_Tru64
	{ "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
	{ "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Tru64 */

#ifdef OSTYPE_OpenBSD
	{ "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
	{ "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_OpenBSD */

	{ NULL, NULLVSFUNC, NULLVVFUNC, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME }
};

#endif /* modules_h */
