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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Command.h"
#include "ksysguardd.h"

#include "i8k.h"

#ifdef HAVE_I8K_SUPPORT

static int I8kOK = 0;
static int cpuTemp, fan0Speed, fan1Speed;

#define I8KBUFSIZE 128
static char I8kBuf[ I8KBUFSIZE ];

/*
================================ public part =================================
*/

void initI8k( struct SensorModul* sm )
{
  if ( updateI8k() < 0 ) {
    I8kOK = -1;
    return;
  } else
    I8kOK = 1;

  registerMonitor( "dell/cputemp", "integer", printI8kCPUTemperature,
                   printI8kCPUTemperatureInfo, sm );
  registerMonitor( "dell/fan0", "integer", printI8kFan0Speed,
                   printI8kFan0SpeedInfo, sm );
  registerMonitor( "dell/fan1", "integer", printI8kFan1Speed,
                   printI8kFan1SpeedInfo, sm );
}

void exitI8k( void )
{
  I8kOK = -1;
}

int updateI8k( void )
{
  size_t n;
  int fd;

  if ( I8kOK < 0 )
    return -1;

  if ( ( fd = open( "/proc/i8k", O_RDONLY ) ) < 0 ) {
    print_error( "Cannot open file \'/proc/i8k\'!\n"
                 "The kernel needs to be compiled with support\n"
                 "for /proc file system enabled!\n" );
    return -1;
  }

  if ( ( n = read( fd, I8kBuf, I8KBUFSIZE - 1 ) ) == I8KBUFSIZE - 1 ) {
    log_error( "Internal buffer too small to read \'/proc/i8k\'" );

    close( fd );
    return -1;
  }

  close( fd );
  I8kBuf[ n ] = '\0';

  sscanf( I8kBuf, "%*f %*s %*s %d %*d %*d %d %d %*d %*d",
          &cpuTemp, &fan0Speed, &fan1Speed );

  return 0;
}

void printI8kCPUTemperature( const char* cmd )
{
  (void)cmd;
  output( "%d\n", cpuTemp );
}

void printI8kCPUTemperatureInfo( const char* cmd )
{
  (void)cmd;
  output( "CPU Temperature\t0\t0\tC\n" );
}

void printI8kFan0Speed( const char* cmd )
{
  (void)cmd;
  output( "%d\n", fan0Speed );
}

void printI8kFan0SpeedInfo( const char* cmd )
{
  (void)cmd;
  output( "Left fan\t0\t0\trpm\n" );
}

void printI8kFan1Speed( const char* cmd )
{
  (void)cmd;
  output( "%d\n", fan1Speed );
}

void printI8kFan1SpeedInfo( const char* cmd )
{
  (void)cmd;
  output( "Right fan\t0\t0\trpm\n" );
}

#else /* HAVE_I8K_SUPPORT */

/* dummy version for systems that have no i8k support */

void initI8k( struct SensorModul* sm )
{
  (void)sm;
}

void exitI8k( void )
{
}

int updateI8k( void )
{
  return 0;
}

#endif
