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

#include <config-workspace.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "Command.h"
#include "ksysguardd.h"

#include "netdev.h"

#define MON_SIZE	128

#define CALC( a, b, c, d, e, f ) \
{ \
  if (f){ \
    NetDevs[ i ].a = a - NetDevs[ i ].Old##a; \
    NetDevs[ i ].Old##a = a; \
  } \
  else{ \
    NetDevs[ i ].a = a; \
  } \
}

#define REGISTERSENSOR( a, b, c, d, e, f ) \
{ \
  snprintf( mon, MON_SIZE, "network/interfaces/%s/%s", tag, b ); \
  registerMonitor( mon, "float", printNetDev##a, printNetDev##a##Info, NetDevSM ); \
}

#define UNREGISTERSENSOR( a, b, c, d, e, f ) \
{ \
  snprintf( mon, MON_SIZE, "network/interfaces/%s/%s", NetDevs[ i ].name, b ); \
  removeMonitor( mon ); \
}

#define DEFMEMBERS( a, b, c, d, e, f ) \
signed long long Old##a; \
signed long long a; \
signed long a##Scale;

#define DEFVARS( a, b, c, d, e, f ) \
signed long long a;

/* The sixth variable is 1 if the quantity variation must be provided, 0 if the absolute value must be provided */
#define FORALL( a ) \
  a( recBytes, "receiver/data", "Received Data", "KB/s", 1024, 1) \
  a( recPacks, "receiver/packets", "Received Packets", "1/s", 1, 1 ) \
  a( recErrs, "receiver/errors", "Receiver Errors", "1/s", 1, 1 ) \
  a( recDrop, "receiver/drops", "Receiver Drops", "1/s", 1, 1 ) \
  a( recFifo, "receiver/fifo", "Receiver FIFO Overruns", "1/s", 1, 1 ) \
  a( recFrame, "receiver/frame", "Receiver Frame Errors", "1/s", 1, 1 ) \
  a( recCompressed, "receiver/compressed", "Received Compressed Packets", "1/s", 1, 1 ) \
  a( recMulticast, "receiver/multicast", "Received Multicast Packets", "1/s", 1, 1 ) \
  a( sentBytes, "transmitter/data", "Sent Data", "KB/s", 1024, 1 ) \
  a( sentPacks, "transmitter/packets", "Sent Packets", "1/s", 1, 1 ) \
  a( sentErrs, "transmitter/errors", "Transmitter Errors", "1/s", 1, 1 ) \
  a( sentDrop, "transmitter/drops", "Transmitter Drops", "1/s", 1, 1 ) \
  a( sentFifo, "transmitter/fifo", "Transmitter FIFO overruns", "1/s", 1, 1 ) \
  a( sentColls, "transmitter/collisions", "Transmitter Collisions", "1/s", 1, 1 ) \
  a( sentCarrier, "transmitter/carrier", "Transmitter Carrier losses", "1/s", 1, 1 ) \
  a( sentCompressed, "transmitter/compressed", "Transmitter Compressed Packets", "1/s", 1, 1 )

#define FORALLWIFI( a ) \
  a( linkQuality, "wifi/quality", "Link Quality", "", 1, 0) \
  a( signalLevel, "wifi/signal", "Signal Level", "dBm", 1, 0) \
  a( noiseLevel, "wifi/noise", "Noise Level", "dBm", 1, 0) \
  a( nwid, "wifi/nwid", "Rx Invalid Nwid Packets", "1/s", 1, 1) \
  a( RxCrypt, "wifi/crypt", "Rx Invalid Crypt Packets", "1/s", 1, 1) \
  a( frag, "wifi/frag", "Rx Invalid Frag Packets", "1/s", 1, 1) \
  a( retry, "wifi/retry", "Tx Excessive Retries Packets", "1/s", 1, 1) \
  a( misc, "wifi/misc", "Invalid Misc Packets", "1/s", 1, 1) \
  a( beacon, "wifi/beacon", "Missed Beacon", "1/s", 1, 1)

#define SETZERO( a, b, c, d, e, f ) \
a = 0;

#define SETMEMBERZERO( a, b, c, d, e, f ) \
NetDevs[ i ].a = 0; \
NetDevs[ i ].a##Scale = e;

#define DECLAREFUNC( a, b, c, d, e, f ) \
void printNetDev##a( const char* cmd ); \
void printNetDev##a##Info( const char* cmd );

typedef struct
{
  FORALL( DEFMEMBERS )
  FORALLWIFI( DEFMEMBERS )
  char name[ 32 ];
  int isWifi;
} NetDevInfo;

/* We have observed deviations of up to 5% in the accuracy of the timer
 * interrupts. So we try to measure the interrupt interval and use this
 * value to calculate timing dependant values. */
static float timeInterval = 0;
static struct timeval lastSampling;
static struct timeval currSampling;
static struct SensorModul* NetDevSM;

#define NETDEVBUFSIZE 4096
static char NetDevBuf[ NETDEVBUFSIZE ];
static char NetDevWifiBuf[ NETDEVBUFSIZE ];
static int NetDevCnt = 0;
static int Dirty = 0;
static int NetDevOk = 0;
static long OldHash = 0;

#define MAXNETDEVS 64
static NetDevInfo NetDevs[ MAXNETDEVS ];

void processNetDev( void );

FORALL( DECLAREFUNC )
FORALLWIFI( DECLAREFUNC )

static int processNetDev_( void )
{
  int i, j;
  char format[ 32 ];
  char devFormat[ 16 ];
  char buf[ 1024 ];
  char tag[ 64 ];
  char* netDevBufP = NetDevBuf;
  char* netDevWifiBufP = NetDevWifiBuf;

  sprintf( format, "%%%d[^\n]\n", (int)sizeof( buf ) - 1 );
  sprintf( devFormat, "%%%ds", (int)sizeof( tag ) - 1 );

  /* skip 2 first lines */
  for ( i = 0; i < 2; i++ ) {
    sscanf( netDevBufP, format, buf );
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevBufP += strlen( buf ) + 1;  /* move netDevBufP to next line */
  }

  for ( i = 0; sscanf( netDevBufP, format, buf ) == 1; ++i ) {
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevBufP += strlen( buf ) + 1;  /* move netDevBufP to next line */

    if ( sscanf( buf, devFormat, tag ) ) {
      char* pos = strchr( tag, ':' );
      if ( pos ) {
        FORALL( DEFVARS );
        *pos = '\0';
        FORALL( SETZERO );
        sscanf( buf + 7, "%lli %lli %lli %lli %lli %lli %lli %lli "
                "%lli %lli %lli %lli %lli %lli %lli %lli",
                &recBytes, &recPacks, &recErrs, &recDrop, &recFifo,
                &recFrame, &recCompressed, &recMulticast,
                &sentBytes, &sentPacks, &sentErrs, &sentDrop,
                &sentFifo, &sentColls, &sentCarrier, &sentCompressed );

        if ( i >= NetDevCnt || strcmp( NetDevs[ i ].name, tag ) != 0 ) {
          /* The network device configuration has changed. We
           * need to reconfigure the netdev module. */
          return -1;
        } else {
          FORALL( CALC );
        }
      }
    }
  }

  if ( i != NetDevCnt )
    return -1;

  /*Update the values for the wifi interfaces*/
  if ( *netDevWifiBufP=='\0')  /*there is no /proc/net/wireless file*/
    return 0;

  /* skip 2 first lines */
  for ( i = 0; i < 2; i++ ) {
    sscanf( netDevWifiBufP, format, buf );
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevWifiBufP += strlen( buf ) + 1;  /* move netDevWifiBufP to next line */
  }

  for ( j = 0; sscanf( netDevWifiBufP, format, buf ) == 1; ++j ) {
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevWifiBufP += strlen( buf ) + 1;  /* move netDevWifiBufP to next line */

    if ( sscanf( buf, devFormat, tag ) ) {
      char* pos = strchr( tag, ':' );
      if ( pos ) {
        FORALLWIFI( DEFVARS );
        *pos = '\0';

       for (i = 0 ; i < NetDevCnt ; ++i){ /*find the corresponding interface*/
           if ( strcmp(tag,NetDevs[ i ].name)==0){
               break;
           }
        }
        sscanf( buf + 12, " %lli. %lli. %lli. %lli %lli %lli %lli %lli %lli",
                &linkQuality, &signalLevel, &noiseLevel, &nwid,
                &RxCrypt, &frag,  &retry, &misc, &beacon );
        signalLevel -= 256; /*the units are dBm*/
        noiseLevel -= 256;
        FORALLWIFI( CALC );
      }
    }
  }

  /* save exact time inverval between this and the last read of
   * /proc/net/dev */
  timeInterval = currSampling.tv_sec - lastSampling.tv_sec +
                 ( currSampling.tv_usec - lastSampling.tv_usec ) / 1000000.0;
  lastSampling = currSampling;
  Dirty = 0;

  return 0;
}

void processNetDev( void )
{
  int i;

  if ( NetDevCnt == 0 )
		return;

  for ( i = 0; i < 5 && processNetDev_() < 0; ++i )
    checkNetDev();

  /* If 5 reconfiguration attemts failed, something is very wrong and
   * we close the netdev module for further use. */
  if ( i == 5 )
    exitNetDev();
}

/*
================================ public part =================================
*/

void initNetDev( struct SensorModul* sm )
{
  int i, j;
  char format[ 32 ];
  char devFormat[ 16 ];
  char buf[ 1024 ];
  char tag[ 64 ];
  char* netDevBufP = NetDevBuf;
  char* netDevWifiBufP = NetDevWifiBuf;

  NetDevSM = sm;

  if ( updateNetDev() < 0 )
    return;

  sprintf( format, "%%%d[^\n]\n", (int)sizeof( buf ) - 1 );
  sprintf( devFormat, "%%%ds", (int)sizeof( tag ) - 1 );

  /* skip 2 first lines */
  for ( i = 0; i < 2; i++ ) {
    sscanf( netDevBufP, format, buf );
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevBufP += strlen( buf ) + 1;  /* move netDevBufP to next line */
  }

  for ( i = 0; sscanf( netDevBufP, format, buf ) == 1; ++i ) {
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevBufP += strlen( buf ) + 1;  /* move netDevBufP to next line */

    if ( sscanf( buf, devFormat, tag ) ) {
      char* pos = strchr( tag, ':' );
      if ( pos ) {
        char mon[ MON_SIZE ];
        *pos = '\0';
        strncpy( NetDevs[ i ].name, tag, sizeof( NetDevs[ i ].name ) );
        NetDevs[ i ].name[ sizeof( NetDevs[ i ].name )-1] = 0;
        FORALL( REGISTERSENSOR );
        sscanf( pos + 1, "%lli %lli %lli %lli %lli %lli %lli %lli"
                "%lli %lli %lli %lli %lli %lli %lli %lli",
                &NetDevs[ i ].recBytes, &NetDevs[ i ].recPacks,
                &NetDevs[ i ].recErrs, &NetDevs[ i ].recDrop,
                &NetDevs[ i ].recFifo, &NetDevs[ i ].recFrame,
                &NetDevs[ i ].recCompressed, &NetDevs[ i ].recMulticast,
                &NetDevs[ i ].sentBytes, &NetDevs[ i ].sentPacks,
                &NetDevs[ i ].sentErrs, &NetDevs[ i ].sentDrop,
                &NetDevs[ i ].sentFifo, &NetDevs[ i ].sentColls,
                &NetDevs[ i ].sentCarrier, &NetDevs[ i ].sentCompressed );
        NetDevCnt++;
			}
      FORALL( SETMEMBERZERO );
    }
  }

  /* detect the wifi interfaces*/
  /* skip 2 first lines */
  for ( i = 0; i < 2; i++ ) {
    sscanf( netDevWifiBufP, format, buf );
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevWifiBufP += strlen( buf ) + 1;  /* move netDevWifiBufP to next line */
  }

  for ( j = 0; sscanf( netDevWifiBufP, format, buf ) == 1; ++j ) {
    buf[ sizeof( buf ) - 1 ] = '\0';
    netDevWifiBufP += strlen( buf ) + 1;  /* move netDevWifiBufP to next line */

    if ( sscanf( buf, devFormat, tag ) ) {
      char * pos = strchr( tag, ':' );
      if ( pos ) {
        char mon[ MON_SIZE ];
        *pos = '\0';
       /*find and tag the corresponding NetDev as wifi enabled.
        At the end of the loop,  i is the index of the device.
        This variable i is used in some macro */
       for (i = 0 ; i < NetDevCnt ; ++i){
           if ( strcmp(tag,NetDevs[ i ].name)==0){
               NetDevs[ i ].isWifi = 1;
               break;
           }
        }
        FORALLWIFI( REGISTERSENSOR );
      }
      FORALLWIFI( SETMEMBERZERO );  /* the variable i must point to the corrrect NetDevs[i]*/
    }
  }

  /* Call processNetDev to elimitate initial peek values. */
  processNetDev();
}

void exitNetDev( void )
{
  int i;

  for ( i = 0; i < NetDevCnt; ++i ) {
    char mon[ MON_SIZE ];
    FORALL( UNREGISTERSENSOR );
    if (NetDevs[ i ].isWifi)
       FORALLWIFI( UNREGISTERSENSOR );
  }
  NetDevCnt = 0;
}

int updateNetDev( void )
{
  /* We read the information about the network interfaces from
     /proc/net/dev. The file should look like this:

  Inter-|   Receive                                                 |  Transmit
  face  | bytes    packets errs drop fifo frame compressed multicast| bytes    packets errs drop fifo colls carrier compressed
      lo:275135772 1437448    0    0    0     0          0         0 275135772 1437448    0    0    0     0       0          0
    eth0:123648812  655251    0    0    0     0          0         0 246847871  889636    0    0    0     0       0          0
	*/

  size_t n;
  int fd;
  long hash;
  char* p;

  if ( NetDevOk < 0 )
    return 0;

  if ( ( fd = open( "/proc/net/dev", O_RDONLY ) ) < 0 ) {
    /* /proc/net/dev may not exist on some machines. */
    NetDevOk = -1;
    return 0;
  }

  n = read( fd, NetDevBuf, NETDEVBUFSIZE - 1 );
  if ( n == NETDEVBUFSIZE - 1 || n <= 0) {
    log_error( "Internal buffer too small to read \'/proc/net/dev\'" );
    NetDevOk = -1;

    close( fd );
    return -1;
  }

  gettimeofday( &currSampling, 0 );
  close( fd );
  NetDevOk = 1;
  NetDevBuf[ n ] = '\0';

  /* Calculate hash over the first 7 characters of each line starting
   * after the first newline. This will detect whether any interfaces
   * have either appeared or disappeared. */
  for ( p = NetDevBuf, hash = 0; *p; ++p )
    if ( *p == '\n' )
      for ( ++p; *p && *p != ':' && *p != '|'; ++p )
        hash = ( ( hash << 6 ) + *p ) % 390389;

  if ( OldHash != 0 && OldHash != hash ) {
    print_error( "RECONFIGURE\n" );
    CheckSetupFlag = 1;
  }
  OldHash = hash;

  /* We read the information about the wifi from /proc/net/wireless and store it into NetDevWifiBuf */
  if ( ( fd = open( "/proc/net/wireless", O_RDONLY ) ) < 0 ) {
    /* /proc/net/wireless may not exist on some machines. */
    NetDevWifiBuf[0]='\0';
    return -1;
  }

  if ( ( n = read( fd, NetDevWifiBuf, NETDEVBUFSIZE - 1 ) ) == NETDEVBUFSIZE - 1 ) {
    log_error( "Internal buffer too small to read \'/proc/net/wireless\'" );
    close( fd );
    return -1;
  }
  close( fd );
  NetDevWifiBuf[ n ] = '\0';

  Dirty = 1;

  return 0;
}

void checkNetDev( void )
{
	/* Values for other network devices are lost, but it is still better
	 * than not detecting any new devices. TODO: Fix after 2.1 is out. */
  exitNetDev();
  initNetDev( NetDevSM );
}

#define PRINTFUNC( a, b, c, d, e, f ) \
void printNetDev##a( const char* cmd ) \
{ \
  int i; \
  char* beg; \
  char* end; \
  char dev[ 64 ]; \
 \
  beg = strchr( cmd, '/' ); \
  beg = strchr( beg + 1, '/' ); \
  end = strchr( beg + 1, '/' ); \
  strncpy( dev, beg + 1, end - beg - 1 ); \
  dev[ end - beg - 1 ] = '\0'; \
 \
  if ( Dirty ) \
    processNetDev(); \
 \
  for ( i = 0; i < MAXNETDEVS; ++i ) \
    if ( strcmp( NetDevs[ i ].name, dev ) == 0) { \
      if (f) \
         fprintf( CurrentClient, "%li\n", (long) \
                ( NetDevs[ i ].a / ( NetDevs[ i ].a##Scale * timeInterval ) ) ); \
      else \
         fprintf( CurrentClient, "%li\n", (long) NetDevs[ i ].a ); \
      return; \
    } \
 \
  fprintf( CurrentClient, "0\n" ); \
} \
 \
void printNetDev##a##Info( const char* cmd ) \
{ \
  int i; \
  char* beg; \
  char* end; \
  char dev[ 64 ]; \
 \
  beg = strchr( cmd, '/' ); \
  beg = strchr( beg + 1, '/' ); \
  end = strchr( beg + 1, '/' ); \
  strncpy( dev, beg + 1, end - beg - 1 ); \
  dev[ end - beg - 1 ] = '\0'; \
\
  fprintf( CurrentClient, "%s %s\t0\t0\t%s\n", dev, c, d ); \
}

FORALL( PRINTFUNC )
FORALLWIFI( PRINTFUNC )
