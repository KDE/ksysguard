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

#include "config-ksysguardd.h"

#include <config-workspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Command.h"
#include "ccont.h"
#include "ksysguardd.h"

#include "lmsensors.h"

#ifdef HAVE_LMSENSORS
#include <sensors/sensors.h>
#define BUFFER_SIZE_LMSEN 300
typedef struct
{
  char* fullName;
  const sensors_chip_name* scn;
  const sensors_feature_data* sfd;
} LMSENSOR;

static CONTAINER LmSensors;
static int LmSensorsOk = -1;

static int sensorCmp( void* s1, void* s2 )
{
  return strcmp( ((LMSENSOR*)s1)->fullName, ((LMSENSOR*)s2)->fullName );
}

static LMSENSOR* findMatchingSensor( const char* name )
{
  INDEX idx;
  LMSENSOR key;
  LMSENSOR* s;

  if(name == NULL || name[0] == '\0') return 0;
  key.fullName = strdup( name );
  int end = strlen(key.fullName)-1;
  if(key.fullName[end] == '?')
    key.fullName[end] = '\0';
  if ( ( idx = search_ctnr( LmSensors, sensorCmp, &key ) ) < 0 ) {
    free( key.fullName );
    return 0;
  }

  free( key.fullName );
  s = get_ctnr( LmSensors, idx );

  return s;
}

static const char *chipName(const sensors_chip_name *chip) {
  static char buffer[256];
  if (chip->bus == SENSORS_CHIP_NAME_BUS_ISA)
    sprintf (buffer, "%s-isa-%04x", chip->prefix, chip->addr);
  else if (chip->bus == SENSORS_CHIP_NAME_BUS_PCI)
    sprintf (buffer, "%s-pci-%04x", chip->prefix, chip->addr);
  else
    sprintf (buffer, "%s-i2c-%d-%02x", chip->prefix, chip->bus, chip->addr);
  return buffer;
}

void initLmSensors( struct SensorModul* sm )
{
  const sensors_chip_name* scn;
  char buffer[BUFFER_SIZE_LMSEN];
  int nr = 0;

  FILE* input;
  if ( ( input = fopen( "/etc/sensors.conf", "r" ) ) == NULL ) {
    LmSensorsOk = -1;
    return;
  }

  if ( sensors_init( input ) ) {
    LmSensorsOk = -1;
    fclose( input );
    return;
  }

  fclose( input );

  LmSensors = new_ctnr();
  while ( ( scn = sensors_get_detected_chips( &nr ) ) != NULL ) {
    int nr1, nr2;
    const sensors_feature_data* sfd;
    nr1 = nr2 = 0;
    while ( ( sfd = sensors_get_all_features( *scn, &nr1, &nr2 ) ) != 0 ) {
      if ( sfd->mapping == SENSORS_NO_MAPPING && sfd->mode & SENSORS_MODE_R /* readable feature */) {
        LMSENSOR* p;
        char* label;

        if(sensors_get_label( *scn, sfd->number, &label ) != 0)
		continue; /*error*/
	if(sensors_get_ignored( *scn, sfd->number) != 1 )
		continue; /* 1 for not ignored, 0 for ignore,  <0 for error */
	double result;
	if(sensors_get_feature( *scn, sfd->number, &result) != 0 )
		continue; /* Make sure this feature actually works.  0 for success, <0 for fail */

        p = (LMSENSOR*)malloc( sizeof( LMSENSOR ) );

        snprintf( buffer, BUFFER_SIZE_LMSEN, "lmsensors/%s/%s", chipName(scn), sfd->name );

        p->fullName = strndup(buffer, BUFFER_SIZE_LMSEN);

        p->scn = scn;
        p->sfd = sfd;
        if ( search_ctnr( LmSensors, sensorCmp, p ) < 0 ) {
          push_ctnr( LmSensors, p );
          registerMonitor( p->fullName, "float", printLmSensor, printLmSensorInfo, sm );
        } else {
          free( p->fullName );
          free( p );
        }
      }
    }
  }
  bsort_ctnr( LmSensors, sensorCmp );
}


void exitLmSensors( void )
{
  destr_ctnr( LmSensors, free );
}

void printLmSensor( const char* cmd )
{
  double value;
  LMSENSOR* s;

  if ( ( s = findMatchingSensor( cmd ) ) == 0 ) { /* should never happen */
    fprintf( CurrentClient, "0\n" );
    return;
  }

  sensors_get_feature( *(s->scn), s->sfd->number, &value );
  fprintf( CurrentClient, "%f\n", value );
}

void printLmSensorInfo( const char* cmd )
{
  LMSENSOR* s;

  if ( ( s = findMatchingSensor( cmd ) ) == 0 ) { /* should never happen */
    fprintf( CurrentClient, "0\n" );
    return;
  }

  /* TODO: print real name here */
  char *label;
  if(sensors_get_label( *s->scn, s->sfd->number, &label ) != 0) {  /*error*/
    fprintf( CurrentClient, "0\n" );
    return;
  }
  if( strncmp(s->sfd->name, "temp", sizeof("temp")-1) == 0)
    fprintf( CurrentClient, "%s\t0\t0\t°C\n", label );
  else if( strncmp(s->sfd->name, "fan", sizeof("fan")-1) == 0)
    fprintf( CurrentClient, "%s\t0\t0\trpm\n", label );
  else
    fprintf( CurrentClient, "%s\t0\t0\tV\n", label );  /* For everything else, say it's in volts. */

}

#else /* HAVE_LMSENSORS */

/* dummy version for systems that have no lmsensors support */

void initLmSensors( struct SensorModul* sm )
{
  (void)sm;
}

void exitLmSensors( void )
{
}

#endif
