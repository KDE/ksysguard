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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!
*/

// $Id$

#ifndef _ktop_h
#define _ktop_h

#include <qpopupmenu.h>
#include <qlayout.h>

#include <kapp.h>
#include <ktmainwindow.h>
#include <kmenubar.h>
#include <kstatusbar.h>

#include "TaskMan.h"
#include "OSStatus.h"

#include "MainMenu.h"

extern KApplication* Kapp;

class TopLevel : public KTMainWindow
{
	Q_OBJECT

public:
	TopLevel(const char *name = 0, int sfolder = 0);
	~TopLevel()
	{
		killTimer(timerID);

		delete taskman;
		delete menubar;
		delete statusbar;
	}

//	QSize sizeHint();
//	void updateRects();

	void closeEvent(QCloseEvent*)
	{
		quitSlot();
	}

protected:
	virtual void timerEvent(QTimerEvent*);

private:
	MainMenu* menubar;
	KStatusBar* statusbar;

	TaskMan* taskman;

	int timerID;
	OSStatus osStatus;

protected slots:

	void quitSlot();

};

#endif
