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

/* This file will read from /proc/diskstats.
  /proc/diskstats support should exist in kernel versions 2.4.20, 2.5.45, 2.6 and up
*/

#include <sys/time.h> /* for gettimeofday */
#include <string.h> /* for strcmp */
#include <stdlib.h> /* for malloc */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

 
#include "Command.h"
#include "ksysguardd.h"

#include "diskstats.h"

#define DISKSTATSBUFSIZE (32 * 1024)
#define DISKDEVNAMELEN 16

typedef struct
{
	unsigned long delta;
	unsigned long old;
} DiskLoadSample;

typedef struct
{
	/* 5 types of samples are taken:
	total, rio, wio, rBlk, wBlk */
	DiskLoadSample s[ 5 ];
} DiskLoadInfo;

typedef struct DiskIOInfo
{
	int major;
	int minor;
	char* devname;
	
	int alive;
	DiskLoadSample total; /* Total accesses - Fields 1+5 */
	DiskLoadSample rio; /* Read Accesses - Field 1 - # of reads issued */
	DiskLoadSample wio; /* Write Accesses - Field 5 - # of writes completed */
	DiskLoadSample rblk; /* Read Data - Field 3 - # of sectors read */
	DiskLoadSample wblk; /* Written Data - Field 7 - # of sectors written */
	DiskLoadSample rtim; /* - Field 4 - # of milliseconds spent reading */
	DiskLoadSample wtim; /* - Field 8 - # of milliseconds spent writing */
	unsigned int ioqueue; /* - Field 9 - # of I/Os currently in progress */
	struct DiskIOInfo* next;
} DiskIOInfo;

/* We have observed deviations of up to 5% in the accuracy of the timer
* interrupts. So we try to measure the interrupt interval and use this
* value to calculate timing dependant values. */
static float timeInterval = 0;
static struct timeval lastSampling;
static struct timeval currSampling;
static struct SensorModul* StatSM;

static DiskLoadInfo* DiskLoad = 0;
static DiskIOInfo* DiskIO = 0;

static char IOStatBuf[ DISKSTATSBUFSIZE ];	/* Buffer for /proc/diskstats */
static int Dirty = 0;

static void cleanup26DiskList( void );
static int process26DiskIO( const char* buf );

void initDiskstats( struct SensorModul* sm ) {
	char format[ 32 ];
	char buf[ 1024 ];
	char* iostatBufP = IOStatBuf;

	StatSM = sm;
	sprintf( format, "%%%d[^\n]\n", (int)sizeof( buf ) - 1 );

	updateDiskstats(); /* reopens /proc/diskstats as IOStatBuf */

	/* Process values from /proc/diskstats (Linux >= 2.6.x) */
	while (sscanf(iostatBufP, format, buf) == 1) {
		buf[sizeof(buf) - 1] = '\0';
		iostatBufP += strlen(buf) + 1;  /* move IOstatBufP to next line */
		
		process26DiskIO(buf);
	}
}

void exitDiskstats( void ) {
	free( DiskLoad );
	DiskLoad = 0;
}

int updateDiskstats( void ) {
	size_t n;
	int fd;

	gettimeofday( &currSampling, 0 );
	Dirty = 1;


	IOStatBuf[ 0 ] = '\0';
	if ( ( fd = open( "/proc/diskstats", O_RDONLY ) ) < 0 )
		return 0; /* failure is okay, only exists for Linux >= 2.6.x */
	
	n = read( fd, IOStatBuf, DISKSTATSBUFSIZE - 1 );
	if ( n == DISKSTATSBUFSIZE - 1 || n <= 0 ) {
		log_error( "Internal buffer too small to read \'/proc/diskstats\'" );
	
		close( fd );
		return -1;
	}
	
	close( fd );
	IOStatBuf[ n ] = '\0';

	return 0;
}

void processDiskstats( void ) {
	/* Process values from /proc/diskstats (Linux >= 2.6.x) */

	char* iostatBufP = IOStatBuf;
	char format[ 32 ];
	char buf[ 1024 ];

	sprintf( format, "%%%d[^\n]\n", (int)sizeof( buf ) - 1 );

	while (sscanf(iostatBufP, format, buf) == 1) {
		buf[sizeof(buf) - 1] = '\0';
		iostatBufP += strlen(buf) + 1;  /* move IOstatBufP to next line */
		
		process26DiskIO(buf);
	}

	/* save exact time inverval between this and the last read of /proc/stat */
	timeInterval = currSampling.tv_sec - lastSampling.tv_sec +
			( currSampling.tv_usec - lastSampling.tv_usec ) / 1000000.0;
	lastSampling = currSampling;
	
	cleanup26DiskList();
	
	Dirty = 0;
}

static int process26DiskIO( const char* buf ) {
	/* Process values from /proc/diskstats (Linux >= 2.6.x) */
	
	/* For each disk /proc/diskstats includes lines as follows:
	*   3    0 hda 1314558 74053 26451438 14776742 1971172 4607401 52658448 202855090 0 9597019 217637839
	*   3    1 hda1 178 360 0 0
	*   3    2 hda2 354 360 0 0
	*   3    3 hda3 354 360 0 0
	*   3    4 hda4 0 0 0 0
	*   3    5 hda5 529506 9616000 4745856 37966848
	*
	* - See Documentation/iostats.txt for details on the changes
	*/
	int                      major, minor;
	char                     devname[DISKDEVNAMELEN];
	unsigned long            total,
				rio, rmrg, rblk, rtim,
				wio, wmrg, wblk, wtim,
				ioqueue, iotim, iotimw;
	DiskIOInfo               *ptr = DiskIO;
	DiskIOInfo               *last = 0;
	char                     sensorName[128];
	
	/*
	From kernel 2.6.22.1's Documentation/iostats.txt:

	First 3 fields of line are major, minor, devname
	Then:	
	Field  1 -- # of reads issued
		This is the total number of reads completed successfully.
	Field  2 -- # of reads merged, field 6 -- # of writes merged
		Reads and writes which are adjacent to each other may be merged for
		efficiency.  Thus two 4K reads may become one 8K read before it is
		ultimately handed to the disk, and so it will be counted (and queued)
		as only one I/O.  This field lets you know how often this was done.
	Field  3 -- # of sectors read
		This is the total number of sectors read successfully.
	Field  4 -- # of milliseconds spent reading
		This is the total number of milliseconds spent by all reads (as
		measured from __make_request() to end_that_request_last()).
	Field  5 -- # of writes completed
		This is the total number of writes completed successfully.
	Field  7 -- # of sectors written
		This is the total number of sectors written successfully.
	Field  8 -- # of milliseconds spent writing
		This is the total number of milliseconds spent by all writes (as
		measured from __make_request() to end_that_request_last()).
	Field  9 -- # of I/Os currently in progress
		The only field that should go to zero. Incremented as requests are
		given to appropriate request_queue_t and decremented as they finish.
	Field 10 -- # of milliseconds spent doing I/Os
		This field is increases so long as field 9 is nonzero.
	Field 11 -- weighted # of milliseconds spent doing I/Os
		This field is incremented at each I/O start, I/O completion, I/O
		merge, or read of these stats by the number of I/Os in progress
		(field 9) times the number of milliseconds spent doing I/O since the
		last update of this field.  This can provide an easy measure of both
		I/O completion time and the backlog that may be accumulating.
	*/

	switch (sscanf(buf, "%d %d %s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		&major, &minor, devname,
		&rio, &rmrg, &rblk, &rtim,
		&wio, &wmrg, &wblk, &wtim,
		&ioqueue, &iotim, &iotimw))
	{
	case 7:
		/* Partition stats entry */
		/* Adjust read fields rio rmrg rblk rtim -> rio rblk wio wblk */
		wblk = rtim;
		wio = rblk;
		rblk = rmrg;
	
		total = rio + wio;
	
		break;
	case 14:
		/* Disk stats entry */
		total = rio + wio;
	
		break;
	default:
		/* Something unexepected */
		return -1;
	}
	
	last = 0;
	ptr = DiskIO;
	while (ptr) {
		if (ptr->major == major && ptr->minor == minor)
		{
			/* The IO device has already been registered. */
			ptr->total.delta = total - ptr->total.old;
			ptr->total.old = total;
			ptr->rio.delta = rio - ptr->rio.old;
			ptr->rio.old = rio;
			ptr->wio.delta = wio - ptr->wio.old;
			ptr->wio.old = wio;
			ptr->rblk.delta = rblk - ptr->rblk.old;
			ptr->rblk.old = rblk;
			ptr->wblk.delta = wblk - ptr->wblk.old;
			ptr->wblk.old = wblk;
			ptr->rtim.delta = rtim - ptr->rtim.old;
			ptr->rtim.old = rtim;
			ptr->wtim.delta = wtim - ptr->wtim.old;
			ptr->wtim.old = wtim;
			/* fyi: ipqueue doesn't have a delta */
			ptr->ioqueue = ioqueue;

			ptr->alive = 1;
			break;
		}
		
		last = ptr;
		ptr = ptr->next;
	}
	
	if (!ptr) {
		/* The IO device has not been registered yet. We need to add it. */
		ptr = (DiskIOInfo*)malloc( sizeof( DiskIOInfo ) );
		ptr->major = major;
		ptr->minor = minor;
		ptr->devname = devname;
		ptr->total.delta = 0;
		ptr->total.old = total;
		ptr->rio.delta = 0;
		ptr->rio.old = rio;
		ptr->wio.delta = 0;
		ptr->wio.old = wio;
		ptr->rblk.delta = 0;
		ptr->rblk.old = rblk;
		ptr->wblk.delta = 0;
		ptr->wblk.old = wblk;
		ptr->rtim.delta = 0;
		ptr->rtim.old = rtim;
		ptr->wtim.delta = 0;
		ptr->wtim.old = wtim;
		/* fyi: ipqueue doesn't have a delta */
		ptr->ioqueue = ioqueue;

		ptr->alive = 1;
		ptr->next = 0;
		if (last) {
			/* Append new entry at end of list. */
			last->next = ptr;
		}
		else {
			/* List is empty, so we insert the fist element into the list. */
			DiskIO = ptr;
		}
		
		sprintf(sensorName, "disk/%s_(%d:%d)/Rate/totalio", devname, major, minor);
		registerMonitor(sensorName, "float", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Rate/rio", devname, major, minor);
		registerMonitor(sensorName, "float", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Rate/wio", devname, major, minor);
		registerMonitor(sensorName, "float", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Rate/rblk", devname, major, minor);
		registerMonitor(sensorName, "float", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Rate/wblk", devname, major, minor);
		registerMonitor(sensorName, "float", print26DiskIO, print26DiskIOInfo,
			StatSM);
		
		sprintf(sensorName, "disk/%s_(%d:%d)/Delta/totalio", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Delta/rio", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Delta/wio", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Delta/rblk", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Delta/wblk", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Delta/rtim", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);
		sprintf(sensorName, "disk/%s_(%d:%d)/Delta/wtim", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);

		sprintf(sensorName, "disk/%s_(%d:%d)/ioqueue", devname, major, minor);
		registerMonitor(sensorName, "integer", print26DiskIO, print26DiskIOInfo,
			StatSM);
	}
	
	return 0;
}

static void cleanup26DiskList( void ) {
	DiskIOInfo* ptr = DiskIO;
	DiskIOInfo* last = 0;
	
	while ( ptr ) {
		if ( ptr->alive == 0 ) {
			DiskIOInfo* newPtr;
			char sensorName[ 128 ];
			
			/* Disk device has disappeared. We have to remove it from
			* the list and unregister the monitors. */
			sprintf( sensorName, "disk/%s_(%d:%d)/Rate/totalio", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Rate/rio", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Rate/wio", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Rate/rblk", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Rate/wblk", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );

			sprintf( sensorName, "disk/%s_(%d:%d)/Delta/totalio", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Delta/rio", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Delta/wio", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Delta/rblk", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Delta/wblk", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Delta/rtim", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );
			sprintf( sensorName, "disk/%s_(%d:%d)/Delta/wtim", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );

			sprintf( sensorName, "disk/%s_(%d:%d)/ioqueue", ptr->devname, ptr->major, ptr->minor );
			removeMonitor( sensorName );

			if ( last ) {
				last->next = ptr->next;
				newPtr = ptr->next;
			}
			else {
				DiskIO = ptr->next;
				newPtr = DiskIO;
				last = 0;
			}
			
			free ( ptr );
			ptr = newPtr;
		}
		else {
			ptr->alive = 0;
			last = ptr;
			ptr = ptr->next;
		}
	}
}

void print26DiskIO( const char* cmd ) {
	int major, minor;
	char devname[DISKDEVNAMELEN];
	char name[ 17 ];
	DiskIOInfo* ptr;
	
	if ( Dirty )
		processDiskstats();

	if(sscanf( cmd, "disk/%[^_]_(%d:%d)/Rate/%16s", devname, &major, &minor, name ) == 4) {
		/* Show rate of change in sensor values in this interval */

		ptr = DiskIO;
		while ( ptr && ( ptr->major != major || ptr->minor != minor ) )
			ptr = ptr->next;
		
		if ( !ptr ) {
			print_error( "RECONFIGURE" );
			fprintf( CurrentClient, "0\n" );
			
			log_error( "Disk device disappeared" );
			return;
		}
		
		if ( strcmp( name, "total" ) == 0 )
			fprintf( CurrentClient, "%f\n", (float)( ptr->total.delta / timeInterval ) );
		else if ( strcmp( name, "rio" ) == 0 )
			fprintf( CurrentClient, "%f\n", (float)( ptr->rio.delta / timeInterval ) );
		else if ( strcmp( name, "wio" ) == 0 )
			fprintf( CurrentClient, "%f\n", (float)( ptr->wio.delta / timeInterval ) );
		else if ( strcmp( name, "rblk" ) == 0 )
			fprintf( CurrentClient, "%f\n", (float)( ptr->rblk.delta / ( timeInterval * 2 ) ) );
		else if ( strcmp( name, "wblk" ) == 0 )
			fprintf( CurrentClient, "%f\n", (float)( ptr->wblk.delta / ( timeInterval * 2 ) ) );
		else {
			fprintf( CurrentClient, "0\n" );
			log_error( "Unknown disk device property \'%s\'", name );
		}
	}
	else if(sscanf( cmd, "disk/%[^_]_(%d:%d)/Delta/%16s", devname, &major, &minor, name ) == 4) {
		/* Show change in sensor values per this interval */
		
		ptr = DiskIO;
		while ( ptr && ( ptr->major != major || ptr->minor != minor ) )
			ptr = ptr->next;
		
		if ( !ptr ) {
			print_error( "RECONFIGURE" );
			fprintf( CurrentClient, "0\n" );
			
			log_error( "Disk device disappeared" );
			return;
		}
		
		if ( strcmp( name, "total" ) == 0 )
			fprintf( CurrentClient, "%lu\n", ptr->total.delta );
		else if ( strcmp( name, "rio" ) == 0 )
			fprintf( CurrentClient, "%lu\n", ptr->rio.delta );
		else if ( strcmp( name, "wio" ) == 0 )
			fprintf( CurrentClient, "%lu\n", ptr->wio.delta );
		else if ( strcmp( name, "rblk" ) == 0 )
			fprintf( CurrentClient, "%lu\n", ptr->rblk.delta );
		else if ( strcmp( name, "wblk" ) == 0 )
			fprintf( CurrentClient, "%lu\n", ptr->wblk.delta );
		else if ( strcmp( name, "rtim" ) == 0 )
			fprintf( CurrentClient, "%lu\n", ptr->rtim.delta );
		else if ( strcmp( name, "wtim" ) == 0 )
			fprintf( CurrentClient, "%lu\n", ptr->wtim.delta );
		else {
			fprintf( CurrentClient, "0\n" );
			log_error( "Unknown disk device property \'%s\'", name );
		}
	}
	else if(sscanf( cmd, "disk/%[^_]_(%d:%d)/%16s", devname, &major, &minor, name ) == 4) {
		/* Show raw sensor values */

		ptr = DiskIO;
		while ( ptr && ( ptr->major != major || ptr->minor != minor ) )
			ptr = ptr->next;
		
		if ( !ptr ) {
			print_error( "RECONFIGURE" );
			fprintf( CurrentClient, "0\n" );
			
			log_error( "Disk device disappeared" );
			return;
		}
		
		if ( strcmp( name, "ioqueue" ) == 0 )
			fprintf( CurrentClient, "%u\n", ptr->ioqueue );
		else {
			fprintf( CurrentClient, "0\n" );
			log_error( "Unknown disk device property \'%s\'", name );
		}
	}
}

void print26DiskIOInfo( const char* cmd ) {
	int major, minor;
	char devname[DISKDEVNAMELEN];
	char name[ 17 ];
	DiskIOInfo* ptr = DiskIO;
	
	/* For now we print the same info regardless of whether it was a rate, delta, or raw monitor */
	if(sscanf( cmd, "disk/%[^_]_(%d:%d)/Rate/%16s", devname, &major, &minor, name ) == 4) {
	}
	else if(sscanf( cmd, "disk/%[^_]_(%d:%d)/Delta/%16s", devname, &major, &minor, name ) == 4) {
	}
	else if(sscanf( cmd, "disk/%[^_]_(%d:%d)/%16s", devname, &major, &minor, name ) == 4) {
	}
	else {
		fprintf( CurrentClient, "Dummy\t0\t0\t\n" );
		log_error( "Request for unknown device property \'%s\'",	cmd );
	}
	
	while ( ptr && ( ptr->major != major || ptr->minor != minor ) )
		ptr = ptr->next;
	
	if ( !ptr ) {
		/* Disk device has disappeared. Print a dummy answer. */
		fprintf( CurrentClient, "Dummy\t0\t0\t\n" );
		return;
	}
	
	/* remove trailing '?' */
	name[ strlen( name ) - 1 ] = '\0';
	
	if ( strcmp( name, "total" ) == 0 )
		fprintf( CurrentClient, "Total accesses device %s (%d:%d)\t0\t0\t1/s\n",
			devname, major, minor );
	else if ( strcmp( name, "rio" ) == 0 )
		fprintf( CurrentClient, "Read data device %s (%d:%d)\t0\t0\t1/s\n",
			devname, major, minor );
	else if ( strcmp( name, "wio" ) == 0 )
		fprintf( CurrentClient, "Write data device %s (%d:%d)\t0\t0\t1/s\n",
			devname, major, minor );
	else if ( strcmp( name, "rblk" ) == 0 )
		fprintf( CurrentClient, "Read accesses device %s (%d:%d)\t0\t0\tkBytes/s\n",
			devname, major, minor );
	else if ( strcmp( name, "wblk" ) == 0 )
		fprintf( CurrentClient, "Write accesses device %s (%d:%d)\t0\t0\tkBytes/s\n",
			devname, major, minor );
	else if ( strcmp( name, "rtim" ) == 0 )
		fprintf( CurrentClient, "# of milliseconds spent reading device %s (%d:%d)\t0\t0\ts\n",
			devname, major, minor );
	else if ( strcmp( name, "wtim" ) == 0 )
		fprintf( CurrentClient, "# of milliseconds spent writing device %s (%d:%d)\t0\t0\ts\n",
			devname, major, minor );
	else if ( strcmp( name, "ioqueue" ) == 0 )
		fprintf( CurrentClient, "# of I/Os currently in progress on device %s (%d:%d)\t0\t0\t\n",
			devname, major, minor );
	else {
		fprintf( CurrentClient, "Dummy\t0\t0\t\n" );
		log_error( "Request for unknown device property \'%s\'",	name );
	}
}
