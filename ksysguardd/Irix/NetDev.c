/*
    KSysGuard, the KDE System Guard

	Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>
	Irix Support by Carsten Kroll <ckroll@pinnaclesys.com>

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

#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/soioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <invent.h>
#include <strings.h>


#include "Command.h"
#include "ksysguardd.h"
#include "NetDev.h"

#ifdef __GNUC__
#define LONGLONG long long
#endif

typedef struct {
	char name[IFNAMSIZ];
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

#define MAXNETDEVS 32
static NetDevInfo  NetDevs[MAXNETDEVS];
static NetDevInfo oNetDevs[MAXNETDEVS];
static int NetDevCnt = 0;

char **parseCommand(const char *cmd)
{
	char *tmp_cmd = strdup(cmd);
	char *begin;
	char *retval = malloc(sizeof(char *)*2);

	begin = rindex(tmp_cmd, '/');
	*begin = '\0';
	begin++;
	retval[1] = strdup(begin); // sensor

	begin = rindex(tmp_cmd, '/');
	*begin = '\0';
	begin = rindex(tmp_cmd, '/');
	begin++;
	retval[0] = strdup(begin); // interface
	free(tmp_cmd);

	return retval;
}

/* ------------------------------ public part --------------------------- */

void initNetDev(struct SensorModul* sm)
{
	int i;
	char monitor[1024];

	memset(NetDevs,0,sizeof(NetDevInfo)*MAXNETDEVS);
	memset(oNetDevs,0,sizeof(NetDevInfo)*MAXNETDEVS);

	updateNetDev();

	for (i = 0; i < NetDevCnt; i++) {

		sprintf(monitor,"network/interfaces/%s/receiver/packets", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, sm);
		sprintf(monitor ,"network/interfaces/%s/receiver/errors", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, sm);
		/*
		[CK] I don't know how to get Bytes sent/received, if someone does please drop me a note.
		sprintf(monitor,"network/interfaces/%s/receiver/data", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, sm);
		sprintf(monitor,"network/interfaces/%s/receiver/drops", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, sm);
		sprintf(monitor ,"network/interfaces/%s/receiver/multicast", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevRecBytes, printNetDevRecBytesInfo, sm);
		*/

		sprintf(monitor,"network/interfaces/%s/transmitter/packets", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, sm);
		sprintf(monitor,"network/interfaces/%s/transmitter/errors", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, sm);
		/*
		sprintf(monitor,"network/interfaces/%s/transmitter/data", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, sm);
		sprintf(monitor,"network/interfaces/%s/transmitter/multicast", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, sm);
		*/
		sprintf(monitor,"network/interfaces/%s/transmitter/collisions", NetDevs[i].name);
		registerMonitor(monitor, "integer", printNetDevSentBytes, printNetDevSentBytesInfo, sm);
	}
}

void exitNetDev(void)
{
	int i;
	char monitor[1024];

	for (i = 0; i < NetDevCnt; i++) {
		sprintf(monitor,"network/interfaces/%s/receiver/packets", NetDevs[i].name);
		removeMonitor(monitor);
		sprintf(monitor,"network/interfaces/%s/receiver/errors", NetDevs[i].name);
		removeMonitor(monitor);
/*
		sprintf(monitor,"network/interfaces/%s/receiver/drops", NetDevs[i].name);
		removeMonitor(monitor);
		sprintf(monitor,"network/interfaces/%s/receiver/multicast", NetDevs[i].name);
		removeMonitor(monitor);
		sprintf(monitor,"network/interfaces/%s/receiver/data", NetDevs[i].name);
		removeMonitor(monitor);
*/

		sprintf(monitor,"network/interfaces/%s/transmitter/packets", NetDevs[i].name);
		removeMonitor(monitor);
		sprintf(monitor,"network/interfaces/%s/transmitter/errors", NetDevs[i].name);
		removeMonitor(monitor);
/*
		sprintf(monitor,"network/interfaces/%s/transmitter/data", NetDevs[i].name);
		removeMonitor(monitor);
		sprintf(monitor,"network/interfaces/%s/transmitter/multicast", NetDevs[i].name);
		removeMonitor(monitor);
*/
		sprintf(monitor,"network/interfaces/%s/transmitter/collisions", NetDevs[i].name);
		removeMonitor(monitor);

	}
}

int updateNetDev(void)
{
	int name[6];
	int num_iface=0, i;
	char buf[MAXNETDEVS*sizeof(struct ifreq)];
	size_t len;
	int s;
	struct ifconf ifc;
	struct ifstats *istat;
	struct timeval tv;
	static LONGLONG timestamp=0;
	register LONGLONG cts,elapsed;
	//struct ipstat ips;

	if ((s=socket(PF_INET,SOCK_DGRAM,0)) < 0){
		print_error("socket creation failed");
		return(-1);
	}

	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
		print_error("cannot get interface configuration");
		return(-1);
	}

	gettimeofday(&tv, 0);
	cts = ((LONGLONG)tv.tv_sec * 100 + (LONGLONG) tv.tv_usec / 10000);/* in 10 ms unit*/
	elapsed = cts - timestamp;
	timestamp=cts;

	NetDevCnt=0;

	for (i = 0; i < MAXNETDEVS; i++) {
		if ( *ifc.ifc_req[i].ifr_name == 0) break;
		if (ioctl(s, SIOCGIFSTATS, &ifc.ifc_req[i]) < 0) {
			print_error("cannot get interface statistics");
			return (-1);
		}
		istat=&ifc.ifc_req[i].ifr_stats;
		//if ( ifc.ifc_req[i].ifr_flags & IFF_UP) {
			strncpy(NetDevs[i].name,ifc.ifc_req[i].ifr_name, IFNAMSIZ);
                        NetDevs[i].name[IFNAMSIZ-1]='\0';
			NetDevs[i].recBytes = (istat->ifs_ipackets - oNetDevs[i].recBytes) * 100 / elapsed;
			NetDevs[i].recPacks = (istat->ifs_ipackets - oNetDevs[i].recPacks) * 100 / elapsed;
			NetDevs[i].recErrs = istat->ifs_ierrors - oNetDevs[i].recErrs;
			//NetDevs[i].recDrop = istat - oNetDevs[i].recDrop;
			//NetDevs[i].recMulticast = istat - oNetDevs[i].recMulticast;
			NetDevs[i].sentBytes = istat->ifs_opackets - oNetDevs[i].sentBytes;
			NetDevs[i].sentPacks = (istat->ifs_opackets - oNetDevs[i].sentPacks) * 100 / elapsed;
			NetDevs[i].sentErrs  = (istat->ifs_oerrors - oNetDevs[i].sentErrs) * 100 / elapsed;
			//NetDevs[i].sentMulticast = istat - NetDevs[i].sentMulticast;
			NetDevs[i].sentColls = (istat->ifs_collisions - oNetDevs[i].sentColls) *100/elapsed;
			/* save it for the next round */
			oNetDevs[i].recBytes = istat->ifs_ipackets;
			oNetDevs[i].recPacks = istat->ifs_ipackets;
			oNetDevs[i].recErrs  = istat->ifs_ierrors;
			//oNetDevs[i].recDrop =
			//oNetDevs[i].recMulticast =
			oNetDevs[i].sentBytes = istat->ifs_opackets;
			oNetDevs[i].sentPacks = istat->ifs_opackets;
			oNetDevs[i].sentErrs  = istat->ifs_oerrors;
			//oNetDevs[i].sentMulticast =
			oNetDevs[i].sentColls = istat->ifs_collisions;
		//}
		NetDevCnt++;
	}
	close(s);
	return (0);
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
				fprintf(CurrentClient, "%lu", NetDevs[i].recBytes);
			if (!strncmp(retval[1], "packets", 7))
				fprintf(CurrentClient, "%lu", NetDevs[i].recPacks);
			if (!strncmp(retval[1], "errors", 6))
				fprintf(CurrentClient, "%lu", NetDevs[i].recErrs);
			if (!strncmp(retval[1], "drops", 5))
				fprintf(CurrentClient, "%lu", NetDevs[i].recDrop);
			if (!strncmp(retval[1], "multicast", 9))
				fprintf(CurrentClient, "%lu", NetDevs[i].recMulticast);
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
				fprintf(CurrentClient, "%lu", NetDevs[i].sentBytes);
			if (!strncmp(retval[1], "packets", 7))
				fprintf(CurrentClient, "%lu", NetDevs[i].sentPacks);
			if (!strncmp(retval[1], "errors", 6))
				fprintf(CurrentClient, "%lu", NetDevs[i].sentErrs);
			if (!strncmp(retval[1], "multicast", 9))
				fprintf(CurrentClient, "%lu", NetDevs[i].sentMulticast);
			if (!strncmp(retval[1], "collisions", 10))
				fprintf(CurrentClient, "%lu", NetDevs[i].sentColls);
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
