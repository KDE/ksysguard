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

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ksysguardd.h"
#include "Command.h"
#include "ccont.h"
#include "netstat.h"

static CONTAINER TcpSocketList = 0;
static CONTAINER UdpSocketList = 0;
static CONTAINER UnixSocketList = 0;
static CONTAINER RawSocketList = 0;

static int num_tcp = 0;
static int num_udp = 0;
static int num_unix = 0;
static int num_raw = 0;

typedef struct {
	char local_addr[128];
	char local_port[128];
	char remote_addr[128];
	char remote_port[128];
	char state[128];
	int uid;
} SocketInfo;

typedef struct {
	int refcount;
	char type[128];
	char state[128];
	int inode;
	char path[256];
} UnixInfo;

char *get_serv_name(int port, const char *proto);
char *get_host_name(int addr);
char *get_proto_name(int number);
int get_num_sockets(FILE *netstat);
void printSocketInfo(SocketInfo* socket_info);

static time_t TcpUdpRaw_timeStamp = 0;
static time_t Unix_timeStamp = 0;
static time_t NetStat_timeStamp = 0;

static const char *raw_type[] =
{
	"",
	"stream",
	"dgram",
	"raw",
	"rdm",
	"seqpacket",
	"packet"
};

static const char *raw_state[] =
{
	"free",
	"unconnected",
	"connecting",
	"connected",
	"disconnecting"
};

static const char *conn_state[] =
{
	"",
	"established",
	"syn_sent",
	"syn_recv",
	"fin_wait1",
	"fin_wait2",
	"time_wait",
	"close",
	"close_wait",
	"last_ack",
	"listen",
	"closing"
};

char *get_serv_name(int port, const char *proto)
{
	static char buffer[1024];
	struct servent *service;

	if (port == 0) {
		return (char *)"*";
	}

	memset(buffer, 0, sizeof(buffer));
	
	if ((service = getservbyport(ntohs(port), proto)) == NULL) {
		snprintf(buffer, sizeof(buffer), "%d", port);
	} else {
		strlcpy(buffer, service->s_name, sizeof(buffer));
	}

	return (char *)buffer;
}

char *get_host_name(int addr)
{
	static char buffer[1024];
	struct hostent *host;
	struct in_addr a_addr;

	if (addr == 0) {
		return (char *)"*";
	}

	memset(buffer, 0, sizeof(buffer));

	if ((host = gethostbyaddr((char *)&addr, 4, AF_INET)) == NULL) {
		a_addr.s_addr = addr;
		return inet_ntoa(a_addr);
	} else {
		strlcpy(buffer, host->h_name, sizeof(buffer));
		return (char *)buffer;
	}
}

char *get_proto_name(int number)
{
	static char buffer[1024];
	struct protoent *protocol;

	if (number == 0) {
		return (char *)"*";
	}

	memset(buffer, 0, sizeof(buffer));

	if ((protocol = getprotobynumber(number)) == NULL) {
		snprintf(buffer, sizeof(buffer), "%d", number);
	} else {
		strlcpy(buffer, protocol->p_name, sizeof(buffer));
	}

	return (char *)buffer;
}

int get_num_sockets(FILE *netstat)
{
	char line[1024];
	int line_count = 0;
	
	while (fgets(line, 1024, netstat) != NULL)
		line_count++;

	return line_count - 1;
}

void printSocketInfo(SocketInfo* socket_info)
{
	fprintf(CurrentClient, "%s\t%s\t%s\t%s\t%s\t%d\n",
		socket_info->local_addr,
		socket_info->local_port,
		socket_info->remote_addr,
		socket_info->remote_port,
		socket_info->state,
		socket_info->uid);
}

/*
================================ public part =================================
*/

void
initNetStat(struct SensorModul* sm)
{
	FILE *netstat;
	
	if ((netstat = fopen("/proc/net/tcp", "r")) != NULL) {
		registerMonitor("network/sockets/tcp/count", "integer", printNetStat, printNetStatInfo, sm);
		registerMonitor("network/sockets/tcp/list", "listview", printNetStatTcpUdpRaw, printNetStatTcpUdpRawInfo, sm);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/udp", "r")) != NULL) {
		registerMonitor("network/sockets/udp/count", "integer", printNetStat, printNetStatInfo, sm);
		registerMonitor("network/sockets/udp/list", "listview", printNetStatTcpUdpRaw, printNetStatTcpUdpRawInfo, sm);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/unix", "r")) != NULL) {
		registerMonitor("network/sockets/unix/count", "integer", printNetStat, printNetStatInfo, sm);
		registerMonitor("network/sockets/unix/list", "listview", printNetStatUnix, printNetStatUnixInfo, sm);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/raw", "r")) != NULL) {
		registerMonitor("network/sockets/raw/count", "integer", printNetStat, printNetStatInfo, sm);
		registerMonitor("network/sockets/raw/list", "listview", printNetStatTcpUdpRaw, printNetStatTcpUdpRawInfo, sm);
		fclose(netstat);
	}

	TcpSocketList = new_ctnr();
	UdpSocketList = new_ctnr();
	RawSocketList = new_ctnr();
	UnixSocketList = new_ctnr();
}

void
exitNetStat(void)
{
	destr_ctnr(TcpSocketList, free);
	destr_ctnr(UdpSocketList, free);
	destr_ctnr(RawSocketList, free);
	destr_ctnr(UnixSocketList, free);
}

int
updateNetStat(void)
{
	FILE *netstat;

	if ((netstat = fopen("/proc/net/tcp", "r")) != NULL) {
		num_tcp = get_num_sockets(netstat);
		fclose(netstat);
	}

	if ((netstat = fopen("/proc/net/udp", "r")) != NULL) {
		num_udp = get_num_sockets(netstat);
		fclose(netstat);
	}

	if ((netstat = fopen("/proc/net/unix", "r")) != NULL) {
		num_unix = get_num_sockets(netstat);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/raw", "r")) != NULL) {
		num_raw = get_num_sockets(netstat);
		fclose(netstat);
	}

	NetStat_timeStamp = time(0);
	return 0;
}

int
updateNetStatTcpUdpRaw(const char *cmd)
{
	FILE *netstat;
	char buffer[1024];
	uint local_addr, local_port;
	uint remote_addr, remote_port;
	int uid, i;
	uint state;
	SocketInfo *socket_info;

	if (strstr(cmd, "tcp")) {
		snprintf(buffer, sizeof(buffer), "/proc/net/tcp");
		for (i = level_ctnr(TcpSocketList); i >= 0; --i)
			free(pop_ctnr(TcpSocketList));
	}

	if (strstr(cmd, "udp")) {
		snprintf(buffer, sizeof(buffer), "/proc/net/udp");
		for (i = level_ctnr(UdpSocketList); i >= 0; --i)
			free(pop_ctnr(UdpSocketList));
	}

	if (strstr(cmd, "raw")) {
		snprintf(buffer, sizeof(buffer), "/proc/net/raw");
		for (i = level_ctnr(RawSocketList); i >= 0; --i)
			free(pop_ctnr(RawSocketList));
	}

	if ((netstat = fopen(buffer, "r")) == NULL) {
		print_error("Cannot open \'%s\'!\n"
		   "The kernel needs to be compiled with support\n"
		   "for /proc filesystem enabled!\n", buffer);
		return -1;
	}

	fgets(buffer, sizeof(buffer), netstat);

	while (fgets(buffer, sizeof(buffer), netstat) != NULL) {
		if (strcmp(buffer, "")) {
			sscanf(buffer, "%*d: %x:%x %x:%x %x %*x:%*x %*x:%*x %d",
			&local_addr, &local_port,
			&remote_addr, &remote_port,
			&state,
			&uid);

			if ((socket_info = (SocketInfo *)malloc(sizeof(SocketInfo))) == NULL) {
				continue;
			}
			strlcpy(socket_info->local_addr, get_host_name(local_addr), sizeof(socket_info->local_addr));
			strlcpy(socket_info->remote_addr, get_host_name(remote_addr), sizeof(socket_info->remote_addr));

			if (strstr(cmd, "tcp")) {
				strlcpy(socket_info->local_port, get_serv_name(local_port, "tcp"), sizeof(socket_info->local_port));
				strlcpy(socket_info->remote_port, get_serv_name(remote_port, "tcp"), sizeof(socket_info->remote_port));
				strlcpy(socket_info->state, conn_state[state], sizeof(socket_info->state));
				socket_info->uid = uid;

				push_ctnr(TcpSocketList, socket_info);
			}

			if (strstr(cmd, "udp")) {
				strlcpy(socket_info->local_port, get_serv_name(local_port, "udp"), sizeof(socket_info->local_port));
				strlcpy(socket_info->remote_port, get_serv_name(remote_port, "udp"), sizeof(socket_info->remote_port));
				strlcpy(socket_info->state, conn_state[state], sizeof(socket_info->state));
				socket_info->uid = uid;

				push_ctnr(UdpSocketList, socket_info);
			}

			if (strstr(cmd, "raw")) {
				strlcpy(socket_info->local_port, get_proto_name(local_port), sizeof(socket_info->local_port));
				strlcpy(socket_info->remote_port, get_proto_name(remote_port), sizeof(socket_info->remote_port));
				snprintf(socket_info->state, sizeof(socket_info->state)-1, "%d", state);
				socket_info->uid = uid;

				push_ctnr(RawSocketList, socket_info);
			}
		}
	}
	fclose(netstat);
	TcpUdpRaw_timeStamp = time(0);

	return 0;
}

int
updateNetStatUnix(void)
{
	FILE *file;
	char buffer[1024];
	char path[256];
	int ref_count, type, state, inode, i;
	UnixInfo *unix_info;

	if ((file = fopen("/proc/net/unix", "r")) == NULL) {
		print_error("Cannot open \'/proc/net/unix\'!\n"
		   "The kernel needs to be compiled with support\n"
		   "for /proc filesystem enabled!\n");
		return -1;
	}

	for (i = level_ctnr(UnixSocketList); i >= 0; --i)
		free(pop_ctnr(UnixSocketList));

	fgets(buffer, sizeof(buffer), file);

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		if (strcmp(buffer, "")) {
			sscanf(buffer, "%*x: %d %*d %*d %d %d %d %255s",
			&ref_count, &type, &state, &inode, path);

			if ((unix_info = (UnixInfo *)malloc(sizeof(UnixInfo))) == NULL) {
				continue;
			}

			unix_info->refcount = ref_count;
			strlcpy(unix_info->type, raw_type[type], sizeof(unix_info->type));
			strlcpy(unix_info->state, raw_state[state], sizeof(unix_info->state));
			unix_info->inode = inode;
			strlcpy(unix_info->path, path, sizeof(unix_info->path));

			push_ctnr(UnixSocketList, unix_info);
		}
	}
	fclose(file);
	Unix_timeStamp = time(0);

	return 0;
}

void
printNetStat(const char* cmd)
{
	if ((time(0) - NetStat_timeStamp) >= UPDATEINTERVAL)
		updateNetStat();

	if (strstr(cmd, "tcp") != NULL)
		fprintf(CurrentClient, "%d\n", num_tcp);
	if (strstr(cmd, "udp") != NULL)
		fprintf(CurrentClient, "%d\n", num_udp);
	if (strstr(cmd, "unix") != NULL)
		fprintf(CurrentClient, "%d\n", num_unix);
	if (strstr(cmd, "raw") != NULL)
		fprintf(CurrentClient, "%d\n", num_raw);
}

void
printNetStatInfo(const char* cmd)
{
	if (strstr(cmd, "tcp") != NULL)
		fprintf(CurrentClient, "Number of TCP-Sockets\t0\t0\tSockets\n");
	if (strstr(cmd, "udp") != NULL)
		fprintf(CurrentClient, "Number of UDP-Sockets\t0\t0\tSockets\n");
	if (strstr(cmd, "unix") != NULL)
		fprintf(CurrentClient, "Number of UnixDomain-Sockets\t0\t0\tSockets\n");
	if (strstr(cmd, "raw") != NULL)
		fprintf(CurrentClient, "Number of Raw-Sockets\t0\t0\tSockets\n");
}

void
printNetStatTcpUdpRaw(const char *cmd)
{
	SocketInfo* socket_info;

	if (strstr(cmd, "tcp")) {
		if ((time(0) - TcpUdpRaw_timeStamp) >= UPDATEINTERVAL)
			updateNetStatTcpUdpRaw("tcp");

		for (socket_info = first_ctnr(TcpSocketList); socket_info; socket_info = next_ctnr(TcpSocketList))
			printSocketInfo(socket_info);

		if (level_ctnr(TcpSocketList) == 0)
			fprintf(CurrentClient, "\n");
	}

	if (strstr(cmd, "udp")) {
		if ((time(0) - TcpUdpRaw_timeStamp) >= UPDATEINTERVAL)
			updateNetStatTcpUdpRaw("udp");

		for (socket_info = first_ctnr(UdpSocketList); socket_info; socket_info = next_ctnr(UdpSocketList))
			printSocketInfo(socket_info);

		if (level_ctnr(UdpSocketList) == 0)
			fprintf(CurrentClient, "\n");
	}

	if (strstr(cmd, "raw")) {
		if ((time(0) - TcpUdpRaw_timeStamp) >= UPDATEINTERVAL)
			updateNetStatTcpUdpRaw("raw");

		for (socket_info = first_ctnr(RawSocketList); socket_info; socket_info = next_ctnr(RawSocketList))
			printSocketInfo(socket_info);

		if (level_ctnr(RawSocketList) == 0)
			fprintf(CurrentClient, "\n");
	}
}

void
printNetStatTcpUdpRawInfo(const char *cmd)
{
	(void) cmd;
	fprintf(CurrentClient, "Local Address\tPort\tForeign Address\tPort\tState\tUID\ns\ts\ts\ts\ts\td\n");
}

void printNetStatUnix(const char *cmd)
{
	UnixInfo* unix_info;

	(void) cmd;
	if ((time(0) - Unix_timeStamp) >= UPDATEINTERVAL)
		updateNetStatUnix();
	
	for (unix_info = first_ctnr(UnixSocketList); unix_info; unix_info = next_ctnr(UnixSocketList)) {
		fprintf(CurrentClient, "%d\t%s\t%s\t%d\t%s\n",
			unix_info->refcount,
			unix_info->type,
			unix_info->state,
			unix_info->inode,
			unix_info->path);
	}

	if (level_ctnr(UnixSocketList) == 0)
		fprintf(CurrentClient, "\n");
}

void printNetStatUnixInfo(const char *cmd)
{
	(void) cmd;
	fprintf(CurrentClient, "RefCount\tType\tState\tInode\tPath\nd\ts\ts\td\ts\n");
}
