/*  This file is part of the KDE project
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "processes_local_p.h"
#include "process.h"

#include <klocale.h>

#include <QFile>
#include <QHash>
#include <QSet>
#include <QMutableSetIterator>
#include <QByteArray>
#include <QTextStream>

//for sysconf
#include <unistd.h>  
//for kill and setNice
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <dirent.h>


#define PROCESS_BUFFER_SIZE 1000


namespace KSysGuard
{

  class ProcessesLocal::Private
  {
    public:
      Private() { mProcDir = opendir( "/proc" );}
      ~Private() {;}
      inline bool readProcStatus(long pid, Process *process);
      inline bool readProcStat(long pid, Process *process);
      inline bool readProcStatm(long pid, Process *process);
      inline bool readProcCmdline(long pid, Process *process);
      QFile mFile;
      char mBuffer[PROCESS_BUFFER_SIZE+1]; //used as a buffer to read data into      
      DIR* mProcDir;
    };
ProcessesLocal::ProcessesLocal() : d(new Private())
{

}

bool ProcessesLocal::Private::readProcStatus(long pid, Process *process)
{
    mFile.setFileName(QString("/proc/%1/status").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */

    process->uid = 0;
    process->gid = 0;
    process->tracerpid = 0;

    int size;
    int found = 0; //count how many fields we found
    while( (size = mFile.readLine( mBuffer, sizeof(mBuffer))) > 0) {  //-1 indicates an error
        switch( mBuffer[0]) {
	  case 'N':
	    if((unsigned int)size > sizeof("Name:") && qstrncmp(mBuffer, "Name:", sizeof("Name:")-1) == 0) {
                process->name = QString::fromLocal8Bit(mBuffer + sizeof("Name:")-1, size-sizeof("Name:")+1).trimmed();
	        if(++found == 4) goto finish;
	    }
	    break;
	  case 'U': 
	    if((unsigned int)size > sizeof("Uid:") && qstrncmp(mBuffer, "Uid:", sizeof("Uid:")-1) == 0) {
		sscanf(mBuffer + sizeof("Uid:") -1, "%ld %ld %ld %ld", &process->uid, &process->euid, &process->suid, &process->fsuid );
	        if(++found == 4) goto finish;
	    }
	    break;
	  case 'G':
	    if((unsigned int)size > sizeof("Gid:") && qstrncmp(mBuffer, "Gid:", sizeof("Gid:")-1) == 0) {
		sscanf(mBuffer + sizeof("Gid:")-1, "%ld %ld %ld %ld", &process->gid, &process->egid, &process->sgid, &process->fsgid );
	        if(++found == 4) goto finish;
	    }
	    break;
	  case 'T':
	    if((unsigned int)size > sizeof("TracerPid:") && qstrncmp(mBuffer, "TracerPid:", sizeof("TracerPid:")-1) == 0) {
		process->uid = atol(mBuffer + sizeof("TracerPid:")-1);
	        if(++found == 4) goto finish;
	    }
	    break;
	  default:
	    break;
	}
    }

    finish:
    mFile.close();
    return true;
}

long ProcessesLocal::getParentPid(long pid) {
    Q_ASSERT(pid != 0);
    d->mFile.setFileName(QString("/proc/%1/stat").arg(pid));
    if(!d->mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;      /* process has terminated in the meantime */

    int size; //amount of data read in
    if( (size = d->mFile.readLine( d->mBuffer, sizeof(d->mBuffer))) <= 0) { //-1 indicates nothing read
        d->mFile.close();
        return 0;
    }

    d->mFile.close();
    int current_word = 0;
    char *word = d->mBuffer;

    while(true) {
	    if(word[0] == ' ' ) {
		    if(++current_word == 3)
			    break;
	    } else if(word[0] == 0) {
	    	return 0; //end of data - serious problem
	    }
	    word++;
    }
    return atol(++word);
}

bool ProcessesLocal::Private::readProcStat(long pid, Process *ps)
{
    QString filename = QString("/proc/%1/stat").arg(pid);
    // As an optomization, if the last file read in was stat, then we already have this info in memory
    if(mFile.fileName() != filename) {  
        mFile.setFileName(filename);
        if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;      /* process has terminated in the meantime */
	if( mFile.readLine( mBuffer, sizeof(mBuffer)) <= 0) { //-1 indicates nothing read
	    mFile.close();
	    return false;
	}
	mFile.close();
    }

    int current_word = 0;  //count from 0
    char *word = mBuffer;
    char status='\0';
    while(current_word < 23) {
	    if(word[0] == ' ' ) {
		    ++current_word;
		    switch(current_word) {
			    case 2: //status
                              status=word[1];  // Look at the first letter of the status.  
			                      // We analyze this after the while loop
			      break;
			    case 6: //ttyNo
			      {
			        int ttyNo = atoi(word+1);
				int major = ttyNo >> 8;
				int minor = ttyNo & 0xff;
				switch(major) {
				  case 136:
				    ps->tty = QByteArray("pts/") + QByteArray::number(minor);
				    break;
				  case 5:
				    ps->tty = QByteArray("tty");
				  case 4:
				    if(minor < 64)
				      ps->tty = QByteArray("tty") + QByteArray::number(minor);
				    else
				      ps->tty = QByteArray("ttyS") + QByteArray::number(minor-64);
				    break;
				  default:
				    ps->tty = QByteArray();
				}
			      }
			      break;
			    case 13: //userTime
			      ps->userTime = atoll(word+1);
			      break;
			    case 14: //sysTime
			      ps->sysTime = atoll(word+1);
			      break;
			    case 18: //niceLevel
			      ps->niceLevel = atoi(word+1);
			      break;
			    case 22: //vmSize
			      ps->vmSize = atol(word+1);
			      break;
			    case 23: //vmRSS
			      ps->vmRSS = atol(word+1);
			      break;
			    default:
			      break;
		    }
	    } else if(word[0] == 0) {
	    	return false; //end of data - serious problem
	    }
	    word++;
    }

    /* There was a "(ps->vmRss+3) * sysconf(_SC_PAGESIZE)" here in the original ksysguard code.  I have no idea why!  After comparing it to
     *   meminfo and other tools, this means we report the RSS by 12 bytes differently compared to them.  So I'm removing the +3
     *   to be consistent.  NEXT TIME COMMENT STRANGE THINGS LIKE THAT! :-) */
    ps->vmRSS = ps->vmRSS * sysconf(_SC_PAGESIZE) / 1024; /*convert to KiB*/
    ps->vmSize /= 1024; /* convert to KiB */

    switch( status) {
      case 'R':
        ps->status = Process::Running;
	break;
      case 'S':
        ps->status = Process::Sleeping;
	break;
      case 'D':
        ps->status = Process::DiskSleep;
	break;
      case 'Z':
        ps->status = Process::Zombie;
	break;
      case 'T':
         ps->status = Process::Stopped;
         break;
      case 'W':
         ps->status = Process::Paging;
         break;
      default:
         ps->status = Process::OtherStatus;
         break;
    }
    return true;
}

bool ProcessesLocal::Private::readProcStatm(long pid, Process *process)
{
    mFile.setFileName(QString("/proc/%1/statm").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */

    if( mFile.readLine( mBuffer, sizeof(mBuffer)) <= 0) { //-1 indicates nothing read
        mFile.close();
        return 0;
    }
    mFile.close();

    int current_word = 0;
    char *word = mBuffer;

    while(true) {
	    if(word[0] == ' ' ) {
		    if(++current_word == 2) //number of pages that are shared
			    break;
	    } else if(word[0] == 0) {
	    	return false; //end of data - serious problem
	    }
	    word++;
    }
    long shared = atol(word+1);

    /* we use the rss - shared  to find the amount of memory just this app uses */
    process->vmURSS = process->vmRSS - (shared * sysconf(_SC_PAGESIZE) / 1024);
    return true;
}


bool ProcessesLocal::Private::readProcCmdline(long pid, Process *process)
{
    if(!process->command.isNull()) return true; //only parse the cmdline once
    mFile.setFileName(QString("/proc/%1/cmdline").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */

    QTextStream in(&mFile);
    process->command = in.readAll();

    //cmdline seperates parameters with the NULL character
    process->command.replace('\0', ' ');
    process->command = process->command.trimmed();

    mFile.close();
    return true;
}
bool ProcessesLocal::updateProcessInfo( long pid, Process *process)
{
    if(!d->readProcStat(pid, process)) return false;
    if(!d->readProcStatus(pid, process)) return false;
    if(!d->readProcStatm(pid, process)) return false;
    if(!d->readProcCmdline(pid, process)) return false;

    return true;
}

QSet<long> ProcessesLocal::getAllPids( )
{
    QSet<long> pids;
    if(d->mProcDir==NULL) return pids; //There's not much we can do without /proc
    struct dirent* entry;
    rewinddir(d->mProcDir);
    while ( ( entry = readdir( d->mProcDir ) ) )
	    if ( entry->d_name[ 0 ] >= '0' && entry->d_name[ 0 ] <= '9' )
		    pids.insert(atol( entry->d_name ));
    return pids;
}

bool ProcessesLocal::sendSignal(long pid, int sig) {
    if ( kill( (pid_t)pid, sig ) ) {
	//Kill failed
        return false;
    }
    return true;
}
bool ProcessesLocal::setNiceness(long pid, int priority) {
    if ( setpriority( PRIO_PROCESS, pid, priority ) ) {
	    //set niceness failed
	    return false;
    }
    return true;
}


ProcessesLocal::~ProcessesLocal()
{
  
}

}
