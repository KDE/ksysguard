/*
    KTop, the KDE Taskmanager
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#ifndef _OSProcess_h_
#define _OSProcess_h_

/*
 * ATTENTION: PORTING INFORMATION!
 * 
 * If you plan to port KTop to a new platform please follow these instructions.
 * For general porting information please look at the file OSStatus.cpp!
 *
 * To keep this file readable and maintainable please keep the number of
 * #ifdef _PLATFORM_ as low as possible. Ideally you dont have to make any
 * platform specific changes in the header files. Please do not add any new
 * features. This is planned for KTop versions after 1.0.0!
 */

#include <unistd.h>
#include <config.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <qlist.h>
#include <qstring.h>

class TimeStampList;

/**
 * This class requests all needed information about a process and stores it
 * for later retrival.
 */
class OSProcess
{
public:
	/**
	 * This constructor must be used if the CPU load values are needed. These
	 * values can only be determined with the help of a list that contains
	 * performance information of a previous measurement.
	 */
	OSProcess(const char* pidStr, TimeStampList* lastTStamps,
			  TimeStampList* newTStamps);
	/**
	 * This constructor can be used if no CPU load values are needed.
	 */
	OSProcess(int pid_);

	virtual ~OSProcess() { }

	const char* getName(void) const
	{
		return (name);
	}
	const char* getStatusTxt(void) const
	{
		return (statusTxt);
	}
	const QString& getUserName(void) const
	{
		return (userName);
	}
	pid_t getPid(void) const
	{
		return (pid);
	}
	pid_t getPpid(void) const
	{
		return (ppid);
	}
	uid_t getUid(void) const
	{
		return (uid);
	}
	gid_t getGid(void) const
	{
		return (gid);
	}
	int getPriority(void) const
	{
		return (priority);
	}
	unsigned int getVm_size(void) const
	{
		return (vm_size);
	}
	unsigned int getVm_rss(void) const
	{
		return (vm_rss);
	}
	unsigned int getVm_lib(void) const
	{
		return (vm_lib);
	}
	unsigned int getUserTime(void) const
	{
		return (userTime);
	}
	unsigned int getSysTime(void) const
	{
		return (sysTime);
	}
	double getUserLoad(void) const
	{
		return (userLoad);
	}
	double getSysLoad(void) const
	{
		return (sysLoad);
	}

	bool ok(void) const
	{
		return (!error);
	}

	const QString& getErrMessage(void)
	{
		error = false;
		return (errMessage);
	}

	bool setPriority(int newPriority);

	bool sendSignal(int sig);

private:
	bool read(const char* pidStr);

	/// the name of the application (executable)
	char name[101];

	/// the command used to start the process
	QString cmdline;

	/// a description of the process status (e.g. Running, Sleep, etc)
	QString statusTxt;

	/// the process ID
	pid_t pid;

	/// the parent process ID
	pid_t ppid;

	/// the login name of the user that owns this process
	QString userName;

	/// the real user ID
	uid_t uid;

	/// the real group ID
	gid_t gid;

	/// the number of the tty the process owns
	int ttyNo;

	/*
	 * The scheduling priority. The range should be -20 to 20. I'm not sure
	 * whether this is true for all platforms.
	 */
	int priority;

	/*
	 * The total amount of memory the process uses. This includes shared and
	 * swapped memory.
	 */
	unsigned int vm_size;

	/*
	 * The amount of physical memory the process currently uses.
	 */
	unsigned int vm_rss;

	/*
	 * The amount of memory (shared/swapped/etc) the process shares with
	 * other processes.
	 */
	unsigned int vm_lib;

	/*
	 * The number of 1/100 of a second the process has spend in user space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
	unsigned int userTime;

	/*
	 * The number of 1/100 of a second the process has spend in system space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
	unsigned int sysTime;

	/// the current CPU load (in %) from user space
	double userLoad;

	/// the current CPU load (in %) from system space
	double sysLoad;

	bool error;
	QString errMessage;
} ;

#endif
