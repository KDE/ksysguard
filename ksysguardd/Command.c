/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>

#include "ccont.h"
#include "ksysguardd.h"

#include "Command.h"

typedef struct {
  char* command;
  cmdExecutor ex;
  char* type;
  int isMonitor;
  int isLegacy;
  struct SensorModul* sm;
} Command;

static CONTAINER CommandList;
static sigset_t SignalSet;

void command_cleanup( void* v );

void command_cleanup( void* v )
{
  if ( v ) {
    Command* c = v;
    if ( c->command )
      free ( c->command );
    if ( c->type )
      free ( c->type );
    free ( v );
  }
}

/*
================================ public part =================================
*/

int ReconfigureFlag = 0;
int CheckSetupFlag = 0;
void output( const char *fmt, ...)
{
  if( !CurrentClient )
    return;
  va_list az;
  va_start( az, fmt);
  if(vfprintf(CurrentClient, fmt, az) < 0) {
    fprintf(stderr, "Error talking to client.  Exiting\n.");
    exit(EXIT_FAILURE);
  }
}
void print_error( const char *fmt, ... )
{
  char errmsg[ 1024 ];
  va_list az;

  va_start( az, fmt );
  vsnprintf( errmsg, sizeof( errmsg ) - 1, fmt, az );
  errmsg[ sizeof( errmsg ) - 1 ] = '\0';
  va_end( az );

  if ( CurrentClient )
    output( "\033%s\033", errmsg );
}

void log_error( const char *fmt, ... )
{
  char errmsg[ 1024 ];
  va_list az;

  va_start( az, fmt );
  vsnprintf( errmsg, sizeof( errmsg ) - 1, fmt, az );
  errmsg[ sizeof( errmsg ) - 1 ] = '\0';
  va_end( az );

  openlog( "ksysguardd", LOG_PID, LOG_DAEMON );
  syslog( LOG_ERR, "%s", errmsg );
  closelog();
}

void initCommand( void )
{
  CommandList = new_ctnr();
  sigemptyset( &SignalSet );
  sigaddset( &SignalSet, SIGALRM );

  registerCommand( "monitors", printMonitors );
  /* registerCommand( "test", printTest ); */

  if ( RunAsDaemon == 0 )
    registerCommand( "quit", exQuit );
}

void exitCommand( void )
{
  destr_ctnr( CommandList, command_cleanup );
}

void registerCommand( const char* command, cmdExecutor ex )
{
  Command* cmd = (Command*)malloc( sizeof( Command ) );
  if(!cmd || !(cmd->command = (char*)malloc( strlen( command ) + 1 ))) {
    print_error("Out of memory");
    free(cmd);
    return;
  }
  strcpy( cmd->command, command );
  cmd->type = 0;
  cmd->ex = ex;
  cmd->isMonitor = 0;
  push_ctnr( CommandList, cmd );
  ReconfigureFlag = 1;
}

void removeCommand( const char* command )
{
  Command* cmd;

  for ( cmd = first_ctnr( CommandList ); cmd; cmd = next_ctnr( CommandList ) ) {
    if ( cmd->command && strcmp( cmd->command, command ) == 0 ) {
      remove_ctnr( CommandList );
      free( cmd->command );
      if ( cmd->type )
        free( cmd->type );
      free( cmd );
    }
  }

  ReconfigureFlag = 1;
}

void registerAnyMonitor( const char* command, const char* type, cmdExecutor ex,
                      cmdExecutor iq, struct SensorModul* sm, int isLegacy )
{
  /* Monitors are similar to regular commands except that every monitor
   * registers two commands. The first is the value request command and
   * the second is the info request command. The info request command is
   * identical to the value request but with an '?' appended. The value
   * command prints a single value. The info request command prints
   * a description of the monitor, the mininum value, the maximum value
   * and the unit. */
  Command* cmd = (Command*)malloc( sizeof( Command ) );
  if(!cmd || !(cmd->command = (char*)malloc( strlen( command ) + 1 ))) {
      print_error("Out of memory");
      free(cmd);
      return;
  }

  strcpy( cmd->command, command );
  cmd->ex = ex;
  cmd->type = (char*)malloc( strlen( type ) + 1 );
  if(!cmd->type ) {
      print_error("Out of memory");
      free(cmd);
      return;
  }

  strcpy( cmd->type, type );
  cmd->isMonitor = 1;
  cmd->isLegacy = isLegacy;
  cmd->sm = sm;
  push_ctnr( CommandList, cmd );

  cmd = (Command*)malloc( sizeof( Command ) );
  if(!cmd ) {
      print_error("Out of memory");
      return;
  }

  cmd->command = (char*)malloc( strlen( command ) + 2 );
  if(!cmd->command ) {
      print_error("Out of memory");
      free(cmd);
      return;
  }

  strcpy( cmd->command, command );
  cmd->command[ strlen( command ) ] = '?';
  cmd->command[ strlen( command ) + 1 ] = '\0';
  cmd->ex = iq;
  cmd->isMonitor = 0;
  cmd->sm = sm;
  cmd->type = 0;
  push_ctnr( CommandList, cmd );
}

void registerMonitor( const char* command, const char* type, cmdExecutor ex,
                      cmdExecutor iq, struct SensorModul* sm )
{
  int legacyFlag = 0;

  registerAnyMonitor( command, type, ex, iq, sm, legacyFlag );
}

void registerLegacyMonitor( const char* command, const char* type, cmdExecutor ex,
                      cmdExecutor iq, struct SensorModul* sm )
{
  int legacyFlag = 1;

  registerAnyMonitor( command, type, ex, iq, sm, legacyFlag );
}

void removeMonitor( const char* command )
{
  char* buf;

  removeCommand( command );
  buf = (char*)malloc( strlen( command ) + 2 );
  if(!buf ) {
      print_error("Out of memory");
      return;
  }

  strcpy( buf, command );
  strcat( buf, "?" );
  removeCommand( buf );
  free( buf );
}

void executeCommand( const char* command )
{
  Command* cmd;
  int i = 0;
  while(command[i] != 0 && command[i] != ' ' && command[i] != '\t' )
      i++;
  if( i <= 0 )
      return; /* No command give at all */
  int lengthOfCommand = i;

  for ( cmd = first_ctnr( CommandList ); cmd; cmd = next_ctnr( CommandList ) ) {
    if ( strncmp( cmd->command, command, lengthOfCommand ) == 0 && cmd->command[lengthOfCommand] == 0) {
      if ( cmd->isMonitor && cmd->sm->updateCommand != NULL) {
        struct timeval currentTime;
        gettimeofday(&currentTime,NULL);
        unsigned long long timeCentiSeconds = (unsigned long long)currentTime.tv_sec * 10 + currentTime.tv_usec / 100000;
        if ( timeCentiSeconds - cmd->sm->timeCentiSeconds >= UPDATEINTERVAL ) {
          cmd->sm->timeCentiSeconds = timeCentiSeconds;
          cmd->sm->updateCommand();
        }
      }

      (*(cmd->ex))( command );

      if ( ReconfigureFlag ) {
        ReconfigureFlag = 0;
        print_error( "RECONFIGURE" );
      }

      fflush( CurrentClient );
      return;
    }
  }

  if ( CurrentClient ) {
    output( "UNKNOWN COMMAND\n" );
    fflush( CurrentClient );
  }
}

void printMonitors( const char *c )
{
  Command* cmd;
  ReconfigureFlag = 0;

  (void)c;

  for ( cmd = first_ctnr( CommandList ); cmd; cmd = next_ctnr( CommandList ) ) {
    if ( cmd->isMonitor && !cmd->isLegacy )
      output( "%s\t%s\n", cmd->command, cmd->type);
  }

  fflush( CurrentClient );
}

void printTest( const char* c )
{
  Command* cmd;

  for ( cmd = first_ctnr( CommandList ); cmd; cmd = next_ctnr( CommandList ) ) {
    if ( strcmp( cmd->command, c + strlen( "test " ) ) == 0 ) {
      output( "1\n" );
      fflush( CurrentClient );
      return;
    }
  }

  output( "0\n" );
  fflush( CurrentClient );
}

void exQuit( const char* cmd )
{
  (void)cmd;

  QuitApp = 1;
}
