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

  class Processes::Private
  {
    public:
      Private() { processesBase = 0;  mProcesses.insert(0, &mFakeProcess); mFlatMode = true; }
      ~Private() {;}

      QSet<long> mToBeProcessed;
      QSet<long> mProcessedLastTime;

      QHash<long, Process *> mProcesses; //This must include mFakeProcess at pid 0
      Process mFakeProcess; //A fake process with pid 0 just so that even init points to a parent

      ProcessesBase *processesBase; //The OS specific code to get the process information 
      bool mFlatMode;
  };

  class Processes::StaticPrivate
  {
    public:
      StaticPrivate() { processesLocal = 0;}
      Processes *processesLocal;
      QHash<QString, Processes*> processesRemote;
      
  };


Processes *Processes::getInstance(QString host) { //static
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

void Processes::returnInstance(QString host) { //static
	//Implement - we need reference counting etc
}
Processes::Processes(ProcessesBase *processesBase) : d(new Private())
{
    d->processesBase = processesBase;
}

bool Processes::updateProcess( Process *ps, long ppid)
{
    Process *parent = d->mProcesses.value(ppid);
    Q_ASSERT(parent);  //even init has a non-null parent - the mFakeProcess
    Process *tree_parent;
    if(d->mFlatMode) 
        tree_parent = &d->mFakeProcess;
    else 
        tree_parent = parent;

    if(ps->tree_parent != tree_parent) {
        //Processes has been reparented
        Process *p = ps;
        do {
            p = p->tree_parent;
            p->numChildren--;
        } while (p->pid!= 0);
        ps->tree_parent->children.removeAll(ps);
        ps->tree_parent = tree_parent;  //the parent has changed
        tree_parent->children.append(ps);
        p = ps;
        do {
            p = p->tree_parent;
            p->numChildren++;
        } while (p->pid!= 0);
    }
    ps->parent = parent;
    ps->parent_pid = ppid;

    //Now we can actually get the process info
    bool success = d->processesBase->updateProcessInfo(ps->pid, ps);
    emit processChanged(ps, false);
    return success;

}
bool Processes::addProcess(long pid, long ppid)
{
    Process *parent = d->mProcesses.value(ppid);
    Q_ASSERT(parent);  //even init has a non-null parent - the mFakeProcess
    //it's a new process - we need to set it up
    Process *ps = new Process(pid, ppid, parent);
    if(d->mFlatMode)
        ps->tree_parent = &d->mFakeProcess;
    else
        ps->tree_parent = parent;

    emit beginAddProcess(ps->tree_parent);

    d->mProcesses.insert(pid, ps);
    ps->tree_parent->children.append(ps);
    Process *p = ps;
    do {
        p = p->tree_parent;
        p->numChildren++;
    } while (p->pid!= 0);
    ps->parent_pid = ppid;

    //Now we can actually get the process info
    bool success = d->processesBase->updateProcessInfo(pid, ps);
    emit endAddProcess();
    return success;

}
bool Processes::updateOrAddProcess( long pid)
{
    long ppid = d->processesBase->getParentPid(pid);

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

bool Processes::updateAllProcesses( )
{
    d->mToBeProcessed = d->processesBase->getAllPids();

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
      }
    }
    
    d->mProcessedLastTime = beingProcessed;  //update the set for next time this function is called 
    return true;
}


QHash<long, Process *> Processes::getProcesses()
{
    updateAllProcesses();
    return d->mProcesses;
}

void Processes::deleteProcess(long pid)
{
    Q_ASSERT(pid > 0);
    kDebug() << "Deleting " << pid << endl;
    Process *process = d->mProcesses.value(pid);
    
    emit beginRemoveProcess(process);

    d->mProcesses.remove(pid);
    process->tree_parent->children.removeAll(process);
    Process *p = process;
    do {
      p = p->tree_parent;
      p->numChildren--;
    } while (p->pid!= 0);
    delete process;

    emit endRemoveProcess();
}


bool Processes::killProcess(long pid) {
  return sendSignal(pid, SIGTERM);
}

bool Processes::sendSignal(long pid, int sig) {
    return d->processesBase->sendSignal(pid, sig);
}

bool Processes::setNiceness(long pid, int priority) {
    return d->processesBase->setNiceness(pid, priority);
}

Processes::~Processes()
{

}

bool Processes::flatMode()
{
    return d->mFlatMode;
}

void Processes::setFlatMode(bool flat)
{
    d->mFlatMode = flat;
}
}
#include "processes.moc"
