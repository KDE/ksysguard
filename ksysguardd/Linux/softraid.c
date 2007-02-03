/*
	KSysGuard, the KDE System Guard
	
	Copyright (c) 2007 Greg Martyn <greg.martyn@gmail.com>, John Tapsell <tapsell@kde.org>
	
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

#include "Command.h"
#include "ksysguardd.h"
#include "softraid.h"

#include <string.h> /* for strlen, strcat and strcmp */
#include <stdio.h> /* for sprintf */
#include <sys/types.h> /* for open */
#include <sys/stat.h> /* for open */
#include <fcntl.h> /* for open */
#include <unistd.h> /* for read, close, exec, fork */
#include <stdlib.h> /* for exit */
#include <sys/wait.h> /* for wait :) */
#include <stdbool.h> /* for bool */
#include "ccont.h" /* for CONTAINER */

#define MDSTATBUFSIZE (1 * 1024)
#define MDADMSTATBUFSIZE (2 * 1024)
#define ARRAYNAMELEN 32
#define ARRAYNAMELENSTRING "32"

static struct SensorModul* StatSM;

static CONTAINER ArrayInfos = 0;
char mdstatBuf[ MDSTATBUFSIZE ];	/* Buffer for /proc/mdstat */

typedef struct Disks {
	char *name;			/* e.g.  hda1 */
	char status;			/* e.g.  F  for failed, S for spare.  U for fully synced, _ for syncing */
	int index;			/* number in the array */
	struct Disks *next;			/* A pointer to the next disk in this array. NULL if no next */
} Disks;

typedef struct {
	bool Alive;


	/* from /sbin/mdadm --detail /dev/ArrayName */
	bool ArraySizeIsAlive;
	bool ArraySizeIsRegistered;
	int ArraySizeKB;

	bool UsedDeviceSizeIsAlive;
	bool UsedDeviceSizeIsRegistered;
	int UsedDeviceSizeKB;

	bool PreferredMinorIsAlive;
	bool PreferredMinorIsRegistered;
	int PreferredMinor;
	
	/* from /proc/mdstat */
	char ArrayName[ ARRAYNAMELEN +1];
	int NumBlocks;

	int NumRaidDevices;		/* Number of 'useful' disks.  Included working and spare disks, but not failed. */
	
	int TotalDevices;		/* Total number of devices, including failed and spare disks */
	
	int ActiveDevices;		/* Active means it's fully synced, and working, so not a spare or failed */
	
	int WorkingDevices;		/* Working means it's in the raid (not a spare or failed) but it may or may not be fully synced.  Active - Working  = number being synced */
	
	int FailedDevices;		/* Number of failed devices */
	
	int SpareDevices;		/* Number of spare devices,  WE DO NOT Include Failed devices here, unlike mdadm. */

        int  devnum; 			/* Raid array number.  e.g. if ArrayName is "md0", then devnum=0 */
        bool ArrayActive; 		/* Whether this raid is active */
        char *level;  			/* Raid1, Raid2, etc */
        char *pattern;			/* U or up, _ for down */
        int  ResyncingPercent; 		/* -1 if not resyncing, otherwise between 0 to 100 */
        bool IsCurrentlyReSyncing; 	/* True if currently resyncing - */
	Disks *first_disk;		/* A linked list of hard disks */
} ArrayInfo;

static int ArrayInfoEqual( void* s1, void* s2 )
{
	/* Returns 0 if both ArrayInfos have the same name.  */
	return strncmp( ((ArrayInfo*)s1)->ArrayName, ((ArrayInfo*)s2)->ArrayName,ARRAYNAMELEN );
}

void printArrayAttribute( const char* cmd ) {
	INDEX idx;
	ArrayInfo key;
	ArrayInfo* foundArray;
	char attribute[40];

	if ( sscanf(cmd, "SoftRaid/%[^/]/%39s", key.ArrayName, attribute) == 2 ) {
		if ( ( idx = search_ctnr( ArrayInfos, ArrayInfoEqual, &key ) ) == 0 ) {
			foundArray = get_ctnr( ArrayInfos, idx );

			if ( strcmp( attribute, "NumBlocks" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->NumBlocks );
			else if ( strcmp( attribute, "ArraySizeKB" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->ArraySizeKB );
			else if ( strcmp( attribute, "UsedDeviceSizeKB" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->UsedDeviceSizeKB );
			else if ( strcmp( attribute, "NumRaidDevices" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->NumRaidDevices );
			else if ( strcmp( attribute, "TotalDevices" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->TotalDevices );
			else if ( strcmp( attribute, "PreferredMinor" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->PreferredMinor );
			else if ( strcmp( attribute, "ActiveDevices" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->ActiveDevices );
			else if ( strcmp( attribute, "WorkingDevices" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->WorkingDevices );
			else if ( strcmp( attribute, "FailedDevices" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->FailedDevices );
			else if ( strcmp( attribute, "SpareDevices" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->SpareDevices );
			else if( strcmp( attribute, "DeviceNumber" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->devnum);
			else if( strcmp( attribute, "ResyncingPercent" ) == 0 )
				fprintf( CurrentClient, "%d\n", foundArray->ResyncingPercent);
			else if( strcmp( attribute, "RaidType" ) == 0 )
				fprintf( CurrentClient, "%s\n", foundArray->level);
			else if( strcmp( attribute, "DiskInfo" ) == 0 ) {
				Disks *disk = foundArray->first_disk;
				while(disk) {
					fprintf( CurrentClient, "%s\t%d\t%c\n", disk->name, disk->index, disk->status);
					disk = disk->next;
				}
				fprintf( CurrentClient, "\n");
			}
		}
		else {
			fprintf( CurrentClient, "\n");
		}
	} else {
		fprintf( CurrentClient, "\n");
	}


}

void printArrayAttributeInfo( const char* cmd ) {
	INDEX idx;
	ArrayInfo key;
	ArrayInfo* foundArray;
	char attribute[40];

	if ( sscanf(cmd, "SoftRaid/%[^/]/%39s", key.ArrayName, attribute) == 2 ) {
		if ( ( idx = search_ctnr( ArrayInfos, ArrayInfoEqual, &key ) ) == 0 ) {
			foundArray = get_ctnr( ArrayInfos, idx );

			if ( strcmp( attribute, "NumBlocks?" ) == 0 )
				fprintf( CurrentClient, "Num blocks\t0\t0\t\n" );
			else if ( strcmp( attribute, "ArraySizeKB?" ) == 0 )
				fprintf( CurrentClient, "Array size\t0\t0\tKB\n" );
			else if ( strcmp( attribute, "UsedDeviceSizeKB?" ) == 0 )
				fprintf( CurrentClient, "Used Device Size\t0\t0\tKB\n" );
			else if ( strcmp( attribute, "NumRaidDevices?" ) == 0 )
				fprintf( CurrentClient, "Total number of raid devices\t0\t0\t\n" );
			else if ( strcmp( attribute, "TotalDevices?" ) == 0 )
				fprintf( CurrentClient, "Total number of devices\t0\t0\t\n" );
			else if ( strcmp( attribute, "PreferredMinor?" ) == 0 )
				fprintf( CurrentClient, "The preferred minor\t0\t0\t\n" );
			else if ( strcmp( attribute, "ActiveDevices?" ) == 0 )
				fprintf( CurrentClient, "Number of active devices\t0\t%d\t\n", foundArray->TotalDevices );
			else if ( strcmp( attribute, "WorkingDevices?" ) == 0 )
				fprintf( CurrentClient, "Number of working devices\t0\t%d\t\n", foundArray->TotalDevices );
			else if ( strcmp( attribute, "FailedDevices?" ) == 0 )
				fprintf( CurrentClient, "Number of failed devices\t0\t%d\t\n", foundArray->TotalDevices );
			else if ( strcmp( attribute, "SpareDevices?" ) == 0 )
				fprintf( CurrentClient, "Number of spare devices\t0\t%d\t\n", foundArray->TotalDevices );
			else if( strcmp( attribute, "DeviceNumber?" ) == 0 )
				fprintf( CurrentClient, "Raid Device Number\t0\t0\t\n");
			else if( strcmp( attribute, "ResyncingPercent?" ) == 0 )
				fprintf( CurrentClient, "Resyncing Percentage Done. -1 if not resyncing\t-1\t100\t%%\n");
			else if( strcmp( attribute, "RaidType?" ) == 0 )
				fprintf( CurrentClient, "Type of RAID array\n");
			else if( strcmp( attribute, "DiskInfo?") == 0 )
				fprintf( CurrentClient, "Disk Name\tIndex\tStatus\ns\td\tS\n");
		}
		else {
			fprintf( CurrentClient, "\n");
		}
	} else {
		fprintf( CurrentClient, "\n");
	}
}



void getMdadmDetail( ArrayInfo* MyArray ) {
	int fd[2];
	pid_t ChildPID;
	int nbytes;
	
	char sensorName[128];
	char arrayDevice[ARRAYNAMELEN + 5];
	char format[ 32 ];
	char lineBuf[ 1024 ];
	char mdadmStatBuf[ MDADMSTATBUFSIZE ];	/* Buffer for mdadm --detail */
	char* mdadmStatBufP;

	/* Create a pipe */
	pipe(fd);

	/* Fork */
	if((ChildPID = fork()) == -1)
	{
		perror("Couldn't fork to launch mdadm.");
		exit(1);
	}

	/* Child will execute the program, parent will listen. */

	if (ChildPID == 0) {
	/* Child process */

		/* Child will execute the program, parent will listen. */
		/* Close stdout, duplicate the input side of pipe to stdout */
		dup2(fd[1], 1);
		/* Close output side of pipe */
		close(fd[0]);
		close(2);

		snprintf( arrayDevice, sizeof( arrayDevice ), "/dev/%s", MyArray->ArrayName );
		execl ("/sbin/mdadm", "mdadm", "--detail", arrayDevice, (char *)0);
		exit(0); /* In case /sbin/mdadm isn't found */
		/* Child is now dead, as per our request */
	}
	
	/* Parent process */
	
	/* Close input side of pipe */
	close(fd[1]);

	waitpid( ChildPID, 0, 0);
	
	/* Fill mdadmStatBuf with pipe's output */
	nbytes = read( fd[0], mdadmStatBuf, MDADMSTATBUFSIZE-1 );
	mdadmStatBuf[nbytes] = '\0';

	/* Now, go through mdadmStatBuf line by line. Register monitors along the way */
	sprintf( format, "%%%d[^\n]\n", (int)sizeof( lineBuf ) - 1 );
	mdadmStatBufP = mdadmStatBuf;
	while (sscanf(mdadmStatBufP, format, lineBuf) != EOF) {
		lineBuf[sizeof(lineBuf) - 1] = '\0';
		mdadmStatBufP += strlen(lineBuf) + 1;  /* move mdadmStatBufP to next line */
		
		if ( sscanf(lineBuf, "  Array Size : %d", &MyArray->ArraySizeKB) == 1 ) {
			MyArray->ArraySizeIsAlive = true;
			if ( !MyArray->ArraySizeIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/ArraySizeKB", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

				MyArray->ArraySizeIsRegistered = true;
			}
		}

		/* Versions of mdadm prior to 2.6 used "Device Size" instead of "Used Dev Size"
		 * Also, note how the if statement takes advantage of short-circuit logic.
		 */
		else if ( ( sscanf(lineBuf, " Device Size : %d", &MyArray->UsedDeviceSizeKB) == 1 ) ||
			( sscanf(lineBuf, " Used Dev Size : %d", &MyArray->UsedDeviceSizeKB) == 1 ) ) {
			MyArray->UsedDeviceSizeIsAlive = true;
			if ( !MyArray->UsedDeviceSizeIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/UsedDeviceSizeKB", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->UsedDeviceSizeIsRegistered = true;
			}
		}

/*		else if ( sscanf(lineBuf, "   Raid Devices : %d", &MyArray->NumRaidDevices) == 1 ) {
			MyArray->NumRaidDevicesIsAlive = true;
			if ( !MyArray->NumRaidDevicesIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/NumRaidDevices", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->NumRaidDevicesIsRegistered = true;
			}
		}

		else if ( sscanf(lineBuf, "  Total Devices : %d", &MyArray->TotalDevices) == 1 ) {
			MyArray->TotalDevicesIsAlive = true;
			if ( !MyArray->TotalDevicesIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/TotalDevices", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->TotalDevicesIsRegistered = true;
			}
		}
*/
		else if ( sscanf(lineBuf, "Preferred Minor : %d", &MyArray->PreferredMinor) == 1 ) {
			MyArray->PreferredMinorIsAlive = true;
			if ( !MyArray->PreferredMinorIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/PreferredMinor", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->PreferredMinorIsRegistered = true;
			}
		}
/*
		else if ( sscanf(lineBuf, " Active Devices : %d", &MyArray->ActiveDevices) == 1 ) {
			MyArray->ActiveDevicesIsAlive = true;
			if ( !MyArray->ActiveDevicesIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/ActiveDevices", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->ActiveDevicesIsRegistered = true;
			}
		}

		else if ( sscanf(lineBuf, "Working Devices : %d", &MyArray->WorkingDevices) == 1 ) {
			MyArray->WorkingDevicesIsAlive = true;
			if ( !MyArray->WorkingDevicesIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/WorkingDevices", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->WorkingDevicesIsRegistered = true;
			}
		}

		else if ( sscanf(lineBuf, " Failed Devices : %d", &MyArray->FailedDevices) == 1 ) {
			MyArray->FailedDevicesIsAlive = true;
			if ( !MyArray->FailedDevicesIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/FailedDevices", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->FailedDevicesIsRegistered = true;
			}
		}

		else if ( sscanf(lineBuf, " Spare Devices : %d", &MyArray->SpareDevices) == 1 ) {
			MyArray->SpareDevicesIsAlive = true;
			if ( !MyArray->SpareDevicesIsRegistered ) {
				sprintf(sensorName, "SoftRaid/%s/SpareDevices", MyArray->ArrayName);
				registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
				MyArray->SpareDevicesIsRegistered = true;
			}
		}
*/
	}

	/* Note: Don't test NumBlocksIsAlive, because it hasn't been set yet */
	if (    (!MyArray->ArraySizeIsAlive      && MyArray->ArraySizeIsRegistered      ) ||
		(!MyArray->UsedDeviceSizeIsAlive && MyArray->UsedDeviceSizeIsRegistered ) ||
		(!MyArray->PreferredMinorIsAlive && MyArray->PreferredMinorIsRegistered ) 
		) {
		print_error( "RECONFIGURE" );
		log_error( "Soft raid device disappeared" );
		return;
	}
}

void openMdstatFile() {
	size_t n;
	int fd;

	mdstatBuf[ 0 ] = '\0';
	if ( ( fd = open( "/proc/mdstat", O_RDONLY ) ) < 0 )
		return; /* couldn't open /proc/mdstat */
	
	n = read( fd, mdstatBuf, MDSTATBUFSIZE - 1 );
	close( fd );

	if ( n == MDSTATBUFSIZE - 1 || n <= 0 ) {
		log_error( "Internal buffer too small to read \'/proc/mdstat\'" );

		return;
	}
	
	mdstatBuf[ n ] = '\0';
}

ArrayInfo *getOrCreateArrayInfo(char *array_name, int array_name_length) {
	ArrayInfo key;
	INDEX idx;
	ArrayInfo* MyArray;
	/*We have the array name.  see if we already have a record for it*/
	strncpy(key.ArrayName, array_name, array_name_length);
	key.ArrayName[array_name_length]='\0';
	if ( ( idx = search_ctnr( ArrayInfos, ArrayInfoEqual, &key ) ) == 0 ) {
		/* Found an existing array device */
		MyArray = get_ctnr( ArrayInfos, idx );
	}
	else {
		/* Found a new array device. Create a data structure for it. */
		MyArray = calloc(1,sizeof (ArrayInfo));
		if (MyArray == NULL) {
			/* Memory could not be allocated, so print an error and exit. */
			fprintf(stderr, "Couldn't allocate memory\n");
			exit(EXIT_FAILURE);
		}
		strcpy( MyArray->ArrayName, key.ArrayName );
		/* Add this array to our list of array devices */
		push_ctnr(ArrayInfos, MyArray);
		char sensorName[128];
		sprintf(sensorName, "SoftRaid/%s/NumBlocks", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/TotalDevices", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/FailedDevices", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/SpareDevices", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/NumRaidDevices", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/WorkingDevices", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/ActiveDevices", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/RaidType", MyArray->ArrayName);
		registerMonitor(sensorName, "string", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/DeviceNumber", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );

		sprintf(sensorName, "SoftRaid/%s/ResyncingPercent", MyArray->ArrayName);
		registerMonitor(sensorName, "integer", printArrayAttribute, printArrayAttributeInfo, StatSM );
		
		sprintf(sensorName, "SoftRaid/%s/DiskInfo", MyArray->ArrayName);
		registerMonitor(sensorName, "listview", printArrayAttribute, printArrayAttributeInfo, StatSM );

	}
	return MyArray;
}

bool scanForArrays() {
	char* mdstatBufP;
	char* current_word;
	int current_word_length = 0;

	ArrayInfo* MyArray;

	/* Mark all data as dead. As we find data, we'll mark it alive. */
	for ( MyArray = first_ctnr( ArrayInfos ); MyArray; MyArray = next_ctnr( ArrayInfos ) ) {
		MyArray->Alive = false;
	}
	MyArray = NULL;

	openMdstatFile();

	current_word = mdstatBufP = mdstatBuf;

	/* Process values from /proc/mdstat */

	bool start = true;
	while(mdstatBufP[0] != '\0') {
		
		if(!start) {
			/*advanced one line  at a time*/
			while( mdstatBufP[0] != '\0' && mdstatBufP[0] != '\n') mdstatBufP++;
			mdstatBufP++;
		}
		start = false;
		
		if( mdstatBufP[0] == '\0') break;
		current_word_length = 0;
		current_word = mdstatBufP;
		
		
		if(mdstatBufP[0]=='\n')
			continue;

		if(mdstatBufP[0]=='#') /*skip comments */
			continue;
		if (strncmp(current_word, "Personalities", sizeof("Personalities")-1)==0)
			continue;
		if (strncmp(current_word, "read_ahead", sizeof("read_ahead")-1)==0)
			continue;
		if (strncmp(current_word, "unused", sizeof("unused")-1)==0)
			continue;
		/* Better be an md line.. */
		if (strncmp(current_word, "md", 2)!= 0)
			continue;
		int devnum;
		if (strncmp(current_word, "md_d", 4) == 0)
			devnum = -1-strtoul(current_word+4, NULL, 10);
		else  /* it's md0 etc */
			devnum = strtoul(current_word+2, NULL, 10);		
		while(  mdstatBufP[0] != '\0' &&mdstatBufP[0] != '\n' && mdstatBufP[0] != ' ' &&  mdstatBufP[0] != '\t') {
			/*find the end of the word*/
			mdstatBufP++; 
			current_word_length++;
		}
		
		MyArray = getOrCreateArrayInfo(current_word, current_word_length);
		MyArray->Alive = true;
		getMdadmDetail ( MyArray );
		MyArray->level = MyArray->pattern= NULL;
		MyArray->ResyncingPercent = -1;
		MyArray->IsCurrentlyReSyncing = false;
		MyArray->devnum = devnum;
		MyArray->ArrayActive = false;
		MyArray->TotalDevices = MyArray->SpareDevices = MyArray->FailedDevices = 0;
		MyArray->NumBlocks = 0;
		
		Disks *disk = MyArray->first_disk;
		MyArray->first_disk = NULL;
		Disks *next;
		while(disk) {
			next =	disk->next;
			free(disk);
			disk = next;
		}

		
		/* In mdstat, we have something that looks like:

md0 : active raid1 sda1[0] sdb1[1]
      312568576 blocks [2/2] [UU]
md1 : active raid1 sda2[0] sdb2[1]
      452568576 blocks [2/2] [UU]
		
		We have so far read in the "md0" bit, and now want to continue reading the details for this raid group until
		we reach the next raid group which we note as starting with a non whitespace.
		*/
		char buffer[100];
		char status[100];
		status[0] = 0;
		int harddisk_index;
		for(;;) {
			
			while(  mdstatBufP[0] != '\0' && ( (mdstatBufP[0] == '\n' && (mdstatBufP[1] == ' ' ||  mdstatBufP[1] == '\t')) || mdstatBufP[0] == ' ' ||  mdstatBufP[0] == '\t')) {				
				mdstatBufP++;   /*Remove any whitespace first*/
			}
			if( mdstatBufP[0] == '\0' || mdstatBufP[0] == '\n') {
				break; /*we are now at the end of the file or line.  Break this for loop*/
			}
			
			current_word=mdstatBufP;  /*we are now pointing to the first non-whitespace of a word*/
			current_word_length=0;
			while(  mdstatBufP[0] != '\0' && mdstatBufP[0] != '\n' && mdstatBufP[0] != ' ' &&  mdstatBufP[0] != '\t') {
				/*find the end of the word.  We do this now so that we know the length of the word*/
				mdstatBufP++; 
				current_word_length++;
			}

			char *eq;
			int in_devs = 0;
			int temp_int =0;
			
			
			if (strncmp(current_word, "active", sizeof("active")-1)==0)
				MyArray->ArrayActive = true;
			else if (strncmp(current_word, "inactive", sizeof("inactive")-1)==0)
				MyArray->ArrayActive = false;
			else if (MyArray->ArrayActive >=0 && MyArray->level == NULL && current_word[0] != '(' && current_word[0] != ':' /*readonly*/) {
				MyArray->level = strndup(current_word, current_word_length);
				in_devs = 1;

			} else if (sscanf(current_word, "%d blocks ", &temp_int) == 1 ) {
				MyArray->NumBlocks = temp_int; /* We have to do it via a temp_int variable otherwise we'll end up with nonsence if it's not found */
			} else if (in_devs && strncmp(current_word, "blocks", sizeof("blocks")-1)==0)
				in_devs = 0;
			else if (in_devs && strncmp(current_word, "md", 2)==0) {
				/* This has an md device as a component.  Maybe we should note this or something*/
			} else if(sscanf(current_word, "%[^[ ][%d]%[^ ]", buffer, &harddisk_index, status) >= 2) {
				/*Each device in the raid has an index.  We can find the total number of devices in the raid by 
				  simply finding the device with the highest index + 1. */
				if(harddisk_index >= MyArray->TotalDevices)  MyArray->TotalDevices = harddisk_index+1;
				Disks *new_disk = malloc(sizeof(Disks));
				new_disk->name = strdup(buffer);
				new_disk->index = harddisk_index;


				if(status[0] == '(') {
					new_disk->status = status[1];
					if(status[1] == 'S') /*Spare hard disk*/
						MyArray->SpareDevices++;
					else if(status[1] == 'F')
						MyArray->FailedDevices++;
				} else
					new_disk->status = 'U';

				/* insert the new disk into the linked list of disks*/
				new_disk->next = MyArray->first_disk;
				MyArray->first_disk = new_disk;

				MyArray->NumRaidDevices = MyArray->TotalDevices - MyArray->FailedDevices;
				status[0]=0; /*make sure we zero it again for next time*/
			} else if (!MyArray->pattern &&
				 current_word[0] == '[' &&
				 (current_word[1] == 'U' || current_word[1] == '_')) {
				MyArray->pattern = strndup(current_word+1, current_word_length-1);
				
				if (MyArray->pattern[current_word_length-2]==']')
					MyArray->pattern[current_word_length-2] = '\0';
				MyArray->ActiveDevices = MyArray->TotalDevices - MyArray->SpareDevices - MyArray->FailedDevices;

				MyArray->WorkingDevices=0;
				int index=0;
				for(;;) {
					current_word++;

					if(current_word[0] == 'U')
						MyArray->WorkingDevices++;
					else if(current_word[0] == '_') {
						Disks *disk = MyArray->first_disk;
						while(disk) {
							if(disk->index == index) {
								if(disk->status == 'U')
									disk->status = '_'; /* The disk hasn't failed, but is syncing */
								break;
							}
							disk = disk->next;
						}
					} else 
						break;
					index++;
				}				
			} else if (MyArray->ResyncingPercent == -1 &&
				   strncmp(current_word, "re", 2)== 0 &&
				   current_word[current_word_length-1] == '%' &&
				   (eq=strchr(current_word, '=')) != NULL ) {
				MyArray->ResyncingPercent = atoi(eq+1);
				if (strncmp(current_word,"resync", 4)==0)
					MyArray->IsCurrentlyReSyncing = true;
			} else if (MyArray->ResyncingPercent == -1 &&
				   strncmp(current_word, "resync", 4)==0) {
				MyArray->IsCurrentlyReSyncing = true;
			} else if (MyArray->ResyncingPercent == -1 &&
				   current_word[0] >= '0' && 
				   current_word[0] <= '9' &&
				   current_word[current_word_length-1] == '%') {
				MyArray->ResyncingPercent = atoi(current_word);
			}
			/*ignore anything not understood*/
		}
	}
	
	/* Look for dead arrays, and for NumBlocksIsRegistered */
	for ( MyArray = first_ctnr( ArrayInfos ); MyArray; MyArray = next_ctnr( ArrayInfos ) ) {
		if ( MyArray->Alive == false ) {
			print_error( "RECONFIGURE" );
			
			log_error( "Soft raid device disappeared" );
			return false;
		}
	}
	return true;
}

/* =========== Public part =========== */

void initSoftRaid( struct SensorModul* sm ) {
  	StatSM = sm;

	ArrayInfos = new_ctnr();
	updateSoftRaid();
}

void exitSoftRaid( void ) {
	destr_ctnr( ArrayInfos, free );
}

int updateSoftRaid( void ) {
	scanForArrays();
	return 0;
}
