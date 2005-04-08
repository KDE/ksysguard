/*
    KSysGuard, the KDE System Guard
    
    Copyright (c) 2003 Stephan Uhlmann <su@su2.info> 
    Copyright (c) 2005 Sirtaj Singh Kang <taj@kde.org> -- Battery fixes and Thermal

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

static int AcpiBatteryNum = 0;
static char AcpiBatteryNames[ ACPIBATTERYNUMMAX ][ 8 ];
static int AcpiBatteryCharge[ ACPIBATTERYNUMMAX ];
static int AcpiBatteryUsage[ ACPIBATTERYNUMMAX ];

static int AcpiThermalZones = -1;
/*
================================ public part =================================
*/

void initAcpi(struct SensorModul* sm)
{
	initAcpiBattery(sm);
	initAcpiThermal(sm);
}

int updateAcpi( void )
{
	if (AcpiBatteryNum > 0) {
		updateAcpiBattery();
	}
	if (AcpiThermalZones > 0) {
		updateAcpiThermal();
	}

	return 0;
}

void exitAcpi( void )
{
  AcpiBatteryNum = -1;
  AcpiThermalZones = -1;
}


/************ ACPI Battery **********/

void initAcpiBattery( struct SensorModul* sm )
{
  DIR *d;
  struct dirent *de;
  char s[ ACPIFILENAMELENGTHMAX ];

  if ( ( d = opendir( "/proc/acpi/battery" ) ) == NULL ) {
	AcpiBatteryNum = -1;
    return;
  } else {
	AcpiBatteryNum = 0;
    while ( ( de = readdir( d ) ) )
      if ( ( strcmp( de->d_name, "." ) != 0 ) && ( strcmp( de->d_name, ".." ) != 0 ) ) {
		  strncpy( AcpiBatteryNames[ AcpiBatteryNum ], de->d_name, 8 );
		  snprintf( s, sizeof( s ), "acpi/battery/%d/batterycharge", AcpiBatteryNum );
		  registerMonitor( s, "integer", printAcpiBatFill, printAcpiBatFillInfo, sm );
		  snprintf( s, sizeof( s ), "acpi/battery/%d/batteryusage", AcpiBatteryNum );
		  registerMonitor( s, "integer", printAcpiBatUsage, printAcpiBatUsageInfo, sm);
		  AcpiBatteryCharge[ AcpiBatteryNum ] = 0;
		  AcpiBatteryNum++;
	  }
  }
}


int updateAcpiBattery( void )
{
  int i, fd;
  char s[ ACPIFILENAMELENGTHMAX ];
  size_t n;
  char AcpiBatInfoBuf[ ACPIBATTERYINFOBUFSIZE ];
  char AcpiBatStateBuf[ ACPIBATTERYSTATEBUFSIZE ];
  char *p;
  int AcpiBatCapacity = 1;
  int AcpiBatRemainingCapacity = 0;
  
  if ( AcpiBatteryNum <= 0 )
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

/************** ACPI Thermal *****************/

#define THERMAL_ZONE_DIR "/proc/acpi/thermal_zone"
#define TEMPERATURE_FILE "temperature"
#define TEMPERATURE_FILE_MAXLEN 255


/*static char **zone_names = NULL;*/

/** Find the thermal zone name from the command.
 * Assumes the command is of the form acpi/thermal_zone/<zone name>/...
 * @p startidx is set to the start of the zone name. May be set to an
 * undefined value if zone name is not found.
 * @return length of found name, or 0 if nothing found.
 */
static int extract_zone_name(char **startidx, const char *cmd)
{
	char *idx = NULL;
	idx = strchr(cmd, '/');
	if (idx == NULL) return 0;
	idx = strchr(idx+1, '/');
	if (idx == NULL) return 0;
	*startidx = idx+1;
	idx = strchr(*startidx, '/');
	if (idx == NULL) return 0;
	return idx - *startidx;
}

void initAcpiThermal(struct SensorModul *sm)
{

  char th_ref[ ACPIFILENAMELENGTHMAX ];
  DIR *d = NULL;
  struct dirent *de;

  d = opendir(THERMAL_ZONE_DIR);
  if (d == NULL) {
/*	  print_error( "Directory \'" THERMAL_ZONE_DIR
		"\' does not exist or is not readable.\n"
	  "Load the ACPI thermal kernel module or compile it into your kernel.\n" );
*/
	  AcpiThermalZones = -1;
	  return;
  }

  AcpiThermalZones = 0;
  while ( (de = readdir(d)) != NULL ) {
	  if ( ( strcmp( de->d_name, "." ) == 0 )
			  || ( strcmp( de->d_name, ".." ) == 0 ) ) {
		  continue;
	  }

	  AcpiThermalZones++;
	  snprintf(th_ref, sizeof(th_ref), 
			  "acpi/thermal_zone/%s/temperature", de->d_name);
	  registerMonitor(th_ref, "integer", printThermalZoneTemperature,
			  printThermalZoneTemperatureInfo, sm);
  }

  return;
}

int updateAcpiThermal()
{
	/* TODO: stub */
	return 0;
}

static int getCurrentTemperature(const char *cmd)
{
	char th_file[ ACPIFILENAMELENGTHMAX ];
	char input_buf[ TEMPERATURE_FILE_MAXLEN ];
	char *zone_name = NULL;
	int read_bytes = 0, fd = 0, len_zone_name = 0;
	int temperature=0;

	len_zone_name = extract_zone_name(&zone_name, cmd);
	if (len_zone_name <= 0) return -1;

	snprintf(th_file, sizeof(th_file),
			THERMAL_ZONE_DIR "/%.*s/" TEMPERATURE_FILE,
			len_zone_name, zone_name);

	fd = open(th_file, O_RDONLY);
	if (fd < 0) {
		print_error( "Cannot open file \'%s\'!\n"
		"Load the thermal ACPI kernel module or\n"
		"compile it into your kernel.\n", th_file );
      return -1;
	}

	read_bytes = read( fd, input_buf, sizeof(input_buf) - 1 );
    if ( read_bytes == sizeof(input_buf) - 1 ) {
      log_error( "Internal buffer too small to read \'%s\'", th_file );
      close( fd );
      return -1;
    }
	close(fd);

	sscanf(input_buf, "temperature: %d C", &temperature);
	return temperature;
}

void printThermalZoneTemperature(const char *cmd) {
	int temperature = getCurrentTemperature(cmd);
	fprintf(CurrentClient, "%d\n", temperature);
}

void printThermalZoneTemperatureInfo(const char *cmd)
{
	fprintf(CurrentClient, "Current temperature\t0\t0\tC\n");
}
