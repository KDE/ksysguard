/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999-2001 Chris Schlaeger <cs@kde.org>
				 Tobias Koenig <tokoe82@yahoo.de>
    
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

#include <config.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>

#include "ksysguardd.h"
#include "Command.h"
#include "ProcessList.h"
#include "Memory.h"
#include "stat.h"
#include "netdev.h"
#include "netstat.h"
#include "apm.h"
#include "cpuinfo.h"
#include "loadavg.h"
#include "lmsensors.h"
#include "diskstat.h"

#define CMDBUFSIZE 128
#define MAX_CLIENTS	100
#define PORT_NUMBER	2001

static int client_list[MAX_CLIENTS];
FILE* clientFDs[MAX_CLIENTS];
static int curr_socket;
static int QuitApp = 0;
int RunAsDaemon = 0;

static void
printWelcome(FILE* out)
{
	fprintf(out,
			"ksysguardd %s  (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>\n"
			"                               Tobias Koenig <tokoe82@yahoo.de>\n"
			"This program is part of the KDE Project and licensed under\n"
			"the GNU GPL version 2. See www.kde.org for details!\n"
			"ksysguardd> ", VERSION);
	fflush(out);
}

static void
readCommand(char* cmdBuf)
{
	int i;
	int c;

	for (i = 0; i < CMDBUFSIZE - 1 && (c = getchar()) != '\n'; i++)
		cmdBuf[i] = (char) c;
	cmdBuf[i] = '\0';
}

void reset_clientlist(void)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		client_list[i] = -1;
		clientFDs[i] = 0;
	}
}

void add_client(int client)
{
	int i;
	FILE* out;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_list[i] == -1)
		{
			client_list[i] = client;
			if ((out = fdopen(client, "w+")) == NULL)
				log_error("fdopen");
			clientFDs[i] = out;
			printWelcome(out);
			return;
		}
	}
}

void del_client(int client)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (client_list[i] == client) {
			client_list[i] = -1;
			fflush(clientFDs[i]);
			fclose(clientFDs[i]);
		}
	}
}

int createServerSocket(int port)
{
	int sock, i = 1;
	struct sockaddr_in sin;

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		log_error("socket");
		return -1;
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);
  
	if (bind(sock, (struct sockaddr_in *)&sin, sizeof(sin)) < 0) {
		log_error("bind");
		return -1;
	}
	
	if (listen(sock, 5) < 0) {
		log_error("listen");
		return -1;
	}
	
	return sock;
}

void make_daemon(void)
{
	switch (fork()) {
		case -1:
			log_error("fork");
			break;
		case 0:
			setsid();
			chdir("/");
			umask(0);
			break;
		default:
			exit(0);
	}
}

void
exQuit(const char* cmd)
{
	QuitApp = 1;
}
	
/*
================================ public part =================================
*/

int
main(int argc, char* argv[])
{
	char cmdBuf[CMDBUFSIZE], option;
	int socket, conn_socket, maxfd, i;
	fd_set fds;
	struct sockaddr_in addr;
	socklen_t addr_len;
	struct timeval tv;
	struct timeval last;
	int socket_port = PORT_NUMBER;

	opterr = 0;
	while ((option = getopt(argc, argv, "-p:d")) != EOF) {
		switch (tolower(option)) {
			case 'p':
				socket_port = atoi(optarg);
				break;
			case 'd':
				RunAsDaemon = 1;
				break;
			case '?':
				fprintf(stderr, "usage: %s [-d] [-p port]\n", argv[0]);
				return -1;
				break;
		}
	}

	/* initialize all sensors */
	initCommand();
	if (RunAsDaemon == 0)
		registerCommand("quit", exQuit);
	initProcessList();
	initMemory();
	initStat();
	initNetDev();
	initNetStat();
	initApm();
	initCpuInfo();
	initLoadAvg();
	initLmSensors();
	initDiskStat();

	ReconfigureFlag = 0;

	printWelcome(stdout);

	if (RunAsDaemon)
	{
		make_daemon();

		if ((socket = createServerSocket(socket_port)) < 0)
			return -1;
		reset_clientlist();
	}
	else
	{
		currentClient = stdout;
		socket = 0;
	}

	tv.tv_sec = 2;
	tv.tv_usec = 0;
	gettimeofday(&last, NULL);

	while (!QuitApp)
	{
		memset(cmdBuf, 0, sizeof(cmdBuf));

		/* fill select structure */
		maxfd = socket;

		FD_ZERO(&fds);
		if (RunAsDaemon)
		{
			FD_SET(socket, &fds);

			for (i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_list[i] != -1)
				{
					FD_SET(client_list[i], &fds);
					maxfd = (client_list[i] > maxfd ? client_list[i] : maxfd);
				}
			}
		}
		else
			FD_SET(fileno(stdin), &fds);

		/* wait for communication or timeouts */
		if (select(maxfd + 1, &fds, NULL, NULL, &tv) >= 0)
		{
			struct timeval now;
			gettimeofday(&now, NULL);
			if (now.tv_sec - last.tv_sec >= 2)
			{
				updateMemory();
				updateStat();
				updateNetDev();
				updateApm();
				updateCpuInfo();
				updateLoadAvg();
				last = now;
			}
			tv.tv_usec = last.tv_usec - now.tv_usec;
			if (tv.tv_usec < 0)
			{
				tv.tv_usec += 1000000;
				tv.tv_sec = last.tv_sec + 1 - now.tv_sec;
			}
			else
				tv.tv_sec = last.tv_sec + 2 - now.tv_sec;

			if (RunAsDaemon)
			{
				if (FD_ISSET(socket, &fds))
				{
					/* a new connection */
					if ((conn_socket = accept(socket, &addr, &addr_len)) < 0)
					{
						log_error("accept");
						exit(1);
					}
					else
						add_client(conn_socket);
				}
			
				for (i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_list[i] != -1)
					{
						curr_socket = client_list[i];
						if (FD_ISSET(client_list[i], &fds))
						{
							if (read(curr_socket, cmdBuf, sizeof(cmdBuf) - 1) < 0
								|| strstr(cmdBuf, "quit"))
							{
								del_client(curr_socket);
								close(curr_socket);
								clientFDs[i] = 0;
							}
							else
							{
								currentClient = clientFDs[i];
								executeCommand(cmdBuf);
								fprintf(currentClient, "ksysguardd> ");
								fflush(currentClient);
							}
						}
					}
				}
			}
			else if (FD_ISSET(fileno(stdin), &fds))
			{
				readCommand(cmdBuf);
				executeCommand(cmdBuf);
				printf("ksysguardd> ");
				fflush(stdout);
			}
		}
	}

	exitDiskStat();
	exitLmSensors();
	exitLoadAvg();
	exitCpuInfo();
	exitApm();
	exitNetDev();
	exitNetDev();
	exitStat();
	exitMemory();
	exitProcessList();
	exitCommand();

	return 0;
}
