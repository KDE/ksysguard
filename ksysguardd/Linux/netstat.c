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

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static time_t TcpUdpRaw_timeStamp = 0;
static time_t Unix_timeStamp = 0;

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

char *get_serv_name(int port, char *proto)
{
	static char buffer[1024];
	struct servent *service;

	if (port == 0) {
		return "*";
	}

	memset(buffer, 0, sizeof(buffer));
	
	if ((service = getservbyport(ntohs(port), proto)) == NULL) {
		snprintf(buffer, sizeof(buffer), "%d", port);
	} else {
		strncpy(buffer, service->s_name, sizeof(buffer));
	}

	return (char *)buffer;
}

char *get_host_name(int addr)
{
	static char buffer[1024];
	struct hostent *host;
	struct in_addr a_addr;

	if (addr == 0) {
		return "*";
	}

	memset(buffer, 0, sizeof(buffer));

	if ((host = gethostbyaddr((char *)&addr, 4, AF_INET)) == NULL) {
		a_addr.s_addr = addr;
		return inet_ntoa(a_addr);
	} else {
		strncpy(buffer, host->h_name, sizeof(buffer));
		return (char *)buffer;
	}
}

char *get_proto_name(int number)
{
	static char buffer[1024];
	struct protoent *protocol;

	if (number == 0) {
		return "*";
	}

	memset(buffer, 0, sizeof(buffer));

	if ((protocol = getprotobynumber(number)) == NULL) {
		snprintf(buffer, sizeof(buffer), "%d", number);
	} else {
		strncpy(buffer, protocol->p_name, sizeof(buffer));
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
	fprintf(currentClient, "%s\t%s\t%s\t%s\t%s\t%d\n",
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
initNetStat(void)
{
	FILE *netstat;
	
	if ((netstat = fopen("/proc/net/tcp", "r")) != NULL) {
		registerMonitor("network/sockets/tcp/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/tcp/list", "listview", printNetStatTcpUdpRaw, printNetStatTcpUdpRawInfo);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/udp", "r")) != NULL) {
		registerMonitor("network/sockets/udp/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/udp/list", "listview", printNetStatTcpUdpRaw, printNetStatTcpUdpRawInfo);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/unix", "r")) != NULL) {
		registerMonitor("network/sockets/unix/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/unix/list", "listview", printNetStatUnix, printNetStatUnixInfo);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/raw", "r")) != NULL) {
		registerMonitor("network/sockets/raw/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/raw/list", "listview", printNetStatTcpUdpRaw, printNetStatTcpUdpRawInfo);
		fclose(netstat);
	}

	TcpSocketList = new_ctnr(CT_DLL);
	UdpSocketList = new_ctnr(CT_DLL);
	RawSocketList = new_ctnr(CT_DLL);
	UnixSocketList = new_ctnr(CT_DLL);
}

void
exitNetStat(void)
{
	if (TcpSocketList)
		destr_ctnr(TcpSocketList, free);
	if (UdpSocketList)
		destr_ctnr(UdpSocketList, free);
	if (RawSocketList)
		destr_ctnr(RawSocketList, free);
	if (UnixSocketList)
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

	return 0;
}

int
updateNetStatTcpUdpRaw(const char *cmd)
{
	FILE *netstat;
	char buffer[1024];
	int local_addr, local_port;
	int remote_addr, remote_port;
	int state, uid, i;
	SocketInfo *socket_info;

	if (strstr(cmd, "tcp")) {
		snprintf(buffer, sizeof(buffer), "/proc/net/tcp");
		for (i = 0; i < level_ctnr(TcpSocketList); i++) {
			free(remove_ctnr(TcpSocketList, i--));
		}
	}

	if (strstr(cmd, "udp")) {
		snprintf(buffer, sizeof(buffer), "/proc/net/udp");
		for (i = 0; i < level_ctnr(UdpSocketList); i++) {
			free(remove_ctnr(UdpSocketList, i--));
		}
	}

	if (strstr(cmd, "raw")) {
		snprintf(buffer, sizeof(buffer), "/proc/net/raw");
		for (i = 0; i < level_ctnr(RawSocketList); i++) {
			free(remove_ctnr(RawSocketList, i--));
		}
	}

	if ((netstat = fopen(buffer, "r")) == NULL) {
		perror("open");
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
			strncpy(socket_info->local_addr, get_host_name(local_addr), 127);
			strncpy(socket_info->remote_addr, get_host_name(remote_addr), 127);

			if (strstr(cmd, "tcp")) {
				strncpy(socket_info->local_port, get_serv_name(local_port, "tcp"), 127);
				strncpy(socket_info->remote_port, get_serv_name(remote_port, "tcp"), 127);
				strncpy(socket_info->state, conn_state[state], 127);
				socket_info->uid = uid;

				push_ctnr(TcpSocketList, socket_info);
			}

			if (strstr(cmd, "udp")) {
				strncpy(socket_info->local_port, get_serv_name(local_port, "udp"), 127);
				strncpy(socket_info->remote_port, get_serv_name(remote_port, "udp"), 127);
				strncpy(socket_info->state, conn_state[state], 127);
				socket_info->uid = uid;

				push_ctnr(UdpSocketList, socket_info);
			}

			if (strstr(cmd, "raw")) {
				strncpy(socket_info->local_port, get_proto_name(local_port), 127);
				strncpy(socket_info->remote_port, get_proto_name(remote_port), 127);
				snprintf(socket_info->state, 127, "%d", state);
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
		perror("open");
		return -1;
	}

	for (i = 0; i < level_ctnr(UnixSocketList); i++) {
		free(remove_ctnr(UnixSocketList, i--));
	}

	fgets(buffer, sizeof(buffer), file);

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		if (strcmp(buffer, "")) {
			sscanf(buffer, "%*x: %d %*d %*d %d %d %d %255s",
			&ref_count, &type, &state, &inode, path);

			if ((unix_info = (UnixInfo *)malloc(sizeof(UnixInfo))) == NULL) {
				continue;
			}

			unix_info->refcount = ref_count;
			strncpy(unix_info->type, raw_type[type], 127);
			strncpy(unix_info->state, raw_state[state], 127);
			unix_info->inode = inode;
			strncpy(unix_info->path, path, 255);

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
	updateNetStat();

	if (strstr(cmd, "tcp") != NULL)
		fprintf(currentClient, "%d\n", num_tcp);
	if (strstr(cmd, "udp") != NULL)
		fprintf(currentClient, "%d\n", num_udp);
	if (strstr(cmd, "unix") != NULL)
		fprintf(currentClient, "%d\n", num_unix);
	if (strstr(cmd, "raw") != NULL)
		fprintf(currentClient, "%d\n", num_raw);
}

void
printNetStatInfo(const char* cmd)
{
	if (strstr(cmd, "tcp") != NULL)
		fprintf(currentClient, "Number of TCP-Sockets\t0\t65536\tSockets\n");
	if (strstr(cmd, "udp") != NULL)
		fprintf(currentClient, "Number of UDP-Sockets\t0\t65536\tSockets\n");
	if (strstr(cmd, "unix") != NULL)
		fprintf(currentClient, "Number of UnixDomain-Sockets\t0\t65536\tSockets\n");
	if (strstr(cmd, "raw") != NULL)
		fprintf(currentClient, "Number of Raw-Sockets\t0\t65536\tSockets\n");
}

void
printNetStatTcpUdpRaw(const char *cmd)
{
	int i;

	if (strstr(cmd, "tcp")) {
		if ((time(0) - TcpUdpRaw_timeStamp) > 2)
			updateNetStatTcpUdpRaw("tcp");

		for (i = 0; i < level_ctnr(TcpSocketList); i++) {
			SocketInfo* socket_info = get_ctnr(TcpSocketList, i);
			printSocketInfo(socket_info);
		}
	}

	if (strstr(cmd, "udp")) {
		if ((time(0) - TcpUdpRaw_timeStamp) > 2)
			updateNetStatTcpUdpRaw("udp");

		for (i = 0; i < level_ctnr(UdpSocketList); i++) {
			SocketInfo* socket_info = get_ctnr(UdpSocketList, i);
			printSocketInfo(socket_info);
		}
	}

	if (strstr(cmd, "raw")) {
		if ((time(0) - TcpUdpRaw_timeStamp) > 2)
			updateNetStatTcpUdpRaw("raw");

		for (i = 0; i < level_ctnr(RawSocketList); i++) {
			SocketInfo* socket_info = get_ctnr(RawSocketList, i);
			printSocketInfo(socket_info);
		}
	}
}

void
printNetStatTcpUdpRawInfo(const char *cmd)
{
	fprintf(currentClient, "Local Address\tPort\tForeign Address\tPort\tState\tUID\n");
}

void printNetStatUnix(const char *cmd)
{
	int i;

	if ((time(0) - Unix_timeStamp) > 2)
		updateNetStatUnix();
	
	for (i = 0; i < level_ctnr(UnixSocketList); i++) {
		UnixInfo* unix_info = get_ctnr(UnixSocketList, i);

		fprintf(currentClient, "%d\t%s\t%s\t%d\t%s\n",
			unix_info->refcount,
			unix_info->type,
			unix_info->state,
			unix_info->inode,
			unix_info->path);
	}
}

void printNetStatUnixInfo(const char *cmd)
{
	fprintf(currentClient, "RefCount\tType\tState\tInode\tPath\n");
}
