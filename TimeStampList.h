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

/*
 * ATTENTION: PORTING INFORMATION!
 * 
 * If you plan to port KTop to a new platform please follow these instructions.
 * For general porting information please look at the file OSStatus.cpp!
 *
 * To keep this file readable and maintainable please keep the number of
 * #ifdef _PLATFORM_ as low as possible. Ideally you dont have to make any
 * platform specific changes in the header files. Please do not add any new
 * features. This is planned for KTop version after 1.0.0!
 */

#ifndef _TimeStampList_h_
#define _TimeStampList_h_

#include <config.h>
#include <unistd.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <qlist.h>

/**
 * The TimeStamp class implements entities that contain the time stamp of
 * it's creation. The resolution is 1/100 of a second. Additionally it stores
 * a process id, user and system time information. This class is needed to	
 * implement the OSProcessList class.
 */
class TimeStamp
{
public:
	TimeStamp(pid_t pid_, int utime, int stime)
		: pid(pid_), userTime(utime), sysTime(stime)
	{
		struct timeval tv;
		gettimeofday(&tv, 0);
		centStamp = tv.tv_sec * 100 + tv.tv_usec / 10000;
	}

	~TimeStamp() { }

	pid_t getPid(void) const
	{
		return (pid);
	}
	int getSysTime(void) const
	{
		return (sysTime);
	}

	int getUserTime(void) const
	{
		return (userTime);
	}

	int getCentStamp(void) const
	{
		return (centStamp);
	}

private:
	pid_t pid;
	int userTime;
	int sysTime;
	int centStamp;
} ;

/**
 * This class implements a list of TimeStamps.
 */
class TimeStampList : public QList<TimeStamp>
{
public:
	TimeStampList()
	{
		setAutoDelete(true);
	}
	~TimeStampList() { }

private:
	int compareItems(GCI it1, GCI it2)
	{
		TimeStamp* ts1 = (TimeStamp*) it1;
		TimeStamp* ts2 = (TimeStamp*) it2;

		return (ts1->getPid() < ts2->getPid() ? -1 :
				(ts1->getPid() > ts2->getPid() ? 1 : 0));
	}
} ;

#endif
