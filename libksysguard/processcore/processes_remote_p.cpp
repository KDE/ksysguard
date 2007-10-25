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

#include "processes_remote_p.h"
#include "process.h"

#include <klocale.h>

#include <QSet>




namespace KSysGuard
{

  class ProcessesRemote::Private
  {
    public:
      Private() {;}
      ~Private() {;}
      QString host;
    };
ProcessesRemote::ProcessesRemote(const QString &hostname) : d(new Private())
{
  d->host = hostname;
}


long ProcessesRemote::getParentPid(long pid) {
    return 0;
}
bool ProcessesRemote::updateProcessInfo( long pid, Process *process)
{
    return false;
}

QSet<long> ProcessesRemote::getAllPids( )
{
    QSet<long> pids;
    return pids;
}

bool ProcessesRemote::sendSignal(long pid, int sig) {
    return false;
}
bool ProcessesRemote::setNiceness(long pid, int priority) {
    return false;
}

bool ProcessesRemote::setIoNiceness(long pid, int priorityClass, int priority) {
    return false; //Not yet supported
}

bool ProcessesRemote::supportsIoNiceness() {
    return false;
}

long long ProcessesRemote::totalPhysicalMemory() {
    return 0;
}
long ProcessesRemote::numberProcessorCores() {
    return 0;
}

ProcessesRemote::~ProcessesRemote()
{
    delete d;
}

}
