/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
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

	$Id$
*/

#ifndef _process_list_h_
#define _process_list_h_

void initProcessList(struct SensorModul* sm);
void exitProcessList(void);

int updateProcessList(void);

void printProcessList(const char*);
void printProcessListInfo(const char*);
void printProcessCount(const char* cmd);
void printProcessCountInfo(const char* cmd);

void killProcess(const char* cmd);
void setPriority(const char* cmd);

#endif
