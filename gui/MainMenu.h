/*
    KTop, the KDE Task Manager and System Monitor
   
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/


#ifndef _MainMenu_h_
#define _MainMenu_h_

#include <kmenubar.h>
#include <khelpmenu.h>

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
		MENU_ID_NEW_WORKSHEET,
		MENU_ID_DELETE_WORKSHEET,
		MENU_ID_LOAD_WORKSHEET,
		MENU_ID_SAVE_WORKSHEET,
		MENU_ID_CONNECT_HOST,
		MENU_ID_DISCONNECT_HOST,
	    MENU_ID_QUIT,
		MENU_ID_HELP
	};
    
	MainMenu(QWidget* parent = 0, const char* name = 0);
	~MainMenu()
	{
		delete file;
		delete sensor;
		delete help;
	}

signals:
	void quit();
	void newWorkSheet();
	void deleteWorkSheet();

private slots:
	void handler(int);

private:
	QPopupMenu* file;
	QPopupMenu* sensor;
	KHelpMenu* help;
} ;

extern MainMenu* MainMenuBar;

#endif
