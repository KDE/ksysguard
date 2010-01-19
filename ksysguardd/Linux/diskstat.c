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

#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/vfs.h>
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
  long blocks;
  long bfree;
  long bused;
  int bused_percent;
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

  ptr = (char*)rindex( device, '/' );
  *ptr = '\0';

  return (char*)device;
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
}
static void removeMonitors(const char* mntpnt) {
    snprintf( monitor, sizeof( monitor ), "partitions%s/usedspace", mntpnt );
    removeMonitor( monitor );
    snprintf( monitor, sizeof( monitor ), "partitions%s/freespace", mntpnt );
    removeMonitor( monitor );
    snprintf( monitor, sizeof( monitor ), "partitions%s/filllevel", mntpnt );
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

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    registerMonitors(disk_info->mntpnt);
  }
}

void exitDiskStat( void )
{
  DiskInfo* disk_info;

  removeMonitor( "partitions/list" );

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
  float percent;
  struct statfs fs_info;

  if ( ( fh = setmntent( "/etc/mtab", "r" ) ) == NULL ) {
    print_error( "Cannot open \'/etc/mtab\'!\n" );
    return -1;
  }
  if(OldDiskStatList == 0) {
    OldDiskStatList = DiskStatList;
    DiskStatList = new_ctnr();
  }
  else	  
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
      strncpy( disk_info->device, mnt_info->mnt_fsname, sizeof( disk_info->device ) );
      disk_info->device[ sizeof(disk_info->device) -1] = 0;
      
      if ( !strcmp( mnt_info->mnt_dir, "/" ) )
        strncpy( disk_info->mntpnt, "/root", sizeof( disk_info->mntpnt ) );
      else
        strncpy( disk_info->mntpnt, mnt_info->mnt_dir, sizeof( disk_info->mntpnt ) );
      disk_info->mntpnt[ sizeof(disk_info->mntpnt) - 1] = 0;
      sanitize(disk_info->mntpnt);

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
    output( "%s\t%ld\t%ld\t%ld\t%d\t%s\n",
             disk_info->device,
             disk_info->blocks,
             disk_info->bused,
             disk_info->bfree,
             disk_info->bused_percent,
             disk_info->mntpnt );
  }

  output( "\n" );
}

void printDiskStatInfo( const char* cmd )
{
  (void)cmd;
  output( "Device\tBlocks\tUsed\tAvailable\tUsed %%\tMount point\nM\tD\tD\tD\td\ts\n" );
}

void printDiskStatUsed( const char* cmd )
{
  char *mntpnt = (char*)getMntPnt( cmd );
  DiskInfo* disk_info;

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    if ( !strcmp( mntpnt, disk_info->mntpnt ) )
      output( "%ld\n", disk_info->bused );
  }

  output( "\n" );
}

void printDiskStatUsedInfo( const char* cmd )
{
  (void)cmd;
  output( "Used Blocks\t0\t0\tBlocks\n" );
}

void printDiskStatFree( const char* cmd )
{
  char *mntpnt = (char*)getMntPnt( cmd );
  DiskInfo* disk_info;

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    if ( !strcmp( mntpnt, disk_info->mntpnt ) )
      output( "%ld\n", disk_info->bfree );
  }

  output( "\n" );
}

void printDiskStatFreeInfo( const char* cmd )
{
  (void)cmd;
  output( "Free Blocks\t0\t0\tBlocks\n" );
}

void printDiskStatPercent( const char* cmd )
{
  char *mntpnt = (char*)getMntPnt( cmd );
  DiskInfo* disk_info;

  for ( disk_info = first_ctnr( DiskStatList ); disk_info; disk_info = next_ctnr( DiskStatList ) ) {
    if ( !strcmp( mntpnt, disk_info->mntpnt ) )
      output( "%d\n", disk_info->bused_percent );
  }

  output( "\n" );
}

void printDiskStatPercentInfo( const char* cmd )
{
  (void)cmd;
  output( "Used Blocks\t0\t100\t%%\n" );
}
