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

/* TODO: Port to Win32 */

#ifndef KSG_MODULES_H
#define KSG_MODULES_H

#include <config-ksysguardd.h>
#include "Command.h"
#include "conf.h"
#include "ksysguardd.h"

#ifdef OSTYPE_Linux
#include "acpi.h"
#include "apm.h"
#include "cpuinfo.h"
#include "diskstat.h"
#include "diskstats.h"
#include "i8k.h"
#include "lmsensors.h"
#include "loadavg.h"
#include "logfile.h"
#include "Memory.h"
#include "netdev.h"
#include "netstat.h"
#include "ProcessList.h"
#include "stat.h"
#include "softraid.h"
#include "uptime.h"
#endif /* OSTYPE_Linux */

#if defined(OSTYPE_FreeBSD) || defined(OSTYPE_DragonFly)
#include <grp.h>
#ifdef __i386__
 #include "apm.h"
#endif
#include "CPU.h"
#include "diskstat.h"
#include "loadavg.h"
#include "logfile.h"
#include "Memory.h"
#include "netdev.h"
#include "ProcessList.h"
#endif /* OSTYPE_FreeBSD */

#ifdef OSTYPE_Irix
#include "cpu.h"
#include "LoadAvg.h"
#include "Memory.h"
#include "NetDev.h"
#include "ProcessList.h"
#endif /* OSTYPE_Irix */

#ifdef OSTYPE_NetBSD
#include <grp.h>
#ifdef __i386__
 #include "apm.h"
#endif
#include "CPU.h"
#include "diskstat.h"
#include "loadavg.h"
#include "logfile.h"
#include "Memory.h"
#include "netdev.h"
#include "ProcessList.h"
#endif /* OSTYPE_NetBSD */

#ifdef OSTYPE_OpenBSD
#include "cpu.h"
#include "memory.h"
#include "ProcessList.h"
#endif /* OSTYPE_OpenBSD */

#if defined(OSTYPE_Solaris) || defined(OSTYPE_SunOS)
#include "LoadAvg.h"
#include "Memory.h"
#include "NetDev.h"
#include "ProcessList.h"
#endif /* OSTYPE_Solaris */

#ifdef OSTYPE_Tru64
#include "LoadAvg.h"
#include "Memory.h"
#include "NetDev.h"
#endif /* OSTYPE_Tru64 */


typedef void (*VSFunc)( struct SensorModul* );
#define NULLVSFUNC ((VSFunc) 0)
typedef void (*VVFunc)( void );
#define NULLVVFUNC ((VVFunc) 0)
typedef int (*IVFunc)( void );
#define NULLIVFUNC ((IVFunc) 0)
#define NULLTIME ((time_t) 0)

struct SensorModul SensorModulList[] = {
#ifdef OSTYPE_Linux
  { "Acpi", initAcpi, exitAcpi, updateAcpi, NULLVVFUNC, 0, NULLTIME },
  { "Apm", initApm, exitApm, updateApm, NULLVVFUNC, 0, NULLTIME },
  { "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
  { "DellLaptop", initI8k, exitI8k, updateI8k, NULLVVFUNC, 0, NULLTIME },
  { "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat, 0, NULLTIME },
  { "DiskStats", initDiskstats, exitDiskstats, updateDiskstats, NULLVVFUNC, 0, NULLTIME },
#ifdef HAVE_LMSENSORS
  { "LmSensors", initLmSensors, exitLmSensors, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
#endif
  { "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
  { "LogFile", initLogFile, exitLogFile, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
  { "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
  { "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
  { "NetStat", initNetStat, exitNetStat, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
  { "ProcessList", initProcessList, exitProcessList, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
  { "Stat", initStat, exitStat, updateStat, NULLVVFUNC, 0, NULLTIME },
  { "SoftRaid", initSoftRaid, exitSoftRaid, updateSoftRaid, NULLVVFUNC, 0, NULLTIME },
  { "Uptime", initUptime, exitUptime, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Linux */

#if defined OSTYPE_FreeBSD || defined OSTYPE_DragonFly
  #ifdef __i386__
    { "Apm", initApm, exitApm, updateApm, NULLVVFUNC, 0, NULLTIME },
  #endif
  { "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
  { "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat, 0, NULLTIME },
  { "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
  { "LogFile", initLogFile, exitLogFile, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
  { "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
  { "NetDev", initNetDev, exitNetDev, updateNetDev, checkNetDev, 0, NULLTIME },
  { "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_FreeBSD */

#ifdef OSTYPE_Irix
  { "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
  { "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
  { "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
  { "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
  { "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Irix */

#ifdef OSTYPE_NetBSD
  #ifdef __i386__
    { "Apm", initApm, exitApm, updateApm, NULLVVFUNC, 0, NULLTIME },
  #endif
  { "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
  { "DiskStat", initDiskStat, exitDiskStat, updateDiskStat, checkDiskStat, 0, NULLTIME },
  { "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
  { "LogFile", initLogFile, exitLogFile, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME },
  { "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
  { "NetDev", initNetDev, exitNetDev, updateNetDev, checkNetDev, 0, NULLTIME },
  { "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_NetBSD */

#ifdef OSTYPE_OpenBSD
  { "CpuInfo", initCpuInfo, exitCpuInfo, updateCpuInfo, NULLVVFUNC, 0, NULLTIME },
  { "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
  { "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_OpenBSD */

#if defined(OSTYPE_Solaris) || defined(OSTYPE_SunOS)
  { "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
  { "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
  { "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
  { "ProcessList", initProcessList, exitProcessList, updateProcessList, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Solaris */

#ifdef OSTYPE_Tru64
  { "LoadAvg", initLoadAvg, exitLoadAvg, updateLoadAvg, NULLVVFUNC, 0, NULLTIME },
  { "Memory", initMemory, exitMemory, updateMemory, NULLVVFUNC, 0, NULLTIME },
  { "NetDev", initNetDev, exitNetDev, updateNetDev, NULLVVFUNC, 0, NULLTIME },
#endif /* OSTYPE_Tru64 */



  { NULL, NULLVSFUNC, NULLVVFUNC, NULLIVFUNC, NULLVVFUNC, 0, NULLTIME }
};

#endif
