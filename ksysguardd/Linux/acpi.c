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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Command.h"
#include "ksysguardd.h"

#include "acpi.h"

#define ACPIFILENAMELENGTHMAX 64
#define ACPIBATTERYNUMMAX 6
#define ACPIBATTERYINFOBUFSIZE 1024
#define ACPIBATTERYSTATEBUFSIZE 512

static int AcpiBatteryOK = 0;
static int AcpiBatteryNum = 0;
static char AcpiBatteryNames[ ACPIBATTERYNUMMAX ][ 8 ];
static int AcpiBatteryCharge[ ACPIBATTERYNUMMAX ];
static int AcpiBatteryUsage[ ACPIBATTERYNUMMAX ];


/*
================================ public part =================================
*/

void initAcpi( struct SensorModul* sm )
{
  int BatteryDirNummer;
  DIR *d;
  struct dirent *de;
  char s[ ACPIFILENAMELENGTHMAX ];

  if ( ( d = opendir( "/proc/acpi/battery" ) ) == NULL ) {
    print_error( "Directory \'/proc/acpi/battery\' does not exist or is not readable.\n"
                 "Load the battery ACPI kernel module or compile it into your kernel.\n" );
    AcpiBatteryOK = -1;
    return;
  } else {
    while ( ( de = readdir( d ) ) )
      if ( ( strcmp( de->d_name, "." ) != 0 ) && ( strcmp( de->d_name, ".." ) != 0 ) ) {
        sscanf(de->d_name,"BAT%d",&BatteryDirNummer);
        strncpy( AcpiBatteryNames[ BatteryDirNummer ], de->d_name, 8 );
        snprintf( s, sizeof( s ), "acpi/battery/%d/batterycharge", AcpiBatteryNum );
        registerMonitor( s, "integer", printAcpiBatFill, printAcpiBatFillInfo, sm );
        snprintf( s, sizeof( s ), "acpi/battery/%d/batteryusage", AcpiBatteryNum );
	registerMonitor( s, "integer", printAcpiBatUsage, printAcpiBatUsageInfo, sm);
        AcpiBatteryCharge[ BatteryDirNummer ] = 0;
        AcpiBatteryNum++;
      }

    AcpiBatteryOK = 1;
  }
}

void exitAcpi( void )
{
  AcpiBatteryOK = -1;
}

int updateAcpi( void )
{
  int i, fd;
  char s[ ACPIFILENAMELENGTHMAX ];
  size_t n;
  char AcpiBatInfoBuf[ ACPIBATTERYINFOBUFSIZE ];
  char AcpiBatStateBuf[ ACPIBATTERYSTATEBUFSIZE ];
  char *p;
  int AcpiBatCapacity = 1;
  int AcpiBatRemainingCapacity = 0;
  
  if ( AcpiBatteryOK < 0 )
    return -1;

  for ( i = 0; i < AcpiBatteryNum; i++ ) {
    /* get total capacity */
    snprintf( s, sizeof( s ), "/proc/acpi/battery/%s/info", AcpiBatteryNames[ i ] );
    if ( ( fd = open( s, O_RDONLY ) ) < 0 ) {
      print_error( "Cannot open file \'%s\'!\n"
                   "Load the battery ACPI kernel module or\n"
                   "compile it into your kernel.\n", s );
      return -1;
    }
    if ( ( n = read( fd, AcpiBatInfoBuf, ACPIBATTERYINFOBUFSIZE - 1 ) ) ==
         ACPIBATTERYINFOBUFSIZE - 1 ) {
      log_error( "Internal buffer too small to read \'%s\'", s );
      close( fd );
      return -1;
    }
    close( fd );
    p = AcpiBatInfoBuf;
    while ( ( p!= NULL ) && ( sscanf( p, "last full capacity: %d ",
                              &AcpiBatCapacity ) != 1 ) ) {
      p = strchr( p, '\n' );
      if ( p )
        p++;
    }
    /* get remaining capacity */
    snprintf( s, sizeof( s ), "/proc/acpi/battery/%s/state", AcpiBatteryNames[ i ] );
    if ( ( fd = open( s, O_RDONLY ) ) < 0 ) {
      print_error( "Cannot open file \'%s\'!\n"
                   "Load the battery ACPI kernel module or\n"
                   "compile it into your kernel.\n", s );
      return -1;
    }
    if ( ( n = read( fd, AcpiBatStateBuf, ACPIBATTERYSTATEBUFSIZE - 1 ) ) ==
         ACPIBATTERYSTATEBUFSIZE - 1 ) {
      log_error( "Internal buffer too small to read \'%s\'", s);
      close( fd );
      return -1;
    }
    close( fd );
    p = AcpiBatStateBuf;
    while ( ( p!= NULL ) && ( sscanf( p, "remaining capacity: %d ",
                              &AcpiBatRemainingCapacity ) != 1 ) ) {
      p = strchr( p, '\n' );
      if ( p )
        p++;
    }
    
    /* get current battery usage, (current Current) */
    p = AcpiBatStateBuf;
    while ( ( p!= NULL ) && ( sscanf( p, "present rate: %d ",
                              &AcpiBatteryUsage[i] ) != 1 ) ) {
      p = strchr( p, '\n' );
      if ( p )
        p++;
    } 
    
    
    /* calculate charge rate */
    if ( AcpiBatCapacity > 0 )
      AcpiBatteryCharge[ i ] = AcpiBatRemainingCapacity * 100 / AcpiBatCapacity;
    else
      AcpiBatteryCharge[ i ] = 0;
    
    

  } 

  return 0;
}

void printAcpiBatFill( const char* cmd )
{
  int i;

  sscanf( cmd + 13, "%d", &i );
  fprintf( CurrentClient, "%d\n", AcpiBatteryCharge[ i ] );
}

void printAcpiBatFillInfo( const char* cmd )
{
  int i;

  sscanf( cmd + 13, "%d", &i );
  fprintf( CurrentClient, "Battery %d charge\t0\t100\t%%\n", i );
}

void printAcpiBatUsage( const char* cmd)
{
 int i;
 
 sscanf( cmd + 13, "%d", &i );
 fprintf(CurrentClient, "%d\n", AcpiBatteryUsage[ i ] );
}

void printAcpiBatUsageInfo( const char* cmd)
{

 int i;

 sscanf(cmd+13, "%d", &i);

 fprintf(CurrentClient, "Battery %d usage\t0\t2500\tmA\n", i );
}
