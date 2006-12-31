/*
    KSysGuard, the KDE System Guard
    Copyright (c) 2006 John Tapsell <john.tapsell@kdemail.net > 

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



#ifndef KSYSGUARD_PROCESS_H
#define KSYSGUARD_PROCESS_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>
#include <QByteArray>

class Process : public QObject {
  Q_OBJECT
  public:
	typedef enum { Daemon, Kernel, Init, Kdeapp, Shell, Tools, Wordprocessing, Term, Other, Invalid } ProcessType;
	Process() { clear();}
	Process(long long _pid, long long _ppid, Process *_parent)  {clear(); pid = _pid; parent_pid = _ppid; parent = _parent; }
	bool isValid() {return processType != Process::Invalid;}
	
	long long pid;    ///The systems ID for this process
	long long parent_pid;  ///The systems ID for the parent of this process.  0 for init.
	
	/** A guaranteed NON-NULL pointer to the parent process except for the fake process with pid 0.
	 *  The Parent's pid is the same value as the parent_pid.  The parent process will be also pointed
	 *  to by ProcessModel::mPidToProcess to there is no need to worry about mem management in using parent.
	 *  For init process, parent will point to a (fake) process with pid 0 to simplify things.
	 */
	Process *parent;
	long long uid; ///The user id that the process is running as
	long long gid; ///The group id that the process is running as
	long long tracerpid; ///If this is being debugged, this is the process that is debugging it
	long userTime; ///The time, in 100ths of a second, spent in total on user calls. -1 if not known
	long sysTime;  ///The time, in 100ths of a second, spent in total on system calls.  -1 if not known
	float userUsage; ///Percentage (0 to 100)
	float sysUsage;  ///Percentage (0 to 100)
	double totalUserUsage; ///Percentage (0 to 100) from the sum of itself and all its children recursively.  If there's no children, it's equal to userUsage
	double totalSysUsage; ///Percentage (0 to 100) from the sum of itself and all its children recursively. If there's no children, it's equal to sysUsage
	unsigned long numChildren; ///Number of children recursively that this process has.  From 0+
	int nice;      ///Niceness (-20 to 20) of this process
	long vmSize;   ///KiloBytes used in total by process (KiB)
	long vmRSS;    ///KiloBytes used by actual process - the main memory it uses without shared/X/etc (KiB). If it's swapped out, it's not counted
	ProcessType processType;
	QByteArray name;  ///The name (e.g. "ksysguard", "konversation", "init")
	QByteArray command; ///The command the process was launched with
	QList<Process *> children;  ///A list of all the direct children that the process has.  Children of children are not listed here, so note that children_pids <= numChildren
	QList<QVariant> data;  ///The column data, excluding the name, pid, ppid and uid
	QString xResIdentifier;  ///The window title.  Empty if unknown
	QByteArray status; ///Running, Stopped, etc.  Untranslated
	long long xResPxmMemBytes;     ///The amount of memory in bytes used in X server pixmaps
	int xResNumPxm;      ///The number of x server pixmaps
	long long xResMemOtherBytes;  ///The amount of memory in bytes used in X server other than pixmaps
	bool isStoppedOrZombie; ///An optomisation value, true iff process->status == "stopped" || process->status == "zombie"
  private:
	void clear() {pid = 0; parent_pid = 0; uid = 0; gid = -1; tracerpid = 0; userTime = -1; sysTime = -1; userUsage=0; sysUsage=0; totalUserUsage=0; totalSysUsage=0; numChildren=0; nice=0; vmSize=0; vmRSS = 0; processType=Invalid; xResPxmMemBytes=0; xResNumPxm=0; xResMemOtherBytes=0; isStoppedOrZombie=false;}
};

#endif
