/*
    KSysGuard, the KDE System Guard
    
    Copyright (c) 2003 Stephan Uhlmann <su@su2.info> 

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

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Command.h"
#include "ksysguardd.h"

#include "acpi.h"

static int AcpiOK = 0;
static int AcpiBatFill = 0;

#define ACPIBATINFOBUFSIZE 1024
static char AcpiBatInfoBuf[ACPIBATINFOBUFSIZE];
#define ACPIBATSTATEBUFSIZE 512
static char AcpiBatStateBuf[ACPIBATSTATEBUFSIZE];
static int Dirty = 0;

static void processAcpi(void)
{
  char* p;
  int AcpiBatDesignCapacity = 1;
  int AcpiBatRemainingCapacity = 0;

  p = AcpiBatInfoBuf;
  while ( ( p!= NULL ) && ( sscanf( p, "design capacity: %d mWh",
                            &AcpiBatDesignCapacity ) != 1 ) ) {
    p = strchr( p, '\n' );
    if ( p )
      p++;
  }
  p = AcpiBatStateBuf;
  while ( ( p != NULL ) && ( sscanf( p, "remaining capacity: %d mWh",
                             &AcpiBatRemainingCapacity ) != 1 ) ) {
    p = strchr( p, '\n' );
    if ( p )
      p++;
  }

  if ( AcpiBatDesignCapacity > 0 )
    AcpiBatFill = AcpiBatRemainingCapacity * 100 / AcpiBatDesignCapacity;
  else
    AcpiBatFill = 0;

  Dirty = 0;
}

/*
================================ public part =================================
*/

void initAcpi( struct SensorModul* sm )
{
  if ( updateAcpi() < 0 ) {
    AcpiOK = -1;
    return;
  } else
    AcpiOK = 1;

  registerMonitor( "acpi/batterycharge", "integer", printAcpiBatFill,
                   printAcpiBatFillInfo, sm );
}

void exitAcpi( void )
{
  AcpiOK = -1;
}

int updateAcpi( void )
{
  size_t n;
  int fd;

  if ( AcpiOK < 0 )
    return -1;

  if ( ( fd = open( "/proc/acpi/battery/BAT0/info", O_RDONLY ) ) < 0 ) {
    if ( AcpiOK != 0 )
      print_error( "Cannot open file \'/proc/acpi/battery/BAT0/info\'!\n"
                   "Load the battery ACPI kernel module or\n"
                   "compile it into your kernel.\n" );
    return -1;
  }

  if ( ( n = read( fd, AcpiBatInfoBuf, ACPIBATINFOBUFSIZE - 1 ) ) ==
       ACPIBATINFOBUFSIZE - 1 ) {
    log_error( "Internal buffer too small to read \'/proc/acpi/battery/BAT0/info\'" );
    close( fd );
    return -1;
  }

  close( fd );
  AcpiBatInfoBuf[ n ] = '\0';

  if ( ( fd = open( "/proc/acpi/battery/BAT0/state", O_RDONLY ) ) < 0 ) {
    if ( AcpiOK != 0 )
      print_error( "Cannot open file \'/proc/acpi/battery/BAT0/state\'!\n"
                   "Load the battery ACPI kernel module or\n"
                   "compile it into your kernel.\n" );
    return -1;
  }

  if ( ( n = read( fd, AcpiBatStateBuf, ACPIBATSTATEBUFSIZE - 1 ) ) ==
       ACPIBATSTATEBUFSIZE - 1 ) {
    log_error( "Internal buffer too small to read \'/proc/acpi/battery/BAT0/state\'" );
    close( fd );
    return -1;
  }

  close( fd );
  AcpiBatStateBuf[ n ] = '\0';

  Dirty = 1;

  return 0;
}

void printAcpiBatFill( const char* c )
{
  (void)c;
  if ( Dirty )
    processAcpi();

  fprintf( CurrentClient, "%d\n", AcpiBatFill );
}

void printAcpiBatFillInfo( const char* c )
{
  (void)c;
  fprintf( CurrentClient, "Battery charge\t0\t100\t%%\n" );
}
