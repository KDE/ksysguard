/*
    KTop, the KDE Task Manager
   
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
#include <string.h>

#include <Command.h>
#include "netstat.h"

static int num_tcp = 0;
static int num_udp = 0;
static int num_unix = 0;
static int num_raw = 0;

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

	bzero(buffer, sizeof(buffer));
	
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

	bzero(buffer, sizeof(buffer));

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

	bzero(buffer, sizeof(buffer));

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

/*
================================ public part =================================
*/

void
initNetStat(void)
{
	FILE *netstat;
	
	if ((netstat = fopen("/proc/net/tcp", "r")) != NULL) {
		registerMonitor("network/sockets/tcp/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/tcp/list", "listview", printNetStatTUR, printNetStatTURInfo);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/udp", "r")) != NULL) {
		registerMonitor("network/sockets/udp/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/udp/list", "listview", printNetStatTUR, printNetStatTURInfo);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/unix", "r")) != NULL) {
		registerMonitor("network/sockets/unix/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/unix/list", "listview", printNetStatUnix, printNetStatUnixInfo);
		fclose(netstat);
	}
	if ((netstat = fopen("/proc/net/raw", "r")) != NULL) {
		registerMonitor("network/sockets/raw/count", "integer", printNetStat, printNetStatInfo);
		registerMonitor("network/sockets/raw/list", "listview", printNetStatTUR, printNetStatTURInfo);
		fclose(netstat);
	}
}

void
exitNetStat(void)
{
}

int
updateNetStat(void)
{
	FILE* netstat;

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

void
printNetStat(const char* cmd)
{
	updateNetStat();

	if (strstr(cmd, "tcp") != NULL)
		printf("%d\n", num_tcp);
	if (strstr(cmd, "udp") != NULL)
		printf("%d\n", num_udp);
	if (strstr(cmd, "unix") != NULL)
		printf("%d\n", num_unix);
	if (strstr(cmd, "raw") != NULL)
		printf("%d\n", num_raw);
}

void
printNetStatInfo(const char* cmd)
{
	if (strstr(cmd, "tcp") != NULL)
		printf("Number of TCP-Sockets\t0\t65536\tSockets\n");
	if (strstr(cmd, "udp") != NULL)
		printf("Number of UDP-Sockets\t0\t65536\tSockets\n");
	if (strstr(cmd, "unix") != NULL)
		printf("Number of UnixDomain-Sockets\t0\t65536\tSockets\n");
	if (strstr(cmd, "raw") != NULL)
		printf("Number of Raw-Sockets\t0\t65536\tSockets\n");
}

void
printNetStatTUR(const char *cmd)
{
	FILE *file;
	char buffer[1024];
	int local_addr, local_port;
	int remote_addr, remote_port;
	int status, uid;

	if (strstr(cmd, "tcp"))
		snprintf(buffer, sizeof(buffer), "/proc/net/tcp");
	if (strstr(cmd, "udp"))
		snprintf(buffer, sizeof(buffer), "/proc/net/udp");
	if (strstr(cmd, "raw"))
		snprintf(buffer, sizeof(buffer), "/proc/net/raw");

	if ((file = fopen(buffer, "r")) == NULL) {
		perror("open");
		return;
	}

	fgets(buffer, sizeof(buffer), file);

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		if (strcmp(buffer, "")) {
			sscanf(buffer, "%*d: %x:%x %x:%x %x %*x:%*x %*x:%*x %d",
			&local_addr, &local_port,
			&remote_addr, &remote_port,
			&status,
			&uid);

			if (strstr(cmd, "tcp")) {
				printf("%s\t%s\t%s\t%s\t%s\t%d\n",
				get_host_name(local_addr),
				get_serv_name(local_port, "tcp"),
				get_host_name(remote_addr),
				get_serv_name(remote_port, "tcp"),
				conn_state[status], uid);
			}

			if (strstr(cmd, "udp")) {
				printf("%s\t%s\t%s\t%s\t%s\t%d\n",
				get_host_name(local_addr),
				get_serv_name(local_port, "udp"),
				get_host_name(remote_addr),
				get_serv_name(remote_port, "udp"),
				conn_state[status], uid);
			}

			if (strstr(cmd, "raw")) {
				printf("%s\t%s\t%s\t%s\t%d\t%d\n",
				get_host_name(local_addr),
				get_proto_name(local_port),
				get_host_name(remote_addr),
				get_proto_name(remote_port),
				status, uid);
			}
		}
	}
	fclose(file);
}

void
printNetStatTURInfo(const char *cmd)
{
	if (strstr(cmd, "tcp") || strstr(cmd, "udp")) {
		printf("Local Address\tPort\tForeign Address\tPort\tState\tUID\n");
	} else {
		printf("Local Address\tPort\tForeign Address\tPort\tState\tUID\n");
	}		
}

void printNetStatUnix(const char *cmd)
{
	FILE *file;
	char buffer[1024];
	char path[256];
	int ref_count, type, state, inode;

	if ((file = fopen("/proc/net/unix", "r")) == NULL) {
		perror("open");
		return;
	}

	fgets(buffer, sizeof(buffer), file);

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		if (strcmp(buffer, "")) {
			sscanf(buffer, "%*x: %d %*d %*d %d %d %d %255s",
			&ref_count, &type, &state, &inode, path);

			printf("%d\t%s\t%s\t%d\t%s\n",
			ref_count,
			raw_type[type],
			raw_state[state],
			inode,
			path);
		}
	}
	fclose(file);
}

void printNetStatUnixInfo(const char *cmd)
{
	printf("RefCount\tType\tState\tInode\tPath\n");
}
