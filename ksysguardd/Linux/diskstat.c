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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "ksysguardd.h"
#include "Command.h"
#include "ccont.h"
#include "diskstat.h"

typedef struct {
	char device[256];
	char mntpnt[256];
	long blocks;
	long bfree;
	long bused;
	int bused_percent;
} DiskInfo;

static CONTAINER DiskStatList = 0;
static time_t DiskStat_timeStamp = 0;

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

void initDiskStat(void)
{
	DiskStatList = new_ctnr(CT_DLL);

	registerMonitor("partitions/list", "listview", printDiskStat, printDiskStatInfo);
	updateDiskStat();
}

void exitDiskStat(void)
{
	if (DiskStatList)
		destr_ctnr(DiskStatList, free);
}

int updateDiskStat(void)
{
	FILE *mounts;
	char line[1024], mntpnt[256], device[256];
	char monitor[300];
	struct statfs fs_info;
	struct stat mtab_info;
	static off_t mtab_size = 0;
	float percent;
	int i;
	DiskInfo *disk_info;
	
	if ((mounts = fopen("/etc/mtab", "r")) == NULL) {
		print_error("Cannot open \'/etc/mtab\'!\n");
		return -1;
	} else {
		stat("/etc/mtab", &mtab_info);
	}

	for (i = 0; i < level_ctnr(DiskStatList); i++) {
		disk_info = remove_ctnr(DiskStatList, i--);

		// was there a change in mtab
		if (mtab_info.st_size != mtab_size) {
			snprintf(monitor, sizeof(monitor), "partitions%s/usedspace", disk_info->mntpnt);
			removeMonitor(monitor);
			snprintf(monitor, sizeof(monitor), "partitions%s/freespace", disk_info->mntpnt);
			removeMonitor(monitor);
			snprintf(monitor, sizeof(monitor), "partitions%s/filllevel", disk_info->mntpnt);
			removeMonitor(monitor);
		}

		free(disk_info);
	}

	while (fgets(line, sizeof(line), mounts) != NULL) {
		memset(mntpnt, 0, sizeof(mntpnt));
		memset(device, 0, sizeof(device));
		memset(monitor, 0, sizeof(monitor));;
		sscanf(line, "%255s %255s", device, mntpnt);
		if (statfs(mntpnt, &fs_info) < 0)
			continue;

		if ((strncmp(mntpnt, "/proc", 5)) && (strncmp(mntpnt, "/dev/", 5))) {
			percent = (((float)fs_info.f_blocks - (float)fs_info.f_bfree)/(float)fs_info.f_blocks);
			percent = percent * 100;
			if ((disk_info = (DiskInfo *)malloc(sizeof(DiskInfo))) == NULL) {
				continue;
			}
			memset(disk_info, 0, sizeof(DiskInfo));
			strncpy(disk_info->device, device, 255);
			if (!strcmp(mntpnt, "/")) {
				strncpy(disk_info->mntpnt, "/root", 6);
			} else {
				strncpy(disk_info->mntpnt, mntpnt, 255);
			}
			disk_info->blocks = fs_info.f_blocks;
			disk_info->bfree = fs_info.f_bfree;
			disk_info->bused = (fs_info.f_blocks - fs_info.f_bfree);
			disk_info->bused_percent = (int)percent;

			push_ctnr(DiskStatList, disk_info);

			if (mtab_info.st_size != mtab_size) {
				snprintf(monitor, sizeof(monitor), "partitions%s/usedspace", disk_info->mntpnt);
				registerMonitor(monitor, "integer", printDiskStatUsed, printDiskStatUsedInfo);
				snprintf(monitor, sizeof(monitor), "partitions%s/freespace", disk_info->mntpnt);
				registerMonitor(monitor, "integer", printDiskStatFree, printDiskStatFreeInfo);
				snprintf(monitor, sizeof(monitor), "partitions%s/filllevel", disk_info->mntpnt);
				registerMonitor(monitor, "integer", printDiskStatPercent, printDiskStatPercentInfo);
			}
		}
	}
	
	fclose(mounts);
	DiskStat_timeStamp = time(0);
	mtab_size = mtab_info.st_size;
	
	return 0;
}

void printDiskStat(const char* cmd)
{
	int i;

	if ((time(0) - DiskStat_timeStamp) > 2)
		updateDiskStat();

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
	fprintf(CurrentClient, "Device\tBlocks\tUsed\tAvailable\tUsed %%\tMountPoint\n");
}

void printDiskStatUsed(const char* cmd)
{
	int i;
	char *mntpnt = (char *)getMntPnt(cmd);

	if ((time(0) - DiskStat_timeStamp) > 2)
		updateDiskStat();

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

	if ((time(0) - DiskStat_timeStamp) > 2)
		updateDiskStat();

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

	if ((time(0) - DiskStat_timeStamp) > 2)
		updateDiskStat();

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
