/*
    KSysGuard, the KDE System Guard
	   
    Copyright (c) 2001 Tobias Koenig <tokoe82@yahoo.de>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

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
	char device[256];
	char mntpnt[256];
	long blocks;
	long bfree;
	long bused;
	int bused_percent;
} DiskInfo;

static CONTAINER DiskStatList = 0;

char *getMntPnt(const char *cmd)
{
	static char device[1024];
	char *ptr;

	memset(device, 0, sizeof(device));
	sscanf(cmd, "partitions%1024s", device);

	ptr = (char *)rindex(device, '/');
	*ptr = '\0';
		
	return (char *)device;
}

/* ----------------------------- public part ------------------------------- */

void initDiskStat(void)
{
	char monitor[1024];
	int i;
	
	DiskStatList = new_ctnr(CT_DLL);

	if (updateDiskStat() < 0)
		return;

	registerMonitor("partitions/list", "listview", printDiskStat, printDiskStatInfo);

	for (i = 0; i < level_ctnr(DiskStatList); i++) {
		DiskInfo* disk_info = get_ctnr(DiskStatList, i);
		
		snprintf(monitor, sizeof(monitor), "partitions%s/usedspace", disk_info->mntpnt);
		registerMonitor(monitor, "integer", printDiskStatUsed, printDiskStatUsedInfo);
		snprintf(monitor, sizeof(monitor), "partitions%s/freespace", disk_info->mntpnt);
		registerMonitor(monitor, "integer", printDiskStatFree, printDiskStatFreeInfo);
		snprintf(monitor, sizeof(monitor), "partitions%s/filllevel", disk_info->mntpnt);
		registerMonitor(monitor, "integer", printDiskStatPercent, printDiskStatPercentInfo);
	}
}

void exitDiskStat(void)
{
	char monitor[1024];
	int i;

	removeMonitor("partitions/list");

	for (i = 0; i < level_ctnr(DiskStatList); i++) {
		DiskInfo* disk_info = get_ctnr(DiskStatList, i);
		
		snprintf(monitor, sizeof(monitor), "partitions%s/usedspace", disk_info->mntpnt);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "partitions%s/freespace", disk_info->mntpnt);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "partitions%s/filllevel", disk_info->mntpnt);
		removeMonitor(monitor);
	}

	if (DiskStatList)
		destr_ctnr(DiskStatList, free);
}

void checkDiskStat(void)
{
	struct stat mtab_info;
	static off_t mtab_size = 0;

	stat("/etc/mtab", &mtab_info);

	if (mtab_info.st_size != mtab_size) {
		exitDiskStat();
		initDiskStat();
		mtab_size = mtab_info.st_size;
	}
}

int updateDiskStat(void)
{
	DiskInfo *disk_info;
	FILE *fh;
	struct mntent *mnt_info;
	float percent;
	int i;
	struct statfs fs_info;
	
	if ((fh = setmntent("/etc/mtab", "r")) == NULL) {
		print_error("Cannot open \'/etc/mtab\'!\n");
		return -1;
	}

	for (i = 0; i < level_ctnr(DiskStatList); i++)
		free(remove_ctnr(DiskStatList, i--));

	while ((mnt_info = getmntent(fh)) != NULL) {
		if (statfs(mnt_info->mnt_dir, &fs_info) < 0)
			continue;

		if (strcmp(mnt_info->mnt_type, "proc") && strcmp(mnt_info->mnt_type, "devfs") && strcmp(mnt_info->mnt_type, "devpts")) {
			percent = (((float)fs_info.f_blocks - (float)fs_info.f_bfree)/(float)fs_info.f_blocks);
			percent = percent * 100;
			if ((disk_info = (DiskInfo *)malloc(sizeof(DiskInfo))) == NULL)
				continue;

			memset(disk_info, 0, sizeof(DiskInfo));
			strncpy(disk_info->device, mnt_info->mnt_fsname, 255);
			if (!strcmp(mnt_info->mnt_dir, "/")) {
				strcpy(disk_info->mntpnt, "/root");
			} else {
				strncpy(disk_info->mntpnt, mnt_info->mnt_dir, 255);
			}
			disk_info->blocks = fs_info.f_blocks;
			disk_info->bfree = fs_info.f_bfree;
			disk_info->bused = (fs_info.f_blocks - fs_info.f_bfree);
			disk_info->bused_percent = (int)percent;

			push_ctnr(DiskStatList, disk_info);
		}
	}

	endmntent(fh);

	return 0;
}

void printDiskStat(const char* cmd)
{
	int i;

	for (i = 0; i < level_ctnr(DiskStatList); i++) {
		DiskInfo* disk_info = get_ctnr(DiskStatList, i);
		fprintf(CurrentClient, "%s\t%ld\t%ld\t%ld\t%d\t%s\n",
			disk_info->device,
			disk_info->blocks,
			disk_info->bused,
			disk_info->bfree,
			disk_info->bused_percent,
			disk_info->mntpnt);
	}

	fprintf(CurrentClient, "\n");
}

void printDiskStatInfo(const char* cmd)
{
	fprintf(CurrentClient, "Device\tBlocks\tUsed\tAvailable\tUsed %%\tMountPoint\nM\td\td\td\td\ts\n");
}

void printDiskStatUsed(const char* cmd)
{
	int i;
	char *mntpnt = (char *)getMntPnt(cmd);

	for (i = 0; i < level_ctnr(DiskStatList); i++) {
		DiskInfo* disk_info = get_ctnr(DiskStatList, i);
		if (!strcmp(mntpnt, disk_info->mntpnt)) {
			fprintf(CurrentClient, "%ld\n", disk_info->bused);
		}
	}

	fprintf(CurrentClient, "\n");
}

void printDiskStatUsedInfo(const char* cmd)
{
	fprintf(CurrentClient, "Used Blocks\t0\t-\tBlocks\n");
}

void printDiskStatFree(const char* cmd)
{
	int i;
	char *mntpnt = (char *)getMntPnt(cmd);

	for (i = 0; i < level_ctnr(DiskStatList); i++) {
		DiskInfo* disk_info = get_ctnr(DiskStatList, i);
		if (!strcmp(mntpnt, disk_info->mntpnt)) {
			fprintf(CurrentClient, "%ld\n", disk_info->bfree);
		}
	}

	fprintf(CurrentClient, "\n");
}

void printDiskStatFreeInfo(const char* cmd)
{
	fprintf(CurrentClient, "Free Blocks\t0\t-\tBlocks\n");
}

void printDiskStatPercent(const char* cmd)
{
	int i;
	char *mntpnt = (char *)getMntPnt(cmd);

	for (i = 0; i < level_ctnr(DiskStatList); i++) {
		DiskInfo* disk_info = get_ctnr(DiskStatList, i);
		if (!strcmp(mntpnt, disk_info->mntpnt)) {
			fprintf(CurrentClient, "%d\n", disk_info->bused_percent);
		}
	}

	fprintf(CurrentClient, "\n");
}

void printDiskStatPercentInfo(const char* cmd)
{
	fprintf(CurrentClient, "Used Blocks\t0\t100\t%%\n");
}
