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

#ifndef _TimeStampList_h_
#define _TimeStampList_h_

#include <unistd.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <qlist.h>

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
