/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999-2001 Chris Schlaeger <cs@kde.org>

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

#ifndef KSG_KSYSGUARDD_H
#define KSG_KSYSGUARDD_H

#include <stdio.h>
#include <time.h>

/* This is the official ksysguardd port assigned by IANA. */
#define PORT_NUMBER	3112

/* Timer interval for running updateCommand on modiles that support it */
#define UPDATEINTERVAL	1

/* Timer interval for running checkCommand on modules that support it */
#define TIMERINTERVAL	1

extern int RunAsDaemon;
extern int QuitApp;

/* This pointer give you access to the client which made the request */
extern FILE* CurrentClient;

struct SensorModul {
  const char *configName;
  void (*initCommand)( struct SensorModul* );
  void (*exitCommand)( void );
  int (*updateCommand)( void );
  void (*checkCommand)( void );
  int available;
  time_t time;
};

char* escapeString( char* string );

#endif
