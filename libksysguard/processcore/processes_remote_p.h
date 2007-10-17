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

#ifndef PROCESSES_REMOTE_P_H_
#define PROCESSES_REMOTE_P_H_

#include "processes_base_p.h"
#include <QSet>
class Process;
namespace KSysGuard
{
    /**
     * This is used to connect to a remote host
     */
    class ProcessesRemote : public AbstractProcesses {
      public:
	ProcessesRemote(const QString &hostname);
	virtual ~ProcessesRemote();
	virtual QSet<long> getAllPids();
	virtual long getParentPid(long pid);
	virtual bool updateProcessInfo(long pid, Process *process);
	virtual bool sendSignal(long pid, int sig);
        virtual bool setNiceness(long pid, int priority);
	virtual long long totalPhysicalMemory();
	virtual bool setIoNiceness(long pid, int priorityClass, int priority);
	virtual bool supportsIoNiceness();

      private:
	/**
	 * You can use this for whatever data you want.  Be careful about preserving state in between getParentPid and updateProcessInfo calls
	 * if you chose to do that. getParentPid may be called several times for different pids before the relevant updateProcessInfo calls are made.
	 * This is because the tree structure has to be sorted out first.
	 */
        class Private;
        Private *d;

    };
}
#endif 
