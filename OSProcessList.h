/*
    KTop, a taskmanager and cpu load monitor
   
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

#ifndef _OSProcessList_h_
#define _OSProcessList_h_

#include <unistd.h>
#include <config.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <qlist.h>
#include <qstring.h>

#include "TimeStampList.h"

class OSProcess
{
public:
	OSProcess(const char* pidStr);
	OSProcess(int pid_) : pid(pid_) { }
	virtual ~OSProcess() {}

	const char* getName(void) const
	{
		return (name);
	}
	char getStatus(void) const
	{
		return (status);
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
	unsigned int getVm_data(void) const
	{
		return (vm_data);
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

	const QString& getErrMessage(void) const
	{
		return (errMessage);
	}

private:
	char name[101];
	char status;
	QString statusTxt;
	pid_t pid;
	pid_t ppid;
	QString userName;
	uid_t uid;
	gid_t gid;
	int ttyNo;
	int priority;
	unsigned int vm_size;
	unsigned int vm_lock;
	unsigned int vm_rss;
	unsigned int vm_data;
	unsigned int vm_stack;
	unsigned int vm_exe;
	unsigned int vm_lib;
	unsigned int userTime;
	unsigned int sysTime;
	double userLoad;
	double sysLoad;

	bool error;
	QString errMessage;
} ;

/**
 * This class encapsulates all OS specific information about the process list.
 * Since inquiring process status is highly OS dependant all these adaptions
 * should be made in this file.
 */
class OSProcessList : public QList<OSProcess>
{
public:
 	enum SORTKEY
	{
		SORTBY_PID = 0,
		SORTBY_PPID,
		SORTBY_NAME, 
		SORTBY_UID,
		SORTBY_USERNAME,
		SORTBY_CPU,
		SORTBY_TIME,
		SORTBY_STATUS,
		SORTBY_VMSIZE,
		SORTBY_VMRSS,
		SORTBY_VMLIB
	};	

	OSProcessList();

	virtual ~OSProcessList() {}

	/**
	 * This function clears the old process list and retrieves a current one
	 * from the OS.
	 */
	bool update(void);

	/**
	 * The 'has...' functions can be used to inquire if the OS supports a
	 * specific process attribute. The return value may be hardcoded and
	 * ifdef'd for each platform.
	 */
	bool hasName(void) const
	{
		return (true);
	}
	bool hasUid(void) const
	{
		return (true);
	}
	bool hasUserTime(void) const
	{
		return (true);
	}
	bool hasSysTime(void) const
	{
		return (true);
	}
	bool hasUserLoad(void) const
	{
		return (true);
	}
	bool hasSysLoad(void) const
	{
		return (true);
	}
	bool hasStatus(void) const
	{
		return (true);
	}
	bool hasPriority(void) const
	{
		return (false); // not yet implemented
	}
	bool hasVmSize(void) const
	{
		return (true);
	}
	bool hasVmRss(void) const
	{
		return (true);
	}
	bool hasVmLib(void) const
	{
		return (true);
	}

	void setSortCriteria(SORTKEY sk)
	{
		sortCriteria = sk;
	}

	SORTKEY getSortCriteria(void)
	{
		return (sortCriteria);
	}

	/**
	 * This function is needed mainly because we can have errors during the
	 * constructor execution. Since it has no return value we can call this
	 * function to find out if the class is fully operational.
	 */
	bool ok(void) const
	{
		return (!error);
	}

	/**
	 * This function can be used when calls to ok() return false to find out
	 * what has happened.
	 */
	const QString& getErrMessage(void) const
	{
		return (errMessage);
	}

private:
	/// This function is needed by the parent class to sort the list.
	virtual int compareItems(GCI it1, GCI it2);

	/// This variabled stores the criteria used to sort the list.
	SORTKEY sortCriteria;

	/**
	 * These variables are used for error handling. When an error has occured
	 * error is set to true and the errMessage contains a more detailed
	 * description of the problem.
	 */
	bool error;
	QString errMessage;
} ;

#endif
