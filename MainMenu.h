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

#ifndef _MainMenu_h_
#define _MainMenu_h_

#include <kmenubar.h>

#include "ProcessMenu.h"

/**
 * This class implements the main menu. All triggered actions are activated
 * through signals. For on-the-fly modification of the menus slots are
 * provided.
 */
class MainMenu : public KMenuBar
{
	Q_OBJECT

public:
    enum
	{
		MENU_ID_ABOUT = 100,
		MENU_ID_MENU_REFRESH,
		MENU_ID_REFRESH_MANUAL,
		MENU_ID_REFRESH_SLOW,
		MENU_ID_REFRESH_MEDIUM,
		MENU_ID_REFRESH_FAST,
		MENU_ID_MENU_PROCESS,
	    MENU_ID_QUIT,
		MENU_ID_HELP
	};
    
	MainMenu(QWidget* parent = 0, const char* name = 0);
	~MainMenu()
	{
		delete file;
		delete refresh;
		delete process;
		delete help;
	}

public slots:
	void checkRefreshRate(int);
	void enableRefreshMenu(bool enable)
	{
		setItemEnabled(MENU_ID_MENU_REFRESH, enable);
	}
	void processSelected(int pid)
	{
		setItemEnabled(MENU_ID_MENU_PROCESS, pid >= 0);
		process->processSelected(pid);
	}

signals:
	void quit(void);
	void setRefreshRate(int);
	void requestUpdate(void);

private slots:
	void handler(int);
	void requestUpdateSlot(void)
	{
		emit(requestUpdate());
	}

private:
	QPopupMenu* file;
	QPopupMenu* refresh;
	ProcessMenu* process;
	QPopupMenu* help;
} ;

extern MainMenu* MainMenuBar;

#endif
