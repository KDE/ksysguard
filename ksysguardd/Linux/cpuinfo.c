/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2000-2001 Chris Schlaeger <cs@kde.org>

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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "Command.h"
#include "ksysguardd.h"

#include "cpuinfo.h"

static int CpuInfoOK = 0;
static int numProcessors = 0; /* Total number of physical processors */
static int HighNumProcessors = 0; /* Highest # number of physical processors ever seen */
static int numCores = 0; /* Total # of cores */
static int HighNumCores = 0; /* Highest # of cores ever seen */
static float* Clocks = 0; /* Array with one entry per core */

/* Enough for 4-6 virtual cores. Larger values will be tried as needed. */
static size_t CpuInfoBufSize = 8 * 1024;
static char* CpuInfoBuf = NULL;
static int Dirty = 0;
static struct SensorModul *CpuInfoSM;

static void processCpuInfo( void )
{
    char format[ 32 ];
    char tag[ 32 ];
    char value[ 256 ];
    char* cibp = CpuInfoBuf;

    /* coreUniqueId is not per processor; it is a counter of the number of cores encountered
     * by the parse thus far */
    int coreUniqueId = 0;

    /* Indicates whether the "cpu MHz" value of the processor in /proc/cpuinfo should be used.
     * This is not done if parsing the frequency from cpufreq worked. */
    int useCpuInfoFreq = 1;

    /* Reset global variables */
    numCores = 0;
    numProcessors = 0;

    if ( !CpuInfoOK )
        return;

    sprintf( format, "%%%d[^:]: %%%d[^\n]\n", (int)sizeof( tag ) - 1,
            (int)sizeof( value ) - 1 );

    while ( sscanf( cibp, format, tag, value ) == 2 ) {
        char* p;

        tag[ sizeof( tag ) - 1 ] = '\0';
        value[ sizeof( value ) - 1 ] = '\0';

        /* remove trailing whitespaces */
        p = tag + strlen( tag ) - 1;
        /* remove trailing whitespaces */
        while ( ( *p == ' ' || *p == '\t' ) && p > tag )
            *p-- = '\0';

        if ( strcmp( tag, "processor" ) == 0 ) {
            if ( sscanf( value, "%d", &coreUniqueId ) == 1 ) {
                if ( coreUniqueId >= HighNumCores ) {
                    /* Found a new processor core. Maybe even a new processor. (We'll check later) */
                    char cmdName[ 24 ];

                    /* Each core has a clock speed. Allocate one per core found. */
                    Clocks = (float*) realloc( Clocks, (coreUniqueId+1) * sizeof( float ) );
                    memset(Clocks + HighNumCores, 0, (coreUniqueId +1 - HighNumCores) * sizeof( float ));

                    HighNumCores = coreUniqueId + 1;

                    snprintf( cmdName, sizeof( cmdName ) - 1, "cpu/cpu%d/clock", coreUniqueId );
                    registerMonitor( cmdName, "float", printCPUxClock, printCPUxClockInfo,
                            CpuInfoSM );
                }

                useCpuInfoFreq = 1;

                const char freqTemplate[] = "/sys/bus/cpu/devices/cpu%d/cpufreq/scaling_cur_freq";
                char freqName[sizeof(freqTemplate) + 3];
                snprintf(freqName, sizeof(freqName) - 1, freqTemplate, coreUniqueId);
                FILE *freqFd = fopen(freqName, "r");
                if (freqFd) {
                    unsigned long khz;
                    if(fscanf(freqFd, "%lu\n", &khz) == 1) {
                        Clocks[coreUniqueId] = khz / 1000.0f;
                        useCpuInfoFreq = 0;
                    }

                    fclose(freqFd);
                }
            }
        } else if ( useCpuInfoFreq && strcmp( tag, "cpu MHz" ) == 0 ) {
            if (HighNumCores > coreUniqueId) {
                /* The if statement above *should* always be true, but there's no harm in being safe. */
                sscanf( value, "%f", &Clocks[ coreUniqueId ] );
            }
        } else if ( strcmp( tag, "core id" ) == 0 ) {
            /* the core id is per processor */
            int curCore;

            if ( (sscanf( value, "%d", &curCore ) == 1) && curCore == 0 ) {
                /* core id is back at 0. We just found a new processor. */
                numProcessors++;

                if (numProcessors > HighNumProcessors)
                    HighNumProcessors = numProcessors;
            }
        }
        /* Move cibp to beginning of next line, if there is one. */
        cibp = strchr( cibp, '\n' );
        if ( cibp )
            cibp++;
        else
            cibp = CpuInfoBuf + strlen( CpuInfoBuf );
    }

    numCores = coreUniqueId + 1;

    Dirty = 0;
}

/*
================================ public part =================================
*/

void initCpuInfo( struct SensorModul* sm )
{
    CpuInfoSM = sm;

    if ( updateCpuInfo() < 0 )
        return;

    registerMonitor( "system/processors", "integer", printNumCpus, printNumCpusInfo,
            CpuInfoSM );
    registerMonitor( "system/cores", "integer", printNumCores, printNumCoresInfo,
            CpuInfoSM );

    processCpuInfo();

    registerMonitor( "cpu/system/AverageClock", "float", printCPUClock, printCPUClockInfo,
            CpuInfoSM );
}

void exitCpuInfo( void )
{
    CpuInfoOK = -1;

    free( Clocks );
}

int updateCpuInfo( void )
{
    size_t n;
    int fd;

    if ( CpuInfoOK < 0 )
        return -1;

    if ( ( fd = open( "/proc/cpuinfo", O_RDONLY ) ) < 0 ) {
        if ( CpuInfoOK != 0 )
            print_error( "Cannot open file \'/proc/cpuinfo\'!\n"
                    "The kernel needs to be compiled with support\n"
                    "for /proc file system enabled!\n" );
        CpuInfoOK = -1;
        return -1;
    }

    if ( CpuInfoBuf == NULL ) {
        CpuInfoBuf = malloc( CpuInfoBufSize );
    }
    n = 0;
    for(;;) {
        ssize_t len = read( fd, CpuInfoBuf + n, CpuInfoBufSize - 1 - n );
        if( len < 0 ) {
            print_error( "Failed to read file \'/proc/cpuinfo\'!\n" );
            CpuInfoOK = -1;
            close( fd );
            return -1;
        }
        n += len;
        if( len == 0 ) /* reading finished */
            break;
        if( n == CpuInfoBufSize - 1 ) {
            /* The buffer was too small. Double its size and keep going. */
            size_t new_size = CpuInfoBufSize * 2;
            char* new_buffer = malloc( new_size );
            memcpy( new_buffer, CpuInfoBuf, n ); /* copy read data */
            free( CpuInfoBuf ); /* free old buffer */
            CpuInfoBuf = new_buffer; /* remember new buffer and size */
            CpuInfoBufSize = new_size;
        }
    }

    close( fd );
    CpuInfoOK = 1;
    CpuInfoBuf[ n ] = '\0';
    Dirty = 1;

    return 0;
}

void printCPUxClock( const char* cmd )
{
    int id;

    if ( Dirty )
        processCpuInfo();

    sscanf( cmd + 7, "%d", &id );
    output( "%f\n", Clocks[ id ] );
}

void printCPUClock( const char* cmd )
{
    int id;
    float clock = 0;
    cmd = cmd; /*Silence warning*/

    if ( Dirty ) {
        processCpuInfo();
    }

    for ( id = 0; id < HighNumCores; id++ ) {
        clock += Clocks[ id ];
    }
    clock /= HighNumCores;
    output( "%f\n", clock );
}

void printCPUxClockInfo( const char* cmd )
{
    int id;

    sscanf( cmd + 7, "%d", &id );
    output( "CPU%d Clock Frequency\t0\t0\tMHz\n", id );
}

void printCPUClockInfo( const char* cmd )
{
    cmd = cmd; /*Silence warning*/
    output( "CPU Clock Frequency\t0\t0\tMHz\n" );
}

void printNumCpus( const char* cmd )
{
    (void) cmd;

    if ( Dirty )
        processCpuInfo();

    output( "%d\n", numProcessors );
}

void printNumCpusInfo( const char* cmd )
{
    (void) cmd;

    output( "Number of physical CPUs\t0\t0\t\n" );
}

void printNumCores( const char* cmd )
{
    (void) cmd;

    if ( Dirty )
        processCpuInfo();

    output( "%d\n", numCores );
}

void printNumCoresInfo( const char* cmd )
{
    (void) cmd;

    output( "Total number of processor cores\t0\t0\t\n" );
}
