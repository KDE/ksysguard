/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

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

#define _XOPEN_SOURCE /* isascii */

#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#include "Command.h"
#include "ccont.h"
#include "diskstat.h"
#include "ksysguardd.h"

typedef struct {
    char device[ 256 ];
    char mntpnt[ 256 ];
    struct statvfs statvfs;
} DiskInfo;

static CONTAINER DiskStatList = 0;
static CONTAINER OldDiskStatList = 0;
static struct SensorModul* DiskStatSM;
char *getMntPnt( const char* cmd );

static void sanitize(char *str)  {
    if(str == NULL)
        return;
    while (*str != 0)  {
        if(*str == '\t' || *str == '\n' || *str == '\r' || *str == ' ' || !isascii(*str) )
            *str = '?';
        ++str;
    }
}

char *getMntPnt( const char* cmd )
{
    static char device[ 1025 ];
    char* ptr;

    memset( device, 0, sizeof( device ) );
    sscanf( cmd, "partitions%1024s", device );

    ptr = strrchr( device, '/' );
    if( ptr ) {
        *ptr = '\0';
    }

    return (char*)device;
}

unsigned long getTotal( const char* mntpnt )
{
    DiskInfo* disk_info;

    unsigned long total = 0;
    int is_all = strcmp( mntpnt, "/all" ) == 0;

    for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
        if ( !strcmp( mntpnt, disk_info->mntpnt ) || is_all ) {
            unsigned long totalSizeKB =  disk_info->statvfs.f_blocks * (disk_info->statvfs.f_frsize/1024);

            if ( is_all ) {
                total += totalSizeKB;
            } else {
                return totalSizeKB;
            }
        }
    }

    return total;
}

/* ----------------------------- public part ------------------------------- */

static char monitor[ 1024 ];
static void registerMonitors(const char* mntpnt) {
    snprintf( monitor, sizeof( monitor ), "partitions%s/usedspace", mntpnt );
    registerMonitor( monitor, "integer", printDiskStatUsed, printDiskStatUsedInfo, DiskStatSM );
    snprintf( monitor, sizeof( monitor ), "partitions%s/freespace", mntpnt );
    registerMonitor( monitor, "integer", printDiskStatFree, printDiskStatFreeInfo, DiskStatSM );
    snprintf( monitor, sizeof( monitor ), "partitions%s/filllevel", mntpnt );
    registerMonitor( monitor, "integer", printDiskStatPercent, printDiskStatPercentInfo, DiskStatSM );
    snprintf( monitor, sizeof( monitor ), "partitions%s/total", mntpnt );
    registerMonitor( monitor, "integer", printDiskStatTotal, printDiskStatTotalInfo, DiskStatSM );
}
static void removeMonitors(const char* mntpnt) {
    snprintf( monitor, sizeof( monitor ), "partitions%s/usedspace", mntpnt );
    removeMonitor( monitor );
    snprintf( monitor, sizeof( monitor ), "partitions%s/freespace", mntpnt );
    removeMonitor( monitor );
    snprintf( monitor, sizeof( monitor ), "partitions%s/filllevel", mntpnt );
    removeMonitor( monitor );
    snprintf( monitor, sizeof( monitor ), "partitions%s/total", mntpnt );
    removeMonitor( monitor );
}

void initDiskStat( struct SensorModul* sm )
{
    DiskInfo* disk_info;

    DiskStatList = NULL;
    OldDiskStatList = NULL;
    DiskStatSM = sm;
    if ( updateDiskStat() < 0 )
        return;

    registerMonitor( "partitions/list", "listview", printDiskStat, printDiskStatInfo, sm );

    registerMonitors( "/all" );

    for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
        registerMonitors(disk_info->mntpnt);
    }
}

void exitDiskStat( void )
{
    DiskInfo* disk_info;

    removeMonitor( "partitions/list" );

    removeMonitors( "/all" );

    for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
        removeMonitors(disk_info->mntpnt);
    }

    destr_ctnr( DiskStatList, free );
    if(OldDiskStatList)
        destr_ctnr( OldDiskStatList, free );
}

void checkDiskStat( void )
{
    updateDiskStat();
    DiskInfo* disk_info_new;
    DiskInfo* disk_info_old;
    int changed = 0;
    for ( disk_info_new = first_ctnr( DiskStatList ); disk_info_new; disk_info_new = next_ctnr( DiskStatList ) ) {
        int found = 0;
        for ( disk_info_old = first_ctnr( OldDiskStatList ); disk_info_old; disk_info_old = next_ctnr( OldDiskStatList ) ) {
            if(strcmp(disk_info_new->mntpnt, disk_info_old->mntpnt) == 0) {
                free( remove_ctnr( OldDiskStatList ) );
                found = 1;
                continue;
            }
        }
        if(!found) {
            /* register all the devices that did not exist before*/
            registerMonitors(disk_info_new->mntpnt);
            changed++;
        }
    }
    /*Now remove all the devices that do not exist anymore*/
    for ( disk_info_old = first_ctnr( OldDiskStatList ); disk_info_old; disk_info_old = next_ctnr( OldDiskStatList ) ) {
        removeMonitors(disk_info_old->mntpnt);
        changed++;
    }
    destr_ctnr( OldDiskStatList, free );
    OldDiskStatList = NULL;
    updateDiskStat();
    if(changed)
        print_error( "RECONFIGURE" ); /*Let ksysguard know that we've added a sensor*/
}

int updateDiskStat( void )
{
    DiskInfo *disk_info;
    FILE *fh;
    struct mntent *mnt_info;

    if ( ( fh = setmntent( "/etc/mtab", "r" ) ) == NULL ) {
        print_error( "Cannot open \'/etc/mtab\'!\n" );
        return -1;
    }
    if(OldDiskStatList == 0) {
        OldDiskStatList = DiskStatList;
        DiskStatList = new_ctnr();
    } else {
        empty_ctnr(DiskStatList);
    }

    while ( ( mnt_info = getmntent( fh ) ) != NULL ) {
        /*
         * An entry which device name doesn't start with a '/' is
         * either a dummy file system or a network file system.
         * Add special handling for smbfs and cifs as is done by
         * coreutils as well.
         */
        if ( (mnt_info->mnt_fsname[0] != '/') ||
             !strncmp( mnt_info->mnt_fsname, "/dev/loop", 9 ) ||
             !strncmp( mnt_info->mnt_type, "fuse", 4 ) ||
             !strcmp( mnt_info->mnt_type, "smbfs" ) ||
             !strcmp( mnt_info->mnt_type, "cifs" ) ||
             !strcmp( mnt_info->mnt_type, "proc" ) ||
             !strcmp( mnt_info->mnt_type, "devfs" ) ||
             !strcmp( mnt_info->mnt_type, "usbfs" ) ||
             !strcmp( mnt_info->mnt_type, "sysfs" ) ||
             !strcmp( mnt_info->mnt_type, "tmpfs" ) ||
             !strcmp( mnt_info->mnt_type, "devpts" ) )
            continue;  /* Skip these file systems */

        if ( ( disk_info = (DiskInfo *)malloc( sizeof( DiskInfo ) ) ) == NULL )
            continue;

        memset( disk_info, 0, sizeof( DiskInfo ) );

        if ( statvfs( mnt_info->mnt_dir, &(disk_info->statvfs) ) < 0 )
            continue;

        strncpy( disk_info->device, mnt_info->mnt_fsname, sizeof( disk_info->device ) );
        disk_info->device[ sizeof(disk_info->device) -1] = 0;

        if ( strcmp(mnt_info->mnt_dir, "/") == 0 ) {
            strncpy( disk_info->mntpnt, "/__root__", 9);
        } else {
            strncpy( disk_info->mntpnt, mnt_info->mnt_dir, sizeof( disk_info->mntpnt ) );
            disk_info->mntpnt[ sizeof(disk_info->mntpnt) - 1] = 0;
        }
        sanitize(disk_info->mntpnt);

        push_ctnr( DiskStatList, disk_info );
    }
    endmntent( fh );

    return 0;
}

int calculatePercentageUsed( unsigned long totalSizeKB, unsigned long available) {
    if (!available)
        return 0;

    unsigned long totalSizeKBdividedBy100 = (50 + totalSizeKB )/ 100;
    if (!totalSizeKBdividedBy100)
        return 0;

    int percentageUsed = 100 - available / totalSizeKBdividedBy100; /* Percentage is 1 - available / totalSizeKB, meaning that we count root-only reserved space as "used" here */
    /* If we have rounded down to 0%, make it 1%, like "df" does */
    if (percentageUsed == 0)
        return 1;
    return percentageUsed;
}

void printDiskStat( const char* cmd )
{
    DiskInfo* disk_info;

    (void)cmd;
    for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
        /* See man statvfs(2) for meaning of fields */
        unsigned long totalSizeKB =  disk_info->statvfs.f_blocks * (disk_info->statvfs.f_frsize/1024);
        unsigned long usedKB = totalSizeKB - (disk_info->statvfs.f_bfree * (disk_info->statvfs.f_bsize/1024)); /* used is the total size minus free blocks including those for root only */
        unsigned long available = disk_info->statvfs.f_bavail * (disk_info->statvfs.f_bsize/1024); /* available is only those for non-root.  So available + used != total because some are reserved for root */
        int percentageUsed = calculatePercentageUsed(totalSizeKB, available);
        output( "%s\t%ld\t%ld\t%ld\t%d\t%s\n",
                disk_info->device,
                totalSizeKB,
                usedKB,
                available,
                percentageUsed,
                disk_info->mntpnt );
    }

    output( "\n" );
}

void printDiskStatInfo( const char* cmd )
{
    (void)cmd;
    output( "Device\tSize\tUsed\tAvailable\tUsed %%\tMount point\nM\tKB\tKB\tKB\t%%\ts\n" );
}

void printDiskStatUsed( const char* cmd )
{
    char *mntpnt = (char*)getMntPnt( cmd );
    DiskInfo* disk_info;

    unsigned long total = 0;
    int is_all = strcmp( mntpnt, "/all" ) == 0;

    for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
        if ( !strcmp( mntpnt, disk_info->mntpnt ) || is_all ) {
            unsigned long totalSizeKB =  disk_info->statvfs.f_blocks * (disk_info->statvfs.f_frsize/1024);
            unsigned long usedKB = totalSizeKB - (disk_info->statvfs.f_bfree * (disk_info->statvfs.f_bsize/1024)); /* used is the total size minus free blocks including those for root only */

            if ( !is_all ) {
                output( "%ld\n", usedKB );
            } else {
                total += usedKB;
            }
        }
    }

    if ( is_all ) {
        output( "%ld\n", total );
    }

    output( "\n" );
}

void printDiskStatUsedInfo( const char* cmd )
{
    char *mntpnt = getMntPnt( cmd );

    if ( strcmp( mntpnt, "/all" ) == 0 ) {
        output( "All Partitions Used\t0\t%ld\tKB\n", getTotal( mntpnt ) );
    } else if ( strcmp( mntpnt, "/__root__" ) == 0 ) {
        output( "Filesystem Root Used\t0\t%ld\tKB\n", getTotal( mntpnt ) );
    } else {
        output( "%s Used\t0\t%ld\tKB\n", mntpnt, getTotal( mntpnt ) );
    }
}

void printDiskStatFree( const char* cmd )
{
    char *mntpnt = (char*)getMntPnt( cmd );
    DiskInfo* disk_info;

    unsigned long total = 0;
    int is_all = strcmp( mntpnt, "/all" ) == 0;

    for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
        if ( !strcmp( mntpnt, disk_info->mntpnt ) || is_all ) {
            unsigned long available = disk_info->statvfs.f_bavail * (disk_info->statvfs.f_bsize/1024); /* available is only those for non-root.  So available + used != total because some are reserved for root */
            if ( !is_all ) {
                output( "%ld\n", available );
            } else {
                total += available;
            }
        }
    }

    if ( is_all ) {
        output( "%ld\n", total );
    }

    output( "\n" );
}

void printDiskStatFreeInfo( const char* cmd )
{
    char *mntpnt = (char*)getMntPnt( cmd );

    if ( strcmp( mntpnt, "/all" ) == 0 ) {
        output( "All Partitions Available\t0\t%ld\tKB\n", getTotal( mntpnt ) );
    } else if ( strcmp( mntpnt, "/__root__" ) == 0 ) {
        output( "Filesystem Root Available\t0\t%ld\tKB\n", getTotal( mntpnt ) );
    } else {
        output( "%s Available\t0\t%ld\tKB\n", mntpnt, getTotal( mntpnt ) );
    }
}

void printDiskStatPercent( const char* cmd )
{
    char *mntpnt = (char*)getMntPnt( cmd );
    DiskInfo* disk_info;

    unsigned long totalSize = 0;
    unsigned long totalAvailable = 0;
    int is_all = strcmp( mntpnt, "/all" ) == 0;

    for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
        if ( !strcmp( mntpnt, disk_info->mntpnt ) || is_all ) {
            unsigned long totalSizeKB =  disk_info->statvfs.f_blocks * (disk_info->statvfs.f_frsize/1024);
            unsigned long available = disk_info->statvfs.f_bavail * (disk_info->statvfs.f_bsize/1024); /* available is only those for non-root.  So available + used != total because some are reserved for root */

            if ( is_all ) {
                totalSize += totalSizeKB;
                totalAvailable += available;
            } else {
                int percentageUsed = calculatePercentageUsed(totalSizeKB, available);
                output( "%d\n", percentageUsed );
            }
        }
    }

    if ( is_all ) {
        int percentageUsed = calculatePercentageUsed(totalSize, totalAvailable);
        output( "%d\n", percentageUsed );
    }

    output( "\n" );
}

void printDiskStatPercentInfo( const char* cmd )
{
    char *mntpnt = (char*)getMntPnt( cmd );
    if ( strcmp( mntpnt, "/all" ) == 0 ) {
        output( "All Partitions\t0\t100\t%%\n" );
    } else if ( strcmp( mntpnt, "/__root__" ) == 0 ) {
        output( "Filesystem Root\t0\t100\t%%\n" );
    } else {
        output( "%s\t0\t100\t%%\n", mntpnt );
    }
}

void printDiskStatTotal( const char* cmd )
{
    char *mntpnt = (char*)getMntPnt( cmd );
    output( "%ld\n\n", getTotal( mntpnt ) );
}

void printDiskStatTotalInfo( const char* cmd )
{
    char *mntpnt = (char*)getMntPnt( cmd );
    if ( strcmp( mntpnt, "/all" ) == 0 ) {
        output("All Partitions Total Size\t0\t0\tKB\n" );
    } else if ( strcmp( mntpnt, "/__root__" ) == 0 ) {
        output( "Filesystem Root Total Size\t0\t0\tKB\n" );
    } else {
        output( "%s Total Size\t0\t0\tKB\n", mntpnt );
    }
}
