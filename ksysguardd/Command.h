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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "ksysguardd.h"

#ifndef KSG_COMMAND_H
#define KSG_COMMAND_H

typedef void (*cmdExecutor)(const char*);

/**
  Set this flag to '1' to request a rescan of the available sensors
  in the front end.
 */
extern int ReconfigureFlag;

/**
  Has nearly the same meaning like the above flag ;)
 */
extern int CheckSetupFlag;

/**
  Delivers the error message to the front end.
 */
void print_error( const char*, ... )
#ifdef __GNUC__
    __attribute__ (  (  format (  printf, 1, 2 ) ) )
#endif
    ;

/**
  Writes the error message to the syslog daemon.
 */
void log_error( const char*, ... )
 #ifdef __GNUC__
    __attribute__ (  (  format (  printf, 1, 2 ) ) )
#endif
    ;

   

/**
  Use this function to register a command with the name
  @ref command and the function pointer @ref ex.
 */
void registerCommand( const char* command, cmdExecutor ex );

/**
  Use this function to remove a command with the name
  @ref command.
 */
void removeCommand( const char* command );

/**
  Use this function to add a new montior with the name @ref monitor
  from the type @ref type.
  @ref ex is a pointer to the function that is called to get a value
  and @ref iq is a pointer to the function that returns informations
  about this monitor.
  @ref sm is a parameter to the sensor modul object that is passed by
  the initXXX method.
 */
void registerMonitor( const char* monitor, const char* type, cmdExecutor ex,
                      cmdExecutor iq, struct SensorModul* sm );

/**
  Use this function to add the montior with the name @ref monitor.
 */
void removeMonitor( const char* monitor );


/**
  Internal usage.
 */
void executeCommand( const char* command );

void initCommand( void );
void exitCommand( void );

void printMonitors( const char* cmd );
void printTest( const char* cmd );

void exQuit( const char* cmd );

#endif
