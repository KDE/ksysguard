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

#ifndef _OSStatus_h_
#define _OSStatus_h_

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

#include <stdio.h>

#include <qstring.h>

/**
 * This class implements an abstract interface to certain system parameters
 * like the system load or the memory usage. The information can be retrieved
 * with member functions of this class.
 */
class OSStatus
{
public:
	OSStatus();

	~OSStatus();

	/**
	 * If an error has occured the return value of this function will be
	 * false, true otherwise.
	 */
	bool ok(void) const
	{
		return (!error);
	}

	/**
	 * If an error has occured the functions return an error message. If no
	 * error has occured it returns an empty string.
	 */
	const QString& getErrMessage(void) const
	{
		return (errMessage);
	}

	/**
	 * This function calculates the system load. The load is split into
	 * user, system, nice and idle load. The values are in percent (0 - 100).
	 * If an error occured the return value is false, otherwise true.
	 */
	bool getCpuLoad(int& user, int& sys, int& nice, int& idle);

	/**
	 * This function determines the memory usage of the system. All values
	 * are for physical memory only. If an error occured the return value is
	 * false, otherwise true. All values are in bytes.
	 */
	bool getMemoryInfo(int& mtotal, int& mfree, int& shared, int& buffers,
					   int& cached);

	/**
	 * This function deterines the total swap space size and the used swap
	 * space size. If an error occured the return value is false, otherwise
	 * true. All values are in bytes.
	 */
	bool getSwapInfo(int& stotal, int& sfree);

private:
	/**
	 * To determine the system load we have to calculate the differences
	 * between the ticks values of two successive calls to getCpuLoad. These
	 * variables are used to store the ticks values for the next call.
	 */
	int userTicks;
	int sysTicks;
	int niceTicks;
	int idleTicks;

	/// This file pointer is used to access the /proc/stat file.
	FILE* stat;

	/// These variables are used for the error handling.
	bool error;
	QString errMessage;
} ;

#endif
