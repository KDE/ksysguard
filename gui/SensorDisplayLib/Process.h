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
	Process(long long _pid, long long _ppid)  {clear(); pid = _pid; parent_pid = _ppid; }
	bool isValid() {return processType != Process::Invalid;}
	
	long long pid;    //The systems ID for this process
	long long parent_pid;  //The systems ID for the parent of this process.  0 for init.
	long long uid; //The user id that the process is running as
	long long gid; //The group id that the process is running as
	long long tracerpid; //If this is being debugged, this is the process that is debugging it
	float userUsage; //Percentage (0 to 100)
	float sysUsage;  //Percentage (0 to 100)
	int nice;      //Niceness (-20 to 20) of this process
	long vmSize;   //KiloBytes used in total by process
	long vmRSS;    //KiloBytes used by actual process - the main memory it uses without shared/X/etc
	ProcessType processType;
	QString name;  //The name (e.g. "ksysguard", "konversation", "init")
	QString command; //The command the process was launched with
	QList<long long> children_pids;
	QList<QVariant> data;  //The column data, excluding the name, pid, ppid and uid
	QString xResIdentifier;  //The window title.  Empty if unknown
	QByteArray status; //Running, Stopped, etc.  Untranslated
	long long xResPxmMemBytes;      //The amount of memory in bytes used in X server pixmaps
	int xResNumPxm;      //The number of x server pixmaps
	long long xResMemOtherBytes;  //The amount of memory in bytes used in X server other than pixmaps
  private:
	void clear() {uid = 0; gid = -1; processType=Invalid; tracerpid = 0; xResPxmMemBytes=0; xResNumPxm=0; xResMemOtherBytes=0; }
};

#endif
