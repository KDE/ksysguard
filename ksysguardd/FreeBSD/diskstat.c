/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ucred.h>
#include <sys/mount.h>
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
    long bsize;
    long files;
    long ffree;
    long fused;
    int fused_percent;
} DiskInfo;

#define BLK2KB(disk_info, prop)                        \
    (disk_info->prop * (disk_info->bsize / 1024))

#define MNTPNT_NAME(disk_info)                        \
    (strcmp(disk_info->mntpnt, "/root") ? disk_info->mntpnt : "/")

static CONTAINER DiskStatList = 0;
static struct SensorModul* DiskStatSM;

char *getMntPnt(const char *cmd)
{
    static char device[1025];
    char *ptr;

    memset(device, 0, sizeof(device));
    sscanf(cmd, "partitions%1024s", device);

    ptr = (char *)rindex(device, '/');
    *ptr = '\0';

    return (char *)device;
}

int numMntPnt(void)
{
    struct statfs *fs_info;
    int i, n, counter = 0;

    n = getmntinfo(&fs_info, MNT_WAIT);
    for (i = 0; i < n; i++)
        if (strcmp(fs_info[i].f_fstypename, "procfs") && strcmp(fs_info[i].f_fstypename, "swap") && strcmp(fs_info[i].f_fstypename, "devfs"))
            counter++;

    return counter;
}

/* ------------------------------ public part --------------------------- */

void initDiskStat(struct SensorModul* sm)
{
    char monitor[1024];
    DiskInfo* disk_info;

    DiskStatList = new_ctnr();
    DiskStatSM = sm;

    updateDiskStat();

    registerMonitor("partitions/list", "listview", printDiskStat, printDiskStatInfo, DiskStatSM);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        snprintf(monitor, sizeof(monitor), "partitions%s/usedspace", disk_info->mntpnt);
        registerMonitor(monitor, "integer", printDiskStatUsed, printDiskStatUsedInfo, DiskStatSM);
        snprintf(monitor, sizeof(monitor), "partitions%s/freespace", disk_info->mntpnt);
        registerMonitor(monitor, "integer", printDiskStatFree, printDiskStatFreeInfo, DiskStatSM);
        snprintf(monitor, sizeof(monitor), "partitions%s/filllevel", disk_info->mntpnt);
        registerMonitor(monitor, "integer", printDiskStatPercent, printDiskStatPercentInfo, DiskStatSM);
        snprintf(monitor, sizeof(monitor), "partitions%s/usedinode", disk_info->mntpnt);
        registerMonitor(monitor, "integer", printDiskStatIUsed, printDiskStatIUsedInfo, DiskStatSM);
        snprintf(monitor, sizeof(monitor), "partitions%s/freeinode", disk_info->mntpnt);
        registerMonitor(monitor, "integer", printDiskStatIFree, printDiskStatIFreeInfo, DiskStatSM);
        snprintf(monitor, sizeof(monitor), "partitions%s/inodelevel", disk_info->mntpnt);
        registerMonitor(monitor, "integer", printDiskStatIPercent, printDiskStatIPercentInfo, DiskStatSM);
    }
}

void checkDiskStat(void)
{
    if (numMntPnt() != level_ctnr(DiskStatList)) {
        /* a file system was mounted or unmounted
           so we do a reset */
        exitDiskStat();
        initDiskStat(DiskStatSM);
    }
}

void exitDiskStat(void)
{
    DiskInfo *disk_info;
    char monitor[1024];

    removeMonitor("partitions/list");

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        snprintf(monitor, sizeof(monitor), "partitions%s/usedspace", disk_info->mntpnt);
        removeMonitor(monitor);
        snprintf(monitor, sizeof(monitor), "partitions%s/freespace", disk_info->mntpnt);
        removeMonitor(monitor);
        snprintf(monitor, sizeof(monitor), "partitions%s/filllevel", disk_info->mntpnt);
        removeMonitor(monitor);
        snprintf(monitor, sizeof(monitor), "partitions%s/usedinode", disk_info->mntpnt);
        removeMonitor(monitor);
        snprintf(monitor, sizeof(monitor), "partitions%s/freeinode", disk_info->mntpnt);
        removeMonitor(monitor);
        snprintf(monitor, sizeof(monitor), "partitions%s/inodelevel", disk_info->mntpnt);
        removeMonitor(monitor);
    }

    destr_ctnr(DiskStatList, free);
}

int updateDiskStat(void)
{
    struct statfs *fs_info;
    struct statfs fs;
    float percent, fpercent;
    int i, mntcount;
    DiskInfo *disk_info;

    /* let's hope there is no difference between the DiskStatList and
       the number of mounted file systems */
    empty_ctnr(DiskStatList);

    mntcount = getmntinfo(&fs_info, MNT_WAIT);

    for (i = 0; i < mntcount; i++) {
        fs = fs_info[i];
        if (strcmp(fs.f_fstypename, "procfs") && strcmp(fs.f_fstypename, "devfs") && strcmp(fs.f_fstypename, "linprocfs")) {
            if ( fs.f_blocks != 0 )
                percent = (((float)fs.f_blocks - (float)fs.f_bfree)*100.0/(float)fs.f_blocks);
            else
                percent = 0;
            if (fs.f_files != 0)
                fpercent = (((float)fs.f_files - (float)fs.f_ffree)*100.0/(float)fs.f_files);
            else
                fpercent = 0;
            if ((disk_info = (DiskInfo *)malloc(sizeof(DiskInfo))) == NULL)
                continue;
            memset(disk_info, 0, sizeof(DiskInfo));
            strlcpy(disk_info->device, fs.f_mntfromname, sizeof(disk_info->device));
            if (!strcmp(fs.f_mntonname, "/"))
                strncpy(disk_info->mntpnt, "/root", 6);
            else
                strlcpy(disk_info->mntpnt, fs.f_mntonname, sizeof(disk_info->mntpnt));
            disk_info->blocks = fs.f_blocks;
            disk_info->bfree = fs.f_bfree;
            disk_info->bused = (fs.f_blocks - fs.f_bfree);
            disk_info->bused_percent = (int)percent;
            disk_info->bsize = fs.f_bsize;
            push_ctnr(DiskStatList, disk_info);
            disk_info->files = fs.f_files;
            disk_info->ffree = fs.f_ffree;
            disk_info->fused = fs.f_files - fs.f_ffree;
            disk_info->fused_percent = (int)fpercent;
        }
    }

    return 0;
}

void printDiskStat(const char* cmd)
{
    DiskInfo* disk_info;

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        fprintf(CurrentClient, "%s\t%ld\t%ld\t%ld\t%d\t%ld\t%ld\t%ld\t%d\t%s\n",
            disk_info->device,
            BLK2KB(disk_info, blocks),
            BLK2KB(disk_info, bused),
            BLK2KB(disk_info, bfree),
            disk_info->bused_percent,
            disk_info->files,
            disk_info->fused,
            disk_info->ffree,
            disk_info->fused_percent,
            MNTPNT_NAME(disk_info));
    }

    fprintf(CurrentClient, "\n");
}

void printDiskStatInfo(const char* cmd)
{
    fprintf(CurrentClient, "Device\tCapacity\tUsed\tAvailable\tUsed %%\tInodes\tUsed Inodes\tFree Inodes\tInodes %%\tMountPoint\nM\tKB\tKB\tKB\td\td\td\td\td\ts\n");
}

void printDiskStatUsed(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "%ld\n", BLK2KB(disk_info, bused));
            return;
        }
    }

    fprintf(CurrentClient, "\n");
}

void printDiskStatUsedInfo(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "Used Space (%s)\t0\t%ld\tKB\n", MNTPNT_NAME(disk_info), BLK2KB(disk_info, blocks));
            return;
        }
    }
    fprintf(CurrentClient, "Used Space (%s)\t0\t-\tKB\n", mntpnt);
}

void printDiskStatFree(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "%ld\n", BLK2KB(disk_info, bfree));
            return;
        }
    }

    fprintf(CurrentClient, "\n");
}

void printDiskStatFreeInfo(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "Free Space (%s)\t0\t%ld\tKB\n", MNTPNT_NAME(disk_info), BLK2KB(disk_info, blocks));
            return;
        }
    }
    fprintf(CurrentClient, "Free Space (%s)\t0\t-\tKB\n", mntpnt);
}

void printDiskStatPercent(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "%d\n", disk_info->bused_percent);
            return;
        }
    }

    fprintf(CurrentClient, "\n");
}

void printDiskStatPercentInfo(const char* cmd)
{
    char *mntpnt = (char *)getMntPnt(cmd);

    fprintf(CurrentClient, "Used Space (%s)\t0\t100\t%%\n", mntpnt);
}

void printDiskStatIUsed(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "%ld\n", disk_info->fused);
            return;
        }
    }

    fprintf(CurrentClient, "\n");
}

void printDiskStatIUsedInfo(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "Used Inodes (%s)\t0\t%ld\tKB\n", MNTPNT_NAME(disk_info), disk_info->files);
            return;
        }
    }
    fprintf(CurrentClient, "Used Inodes(%s)\t0\t-\tKB\n", mntpnt);
}

void printDiskStatIFree(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "%ld\n", disk_info->ffree);
            return;
        }
    }

    fprintf(CurrentClient, "\n");
}

void printDiskStatIFreeInfo(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "Free Inodes (%s)\t0\t%ld\tKB\n", MNTPNT_NAME(disk_info), disk_info->files);
            return;
        }
    }
    fprintf(CurrentClient, "Free Inodes (%s)\t0\t-\tKB\n", mntpnt);
}

void printDiskStatIPercent(const char* cmd)
{
    DiskInfo* disk_info;
    char *mntpnt = (char *)getMntPnt(cmd);

    for (disk_info = first_ctnr(DiskStatList); disk_info; disk_info = next_ctnr(DiskStatList)) {
        if (!strcmp(mntpnt, disk_info->mntpnt)) {
            fprintf(CurrentClient, "%d\n", disk_info->fused_percent);
            return;
        }
    }

    fprintf(CurrentClient, "\n");
}

void printDiskStatIPercentInfo(const char* cmd)
{
    char *mntpnt = (char *)getMntPnt(cmd);

    fprintf(CurrentClient, "Used Inodes (%s)\t0\t100\t%%\n", mntpnt);
}

