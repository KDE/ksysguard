/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!
*/

// $Id$

#ifndef _OSProcessList_h_
#define _OSProcessList_h_

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

#include "TimeStampList.h"
#include "OSProcess.h"

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
		SORTBY_PRIORITY,
		SORTBY_STATUS,
		SORTBY_VMSIZE,
		SORTBY_VMRSS,
		SORTBY_VMLIB
	};	

	OSProcessList();

	virtual ~OSProcessList()
	{
		delete lastTStamps;
	}

	/**
	 * This function clears the old process list and retrieves a current one
	 * from the OS.
	 */
	bool update(void);

	/**
	 * The 'has...' functions can be used to inquire if the OS supports a
	 * specific process attribute. The return value may be hardcoded
	 * for each platform.
	 */
	bool hasName(void) const;
	bool hasCmdLine(void) const;
	bool hasUid(void) const;
	bool hasUserTime(void) const;
	bool hasSysTime(void) const;
	bool hasUserLoad(void) const;
	bool hasSysLoad(void) const;
	bool hasStatus(void) const;
	bool hasPriority(void) const;
	bool hasNiceLevel(void) const;
	bool hasVmSize(void) const;
	bool hasVmRss(void) const;
	bool hasVmLib(void) const;

	void setSortCriteria(SORTKEY sk)
	{
		sortCriteria = sk;
	}

	SORTKEY getSortCriteria(void) const
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
	 * what has happened. The error variable is cleared after a call to
	 * getErrMessage().
	 */
	const QString& getErrMessage(void)
	{
		error = false;
		return (errMessage);
	}

private:
	/// This function is needed by the parent class to sort the list.
	virtual int compareItems(Item it1, Item it2);

	/// This variabled stores the criteria used to sort the list.
	SORTKEY sortCriteria;

	TimeStampList* lastTStamps;

	/**
	 * These variables are used for error handling. When an error has occured
	 * error is set to true and the errMessage contains a more detailed
	 * description of the problem.
	 */
	bool error;
	QString errMessage;
} ;

#endif
