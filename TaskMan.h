/*
    KTop, the KDE Task Manager
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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
*/

// $Id$

#ifndef _TaskMan_h_
#define _TaskMan_h_

#include <signal.h>

#include <qwidget.h>
#include <qtabdialog.h>

#include <ktablistbox.h>

#include "IconList.h"
#include "ProcessList.h"
#include "ProcListPage.h"
#include "PerfMonPage.h"

/**
 * This class creates the main dialog widget of ktop. It consists of a 
 * 3 entry tab dialog. The tabs contain a process list, a process tree and
 * a performance meter.
 */
class TaskMan : public QTabDialog
{
	Q_OBJECT

public:
	enum 
	{
		PAGE_PLIST = 0, 
		PAGE_PERF
	};

	TaskMan(QWidget* parent = 0, const char* name = 0,
			int sfolder = 0);
    ~TaskMan()
	{
		delete procListPage;
		delete perfMonPage;
	}
     
	void invokeSettings();
	void raiseStartUpPage();
	void saveSettings();

signals:
	void refreshRateSelected(int);
	void enableRefreshMenu(bool);

private:
	void reniceProcess(int pid);
	void killProcess(int, int sig, const char*);

private slots:
	void tabBarSelected(int);

private:
	ProcListPage* procListPage;

	PerfMonPage* perfMonPage;

	/// This is only an alias for procListPage, procTreePage and perfMonPage
    QWidget* pages[2];

	int currentPage;
};

#endif
