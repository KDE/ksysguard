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
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
#include "logfile.h"

#define CMDBUFSIZE	128
#define MAX_CLIENTS	100
#define PORT_NUMBER	2001
#define TIMERINTERVAL	2

typedef struct
{
	int socket;
	FILE* out;
} ClientInfo;

static int ServerSocket;
static ClientInfo ClientList[MAX_CLIENTS];
static int SocketPort = PORT_NUMBER;
static int CurrentSocket;
static char *LockFile = "/var/run/ksysguardd.pid";

int QuitApp = 0;
int RunAsDaemon = 0;

static int
processArguments(int argc, char* argv[])
{
	int option;

	opterr = 0;
	while ((option = getopt(argc, argv, "-p:dh")) != EOF) 
	{
		switch (tolower(option))
		{
		case 'p':
			SocketPort = atoi(optarg);
			break;
		case 'd':
			RunAsDaemon = 1;
			break;
		case '?':
		case 'h':
			fprintf(stderr, "usage: %s [-d] [-p port]\n", argv[0]);
			return (-1);
			break;
		}
	}

	return (0);
}

static void
printWelcome(FILE* out)
{
	fprintf(out,
			"ksysguardd %s\n"
			"(c) 1999 - 2001 Chris Schlaeger <cs@kde.org> and\n"
			"                Tobias Koenig <tokoe82@yahoo.de>\n"
			"This program is part of the KDE Project and licensed under\n"
			"the GNU GPL version 2. See www.kde.org for details!\n"
			"ksysguardd> ", VERSION);
	fflush(out);
}

static int
createLockFile()
{
	FILE *file;
	
	if ((file = fopen(LockFile, "r")) != NULL)
	{
		log_error("Server is already running");
		fclose(file);
		return -1;
	}
	else
	{
		if ((file = fopen(LockFile, "w")) == NULL)
		{
			log_error("Cannot create lockfile '%s'", LockFile);
			return -2;
		}
		fprintf(file, "%d", getpid());
		fclose(file);

		return 0;
	}
}

static void
removeLockFile(void)
{
	/* setuid back to root for removing lockfile */
	if (seteuid(0) < 0) {
		log_error("Cannot seteuid back to 'root'");
	}

	if (unlink(LockFile) < 0) {
		log_error("Cannot remove lockfile '%s'", LockFile);
	}
}

void
signalHandler(int sig)
{
	switch (sig)
	{
		case SIGQUIT:
		case SIGTERM:
			removeLockFile();
			exit(0);
			break;
	}
}

static void
installSignalHandler(void)
{
	struct sigaction Action;

	Action.sa_handler = signalHandler;
	sigemptyset(&Action.sa_mask);
	/* make sure that interrupted system calls are restarted. */
	Action.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &Action, 0);
	sigaction(SIGQUIT, &Action, 0);
}

static void
changeUID(void)
{
	struct passwd *pwd;
	
	if ((pwd = getpwnam("nobody")) != NULL)
	{
		seteuid(pwd->pw_uid);
	}
	else
	{
		log_error("Cannot seteuid to 'nobody'");
		/* We exit here to avoid becoming vulnerable just because
		 * user nobody does not exist. */
		exit(1);
	}
}

void
makeDaemon(void)
{

	switch (fork())
	{
	case -1:
		log_error("fork()");
		break;
	case 0:
		setsid();
		chdir("/");
		umask(0);
		if (createLockFile() < 0)
			exit(1);

		installSignalHandler();

		changeUID();
		break;
	default:
		exit(0);
	}
}

static int
readCommand(int fd, char* cmdBuf, size_t len)
{
	int i;
	char c;
	for (i = 0; (i < len) && (read(fd, &c, 1) == 1) && (c != '\n'); ++i)
		cmdBuf[i] = c;
	cmdBuf[i] = '\0';

	return (i);
}

void
reset_clientlist(void)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		ClientList[i].socket = -1;
		ClientList[i].out = 0;
	}
}

/* addClient adds a new client to the ClientList. */
int
addClient(int client)
{
	int i;
	FILE* out;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (ClientList[i].socket == -1)
		{
			ClientList[i].socket = client;
			if ((out = fdopen(client, "w+")) == NULL)
			{
				log_error("fdopen()");
				return (-1);
			}
			/* We use unbuffered IO */
			fcntl(fileno(out), F_SETFL, O_NDELAY);
			ClientList[i].out = out;
			printWelcome(out);

			return (0);
		}
	}

	return (-1);
}

/* delClient removes a client from the ClientList. */
int
delClient(int client)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (ClientList[i].socket == client)
		{
			fclose(ClientList[i].out);
			ClientList[i].out = 0;
			close(ClientList[i].socket);
			ClientList[i].socket = -1;
			return (0);
		}
	}

	return (-1);
}

int
createServerSocket(int port)
{
	int i = 1;
	int newSocket;
	struct sockaddr_in sin;

	if ((newSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
	{
		log_error("socket()");
		return (-1);
	}

	setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);
  
	if (bind(newSocket, (struct sockaddr_in*) &sin, sizeof(sin)) < 0)
	{
		log_error("bind()");
		return -1;
	}
	
	if (listen(newSocket, 5) < 0)
	{
		log_error("listen()");
		return -1;
	}
	
	return (newSocket);
}

static int
setupSelect(fd_set* fds)
{
	int highestFD = ServerSocket;
	FD_ZERO(fds);
	/* Fill the filedescriptor array with all relevant descriptors. If we
	 * not in daemon mode we only need to watch stdin. */
	if (RunAsDaemon)
	{
		int i;
		FD_SET(ServerSocket, fds);

		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (ClientList[i].socket != -1)
			{
				FD_SET(ClientList[i].socket, fds);
				if (highestFD < ClientList[i].socket)
					highestFD = ClientList[i].socket;
			}
		}
	}
	else
	{
		FD_SET(STDIN_FILENO, fds);
		if (highestFD < STDIN_FILENO)
			highestFD = STDIN_FILENO;
	}

	return (highestFD);
}

static void
handleTimerEvent(struct timeval* tv, struct timeval* last)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	/* Check if the last event was really TIMERINTERVAL seconds ago */
	if (now.tv_sec - last->tv_sec >= TIMERINTERVAL)
	{
		/* If so, update all sensors and save current time to last. */
		updateMemory();
		updateStat();
		updateNetDev();
		updateApm();
		updateCpuInfo();
		updateLoadAvg();
		*last = now;
	}
	/* Set tv so that the next timer event will be generated in
	 * TIMERINTERVAL seconds. */
	tv->tv_usec = last->tv_usec - now.tv_usec;
	if (tv->tv_usec < 0)
	{
		tv->tv_usec += 1000000;
		tv->tv_sec = last->tv_sec + TIMERINTERVAL - 1 - now.tv_sec;
	}
	else
		tv->tv_sec = last->tv_sec + TIMERINTERVAL - now.tv_sec;
}

static void
handleSocketTraffic(int socket, const fd_set* fds)
{
	char cmdBuf[CMDBUFSIZE];

	if (RunAsDaemon)
	{
		int i;

		if (FD_ISSET(socket, fds))
		{
			int clientSocket;
			struct sockaddr_in addr;
			socklen_t addr_len;

			/* a new connection */
			if ((clientSocket = accept(socket, &addr, &addr_len)) < 0)
			{
				log_error("accept()");
				exit(1);
			}
			else
				addClient(clientSocket);
		}

		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (ClientList[i].socket != -1)
			{
				CurrentSocket = ClientList[i].socket;
				if (FD_ISSET(ClientList[i].socket, fds))
				{
					ssize_t cnt;
					if ((cnt = readCommand(CurrentSocket, cmdBuf,
										   sizeof(cmdBuf) - 1)) <= 0)
					{
						delClient(CurrentSocket);
					}
					else
					{
						cmdBuf[cnt] = '\0';
						if (strncmp(cmdBuf, "quit", 4) == 0)
							delClient(CurrentSocket);
						else
						{
							currentClient = ClientList[i].out;
							fflush(stdout);
							executeCommand(cmdBuf);
							fprintf(currentClient, "ksysguardd> ");
							fflush(currentClient);
						}
					}
				}
			}
		}
	}
	else if (FD_ISSET(STDIN_FILENO, fds))
	{
		readCommand(STDIN_FILENO, cmdBuf, sizeof(cmdBuf));
		executeCommand(cmdBuf);
		printf("ksysguardd> ");
		fflush(stdout);
	}
}

static void
initModules()
{
	/* initialize all sensors */
	initCommand();
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
	initLogFile();

	ReconfigureFlag = 0;
}

static void
exitModules()
{
	exitLogFile();
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
}

/*
================================ public part =================================
*/

int
main(int argc, char* argv[])
{
	fd_set fds;
	struct timeval tv;
	struct timeval last;

	printWelcome(stdout);

	if (processArguments(argc, argv) < 0)
		return (-1);

	initModules();

	if (RunAsDaemon)
	{
		makeDaemon();

		if ((ServerSocket = createServerSocket(SocketPort)) < 0)
			return (-1);
		reset_clientlist();
	}
	else
	{
		fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
		setvbuf(stdin, malloc(4096), _IOFBF, 4096);
		fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
		currentClient = stdout;
		ServerSocket = 0;
	}

	tv.tv_sec = TIMERINTERVAL;
	tv.tv_usec = 0;
	gettimeofday(&last, NULL);

	while (!QuitApp)
	{
		int highestFD = setupSelect(&fds);
		/* wait for communication or timeouts */
		if (select(highestFD + 1, &fds, NULL, NULL, &tv) >= 0)
		{
			handleTimerEvent(&tv, &last);
			handleSocketTraffic(ServerSocket, &fds);
		}
	}

	exitModules();

	return 0;
}
