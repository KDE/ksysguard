/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Command.h"
#include "ccont.h"
#include "conf.h"
#include "ksysguardd.h"

#include "logfile.h"

static CONTAINER LogFiles = 0;
static unsigned long counter = 1;

typedef struct {
  char name[ 256 ];
  FILE* fh;
  unsigned long id;
} LogFileEntry;

extern CONTAINER LogFileList;

/*
================================ public part =================================
*/

void initLogFile( struct SensorModul* sm )
{
  char monitor[ 1024 ];
  ConfigLogFile *entry;

  registerCommand( "logfile_register", registerLogFile );
  registerCommand( "logfile_unregister", unregisterLogFile );
  registerCommand( "logfile_registered", printRegistered );

  for ( entry = first_ctnr( LogFileList ); entry; entry = next_ctnr( LogFileList ) ) {
    FILE* fp;
    /* Register the log file only if we can actually read the file. */
    if ( ( fp = fopen( entry->path, "r" ) ) != NULL ) {
      snprintf( monitor, 1024, "logfiles/%s", entry->name );
      registerMonitor( monitor, "logfile", printLogFile, printLogFileInfo, sm );
      registerLogFile(entry->name);
      fclose( fp );
    }
  }

  LogFiles = new_ctnr();
}

void exitLogFile( void )
{
  destr_ctnr( LogFiles, free );
}

void printLogFile( const char* cmd )
{
  char line[ 1024 ];
  unsigned long id;
  LogFileEntry *entry;

  sscanf( cmd, "%*s %lu", &id );

  for ( entry = first_ctnr( LogFiles ); entry; entry = next_ctnr( LogFiles ) ) {
    if ( entry->id == id ) {
      while ( fgets( line, 1024, entry->fh ) != NULL )
        output( "%s", line );

      /* delete the EOF */
      clearerr( entry->fh );
    }
  }

  output( "\n" );
}

void printLogFileInfo( const char* cmd )
{
	(void)cmd;
  output( "LogFile\n" );
}

void registerLogFile( const char* cmd )
{
  char name[ 257 ];
  FILE* file;
  LogFileEntry *entry;
  int i;

  memset( name, 0, sizeof( name ) );
  sscanf( cmd, "%*s %256s", name );

  for ( i = 0; i < level_ctnr( LogFileList ); i++ ) {
    ConfigLogFile *conf = get_ctnr( LogFileList, i );
    if ( !strcmp( conf->name, name ) ) {
      if ( ( file = fopen( conf->path, "r" ) ) == NULL ) {
        print_error( "fopen()" );
        output( "0\n" );
        return;
      }

      fseek( file, 0, SEEK_END );

      if ( ( entry = (LogFileEntry*)malloc( sizeof( LogFileEntry ) ) ) == NULL ) {
        print_error( "malloc()" );
        output( "0\n" );
        fclose(file);
        return;
      }

      entry->fh = file;
      strncpy( entry->name, conf->name, 256 );
      entry->id = counter;

      push_ctnr( LogFiles, entry );

      output( "%lu\n", counter );
      counter++;

      return;
    }
  }

  output( "\n" );
}

void unregisterLogFile( const char* cmd )
{
  unsigned long id;
  LogFileEntry *entry;

  sscanf( cmd, "%*s %lu", &id );

  for ( entry = first_ctnr( LogFiles ); entry; entry = next_ctnr( LogFiles ) ) {
    if ( entry->id == id ) {
      fclose( entry->fh );
      free( remove_ctnr( LogFiles ) );
      output( "\n" );
      return;
    }
  }

  output( "\n" );
}

void printRegistered( const char* cmd )
{
  LogFileEntry *entry;

  (void)cmd;
  for ( entry = first_ctnr( LogFiles ); entry; entry = next_ctnr( LogFiles ) )
    output( "%s:%lu\n", entry->name, entry->id );

  output( "\n" );
}
