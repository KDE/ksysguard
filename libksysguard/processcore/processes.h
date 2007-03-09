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

#ifndef PROCESSES_H_
#define PROCESSES_H_

#include <kdelibs_export.h>

#include "process.h"
#include <QHash>

namespace KSysGuard
{
    class ProcessesBase;
    /**
     * This class retrieves the processes currently running in an OS independant way.
     *
     * To use, do something like:
     *
     * \code
     *   #include <ksysguard/processes.h>
     *   #include <ksysguard/process.h>
     *
     *   KSysGuard::Processes *processes = KSysGuard::Processes::getInstance();
     *   QHash<long, Process *> processlist = processes->getProcesses();
     *   foreach( Process * process, processlist) {
     *     kDebug() << "Process with pid " << process->pid << " is called " << process->name << endl;
     *   }
     *   KSysGuard::Processes::returnInstance(processes);
     *   processes = NULL;
     * \endcode
     *
     * @author John Tapsell <tapsell@kde.org>
     */
    class KDE_EXPORT Processes : public QObject
    {
    Q_OBJECT

    public:
        /**
	 *  Singleton pattern to return the instance associated with @p host.
	 *  Leave as the default for the current machine
	 */
        static Processes *getInstance(QString host = QString::null);
	/**
	 *  Call when you are finished with the Processes pointer from getInstance.
	 *  The pointer from getInstance may not be valid after calling this.
	 *  This is reference counted - once all the instances are returned, the object is deleted
	 */
        static void returnInstance(QString host = QString::null);
	/**
	 *  Retrieve the list of processes.
	 *  The key to the hash is the pid of the process.
	 */
	QHash<long, Process *> getProcesses();

	/**
	 *  Get information for one specific process
	 */
	Process *getProcess(long pid);

	/**
	 *  Kill the specified process.  You may not have the privillage to kill the process.
	 *  The process may also chose to ignore the command.  Send the SIGKILL signal to kill
	 *  the process immediately.  You may lose any unsaved data.
	 *
	 *  @returns Successful or not in killing the process
	 */
	bool killProcess(long pid);

        /**
	 *  Send the specified named POSIX signal to the process given.
	 *
	 *  For example, to indicate for process 324 to STOP do:
	 *  \code
	 *    #include <signals.h>
	 *     ...
	 *
	 *    KSysGuard::Processes::sendSignal(23, SIGSTOP);
	 *  \endcode
	 *
	 */
	bool sendSignal(long pid, int sig);

	/**
	 *  Set the priority for a process.  This is from 19 (very nice, lowest priority) to 
	 *    -20 (highest priority).  The default value for a process is 0.
	 *  
	 *  @return false if you do not have permission to set the priority
	 */
	bool setNiceness(long pid, int priority);

	/** When the tree is in flat mode the tree_parent for all the processes is set to the fake process
	 *  rather than the usual tree heirachy.  The signals are also emitted to reorginize the tree to/from
	 *  flat mode.
	 */
	bool flatMode();

	/** When the tree is in flat mode the tree_parent for all the processes is set to the fake process
	 *  rather than the usual tree heirachy.  The signals are also emitted to reorginize the tree to/from
	 *  flat mode.
	 *
	 *  It's recommended to call updateAllProcesses straight after calling this so that it reorders quickly
	 */
	void setFlatMode(bool flat);
    signals:
	/** The data for a process has changed.
	 *  if @p onlyCpuOrMem is set, only the cpu usage or memory information has been updated.  This is for optomization reasons - the cpu percentage
	 *  and memory usage change quite often, but if they are the only thing changed then there's no reason to repaint the whole row
	 */
        void processChanged( KSysGuard::Process *process, bool onlyCpuOrMem);
        /**
	 *  This indicates we are about to add a process in the model.
	 */
	void beginAddProcess( KSysGuard::Process *parent);
        /**
	 *  We have finished inserting a process
	 */
        void endAddProcess();
        /** 
	 *  This indicates we are about to remove a process in the model.  Emit the appropriate signals
	 */
	void beginRemoveProcess( KSysGuard::Process *process);
        /** 
	 *  We have finished removing a process
	 */
        void endRemoveProcess();
	/**
	 *  This indicates we are about move a process from one parent to another.
	 */
        void beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent);
        /**
	 *  We have finished moving the process
	 */
        void endMoveProcess();
    protected:
        Processes(ProcessesBase *processesBase);
	~Processes();
        class Private;
        Private *d;
	class StaticPrivate;
	static StaticPrivate *d2;
    private:
        bool updateAllProcesses();
        bool updateOrAddProcess( long pid);
        inline void deleteProcess(long pid);
        bool updateProcess( Process *process, long ppid, bool onlyReparent = false);
        bool addProcess(long pid, long ppid);

    };
    Processes::StaticPrivate *Processes::d2 = 0;
}
#endif 
