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
*/

// $Id$

#ifndef _ProcessMenu_h_
#define _ProcessMenu_h_

#include <signal.h>

#include <qpopupmenu.h>

class ProcessMenu : public QPopupMenu
{
	Q_OBJECT

public:
	enum 
	{
		MENU_ID_SIGINT = SIGINT,
		MENU_ID_SIGQUIT = SIGQUIT,
		MENU_ID_SIGTERM = SIGTERM,
		MENU_ID_SIGKILL = SIGKILL,
		MENU_ID_SIGUSR1 = SIGUSR1,
		MENU_ID_SIGUSR2 = SIGUSR2,
	    MENU_ID_RENICE
	};

	ProcessMenu(QWidget* parent = 0, const char* name = 0);
	~ProcessMenu() {}

public slots:
	void processSelected(int pid)
	{
		selectedProcess = pid;
	}
	void killProcess(int, int sig = SIGKILL);

signals:
	void requestUpdate(void);

private:
	void reniceProcess(int pid);

private slots:
	void handler(int);

private:
	int selectedProcess;
} ;

#endif
