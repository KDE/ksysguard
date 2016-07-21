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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

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

static int AcpiBatteryOk = 1;
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
	if (AcpiBatteryOk && AcpiBatteryNum > 0) updateAcpiBattery();
	return 0;
}

void exitAcpi( void )
{
  AcpiBatteryNum = -1;
  AcpiBatteryOk = 0;
}


/************ ACPI Battery **********/

void initAcpiBattery( struct SensorModul* sm )
{
  DIR *d;
  struct dirent *de;
  char s[ ACPIFILENAMELENGTHMAX ];

  if ( ( d = opendir( "/proc/acpi/battery" ) ) == NULL ) {
	AcpiBatteryNum = -1;
	AcpiBatteryOk = 0;
	return;
  } else {
	AcpiBatteryNum = 0;
	AcpiBatteryOk = 1;
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
    closedir( d );
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
      AcpiBatteryOk = 0;
      return -1;
    }
    if ( ( n = read( fd, AcpiBatInfoBuf, ACPIBATTERYINFOBUFSIZE - 1 ) ) ==
         ACPIBATTERYINFOBUFSIZE - 1 ) {
      log_error( "Internal buffer too small to read \'%s\'", s );
      close( fd );
      AcpiBatteryOk = 0;
      return -1;
    }
    close( fd );
    p = AcpiBatInfoBuf;
    if ( p && strstr(p, "ERROR: Unable to read battery") )
            return 0;  /* If we can't read the battery, reuse the last value */
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
      AcpiBatteryOk = 0;
      return -1;
    }
    if ( ( n = read( fd, AcpiBatStateBuf, ACPIBATTERYSTATEBUFSIZE - 1 ) ) ==
         ACPIBATTERYSTATEBUFSIZE - 1 ) {
      log_error( "Internal buffer too small to read \'%s\'", s);
      close( fd );
      AcpiBatteryOk = 0;
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
  AcpiBatteryOk = 1;
  return 0;
}

void printAcpiBatFill( const char* cmd )
{
  int i;

  sscanf( cmd + 13, "%d", &i );
  output( "%d\n", AcpiBatteryCharge[ i ] );
}

void printAcpiBatFillInfo( const char* cmd )
{
  int i;

  sscanf( cmd + 13, "%d", &i );
  output( "Battery %d charge\t0\t100\t%%\n", i );
}

void printAcpiBatUsage( const char* cmd)
{
 int i;

 sscanf( cmd + 13, "%d", &i );
 output( "%d\n", AcpiBatteryUsage[ i ] );
}

void printAcpiBatUsageInfo( const char* cmd)
{

 int i;

 sscanf(cmd+13, "%d", &i);

 output( "Battery %d usage\t0\t2500\tmA\n", i );
}

/************** ACPI Thermal *****************/

#define OLD_THERMAL_ZONE_DIR "/proc/acpi/thermal_zone"
#define OLD_TEMPERATURE_FILE "temperature"
#define OLD_TEMPERATURE_FILE_MAXLEN 255

#define OLD_FAN_DIR "/proc/acpi/fan"
#define OLD_FAN_STATE_FILE "state"
#define OLD_FAN_STATE_FILE_MAXLEN 255


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

void readTypeFile(const char *fileFormat, int number, char *buffer, int bufferSize)
{
    char filename[ ACPIFILENAMELENGTHMAX ];
    snprintf(filename, sizeof(filename), fileFormat, number);

    int typeFile = open(filename, O_RDONLY);
    if (typeFile < 0) {
        print_error( "Cannot open file \'%s\'!\n"
                     "Unable to fetch ACPI type", filename);
        snprintf(buffer, bufferSize, "unknown-%d", number);
        return;
    }

    int readBytes = read( typeFile, buffer, bufferSize - 1 );
    assert(readBytes > 1);
    assert(readBytes < bufferSize);
    buffer[readBytes-1] = '\0'; /* strip newline */
}

void registerThermalZone(int number, struct SensorModul *sm)
{
    char name[ ACPIFILENAMELENGTHMAX ];
    readTypeFile("/sys/class/thermal/thermal_zone%d/type", number, name, sizeof(name));

    char sensorName [ ACPIFILENAMELENGTHMAX ];
    snprintf(sensorName, sizeof(sensorName), "acpi/Thermal_Zone/%d-%s/Temperature", number, name);

    registerMonitor(sensorName, "integer", printSysThermalZoneTemperature,
                    printSysThermalZoneTemperatureInfo, sm);
}

void registerCoolingDevice(int number, struct SensorModul *sm)
{
    char name[ ACPIFILENAMELENGTHMAX ];
    readTypeFile("/sys/class/thermal/cooling_device%d/type", number, name, sizeof(name));

    char sensorName [ ACPIFILENAMELENGTHMAX ];
    snprintf(sensorName, sizeof(sensorName), "acpi/Cooling_Device/%d-%s/Activity", number, name);

    registerMonitor(sensorName, "integer", printCoolingDeviceState,
                    printCoolingDeviceStateInfo, sm);
}

void initAcpiThermal(struct SensorModul *sm)
{
  char th_ref[ ACPIFILENAMELENGTHMAX ];
  DIR *d = NULL;
  struct dirent *de;

  d = opendir("/sys/class/thermal/");
  if (d != NULL) {
      while ( (de = readdir(d)) != NULL ) {
          if (!de->d_name || de->d_name[0] == '.')
              continue;
          if (strncmp( de->d_name, "thermal_zone", sizeof("thermal_zone")-1) == 0) {
              int number = atoi(de->d_name + (sizeof("thermal_zone")-1));
              registerThermalZone(number, sm);

              /*For compatibility, register a legacy sensor*/
              int zone_number;
              if (sscanf(de->d_name, "thermal_zone%d", &zone_number) > 0) {
                  snprintf(th_ref, sizeof(th_ref),
                          "acpi/thermal_zone/TZ%02d/temperature", zone_number);
                  registerLegacyMonitor(th_ref, "integer", printSysCompatibilityThermalZoneTemperature,
                          printThermalZoneTemperatureInfo, sm);
              }
          } else if (strncmp( de->d_name, "cooling_device", sizeof("cooling_device")-1) == 0) {
              int number = atoi(de->d_name+( sizeof("cooling_device")-1));
              registerCoolingDevice(number, sm);
          }
      }
      closedir( d );
  } else {
      d = opendir(OLD_THERMAL_ZONE_DIR);
      if (d != NULL) {
          while ( (de = readdir(d)) != NULL ) {
              if (!de->d_name || de->d_name[0] == '.')
                  continue;

              snprintf(th_ref, sizeof(th_ref),
                      "acpi/thermal_zone/%s/temperature", de->d_name);
              registerMonitor(th_ref, "integer", printThermalZoneTemperature,
                      printThermalZoneTemperatureInfo, sm);
          }
          closedir( d );
      }

      d = opendir(OLD_FAN_DIR);
      if (d != NULL) {
          while ( (de = readdir(d)) != NULL ) {
              if (!de->d_name || de->d_name[0] == '.')
                  continue;

              snprintf(th_ref, sizeof(th_ref),
                      "acpi/fan/%s/state", de->d_name);
              registerMonitor(th_ref, "integer", printFanState,
                      printFanStateInfo, sm);
          }
          closedir( d );
      }
  }

  return;
}

static int getSysFileValue(const char *group, int value, const char *file) {
    static int shownError = 0;
    char th_file[ ACPIFILENAMELENGTHMAX ];
    char input_buf[ 100 ];
    snprintf(th_file, sizeof(th_file), "/sys/class/thermal/%s%d/%s",group, value, file);
    int fd = open(th_file, O_RDONLY);
    if (fd < 0) {
        if (!shownError)
            print_error( "Cannot open file \'%s\'!\n"
                    "Load the thermal ACPI kernel module or\n"
                    "compile it into your kernel.\n", th_file );
        shownError = 1;
        return -1;
    }
    int read_bytes = read( fd, input_buf, sizeof(input_buf) - 1 );
    if ( read_bytes == sizeof(input_buf) - 1 ) {
        if (!shownError)
            log_error( "Internal buffer too small to read \'%s\'", th_file );
        shownError = 1;
        close( fd );
        return -1;
    }
    close(fd);

    int result=0;
    sscanf(input_buf, "%d", &result);
    return result;
}

void printSysThermalZoneTemperature(const char *cmd) {
    int zone = 0;
    if (sscanf(cmd, "acpi/Thermal_Zone/%d", &zone) <= 0) {
        output("-1\n");
        return;
    }

    output( "%d\n", getSysFileValue("thermal_zone", zone, "temp") / 1000);
}
void printSysCompatibilityThermalZoneTemperature(const char *cmd) {
    int zone = 0;
    if (sscanf(cmd, "acpi/thermal_zone/TZ%d", &zone) <= 0) {
        output( "-1\n");
        return;
    }
    output( "%d\n", getSysFileValue("thermal_zone", zone, "temp")/1000);
}

void printCoolingDeviceStateInfo(const char *cmd)
{
    (void)cmd;
    char name [ 200 ];
    if (sscanf(cmd, "acpi/Cooling_Device/%199[^/]", name) > 0) {
        output( "%s Cooling Activity\t0\t100\t%%\n",  name);
    } else {
        output( "Cooling Device Activity\t0\t100\t%%\n");
    }
}

void printCoolingDeviceState(const char *cmd) {
    int fan = 0;
    if (sscanf(cmd, "acpi/Cooling_Device/%d", &fan) <= 0) {
        output( "-1\n");
        return;
    }
    int current = getSysFileValue("cooling_device", fan, "cur_state");
    int maximum = getSysFileValue("cooling_device", fan, "max_state");
    int state = 0;
    if (current > 0 && maximum > 0) {
        state = current / maximum;
    }
    output( "%d\n", state);
}

static int getCurrentTemperature(const char *cmd)
{
	char th_file[ ACPIFILENAMELENGTHMAX ];
	char input_buf[ OLD_TEMPERATURE_FILE_MAXLEN ];
	char *zone_name = NULL;
	int read_bytes = 0, fd = 0, len_zone_name = 0;
	int temperature=0;

	len_zone_name = extract_zone_name(&zone_name, cmd);
	if (len_zone_name <= 0) return -1;

	snprintf(th_file, sizeof(th_file),
			OLD_THERMAL_ZONE_DIR "/%.*s/" OLD_TEMPERATURE_FILE,
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
	output( "%d\n", temperature);
}

void printSysThermalZoneTemperatureInfo(const char *cmd)
{
    char name [ 200 ];
    if (sscanf(cmd, "acpi/Thermal_Zone/%199[^/]", name) > 0) {
        output( "%s temperature\t0\t0\tC\n", name);
    } else {
        output( "Current temperature\t0\t0\tC\n");
    }
}

void printThermalZoneTemperatureInfo(const char *cmd)
{
	(void)cmd;

	output( "Current temperature\t0\t0\tC\n");
}

/********** ACPI Fan State***************/

static int getFanState(const char *cmd)
{
	char fan_state_file[ ACPIFILENAMELENGTHMAX ];
	char input_buf[ OLD_FAN_STATE_FILE_MAXLEN ];
	char *fan_name = NULL;
	int read_bytes = 0, fd = 0, len_fan_name = 0;
	char fan_state[4];

	len_fan_name = extract_zone_name(&fan_name, cmd);
	if (len_fan_name <= 0) {
		return -1;
	}

	snprintf(fan_state_file, sizeof(fan_state_file),
			OLD_FAN_DIR "/%.*s/" OLD_FAN_STATE_FILE,
			len_fan_name, fan_name);

	fd = open(fan_state_file, O_RDONLY);
	if (fd < 0) {
		print_error( "Cannot open file \'%s\'!\n"
		"Load the fan ACPI kernel module or\n"
		"compile it into your kernel.\n", fan_state_file );

		return -1;
	}

	read_bytes = read( fd, input_buf, sizeof(input_buf) - 1 );
	if ( read_bytes == sizeof(input_buf) - 1 ) {
		log_error( "Internal buffer too small to read \'%s\'", fan_state_file );
		close( fd );
		return -1;
	}
	close(fd);

	sscanf(input_buf, "status: %2s", fan_state);
	return (fan_state[1] == 'n') ? 1 : 0;
}

void printFanState(const char *cmd) {
	int fan_state = getFanState(cmd);
	output( "%d\n", fan_state);
}

void printFanStateInfo(const char *cmd)
{
	(void)cmd;

	output( "Fan status\t0\t1\tboolean\n");
}
