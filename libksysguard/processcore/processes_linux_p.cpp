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
#include <QDir>
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




namespace KSysGuard
{

  class ProcessesLocal::Private
  {
    public:
      Private() {;}
      ~Private() {;}
      inline bool readProcStatus(long pid, Process *process);
      inline bool readProcStat(long pid, Process *process);
      inline bool readProcStatm(long pid, Process *process);
      inline bool readProcCmdline(long pid, Process *process);
      QFile mFile;
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

    QTextStream in(&mFile);
    QString line = in.readLine();
    while (!line.isNull()) {
	if(line.startsWith( "Name:")) {
		process->name = line.section('\t', 1,1, QString::SectionSkipEmpty);
	} else if( line.startsWith("Uid:")) {
		process->uid = line.section('\t', 1,1, QString::SectionSkipEmpty).toLongLong();
	} else if( line.startsWith("Gid:")) {
		process->gid = line.section('\t', 1,1, QString::SectionSkipEmpty).toLongLong();
	} else if( line.startsWith("TracerPid:")) {
		process->tracerpid = line.section('\t', 1,1, QString::SectionSkipEmpty).toLongLong();
	}
        line = in.readLine();
    }

    mFile.close();
    return true;
}

long ProcessesLocal::getParentPid(long pid) {
    Q_ASSERT(pid != 0);
    d->mFile.setFileName(QString("/proc/%1/stat").arg(pid));
    if(!d->mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;      /* process has terminated in the meantime */

    QTextStream in(&d->mFile);

    QByteArray ignore;
    long long ppid = 0;
    in >> ignore >> ignore >> ignore >> ppid;
    d->mFile.close();
    return ppid;
}

bool ProcessesLocal::Private::readProcStat(long pid, Process *ps)
{
    mFile.setFileName(QString("/proc/%1/stat").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */
    QTextStream in(&mFile);

    QByteArray ignore;
    QByteArray status;
    in >> ignore >> ignore >> status >> ignore;
    in >> ignore >> ignore >> ignore /*ttyNo*/ >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore;
    in >> ps->userTime >> ps->sysTime >> ignore >> ignore >> ignore >> ps->niceLevel >> ignore >> ignore >> ignore >> ps->vmSize >> ps->vmRSS;


    /* There was a "(ps->vmRss+3) * sysconf(_SC_PAGESIZE)" here in the original ksysguard code.  I have no idea why!  After comparing it to
     *   meminfo and other tools, this means we report the RSS by 12 bytes differently compared to them.  So I'm removing the +3
     *   to be consistent.  NEXT TIME COMMENT STRANGE THINGS LIKE THAT! :-) */
    ps->vmRSS = ps->vmRSS * sysconf(_SC_PAGESIZE) / 1024; /*convert to KiB*/
    ps->vmSize /= 1024; /* convert to KiB */

    if(in.status() != QTextStream::Ok) {
        mFile.close();
	return false;  //something went horribly wrong
    }

    switch( status[0]) {
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

    mFile.close();
    return true;
}

bool ProcessesLocal::Private::readProcStatm(long pid, Process *process)
{
    mFile.setFileName(QString("/proc/%1/statm").arg(pid));
    if(!mFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;      /* process has terminated in the meantime */

    QTextStream in(&mFile);

    QByteArray ignore;
    unsigned long shared;
    in >> ignore >> ignore >> shared;
    if(in.status() != QTextStream::Ok) {
        mFile.close();
        return false;  //something went horribly wrong
    }
    
    /* we use the rss - shared  to find the amount of memory just this app uses */
    process->vmURSS = process->vmRSS - (shared * sysconf(_SC_PAGESIZE) / 1024);

    mFile.close();
    return true;
}


bool ProcessesLocal::Private::readProcCmdline(long pid, Process *process)
{
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
    QDir dir("/proc");
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Unsorted);

    long pid;
    bool ok;

    QStringList list = dir.entryList();
    for (int i = 0; i < list.size(); ++i) {
        pid = list.at(i).toLong(&ok);
        if(ok)  //we have indeed read in a number rather than some other folder
            pids.insert(pid);
    }
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
