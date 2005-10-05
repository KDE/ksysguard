/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

	Solaris support by Torsten Kasch <tk@Genetik.Uni-Bielefeld.DE>
    
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

/* Stop <sys/procfs.h> from crapping out on 32-bit architectures. */

#if !defined(_LP64) && _FILE_OFFSET_BITS == 64
# undef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS 32
#endif

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
#include <procfs.h>
#include <sys/proc.h>
#include <sys/resource.h>

#include "ccont.h"
#include "../../gui/SignalIDs.h"
#include "ksysguardd.h"

#include "Command.h"
#include "ProcessList.h"

#define BUFSIZE 1024

typedef struct {
	int	alive;		/*  for "garbage collection"	  */
	pid_t	pid;		/*  process ID			  */
	pid_t	ppid;		/*  parent process ID		  */
	uid_t	uid;		/*  process owner (real UID)	  */
	gid_t	gid;		/*  process group (real GID)	  */
	char	*userName;	/*  process owner (name)	  */
	int	nThreads;	/*  # of threads in this process  */
	int	Prio;		/*  scheduling priority		  */
	size_t	Size;		/*  total size of process image	  */
	size_t	RSSize;		/*  resident set size		  */
	char	*State;		/*  process state		  */
	int	Time;		/*  CPU time for the process	  */
	double	Load;		/*  CPU load in %		  */
	char	*Command;	/*  command name		  */
	char	*CmdLine;	/*  command line		  */
} ProcessInfo;

static CONTAINER ProcessList = 0;
static unsigned ProcessCount = 0;	/*  # of processes	  */
static DIR *procdir;			/*  handle for /proc	  */

/*
 *  lwpStateName()  --  return string representation of process state
 */
char *lwpStateName( lwpsinfo_t lwpinfo ) {

	char	result[8];
	int	processor;

	switch( (int) lwpinfo.pr_state ) {
		case SSLEEP:
			sprintf( result, "%s", "sleep" );
			break;
		case SRUN:
			sprintf( result, "%s", "run" );
			break;
		case SZOMB:
			sprintf( result, "%s", "zombie" );
			break;
		case SSTOP:
			sprintf( result, "%s", "stop" );
			break;
		case SIDL:
			sprintf( result, "%s", "start" );
			break;
		case SONPROC:
			processor = (int) lwpinfo.pr_onpro;
			sprintf( result, "%s/%d", "cpu", processor );
			break;
		default:
			sprintf( result, "%s", "???" );
			break;
	}

	return( strdup( result ));
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
	psinfo_t	psinfo;
	struct passwd	*pw;

	if( (ps = findProcessInList( pid )) == NULL ) {
		if( (ps = (ProcessInfo *) malloc( sizeof( ProcessInfo )))
				== NULL ) {
			print_error( "cannot malloc()\n" );
			return( -1 );
		}
		ps->pid = pid;
		ps->userName = NULL;
		ps->State = NULL;
		ps->Command = NULL;
		ps->CmdLine = NULL;
		ps->alive = 0;

		push_ctnr( ProcessList, ps );
		bsort_ctnr( ProcessList, processCmp );
	}

	snprintf( buf, BUFSIZE - 1, "%s/%ld/psinfo", PROCDIR, pid );
	if( (fd = open( buf, O_RDONLY )) < 0 ) {
		return( -1 );
	}

	if( read( fd, &psinfo, sizeof( psinfo_t )) != sizeof( psinfo_t )) {
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

	if( ps->State != NULL )
		free( ps->State );
	ps->State = lwpStateName( psinfo.pr_lwp );

	/*
	 * the following data is invalid for zombies, so...
	 */
	if( (ps->nThreads = psinfo.pr_nlwp ) != 0 ) {
		ps->Prio = psinfo.pr_lwp.pr_pri;
		ps->Time = psinfo.pr_lwp.pr_time.tv_sec * 100
			+ psinfo.pr_lwp.pr_time.tv_nsec * 10000000;
		ps->Load = (double) psinfo.pr_lwp.pr_pctcpu
			/ (double) 0x8000 * 100.0;
	} else {
		ps->Prio = 0;
		ps->Time = 0;
		ps->Load = 0.0;
	}

	ps->Size = psinfo.pr_size;
	ps->RSSize = psinfo.pr_rssize;

	if( ps->Command != NULL )
		free( ps->Command );
	ps->Command = strdup( psinfo.pr_fname );
	if( ps->CmdLine != NULL )
		free( ps->CmdLine );
	ps->CmdLine = strdup( psinfo.pr_psargs );

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

	ProcessList = new_ctnr();

	/*
	 *  register the supported monitors & commands
	 */
	registerMonitor( "pscount", "integer",
				printProcessCount, printProcessCountInfo, sm );
	registerMonitor( "ps", "table",
				printProcessList, printProcessListInfo, sm );

	registerCommand( "kill", killProcess );
	registerCommand( "setpriority", setPriority );
}

void exitProcessList( void ) {

	destr_ctnr( ProcessList, free );
}

int updateProcessList( void ) {

	struct dirent	*de;

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
	fprintf(CurrentClient, "Name\tPID\tPPID\tGID\tStatus\tUser\tThreads"
		"\tSize\tResident\t%% CPU\tPriority\tCommand\n" );
	fprintf(CurrentClient, "s\td\td\td\ts\ts\td\tD\tD\tf\td\ts\n" );
}

void printProcessList( const char *cmd ) {

	ProcessInfo *ps;

	for( ps = first_ctnr( ProcessList ); ps; ps = next_ctnr( ProcessList )) {
		fprintf(CurrentClient,
			"%s\t%ld\t%ld\t%ld\t%s\t%s\t%d\t%d\t%d\t%.2f\t%d\t%s\n",
			ps->Command,
			(long) ps->pid,
			(long) ps->ppid,
			(long) ps->gid,
			ps->State,
			ps->userName,
			ps->nThreads,
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
