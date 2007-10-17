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

#include "processes.h"
#include "processes_base_p.h"
#include "processes_local_p.h"
#include "processes_remote_p.h"
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

/* if porting to an OS without signal.h  please #define SIGTERM to something */
#include <signal.h>


namespace KSysGuard
{
  Processes::StaticPrivate *Processes::d2 = 0;

  class Processes::Private
  {
    public:
      Private() { mAbstractProcesses = 0;  mProcesses.insert(0, &mFakeProcess); mElapsedTimeCentiSeconds = -1; }
      ~Private() {;}

      QSet<long> mToBeProcessed;
      QSet<long> mProcessedLastTime;

      QHash<long, Process *> mProcesses; //This must include mFakeProcess at pid 0
      QList<Process *> mListProcesses;   //A list of the processes.  Does not include mFakeProcesses
      Process mFakeProcess; //A fake process with pid 0 just so that even init points to a parent

      AbstractProcesses *mAbstractProcesses; //The OS specific code to get the process information 
      QTime mLastUpdated; //This is the time we last updated.  Used to calculate cpu usage.
      long mElapsedTimeCentiSeconds; //The number of centiseconds  (100ths of a second) that passed since the last update
  };

  class Processes::StaticPrivate
  {
    public:
      StaticPrivate() { processesLocal = 0;}
      Processes *processesLocal;
      QHash<QString, Processes*> processesRemote;
      
  };


Processes *Processes::getInstance(const QString &host) { //static
    if(!d2) {
        d2 = new StaticPrivate();
    }
    if(host.isEmpty()) {
        //Localhost processes
        if(!d2->processesLocal) {
            d2->processesLocal = new Processes(new ProcessesLocal());
        }
	return d2->processesLocal;
    } else {
        Processes *processes = d2->processesRemote.value(host, NULL);
        if( !processes ) {
            //connect to it
	    processes = new Processes( new ProcessesRemote(host) );
            d2->processesRemote.insert(host, processes);
	}
	return processes;
    }
}

void Processes::returnInstance(const QString &/*host*/) { //static
	//Implement - we need reference counting etc
}
Processes::Processes(AbstractProcesses *abstractProcesses) : d(new Private())
{
    d->mAbstractProcesses = abstractProcesses;
}

Process *Processes::getProcess(long pid) const
{
	return d->mProcesses.value(pid);
}
	
QList<Process *> Processes::getAllProcesses() const
{
	return d->mListProcesses;
}
bool Processes::updateProcess( Process *ps, long ppid, bool onlyReparent)
{
    Process *parent = d->mProcesses.value(ppid);
    Q_ASSERT(parent);  //even init has a non-null parent - the mFakeProcess

    if(ps->parent != parent) {
        emit beginMoveProcess(ps, parent/*new parent*/);
        //Processes has been reparented
        Process *p = ps;
        do {
            p = p->parent;
            p->numChildren--;
        } while (p->pid!= 0);
        ps->parent->children.removeAll(ps);
        ps->parent = parent;  //the parent has changed
        parent->children.append(ps);
        p = ps;
        do {
            p = p->parent;
            p->numChildren++;
        } while (p->pid!= 0);
	emit endMoveProcess();
    }
    if(onlyReparent) 
	    return true; 

    ps->parent = parent;
    ps->parent_pid = ppid;

    //Now we can actually get the process info
    Process old_process(*ps);
    bool success = d->mAbstractProcesses->updateProcessInfo(ps->pid, ps);

    //Now we have the process info.  Calculate the cpu usage and total cpu usage for itself and all its parents
    if(old_process.userTime != -1 && d->mElapsedTimeCentiSeconds!= 0) {  //Update the user usage and sys usage
        ps->userUsage = (int)(((ps->userTime - old_process.userTime)*100.0 + 0.5) / d->mElapsedTimeCentiSeconds);
        ps->sysUsage  = (int)(((ps->sysTime - old_process.sysTime)*100.0 + 0.5) / d->mElapsedTimeCentiSeconds);
        ps->totalUserUsage = ps->userUsage;
	ps->totalSysUsage = ps->sysUsage;
	if(ps->userUsage != 0 || ps->sysUsage != 0) {
	    Process *p = ps->parent;
	    while(p->pid != 0) {
	        p->totalUserUsage += ps->userUsage;
	        p->totalSysUsage += ps->sysUsage;
                emit processChanged(p, true);
	        p= p->parent;
	    }
	}
    }

    if(
       ps->name != old_process.name ||
       ps->command != old_process.command ||
       ps->status != old_process.status ||
       ps->uid != old_process.uid ) {

       emit processChanged(ps, false);

    } else if(
       ps->vmSize != old_process.vmSize ||
       ps->vmRSS != old_process.vmRSS ||
       ps->vmURSS != old_process.vmURSS ||
       ps->userUsage != old_process.userUsage ||
       ps->sysUsage != old_process.sysUsage ||
       ps->totalUserUsage != old_process.totalUserUsage ||
       ps->totalSysUsage != old_process.totalSysUsage ) {

       emit processChanged(ps, true);
    }

    return success;

}
bool Processes::addProcess(long pid, long ppid)
{
    Process *parent = d->mProcesses.value(ppid);
    Q_ASSERT(parent);  //even init has a non-null parent - the mFakeProcess
    //it's a new process - we need to set it up
    Process *ps = new Process(pid, ppid, parent);

    emit beginAddProcess(ps);

    d->mProcesses.insert(pid, ps);
    
    ps->index = d->mListProcesses.count();
    d->mListProcesses.append(ps);

    ps->parent->children.append(ps);
    Process *p = ps;
    do {
        p = p->parent;
        p->numChildren++;
    } while (p->pid!= 0);
    ps->parent_pid = ppid;

    //Now we can actually get the process info
    bool success = d->mAbstractProcesses->updateProcessInfo(pid, ps);
    emit endAddProcess();
    return success;

}
bool Processes::updateOrAddProcess( long pid)
{
    long ppid = d->mAbstractProcesses->getParentPid(pid);

    if(d->mToBeProcessed.contains(ppid)) {
        //Make sure that we update the parent before we update this one.  Just makes things a bit easier.
        d->mToBeProcessed.remove(ppid);
        d->mProcessedLastTime.remove(ppid); //It may or may not be here - remove it if it is there
        updateOrAddProcess(ppid);
    }

    Process *ps = d->mProcesses.value(pid, 0);
    if(!ps) 
        return addProcess(pid, ppid);
    else 
	return updateProcess(ps, ppid);
}

void Processes::updateAllProcesses( long updateDurationMS )
{
    if(d->mElapsedTimeCentiSeconds == -1) {
        //First time update has been called
        d->mLastUpdated.start();
	d->mElapsedTimeCentiSeconds = 0;
    } else {
        if(d->mLastUpdated.elapsed() < updateDurationMS) //don't update more often than the time given
		return;
        d->mElapsedTimeCentiSeconds = d->mLastUpdated.restart() / 10;
    }

    d->mToBeProcessed = d->mAbstractProcesses->getAllPids();

    QSet<long> beingProcessed(d->mToBeProcessed); //keep a copy so that we can replace mProcessedLastTime with this at the end of this function

    long pid;
    {
      QMutableSetIterator<long> i(d->mToBeProcessed);
      while( i.hasNext()) {
          pid = i.next();
          i.remove();
          d->mProcessedLastTime.remove(pid); //It may or may not be here - remove it if it is there
          updateOrAddProcess(pid);  //This adds the process or changes an extisting one
	  i.toFront(); //we can remove entries from this set elsewhere, so our iterator might be invalid.  reset it back to the start of the set
      }
    }
    {
      QMutableSetIterator<long> i(d->mProcessedLastTime);
      while( i.hasNext()) {
          //We saw these pids last time, but not this time.  That means we have to delete them now
          pid = i.next();
	  i.remove();
          deleteProcess(pid);
	  i.toFront();
      }
    }
    
    d->mProcessedLastTime = beingProcessed;  //update the set for next time this function is called 
    return;
}


void Processes::deleteProcess(long pid)
{
    Q_ASSERT(pid > 0);

    Process *process = d->mProcesses.value(pid);
    foreach( Process *it, process->children) {
        d->mProcessedLastTime.remove(it->pid);
	deleteProcess(it->pid);
    }

    emit beginRemoveProcess(process);

    d->mProcesses.remove(pid);
    d->mListProcesses.removeAll(process);
    process->parent->children.removeAll(process);
    Process *p = process;
    do {
      p = p->parent;
      p->numChildren--;
    } while (p->pid!= 0);

    int i=0;
    foreach( Process *it, d->mListProcesses ) {
	if(it->index > process->index)
	    	it->index--;
	Q_ASSERT(it->index == i++);
    }

    delete process;
    emit endRemoveProcess();
}


bool Processes::killProcess(long pid) {
  return sendSignal(pid, SIGTERM);
}

bool Processes::sendSignal(long pid, int sig) {
    return d->mAbstractProcesses->sendSignal(pid, sig);
}

bool Processes::setNiceness(long pid, int priority) {
    return d->mAbstractProcesses->setNiceness(pid, priority);
}

bool Processes::setIoNiceness(long pid, KSysGuard::Process::IoPriorityClass priorityClass, int priority) {
    return d->mAbstractProcesses->setIoNiceness(pid, priorityClass, priority);
}

bool Processes::supportsIoNiceness() { 
    return d->mAbstractProcesses->supportsIoNiceness();
}

long long Processes::totalPhysicalMemory() {
    return d->mAbstractProcesses->totalPhysicalMemory();
}
Processes::~Processes()
{

}

}
#include "processes.moc"
