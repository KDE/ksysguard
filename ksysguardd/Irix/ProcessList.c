/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

	Irix support by Carsten Kroll <ckroll@pinnaclesys.com>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/procfs.h>
#include <sys/statfs.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h>

#include "ccont.h"
#include "../../gui/SignalIDs.h"
#include "ksysguardd.h"

#include "Command.h"
#include "ProcessList.h"

#define BUFSIZE 1024
#define KDEINITLEN strlen("kdeinit: ")

typedef struct {
	int	alive;		/*  for "garbage collection"	  	*/
	pid_t	pid;		/*  process ID			  	*/
	pid_t	ppid;		/*  parent process ID		  	*/
	uid_t	uid;		/*  process owner (real UID)	  	*/
	gid_t	gid;		/*  process group (real GID)	  	*/
	char	*userName;	/*  process owner (name)	  	*/
	int	nThreads;	/*  # of threads in this process  	*/
	int	Prio;		/*  scheduling priority		  	*/
	size_t	Size;		/*  total size of process image	  	*/
	size_t	RSSize;		/*  resident set size		  	*/
	char	State[8];	/*  process state		  	*/
	double	Time;		/*  CPU time for the process in 100ms	*/
	double	Load;		/*  CPU load in %		  	*/
	char	Command[PRCOMSIZ];/*  command name			*/
	char	CmdLine[PRARGSZ];/*  command line		  	*/
	double	centStamp;	/*  timestamp for CPU load		*/
} ProcessInfo;

static CONTAINER ProcessList = 0;
static unsigned ProcessCount = 0;	/*  # of processes	  */
static DIR *procdir;			/*  handle for /proc	  */
static int pagesz;

#define KBYTES 1024

/*
 *  lwpStateName()  --  return string representation of process state
 */
char *lwpStateName( prpsinfo_t lwpinfo ) {

	static char result[8];

	switch( lwpinfo.pr_sname ) {
		case 'S':
			sprintf( result, "%s", "sleep" );
			break;
		case 'R':
			sprintf( result, "%s", "run" );
			break;
		case 'Z':
			sprintf( result, "%s", "zombie" );
			break;
		case 'T':
			sprintf( result, "%s", "stop" );
			break;
		case 'I':
			sprintf( result, "%s", "start" );
			break;
		case 'X':
			sprintf( result, "%s", "wmem" );
		case '0':
			sprintf( result, "%s/%d", "cpu", (int) lwpinfo.pr_sonproc );
			break;
		default:
			sprintf( result, "%s", "???" );
			break;
	}

	return( result );
}

static void validateStr( char *string ) {

	char	*ptr = string;

	/*
	 *  remove all chars that might screw up communication
	 */
	while( *ptr != '\0' ) {
		if( *ptr == '\t' || *ptr == '\n' || *ptr == '\r' )
			*ptr = ' ';
		ptr++;
	}
	/*
	 *  make sure there's at least one char 
	 */
	if( string[0] == '\0' )
		strcpy( string, " " );
}

static int processCmp( void *p1, void *p2 ) {

	return( ((ProcessInfo *) p1)->pid - ((ProcessInfo *) p2)->pid );
}

static ProcessInfo *findProcessInList( pid_t pid ) {

	ProcessInfo	key;
	long		index;
	
	key.pid = pid;
	if( (index = search_ctnr( ProcessList, processCmp, &key )) < 0 )
		return( NULL );

	return( get_ctnr( ProcessList, index ));
}

static int updateProcess( pid_t pid ) {
	ProcessInfo	*ps;
	int		fd;
	char		buf[BUFSIZE];
	prpsinfo_t	psinfo;
	struct passwd	*pw;
	register double newCentStamp,timeDiff, usDiff,usTime;
	struct timeval tv;

	if( (ps = findProcessInList( pid )) == NULL ) {
		if( (ps = (ProcessInfo *) malloc( sizeof( ProcessInfo )))
				== NULL ) {
			print_error( "cannot malloc()\n" );
			return( -1 );
		}
		ps->pid = pid;
		ps->userName = NULL;
		ps->alive = 0;

		gettimeofday(&tv, 0);
		ps->centStamp = (double)tv.tv_sec * 100.0 + (double)tv.tv_usec / 10000.0;

		push_ctnr( ProcessList, ps );
		bsort_ctnr( ProcessList, processCmp );
	}

	sprintf( buf, "%s/pinfo/%ld", PROCDIR, pid );
	if( (fd = open( buf, O_RDONLY )) < 0 ) {
		/* process terminated */
		return( -1 );
	}



	if( ioctl(fd,PIOCPSINFO,&psinfo) < 0) {
		print_error( "cannot read psinfo from \"%s\"\n", buf );
		close( fd );
		return( -1 );
	}
	close( fd );

	ps->ppid = psinfo.pr_ppid;
	ps->uid = psinfo.pr_uid;
	ps->gid = psinfo.pr_gid;

	pw = getpwuid( psinfo.pr_uid );
	if( ps->userName != NULL )
		free( ps->userName );
	ps->userName = strdup( pw->pw_name );

	strncpy (ps->State,lwpStateName( psinfo ),8);
        ps->State[7]='\0';


	ps->Prio = psinfo.pr_pri;

	gettimeofday(&tv, 0);
	newCentStamp = (double)tv.tv_sec * 100.0 + (double) tv.tv_usec / 10000.0;
	usTime = (double) psinfo.pr_time.tv_sec * 100.0 + (double)psinfo.pr_time.tv_nsec / 10000000.0;

	timeDiff = newCentStamp - ps->centStamp;
	usDiff = usTime - ps->Time;

	if ((timeDiff > 0.0) && (usDiff >= 0.0))
	{
		ps->Load = (usDiff / timeDiff) * 100.0;
		/* During startup we get bigger loads since the time diff
		* cannot be correct. So we force it to 0. */
		ps->Load = (ps->Load > 100.0) ? 0.0 : ps->Load;
	}
	else
		ps->Load = 0.0;

	ps->centStamp = newCentStamp;
	ps->Time = usTime;

	ps->Size = (psinfo.pr_size * pagesz)/KBYTES;
	ps->RSSize = (psinfo.pr_rssize * pagesz)/KBYTES;

	strncpy(ps->Command,psinfo.pr_fname,PRCOMSIZ);
        ps->Command[PRCOMSIZ-1]='\0';

	strncpy(ps->CmdLine,psinfo.pr_psargs,PRARGSZ);
        ps->CmdLine[PRARGSZ-1]='\0';

	validateStr( ps->Command );
	validateStr( ps->CmdLine );

	ps->alive = 1;
	return( 0 );
}

static void cleanupProcessList( void ) {

	ProcessInfo *ps;

	ProcessCount = 0;
	for( ps = first_ctnr( ProcessList ); ps; ps = next_ctnr( ProcessList )) {
		if( ps->alive ) {
			ps->alive = 0;
			ProcessCount++;
		} else {
			free( remove_ctnr( ProcessList ));
		}
	}
}

void initProcessList( struct SensorModul* sm ) {

	if( (procdir = opendir( PROCDIR )) == NULL ) {
		print_error( "cannot open \"%s\" for reading\n", PROCDIR );
		return;
	}
	pagesz=getpagesize();
	ProcessList = new_ctnr();
	updateProcessList();

	/*
	 *  register the supported monitors & commands
	 */
	registerMonitor( "pscount", "integer",
				printProcessCount, printProcessCountInfo, sm );
	registerMonitor( "ps", "table",
				printProcessList, printProcessListInfo, sm );

	if (!RunAsDaemon)
	{
		registerCommand("kill", killProcess);
		registerCommand("setpriority", setPriority);
	}
}

void exitProcessList( void ) {

	removeMonitor("ps");
	removeMonitor("pscount");

	if (!RunAsDaemon)
	{
		removeCommand("kill");
		removeCommand("setpriority");
	}

	destr_ctnr( ProcessList, free );
}

int updateProcessList( void ) {

	struct dirent	*de;
	struct statfs sf;

	statfs("/proc/pinfo",&sf,sizeof(sf),0);
	ProcessCount = sf.f_files;

	rewinddir( procdir );
	while( (de = readdir( procdir )) != NULL ) {
		/*
		 *  skip '.' and '..'
		 */
		if( de->d_name[0] == '.' )
			continue;

		/*
		 *  fetch the process info and insert it into the info table
		 */
		updateProcess( (pid_t) atol( de->d_name ));
	}
	cleanupProcessList();

	return( 0 );
}

void printProcessListInfo( const char *cmd ) {
	fprintf(CurrentClient, "Name\tPID\tPPID\tGID\tStatus\tUser"
		"\tSize\tResident\t%% CPU\tPriority\tCommand\n" );
	fprintf(CurrentClient, "s\td\td\td\ts\ts\tD\tD\tf\td\ts\n" );
}

void printProcessList( const char *cmd ) {

	ProcessInfo *ps;

	for( ps = first_ctnr( ProcessList ); ps; ps = next_ctnr( ProcessList )) {
		fprintf(CurrentClient,
			"%s\t%ld\t%ld\t%ld\t%s\t%s\t%d\t%d\t%.2f\t%d\t%s\n",
			ps->Command,
			(long) ps->pid,
			(long) ps->ppid,
			(long) ps->gid,
			ps->State,
			ps->userName,
			ps->Size,
			ps->RSSize,
			ps->Load,
			ps->Prio,
			ps->CmdLine);
	}

	fprintf(CurrentClient, "\n");
}

void printProcessCount( const char *cmd ) {
	fprintf(CurrentClient, "%d\n", ProcessCount );
}

void printProcessCountInfo( const char *cmd ) {
	fprintf(CurrentClient, "Number of Processes\t0\t0\t\n" );
}

void killProcess( const char *cmd ) {

	int sig, pid;

	sscanf( cmd, "%*s %d %d", &pid, &sig );

	switch( sig ) {
		case MENU_ID_SIGABRT:
			sig = SIGABRT;
			break;
		case MENU_ID_SIGALRM:
			sig = SIGALRM;
			break;
		case MENU_ID_SIGCHLD:
			sig = SIGCHLD;
			break;
		case MENU_ID_SIGCONT:
			sig = SIGCONT;
			break;
		case MENU_ID_SIGFPE:
			sig = SIGFPE;
			break;
		case MENU_ID_SIGHUP:
			sig = SIGHUP;
			break;
		case MENU_ID_SIGILL:
			sig = SIGILL;
			break;
		case MENU_ID_SIGINT:
			sig = SIGINT;
			break;
		case MENU_ID_SIGKILL:
			sig = SIGKILL;
			break;
		case MENU_ID_SIGPIPE:
			sig = SIGPIPE;
			break;
		case MENU_ID_SIGQUIT:
			sig = SIGQUIT;
			break;
		case MENU_ID_SIGSEGV:
			sig = SIGSEGV;
			break;
		case MENU_ID_SIGSTOP:
			sig = SIGSTOP;
			break;
		case MENU_ID_SIGTERM:
			sig = SIGTERM;
			break;
		case MENU_ID_SIGTSTP:
			sig = SIGTSTP;
			break;
		case MENU_ID_SIGTTIN:
			sig = SIGTTIN;
			break;
		case MENU_ID_SIGTTOU:
			sig = SIGTTOU;
			break;
		case MENU_ID_SIGUSR1:
			sig = SIGUSR1;
			break;
		case MENU_ID_SIGUSR2:
			sig = SIGUSR2;
			break;
	}
	if( kill( (pid_t) pid, sig )) {
		switch( errno ) {
			case EINVAL:
				fprintf(CurrentClient, "4\n" );
				break;
			case ESRCH:
				fprintf(CurrentClient, "3\n" );
				break;
			case EPERM:
				fprintf(CurrentClient, "2\n" );
				break;
			default:
				fprintf(CurrentClient, "1\n" );	/* unknown error */
				break;
		}
	} else
		fprintf(CurrentClient, "0\n");
}

void setPriority( const char *cmd ) {
	int pid, prio;

	sscanf( cmd, "%*s %d %d", &pid, &prio );
	if( setpriority( PRIO_PROCESS, pid, prio )) {
		switch( errno ) {
			case EINVAL:
				fprintf(CurrentClient, "4\n" );
				break;
			case ESRCH:
				fprintf(CurrentClient, "3\n" );
				break;
			case EPERM:
			case EACCES:
				fprintf(CurrentClient, "2\n" );
				break;
			default:
				fprintf(CurrentClient, "1\n" );	/* unknown error */
				break;
		}
	} else
		fprintf(CurrentClient, "0\n");
}
