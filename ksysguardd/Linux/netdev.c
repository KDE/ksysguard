/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

	$Id$
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "Command.h"
#include "netdev.h"

/* Special version of perror for use in signal handler functions. */
#define perror(a) write(STDERR_FILENO, (a), strlen(a))

typedef struct
{
	long OldRxBytes;
	long OldTxBytes;
	long rxBytes;
	long txBytes;
	char name[32];
} NetDevInfo;

/* We have observed deviations of up to 5% in the accuracy of the timer
 * interrupts. So we try to measure the interrupt interval and use this
 * value to calculate timing dependant values. */
static float timeInterval = TIMERINTERVAL;
static struct timeval lastSampling;
static struct timeval currSampling;

#define NETDEVBUFSIZE 4096
static char NetDevBuf[NETDEVBUFSIZE];
static int NetDevCnt = 0;
static int Dirty = 0;
static int NetDevOk = 0;
static long OldHash = 0;

#define MAXNETDEVS 64
static NetDevInfo NetDevs[MAXNETDEVS];

static int
processNetDev_(void)
{
	int i;
	char format[32];
	char devFormat[16];
	char buf[1024];
	char tag[64];
	char* netDevBufP = NetDevBuf;

	sprintf(format, "%%%d[^\n]\n", (int) sizeof(buf) - 1);
	sprintf(devFormat, "%%%ds", (int) sizeof(tag) - 1);

	/* skip 2 first lines */
	for (i = 0; i < 2; i++)

	{
		sscanf(netDevBufP, format, buf);
		buf[sizeof(buf) - 1] = '\0';
		netDevBufP += strlen(buf) + 1;	/* move netDevBufP to next line */
	}
	for (i = 0; sscanf(netDevBufP, format, buf) == 1; ++i)
	{
		buf[sizeof(buf) - 1] = '\0';
		netDevBufP += strlen(buf) + 1;	/* move netDevBufP to next line */

		if (sscanf(buf, devFormat, tag))
		{
			char* pos = strchr(tag, ':');
			if (pos)
			{
				unsigned long rxBytes, txBytes, rxPacks, txPacks;
				*pos = '\0';
				rxBytes = txBytes = rxPacks = txPacks = 0;
				sscanf(buf + 7,
					   "%lu %lu %*d %*d %*d %*d %*d %*d " 
					   "%lu %lu %*d %*d %*d %*d %*d %*d",
					   &rxBytes, &rxPacks, &txBytes, &txPacks);

				if (i >= NetDevCnt || strcmp(NetDevs[i].name, tag) != 0)
				{
					/* The network device configuration has changed. We
					 * need to reconfigure the netdev module. */
					return (-1);
				}
				else
				{
					NetDevs[i].rxBytes = rxBytes - NetDevs[i].OldRxBytes;
					NetDevs[i].txBytes = txBytes - NetDevs[i].OldTxBytes;
					NetDevs[i].OldRxBytes = rxBytes;
					NetDevs[i].OldTxBytes = txBytes;
				}
			}
		}
	}
	if (i != NetDevCnt)
		return (-1);

	/* save exact time inverval between this and the last read of
	 * /proc/stat */
	timeInterval = currSampling.tv_sec + currSampling.tv_usec / 100000.0 -
		lastSampling.tv_sec - lastSampling.tv_usec / 100000.0;
	lastSampling = currSampling;

	Dirty = 0;
	return (0);
}

void
processNetDev(void)
{
	int i;

	if (NetDevCnt == 0)
		return;

	for (i = 0; i < 5 && processNetDev_() < 0; ++i)
		checkNetDev();

	/* If 5 reconfiguration attemts failed, something is very wrong and
	 * we close the netdev module for further use. */
	if (i == 5)
		exitNetDev();
}

/*
================================ public part =================================
*/

void
initNetDev(void)
{
	int i;
	char format[32];
	char devFormat[16];
	char buf[1024];
	char tag[64];
	char* netDevBufP = NetDevBuf;

	if (updateNetDev() < 0)
		return;

	sprintf(format, "%%%d[^\n]\n", (int) sizeof(buf) - 1);
	sprintf(devFormat, "%%%ds", (int) sizeof(tag) - 1);

	/* skip 2 first lines */
	for (i = 0; i < 2; i++)
	{
		sscanf(netDevBufP, format, buf);
		buf[sizeof(buf) - 1] = '\0';
		netDevBufP += strlen(buf) + 1;	/* move netDevBufP to next line */
	}
	for (i = 0; sscanf(netDevBufP, format, buf) == 1; ++i)
	{
		buf[sizeof(buf) - 1] = '\0';
		netDevBufP += strlen(buf) + 1;	/* move netDevBufP to next line */

		if (sscanf(buf, devFormat, tag))
		{
			char* pos = strchr(tag, ':');
			if (pos)
			{
				char mon[128];
				*pos = '\0';
				strcpy(NetDevs[i].name, tag);
				sprintf(mon, "network/%s/recBytes", tag);
				registerMonitor(mon, "integer", printNetDevRecBytes,
								printNetDevRecBytesInfo);
				sprintf(mon, "network/%s/sentBytes", tag);
				registerMonitor(mon, "integer", printNetDevSentBytes,
								printNetDevRecBytesInfo);
				sscanf(pos + 1, "%ld %*d %*d %*d %*d %*d %*d %*d" 
					   "%ld %*d %*d %*d %*d %*d %*d %*d",
					   &NetDevs[i].OldRxBytes, &NetDevs[i].OldTxBytes);
				NetDevCnt++;
			}
			NetDevs[i].rxBytes = NetDevs[i].txBytes = 0;
		}
	}

	/* Call processNetDev to elimitate initial peek values. */
	processNetDev();
}

void
exitNetDev(void)
{
	int i;

	for (i = 0; i < NetDevCnt; ++i)
	{
		char mon[128];
		sprintf(mon, "network/%s/recBytes", NetDevs[i].name);
		removeMonitor(mon);
		sprintf(mon, "network/%s/sentBytes", NetDevs[i].name);
		removeMonitor(mon);
	}
	NetDevCnt = 0;
}

int
updateNetDev(void)
{
	/* ATTENTION: This function is called from a signal handler! Rules for
	 * signal handlers must be obeyed! */

	/* We read the information about the network interfaces from
	   /proc/net/dev. The file should look like this:

Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo:275135772 1437448    0    0    0     0          0         0 275135772 1437448    0    0    0     0       0          0
  eth0:123648812  655251    0    0    0     0          0         0 246847871  889636    0    0    0     0       0          0       Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo:275135772 1437448    0    0    0     0          0         0 275135772 1437448    0    0    0     0       0          0
  eth0:123648812  655251    0    0    0     0          0         0 246847871  889636    0    0    0     0       0          0

	*/

	size_t n;
	int fd;
	long hash;
	char* p;

	if (NetDevOk < 0)
		return(0);

	if ((fd = open("/proc/net/dev", O_RDONLY)) < 0)
	{
		/* /proc/net/dev may not exist on some machines. */
		NetDevOk = -1;
		return (0);
	}
	if ((n = read(fd, NetDevBuf, NETDEVBUFSIZE - 1)) == NETDEVBUFSIZE - 1)
	{
		perror("ERROR: Internal buffer too small to read "
			   "/proc/net/dev!");
		NetDevOk = -1;
		return (-1);
	}
	gettimeofday(&currSampling, 0);
	close(fd);
	NetDevOk = 1;
	NetDevBuf[n] = '\0';

	/* Calculate hash over the first 7 characters of each line starting
	 * after the first newline. */
	for (p = NetDevBuf, hash = 0; *p; ++p)
		if (*p == '\n')
			for (++p; *p && *p != ':' && *p != '|'; ++p)
				hash = ((hash << 6) + *p) % 390389;

	if (OldHash != 0 && OldHash != hash)
	{
		/* TODO: Check whether fwrite() is reentrant! */
		fwrite("RECONFIGURE\n", strlen("RECONFIGURE\n"), 1, currentClient);
		CheckSetupFlag = 1;
	}
	OldHash = hash;

	Dirty = 1;

	return (0);
}

void
checkNetDev(void)
{
	/* Values for other network devices are lost, but it is still better
	 * than not detecting any new devices. TODO: Fix after 2.1 is out. */
	exitNetDev();
	initNetDev();
}

void
printNetDevRecBytes(const char* cmd)
{
	int i;
	char* beg;
	char* end;
	char dev[64];

	beg = strchr(cmd, '/');
	end = strchr(beg + 1, '/');
	strncpy(dev, beg + 1, end - beg - 1);
	dev[end - beg - 1] = '\0';
	if (Dirty)
		processNetDev();
	for (i = 0; i < MAXNETDEVS; ++i)
		if (strcmp(NetDevs[i].name, dev) == 0)
		{
			fprintf(currentClient, "%lu\n", (unsigned long)
				   (NetDevs[i].rxBytes / (1024 * timeInterval)));
			return;
		}

	fprintf(currentClient, "0\n");
}

void
printNetDevRecBytesInfo(const char* cmd)
{
	fprintf(currentClient, "Received Bytes\t0\t0\tkBytes/s\n");
}

void
printNetDevSentBytes(const char* cmd)
{
	int i;
	char* beg;
	char* end;
	char dev[64];

	beg = strchr(cmd, '/');
	end = strchr(beg + 1, '/');
	strncpy(dev, beg + 1, end - beg - 1);
	dev[end - beg - 1] = '\0';
	if (Dirty)
		processNetDev();
	for (i = 0; i < MAXNETDEVS; ++i)
		if (strcmp(NetDevs[i].name, dev) == 0)
		{
			fprintf(currentClient, "%lu\n", (unsigned long)
				   (NetDevs[i].txBytes / (1024 * timeInterval)));
			return;
		}

	fprintf(currentClient, "0\n");
}

void
printNetDevRecSendInfo(const char* cmd)
{
	fprintf(currentClient, "Send Bytes\t0\t0\tkBytes/s\n");
}
