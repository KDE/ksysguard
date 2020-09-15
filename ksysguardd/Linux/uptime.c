/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2006 Greg Martyn <greg.martyn@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 or later of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

/*
 * This file will read from /proc/uptime.
*/

#include <string.h> /* for strcmp */
#include <stdlib.h> /* for malloc */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "Command.h"
#include "ksysguardd.h"

#include "uptime.h"

#define UPTIMEBUFSIZE 64

static char UptimeBuf[ UPTIMEBUFSIZE ];	/* Buffer for /proc/uptime */

static struct SensorModul* StatSM;

void printUptime( const char* cmd );
void printUptimeInfo( const char* cmd );
static void openUptimeFile();

void initUptime( struct SensorModul* sm ) {
	char format[ 32 ];
	char buf[ 1024 ];
	char* uptimeBufP;

	StatSM = sm;

	openUptimeFile();

	uptimeBufP = UptimeBuf;
	sprintf( format, "%%%d[^\n]\n", (int)sizeof( buf ) - 1 );

	/* Process values from /proc/uptime */
	if (sscanf(uptimeBufP, format, buf) == 1) {
		buf[sizeof(buf) - 1] = '\0';
		registerMonitor( "system/uptime", "float", printUptime, printUptimeInfo, StatSM );
		registerMonitor( "system/uptime/uptime", "float", printUptime, printUptimeInfo, StatSM );
	}
}

void exitUptime( void ) {

}

void printUptime( const char* cmd ) {
	/* Process values from /proc/uptime */
	(void)cmd;
	
	char format[ 32 ];
	char buf[ 1024 ];
	char* uptimeBufP = UptimeBuf;
	float uptime;
	
	sprintf( format, "%%%d[^\n]\n", (int)sizeof( buf ) - 1 );
	
	openUptimeFile();
	
	if (sscanf(uptimeBufP, format, buf) == 1)
	{
		buf[sizeof(buf) - 1] = '\0';
		sscanf( buf, "%f", &uptime );
		output( "%f\n", uptime );
	}
}

void printUptimeInfo( const char* cmd ) {
	(void)cmd;
	
	output( "System uptime\t0\t0\ts\n" );
}

static void openUptimeFile() {
	size_t n;
	int fd;

	UptimeBuf[ 0 ] = '\0';
	if ( ( fd = open( "/proc/uptime", O_RDONLY ) ) < 0 )
		return; /* couldn't open /proc/uptime */
	
	n = read( fd, UptimeBuf, UPTIMEBUFSIZE - 1 );
	close( fd );

	if ( n == UPTIMEBUFSIZE - 1 || n <= 0 ) {
		log_error( "Internal buffer too small to read \'/proc/uptime\'" );

		return;
	}
	
	UptimeBuf[ n ] = '\0';
}
