/*
    KTop, the KDE Taskmanager
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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

#include <qmenubar.h>
#include <qpopupmenu.h>

#include <kapp.h>

#include "TaskMan.h"

extern KApplication* Kapp;

class TopLevel : public QWidget
{
	Q_OBJECT

public:

    enum
	{
		MENU_ID_ABOUT = 100,
		MENU_ID_PROCSETTINGS = 50, 
	    MENU_ID_QUIT = 20,
		MENU_ID_HELP = 30
	};
    
	TopLevel(QWidget *parent = 0, const char *name = 0, int sfolder = 0);
	~TopLevel()
	{
		delete taskman;
		delete menubar;
		delete file;
		delete settings;
		delete help;
	}

	void closeEvent(QCloseEvent*)
	{
		quitSlot();
	}

protected:

	void resizeEvent(QResizeEvent*);

	QMenuBar* menubar;
	QPopupMenu* file;
	QPopupMenu* settings;
	QPopupMenu* help;

	TaskMan* taskman;

protected slots:

	void menuHandler(int);
	void quitSlot();

};


