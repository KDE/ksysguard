/*
    KSysGuard, the KDE Task Manager
   
	Copyright (c) 2000 Chris Schlaeger
	                   cs@kde.org
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _SignalIDs_h_
#define _SignalIDs_h_

/* This file is used to correlate the entries of the process popup menu
 * of the ProcessList class and the value received by the kill command
 * in ksysguardd. We limit the set of available signals to the POSIX.1
 * set with job control. */

#define MENU_ID_SIGABRT 11
#define MENU_ID_SIGALRM 12
#define MENU_ID_SIGCHLD 13
#define MENU_ID_SIGCONT 14
#define MENU_ID_SIGFPE  15
#define MENU_ID_SIGHUP  16
#define MENU_ID_SIGILL  17
#define MENU_ID_SIGINT  18
#define MENU_ID_SIGKILL 19
#define MENU_ID_SIGPIPE 20
#define MENU_ID_SIGQUIT 21 
#define MENU_ID_SIGSEGV 22
#define MENU_ID_SIGSTOP 23
#define MENU_ID_SIGTERM 24
#define MENU_ID_SIGTSTP 25
#define MENU_ID_SIGTTIN 26
#define MENU_ID_SIGTTOU 27
#define MENU_ID_SIGUSR1 28
#define MENU_ID_SIGUSR2 29

#endif
