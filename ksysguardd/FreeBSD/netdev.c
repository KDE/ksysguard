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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>

#include <net/if.h>
#include <net/if_mib.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "Command.h"
#include "ksysguardd.h"
#include "netdev.h"


typedef struct {
	char name[32];
	u_long recBytes;
	u_long recPacks;
	u_long recErrs;
	u_long recDrop;
	u_long recMulticast;
	u_long sentBytes;
	u_long sentPacks;
	u_long sentErrs;
	u_long sentMulticast;
	u_long sentColls;
} NetDevInfo;

#define MAXNETDEVS 64
static NetDevInfo NetDevs[MAXNETDEVS];
static NetDevInfo NetDevsOld[MAXNETDEVS];
static int NetDevCnt = 0;
static struct SensorModul* NetDevSM;

static float elapsed = 0.0;
static struct timeval old_tv;

char **parseCommand(const char *cmd)
{
	char *tmp_cmd = strdup(cmd);
	char *begin;
	char **retval = malloc(sizeof(char *)*2);

	begin = rindex(tmp_cmd, '/');
	*begin = '\0';
	begin++;
	retval[1] = strdup((const char *)begin); /* sensor */

	begin = rindex(tmp_cmd, '/');
	*begin = '\0';
	begin = rindex(tmp_cmd, '/');
	begin++;
	retval[0] = strdup((const char *)begin); /* interface  */
	free(tmp_cmd);

	return retval;
}

int numActivIfaces(void)
{
	int counter = 0;
	int name[6];
	int num_iface, i;
	size_t len;
	struct ifmibdata ifmd;

	len = sizeof(num_iface);
	sysctlbyname("net.link.generic.system.ifcount", &num_iface, &len, NULL, 0);

	for (i = 1; i < num_iface + 1; i++) {
		name[0] = CTL_NET;
		name[1] = PF_LINK;
		name[2] = NETLINK_GENERIC;
		name[3] = IFMIB_IFDATA;
		name[4] = i;
		name[5] = IFDATA_GENERAL;

		len = sizeof(ifmd);
		sysctl(name, 6, &ifmd, &len, NULL, 0);
		if (ifmd.ifmd_flags & IFF_UP)
			counter++;
	}

	return counter;
}

/* ------------------------------ public part --------------------------- */

void initNetDev(struct SensorModul* sm)
{
	int i;
	char monitor[1024];
	gettimeofday(&old_tv, (struct timezone *)0);

	NetDevSM = sm;

	updateNetDev();
	
	for (i = 0; i < NetDevCnt; i++) {
		/* init data */
		NetDevsOld[i] = NetDevs[i];

		/* register monitors */
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/data", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/packets", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/errors", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/drops", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/multicast", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, NetDevSM);

		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/data", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/packets", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/errors", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/multicast", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, NetDevSM);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/collisions", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, NetDevSM);
	}
}

void exitNetDev(void)
{
	int i;
	char monitor[1024];

	for (i = 0; i < NetDevCnt; i++) {
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/data", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/packets", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/errors", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/drops", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/receiver/multicast", NetDevs[i].name);
		removeMonitor(monitor);

		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/data", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/packets", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/errors", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/multicast", NetDevs[i].name);
		removeMonitor(monitor);
		snprintf(monitor, sizeof(monitor), "network/interfaces/%s/transmitter/collisions", NetDevs[i].name);
		removeMonitor(monitor);
	}
}

int updateNetDev(void)
{
	int name[6];
	int num_iface, i;
	size_t len;
	struct ifmibdata ifmd;
	struct timeval new_tv, tv;

	len = sizeof(num_iface);
	sysctlbyname("net.link.generic.system.ifcount", &num_iface, &len, NULL, 0);

	NetDevCnt = 0;
	for (i = 1; i < num_iface + 1; i++) {
		name[0] = CTL_NET;
		name[1] = PF_LINK;
		name[2] = NETLINK_GENERIC;
		name[3] = IFMIB_IFDATA;
		name[4] = i;
		name[5] = IFDATA_GENERAL;

		len = sizeof(ifmd);
		sysctl(name, 6, &ifmd, &len, NULL, 0);
		if (ifmd.ifmd_flags & IFF_UP) {
			NetDevsOld[NetDevCnt] = NetDevs[NetDevCnt];

			strlcpy(NetDevs[NetDevCnt].name, ifmd.ifmd_name, sizeof(NetDevs[NetDevCnt].name));
			NetDevs[NetDevCnt].recBytes = ifmd.ifmd_data.ifi_ibytes;
			NetDevs[NetDevCnt].recPacks = ifmd.ifmd_data.ifi_ipackets;
			NetDevs[NetDevCnt].recErrs = ifmd.ifmd_data.ifi_ierrors;
			NetDevs[NetDevCnt].recDrop = ifmd.ifmd_data.ifi_iqdrops;
			NetDevs[NetDevCnt].recMulticast = ifmd.ifmd_data.ifi_imcasts;
			NetDevs[NetDevCnt].sentBytes = ifmd.ifmd_data.ifi_obytes;
			NetDevs[NetDevCnt].sentPacks = ifmd.ifmd_data.ifi_opackets;
			NetDevs[NetDevCnt].sentErrs = ifmd.ifmd_data.ifi_oerrors;
			NetDevs[NetDevCnt].sentMulticast = ifmd.ifmd_data.ifi_omcasts;
			NetDevs[NetDevCnt].sentColls = ifmd.ifmd_data.ifi_collisions;
			NetDevCnt++;
		}
	}

	gettimeofday(&new_tv, (struct timezone *)0);
	timersub(&new_tv, &old_tv, &tv);
	elapsed = tv.tv_sec + (tv.tv_usec * 1e-6);
	old_tv = new_tv;

	return 0;
}

void checkNetDev(void)
{
	if (numActivIfaces() != NetDevCnt) {
		/* interface has been added or removed
		   so we do a reset */
		exitNetDev();
		initNetDev(NetDevSM);
	}
}

void printNetDevRecBytes(const char *cmd)
{
	int i;
	char **retval;
	
	retval = parseCommand(cmd);
	
	if (retval == NULL)
		return;

	for (i = 0; i < NetDevCnt; i++) {
		if (!strcmp(NetDevs[i].name, retval[0])) {
			if (!strncmp(retval[1], "data", 4))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].recBytes - NetDevsOld[i].recBytes) / (1024 * elapsed)));
			if (!strncmp(retval[1], "packets", 7))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].recPacks - NetDevsOld[i].recPacks) / elapsed));
			if (!strncmp(retval[1], "errors", 6))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].recErrs - NetDevsOld[i].recErrs) / elapsed));
			if (!strncmp(retval[1], "drops", 5))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].recDrop - NetDevsOld[i].recDrop) / elapsed));
			if (!strncmp(retval[1], "multicast", 9))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].recMulticast - NetDevsOld[i].recMulticast) / elapsed));
		}
	}
	free(retval[0]);
	free(retval[1]);
	free(retval);

	fprintf(CurrentClient, "\n");
}

void printNetDevRecBytesInfo(const char *cmd)
{
	char **retval;
	
	retval = parseCommand(cmd);
	
	if (retval == NULL)
		return;

	if (!strncmp(retval[1], "data", 4))
		fprintf(CurrentClient, "Received Data\t0\t0\tkBytes/s\n");
	if (!strncmp(retval[1], "packets", 7))
		fprintf(CurrentClient, "Received Packets\t0\t0\t1/s\n");
	if (!strncmp(retval[1], "errors", 6))
		fprintf(CurrentClient, "Receiver Errors\t0\t0\t1/s\n");
	if (!strncmp(retval[1], "drops", 5))
		fprintf(CurrentClient, "Receiver Drops\t0\t0\t1/s\n");
	if (!strncmp(retval[1], "multicast", 9))
		fprintf(CurrentClient, "Received Multicast Packets\t0\t0\t1/s\n");
	
	free(retval[0]);
	free(retval[1]);
	free(retval);
}

void printNetDevSentBytes(const char *cmd)
{
	int i;
	char **retval;
	
	retval = parseCommand(cmd);
	
	if (retval == NULL)
		return;

	for (i = 0; i < NetDevCnt; i++) {
		if (!strcmp(NetDevs[i].name, retval[0])) {
			if (!strncmp(retval[1], "data", 4))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].sentBytes - NetDevsOld[i].sentBytes) / (1024 * elapsed)));
			if (!strncmp(retval[1], "packets", 7))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].sentPacks - NetDevsOld[i].sentPacks) / elapsed));
			if (!strncmp(retval[1], "errors", 6))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].sentErrs - NetDevsOld[i].sentErrs) / elapsed));
			if (!strncmp(retval[1], "multicast", 9))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].sentMulticast - NetDevsOld[i].sentMulticast) / elapsed));
			if (!strncmp(retval[1], "collisions", 10))
				fprintf(CurrentClient, "%lu", (u_long)((NetDevs[i].sentColls - NetDevsOld[i].sentColls) / elapsed));
		}
	}
	free(retval[0]);
	free(retval[1]);
	free(retval);

	fprintf(CurrentClient, "\n");
}

void printNetDevSentBytesInfo(const char *cmd)
{
	char **retval;
	
	retval = parseCommand(cmd);
	
	if (retval == NULL)
		return;

	if (!strncmp(retval[1], "data", 4))
		fprintf(CurrentClient, "Sent Data\t0\t0\tkBytes/s\n");
	if (!strncmp(retval[1], "packets", 7))
		fprintf(CurrentClient, "Sent Packets\t0\t0\t1/s\n");
	if (!strncmp(retval[1], "errors", 6))
		fprintf(CurrentClient, "Transmitter Errors\t0\t0\t1/s\n");
	if (!strncmp(retval[1], "multicast", 9))
		fprintf(CurrentClient, "Sent Multicast Packets\t0\t0\t1/s\n");
	if (!strncmp(retval[1], "collisions", 10))
		fprintf(CurrentClient, "Transmitter Collisions\t0\t0\t1/s\n");

	free(retval[0]);
	free(retval[1]);
	free(retval);
}
