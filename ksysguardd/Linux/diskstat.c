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

#include <config.h>

#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <time.h>
#include <unistd.h>

#include "Command.h"
#include "ccont.h"
#include "diskstat.h"
#include "ksysguardd.h"

typedef struct {
  char device[ 256 ];
  char mntpnt[ 256 ];
  long blocks;
  long bfree;
  long bused;
  int bused_percent;
} DiskInfo;

static CONTAINER DiskStatList = 0;
static struct SensorModul* DiskStatSM;
char *getMntPnt( const char* cmd );

char *getMntPnt( const char* cmd )
{
  static char device[ 1025 ];
  char* ptr;

  memset( device, 0, sizeof( device ) );
  sscanf( cmd, "partitions%1024s", device );

  ptr = (char*)rindex( device, '/' );
  *ptr = '\0';

  return (char*)device;
}

/* ----------------------------- public part ------------------------------- */

void initDiskStat( struct SensorModul* sm )
{
  char monitor[ 1024 ];
  DiskInfo* disk_info;

  DiskStatList = new_ctnr();
  DiskStatSM = sm;

  if ( updateDiskStat() < 0 )
    return;

  registerMonitor( "partitions/list", "listview", printDiskStat, printDiskStatInfo, sm );

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    snprintf( monitor, sizeof( monitor ), "partitions%s/usedspace", disk_info->mntpnt );
    registerMonitor( monitor, "integer", printDiskStatUsed, printDiskStatUsedInfo, DiskStatSM );
    snprintf( monitor, sizeof( monitor ), "partitions%s/freespace", disk_info->mntpnt );
    registerMonitor( monitor, "integer", printDiskStatFree, printDiskStatFreeInfo, DiskStatSM );
    snprintf( monitor, sizeof( monitor ), "partitions%s/filllevel", disk_info->mntpnt );
    registerMonitor( monitor, "integer", printDiskStatPercent, printDiskStatPercentInfo, DiskStatSM );
  }
}

void exitDiskStat( void )
{
  char monitor[ 1024 ];
  DiskInfo* disk_info;

  removeMonitor( "partitions/list" );

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    snprintf( monitor, sizeof( monitor ), "partitions%s/usedspace", disk_info->mntpnt );
    removeMonitor( monitor );
    snprintf( monitor, sizeof( monitor ), "partitions%s/freespace", disk_info->mntpnt );
    removeMonitor( monitor );
    snprintf( monitor, sizeof( monitor ), "partitions%s/filllevel", disk_info->mntpnt );
    removeMonitor( monitor );
  }

  destr_ctnr( DiskStatList, free );
}

void checkDiskStat( void )
{
  struct stat mtab_info;
  static off_t mtab_size = 0;

  stat( "/etc/mtab", &mtab_info );
  if ( !mtab_size )
    mtab_size = mtab_info.st_size;

  if ( mtab_info.st_size != mtab_size ) {
    exitDiskStat();
    initDiskStat( DiskStatSM );
    mtab_size = mtab_info.st_size;
  }
}

int updateDiskStat( void )
{
  DiskInfo *disk_info;
  FILE *fh;
  struct mntent *mnt_info;
  float percent;
  int i;
  struct statfs fs_info;

  if ( ( fh = setmntent( "/etc/mtab", "r" ) ) == NULL ) {
    print_error( "Cannot open \'/etc/mtab\'!\n" );
    return -1;
  }

  empty_ctnr(DiskStatList);

  while ( ( mnt_info = getmntent( fh ) ) != NULL ) {
    if ( statfs( mnt_info->mnt_dir, &fs_info ) < 0 )
      continue;

    if ( strcmp( mnt_info->mnt_type, "proc" ) &&
         strcmp( mnt_info->mnt_type, "devfs" ) &&
         strcmp( mnt_info->mnt_type, "usbfs" ) &&
         strcmp( mnt_info->mnt_type, "sysfs" ) &&
         strcmp( mnt_info->mnt_type, "tmpfs" ) &&
         strcmp( mnt_info->mnt_type, "devpts" ) ) {
      if ( fs_info.f_blocks != 0 )
      {
          percent = ( ( (float)fs_info.f_blocks - (float)fs_info.f_bfree ) * 100.0/
                (float)fs_info.f_blocks );
      }
      else
          percent = 0;

      if ( ( disk_info = (DiskInfo *)malloc( sizeof( DiskInfo ) ) ) == NULL )
        continue;

      memset( disk_info, 0, sizeof( DiskInfo ) );
      strlcpy( disk_info->device, mnt_info->mnt_fsname, sizeof( disk_info->device ) );
      if ( !strcmp( mnt_info->mnt_dir, "/" ) )
        strlcpy( disk_info->mntpnt, "/root", sizeof( disk_info->mntpnt ) );
      else
        strlcpy( disk_info->mntpnt, mnt_info->mnt_dir, sizeof( disk_info->mntpnt ) );

      disk_info->blocks = fs_info.f_blocks;
      disk_info->bfree = fs_info.f_bfree;
      disk_info->bused = fs_info.f_blocks - fs_info.f_bfree;
      disk_info->bused_percent = (int)percent;

      push_ctnr( DiskStatList, disk_info );
    }
  }

  endmntent( fh );

  return 0;
}

void printDiskStat( const char* cmd )
{
  DiskInfo* disk_info;

  (void)cmd;
  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    fprintf( CurrentClient, "%s\t%ld\t%ld\t%ld\t%d\t%s\n",
             disk_info->device,
             disk_info->blocks,
             disk_info->bused,
             disk_info->bfree,
             disk_info->bused_percent,
             disk_info->mntpnt );
  }

  fprintf( CurrentClient, "\n" );
}

void printDiskStatInfo( const char* cmd )
{
  (void)cmd;
  fprintf( CurrentClient, "Device\tBlocks\tUsed\tAvailable\tUsed %%\tMountPoint\nM\tD\tD\tD\td\ts\n" );
}

void printDiskStatUsed( const char* cmd )
{
  char *mntpnt = (char*)getMntPnt( cmd );
  DiskInfo* disk_info;

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    if ( !strcmp( mntpnt, disk_info->mntpnt ) )
      fprintf( CurrentClient, "%ld\n", disk_info->bused );
  }

  fprintf( CurrentClient, "\n" );
}

void printDiskStatUsedInfo( const char* cmd )
{
  (void)cmd;
  fprintf( CurrentClient, "Used Blocks\t0\t-\tBlocks\n" );
}

void printDiskStatFree( const char* cmd )
{
  char *mntpnt = (char*)getMntPnt( cmd );
  DiskInfo* disk_info;

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    if ( !strcmp( mntpnt, disk_info->mntpnt ) )
      fprintf( CurrentClient, "%ld\n", disk_info->bfree );
  }

  fprintf( CurrentClient, "\n" );
}

void printDiskStatFreeInfo( const char* cmd )
{
  (void)cmd;
  fprintf( CurrentClient, "Free Blocks\t0\t-\tBlocks\n" );
}

void printDiskStatPercent( const char* cmd )
{
  char *mntpnt = (char*)getMntPnt( cmd );
  DiskInfo* disk_info;

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    if ( !strcmp( mntpnt, disk_info->mntpnt ) )
      fprintf( CurrentClient, "%d\n", disk_info->bused_percent );
  }

  fprintf( CurrentClient, "\n" );
}

void printDiskStatPercentInfo( const char* cmd )
{
  (void)cmd;
  fprintf( CurrentClient, "Used Blocks\t0\t100\t%%\n" );
}
