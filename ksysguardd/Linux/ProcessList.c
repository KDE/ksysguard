/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include "../../gui/SignalIDs.h"
#include "Command.h"
#include "PWUIDCache.h"
#include "ccont.h"
#include "ksysguardd.h"

#include "ProcessList.h"

#define BUFSIZE 1024
#define TAGSIZE 32
#define KDEINITLEN strlen( "kdeinit: " )

static CONTAINER ProcessList = 0;

#include "config.h" /*For HAVE_XRES*/
#ifdef HAVE_XRES
extern int setup_xres();
extern void xrestop_populate_client_data();
static int have_xres = 0;
#endif

typedef struct {
  /**
    This flag is set for all found processes at the beginning of the
    process list update. Processes that do not have this flag set will
    be assumed dead and removed from the list. The flag is cleared after
    each list update.
   */
  int alive;

  /* The process ID */
  pid_t pid;

  /* The parent process ID */
  pid_t ppid;

  /* The real user ID */
  uid_t uid;

  /* The real group ID */
  gid_t gid;

  /* The process ID of any application that is debugging this one. 0 if none */
  pid_t tracerpid;

  /* A character description of the process status */
  char status[ 16 ];

  /* The number of the tty the process owns */
  int ttyNo;

	/**
    The nice level. The range should be -20 to 20. I'm not sure
    whether this is true for all platforms.
   */
  int niceLevel;

	/* The scheduling priority. */
  int priority;

	/**
    The total amount of memory the process uses. This includes shared and
    swapped memory.
   */
  unsigned int vmSize;

	/* The amount of physical memory the process currently uses. */
  unsigned int vmRss;

	/**
    The number of 1/100 of a second the process has spend in user space.
    If a machine has an uptime of 1 1/2 years or longer this is not a
    good idea. I never thought that the stability of UNIX could get me
    into trouble! ;)
   */
  unsigned int userTime;

	/**
    The number of 1/100 of a second the process has spend in system space.
    If a machine has an uptime of 1 1/2 years or longer this is not a
    good idea. I never thought that the stability of UNIX could get me
    into trouble! ;)
   */
  unsigned int sysTime;

  /* The system time as multime of 100ms */
  int centStamp;

  /* The current CPU load (in %) from user space */
  double userLoad;

  /* The current CPU load (in %) from system space */
  double sysLoad;

  /* The name of the process */
  char name[ 64 ];

  /* The command used to start the process */
  char cmdline[ 256 ];

  /* The login name of the user that owns this process */
  char userName[ 32 ];

} ProcessInfo;

static unsigned ProcessCount;

static void validateStr( char* str )
{
  char* s = str;

  /* All characters that could screw up the communication will be removed. */
  while ( *s ) {
    if ( *s == '\t' || *s == '\n' || *s == '\r' )
      *s = ' ';
    ++s;
  }

  /* Make sure that string contains at least one character (blank). */
  if ( str[ 0 ] == '\0' )
    strcpy( str, " " );
}

static int processCmp( void* p1, void* p2 )
{
  return ( ((ProcessInfo*)p1)->pid - ((ProcessInfo*)p2)->pid );
}

static ProcessInfo* findProcessInList( int pid )
{
  ProcessInfo key;
  long idx;

  key.pid = pid;
  if ( ( idx = search_ctnr( ProcessList, processCmp, &key ) ) < 0 )
    return 0;

  return get_ctnr( ProcessList, idx );
}

static int updateProcess( int pid )
{
  ProcessInfo* ps;
  FILE* fd;
  char buf[ BUFSIZE ];
  char tag[ TAGSIZE ];
  char format[ 32 ];
  char tagformat[ 32 ];
  int userTime, sysTime;
  const char* uName;
  char status;

  if ( ( ps = findProcessInList( pid ) ) == 0 ) {
    struct timeval tv;

    ps = (ProcessInfo*)malloc( sizeof( ProcessInfo ) );
    ps->pid = pid;
    ps->alive = 0;

    gettimeofday( &tv, 0 );
    ps->centStamp = tv.tv_sec * 100 + tv.tv_usec / 10000;

    push_ctnr( ProcessList, ps );
    bsort_ctnr( ProcessList, processCmp );
  }

  snprintf( buf, BUFSIZE - 1, "/proc/%d/status", pid );
  if ( ( fd = fopen( buf, "r" ) ) == 0 ) {
    /* process has terminated in the mean time */
    return -1;
  }
  ps->uid = 0;
  ps->gid = 0;
  ps->tracerpid = 0;
  
  sprintf( format, "%%%d[^\n]\n", (int)sizeof( buf ) - 1 );
  sprintf( tagformat, "%%%ds", (int)sizeof( tag ) - 1 );
  for ( ;; ) {
    if ( fscanf( fd, format, buf ) != 1 )
      break;
    buf[ sizeof( buf ) - 1 ] = '\0';
    sscanf( buf, tagformat, tag );
    tag[ sizeof( tag ) - 1 ] = '\0';
    if ( strcmp( tag, "Name:" ) == 0 ) {
      sscanf( buf, "%*s %63s", ps->name );
      validateStr( ps->name );
    } else if ( strcmp( tag, "Uid:" ) == 0 ) {
      sscanf( buf, "%*s %d %*d %*d %*d", (int*)&ps->uid );
    } else if ( strcmp( tag, "Gid:" ) == 0 ) {
      sscanf( buf, "%*s %d %*d %*d %*d", (int*)&ps->gid );
    } else if ( strcmp( tag, "TracerPid:" ) == 0 ) {
      sscanf( buf, "%*s %d", (int*)&ps->tracerpid );
    }
  }

  if ( fclose( fd ) )
    return -1;

  snprintf( buf, BUFSIZE - 1, "/proc/%d/stat", pid );
  buf[ BUFSIZE - 1 ] = '\0';
  if ( ( fd = fopen( buf, "r" ) ) == 0 )
    return -1;

  if ( fscanf( fd, "%*d %*s %c %d %*d %*d %d %*d %*u %*u %*u %*u %*u %d %d"
                   "%*d %*d %*d %d %*u %*u %*d %u %u",
                   &status, (int*)&ps->ppid, &ps->ttyNo,
                   &userTime, &sysTime, &ps->niceLevel, &ps->vmSize,
                   &ps->vmRss) != 8 ) {
    fclose( fd );
    return -1;
  }

  if ( fclose( fd ) )
    return -1;

  /* status decoding as taken from fs/proc/array.c */
  if ( status == 'R' )
    strcpy( ps->status, "running" );
  else if ( status == 'S' )
    strcpy( ps->status, "sleeping" );
  else if ( status == 'D' )
    strcpy( ps->status, "disk sleep" );
  else if ( status == 'Z' )
    strcpy( ps->status, "zombie" );
  else if ( status == 'T' )
    strcpy( ps->status, "stopped" );
  else if ( status == 'W' )
    strcpy( ps->status, "paging" );
  else
    sprintf( ps->status, "Unknown: %c", status );

  ps->vmRss = ( ps->vmRss + 3 ) * sysconf(_SC_PAGESIZE);

  {
    int newCentStamp;
    int timeDiff, userDiff, sysDiff;
    struct timeval tv;

    gettimeofday( &tv, 0 );
    newCentStamp = tv.tv_sec * 100 + tv.tv_usec / 10000;

    timeDiff = newCentStamp - ps->centStamp;
    userDiff = userTime - ps->userTime;
    sysDiff = sysTime - ps->sysTime;

    if ( ( timeDiff > 0 ) && ( userDiff >= 0 ) && ( sysDiff >= 0 ) ) {
      ps->userLoad = ( (double)userDiff / timeDiff ) * 100.0;
      ps->sysLoad = ( (double)sysDiff / timeDiff ) * 100.0;
      /**
        During startup we get bigger loads since the time diff
        cannot be correct. So we force it to 0.
       */
      if ( ps->userLoad > 100.0 )
        ps->userLoad = 0.0;
      if ( ps->sysLoad > 100.0 )
        ps->sysLoad = 0.0;
    } else
      ps->sysLoad = ps->userLoad = 0.0;

    ps->centStamp = newCentStamp;
    ps->userTime = userTime;
    ps->sysTime = sysTime;
  }

  snprintf( buf, BUFSIZE - 1, "/proc/%d/cmdline", pid );
  if ( ( fd = fopen( buf, "r" ) ) == 0 )
    return -1;

  ps->cmdline[ 0 ] = '\0';
  sprintf( buf, "%%%d[^\n]", (int)sizeof( ps->cmdline ) - 1 );
  fscanf( fd, buf, ps->cmdline );
  ps->cmdline[ sizeof( ps->cmdline ) - 1 ] = '\0';
  validateStr( ps->cmdline );
  if ( fclose( fd ) )
    return -1;

  /* Ugly hack to "fix" program name for kdeinit launched programs. */
  if ( strcmp( ps->name, "kdeinit" ) == 0 &&
       strncmp( ps->cmdline, "kdeinit: ", KDEINITLEN ) == 0 &&
       strcmp( ps->cmdline + KDEINITLEN, "Running..." ) != 0 ) {
    size_t len;
    char* end = strchr( ps->cmdline + KDEINITLEN, ' ' );
    if ( end )
      len = ( end - ps->cmdline ) - KDEINITLEN;
    else
      len = strlen( ps->cmdline + KDEINITLEN );
    if ( len > 0 ) {
      if ( len > sizeof( ps->name ) - 1 )
        len = sizeof( ps->name ) - 1;
      strncpy( ps->name, ps->cmdline + KDEINITLEN, len );
      ps->name[ len ] = '\0';
    }
  }

  /* find out user name with the process uid */
  uName = getCachedPWUID( ps->uid );
  strncpy( ps->userName, uName, sizeof( ps->userName ) - 1 );
  ps->userName[ sizeof( ps->userName ) - 1 ] = '\0';
  validateStr( ps->userName );
  
  ps->alive = 1;


  

  return 0;
}

static void cleanupProcessList( void )
{
  ProcessInfo* ps;

  ProcessCount = 0;
  /**
    All processes that do not have the active flag set are assumed dead
    and will be removed from the list. The alive flag is cleared.
   */
  for ( ps = first_ctnr( ProcessList ); ps; ps = next_ctnr( ProcessList ) ) {
    if ( ps->alive ) {
      /* Process is still alive. Just clear flag. */
      ps->alive = 0;
      ProcessCount++;
    } else {
      /**
        Process has probably died. We remove it from the list and
        destruct the data structure. i needs to be decremented so
        that after i++ the next list element will be inspected.
       */
      free( remove_ctnr( ProcessList ) );
    }
  }
}

int updateProcessList( void )
{
  DIR* dir;
  struct dirent* entry;

  /* read in current process list via the /proc filesystem entry */
  if ( ( dir = opendir( "/proc" ) ) == NULL ) {
    print_error( "Cannot open directory \'/proc\'!\n"
                 "The kernel needs to be compiled with support\n"
                 "for /proc filesystem enabled!\n" );
    return 0;
  }

  while ( ( entry = readdir( dir ) ) ) {
    if ( isdigit( entry->d_name[ 0 ] ) ) {
      int pid;

      pid = atoi( entry->d_name );
      updateProcess( pid ); 
    }
  }
  closedir( dir );

  cleanupProcessList();
  return 0;
}

/*
================================ public part =================================
*/

void initProcessList( struct SensorModul* sm )
{
  initPWUIDCache();

  ProcessList = new_ctnr();

  registerMonitor( "pscount", "integer", printProcessCount, printProcessCountInfo, sm );
  registerMonitor( "ps", "table", printProcessList, printProcessListInfo, sm );
#ifdef HAVE_XRES
  registerMonitor( "xres", "table", printXresList, printXresListInfo, sm);
#endif

  if ( !RunAsDaemon ) {
    registerCommand( "kill", killProcess );
    registerCommand( "setpriority", setPriority );
  }

#ifdef HAVE_XRES
  have_xres = setup_xres();
#endif

  updateProcessList();
}

void exitProcessList( void )
{
  removeMonitor( "ps" );
  removeMonitor( "pscount" );

#ifdef HAVE_XRES
  removeMonitor( "xres" );
#endif
  if ( !RunAsDaemon ) {
    removeCommand( "kill" );
    removeCommand( "setpriority" );
  }

  destr_ctnr( ProcessList, free );

  exitPWUIDCache();
}
#ifdef HAVE_XRES
void printXresListInfo( const char *cmd)
{
  (void)cmd;
  fprintf(CurrentClient, "XPid\tXIdentifier\tXPxmMem\tXNumPxm\tXNumOther\n");
  fprintf(CurrentClient, "d\ts\tD\td\tD\n");
}

void printXresList(const char*cmd)
{
  (void)cmd;
  printXres(CurrentClient);
}

#endif

void printProcessListInfo( const char* cmd )
{
  (void)cmd;
  fprintf( CurrentClient, "Name\tPID\tPPID\tUID\tGID\tStatus\tUser%%\tSystem%%\tNice\tVmSize"
                          "\tVmRss\tLogin\tTracerPID\tCommand\n" );
  fprintf( CurrentClient, "s\td\td\td\td\tS\tf\tf\td\tD\tD\ts\td\ts\n" );
}

void printProcessList( const char* cmd )
{
  ProcessInfo* ps;

  (void)cmd;
  /*We return 0 for tracerpid (the pid of any debugger attached to the process) if there is none
    x_identifier is empty if none was found
    x_pxmmem is -1 if not found
    x_numpxm is -1 if not found
    */
  for ( ps = first_ctnr( ProcessList ); ps; ps = next_ctnr( ProcessList ) ) {
    fprintf( CurrentClient, "%s\t%ld\t%ld\t%ld\t%ld\t%s\t%.2f\t%.2f\t%d\t%d\t%d\t%s\t%ld\t%s\n",
	     ps->name, (long)ps->pid, (long)ps->ppid,
             (long)ps->uid, (long)ps->gid, ps->status, ps->userLoad,
             ps->sysLoad, ps->niceLevel, ps->vmSize / 1024, ps->vmRss / 1024,
             ps->userName, (long)ps->tracerpid, ps->cmdline
	     );
  }

  fprintf( CurrentClient, "\n" );
}

void printProcessCount( const char* cmd )
{
  (void)cmd;
  fprintf( CurrentClient, "%d\n", ProcessCount );
}

void printProcessCountInfo( const char* cmd )
{
  (void)cmd;
  fprintf( CurrentClient, "Number of Processes\t0\t0\t\n" );
}

void killProcess( const char* cmd )
{
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

  if ( kill( (pid_t)pid, sig ) ) {
    switch ( errno ) {
      case EINVAL:
        fprintf( CurrentClient, "4\t%d\n", pid );
        break;
      case ESRCH:
        fprintf( CurrentClient, "3\t%d\n", pid );
        break;
      case EPERM:
	if(vfork() == 0) {
	  exit(0);/* Won't execute unless execve fails.  Need this for the parent process to continue */
	}
        fprintf( CurrentClient, "2\t%d\n", pid );
        break;
      default: /* unknown error */
        fprintf( CurrentClient, "1\t%d\n", pid );
        break;
    }
  } else
    fprintf( CurrentClient, "0\t%d\n", pid );
}

void setPriority( const char* cmd )
{
  int pid, prio;

  sscanf( cmd, "%*s %d %d", &pid, &prio );
  if ( setpriority( PRIO_PROCESS, pid, prio ) ) {
    switch ( errno ) {
      case EINVAL:
        fprintf( CurrentClient, "4\n" );
        break;
      case ESRCH:
        fprintf( CurrentClient, "3\n" );
        break;
      case EPERM:
      case EACCES:
        fprintf( CurrentClient, "2\n" );
        break;
      default: /* unknown error */
        fprintf( CurrentClient, "1\n" );
        break;
    }
  } else
    fprintf( CurrentClient, "0\n" );
}
