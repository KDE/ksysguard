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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Command.h"
#include "ksysguardd.h"

#include "Memory.h"

#define MEMINFOBUFSIZE 1024
static char MemInfoBuf[ MEMINFOBUFSIZE ];
static int Dirty = 1;

static unsigned long Total = 0;
static unsigned long MFree = 0;
static unsigned long Appl = 0;
static unsigned long Used = 0;
static unsigned long Buffers = 0;
static unsigned long Cached = 0;
static unsigned long STotal = 0;
static unsigned long SFree = 0;
static unsigned long SUsed = 0;

static void scan_one( const char* buff, const char *key, unsigned long int* val )
{   
  int o;
  char *b = strstr( buff, key );
  if ( b )  
    o = sscanf( b + strlen( key ), ": %lu", val );
}

static void processMemInfo()
{
  scan_one( MemInfoBuf, "MemTotal", &Total );
  scan_one( MemInfoBuf, "MemFree", &MFree );
  scan_one( MemInfoBuf, "Buffers", &Buffers );
  scan_one( MemInfoBuf, "Cached", &Cached );
  scan_one( MemInfoBuf, "SwapTotal", &STotal );
  scan_one( MemInfoBuf, "SwapFree", &SFree );
  Used = Total - MFree;
  Appl = ( Used - ( Buffers + Cached ) );
  SUsed = STotal - SFree;

  Dirty = 0;
}

/*
================================ public part =================================
*/

void initMemory( struct SensorModul* sm )
{
  /**
    Make sure that /proc/meminfo exists and is readable. If not we do
    not register any monitors for memory.
   */
  if ( updateMemory() < 0 )
    return;

  registerMonitor( "mem/physical/free", "integer", printMFree, printMFreeInfo, sm );
  registerMonitor( "mem/physical/used", "integer", printUsed, printUsedInfo, sm );
  registerMonitor( "mem/physical/application", "integer", printAppl, printApplInfo, sm );
  registerMonitor( "mem/physical/buf", "integer", printBuffers, printBuffersInfo, sm );
  registerMonitor( "mem/physical/cached", "integer", printCached, printCachedInfo, sm );
  registerMonitor( "mem/swap/used", "integer", printSwapUsed, printSwapUsedInfo, sm );
  registerMonitor( "mem/swap/free", "integer", printSwapFree, printSwapFreeInfo, sm );
}

void exitMemory( void )
{
}

int updateMemory( void )
{
  /**
    The amount of total and used memory is read from the /proc/meminfo.
    It also contains the information about the swap space.
    The 'file' looks like this:

    MemTotal:       516560 kB
    MemFree:          7812 kB
    MemShared:           0 kB
    Buffers:         80312 kB
    Cached:         236432 kB
    SwapCached:        468 kB
    Active:         291992 kB
    Inactive:       133556 kB
    HighTotal:           0 kB
    HighFree:            0 kB
    LowTotal:       516560 kB
    LowFree:          7812 kB
    SwapTotal:      899632 kB
    SwapFree:       898932 kB
    Dirty:            2736 kB
    Writeback:           0 kB
    Mapped:         155996 kB
    Slab:            73920 kB
    Committed_AS:   315588 kB
    PageTables:       1764 kB
    ReverseMaps:    103458
   */

  int fd;
  size_t n;

  if ( ( fd = open( "/proc/meminfo", O_RDONLY ) ) < 0 ) {
    print_error( "Cannot open \'/proc/meminfo\'!\n"
                 "The kernel needs to be compiled with support\n"
                 "for /proc filesystem enabled!\n" );
    return -1;
  }

  if ( ( n = read( fd, MemInfoBuf, MEMINFOBUFSIZE - 1 ) ) == MEMINFOBUFSIZE - 1 ) {
    log_error( "Internal buffer too small to read \'/proc/mem\'" );
    close( fd );
    return -1;
  }

  close( fd );
  MemInfoBuf[ n ] = '\0';
  Dirty = 1;

  return 0;
}

void printMFree( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "%ld\n", MFree );
}

void printMFreeInfo( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "Free Memory\t0\t%ld\tKB\n", Total );
}

void printUsed( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "%ld\n", Used );
}

void printUsedInfo( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "Used Memory\t0\t%ld\tKB\n", Total );
}

void printAppl( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "%ld\n", Appl );
}

void printApplInfo( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "Application Memory\t0\t%ld\tKB\n", Total );
}

void printBuffers( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "%ld\n", Buffers );
}

void printBuffersInfo( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "Buffer Memory\t0\t%ld\tKB\n", Total );
}

void printCached( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "%ld\n", Cached );
}

void printCachedInfo( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "Cached Memory\t0\t%ld\tKB\n", Total );
}

void printSwapUsed( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "%ld\n", SUsed );
}

void printSwapUsedInfo( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "Used Swap Memory\t0\t%ld\tKB\n", STotal );
}

void printSwapFree( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "%ld\n", SFree );
}

void printSwapFreeInfo( const char* cmd )
{
  (void)cmd;

  if ( Dirty )
    processMemInfo();

  fprintf( CurrentClient, "Free Swap Memory\t0\t%ld\tKB\n", STotal );
}
