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

#ifndef _ktop_h_
#define _ktop_h_

#include <qpopupmenu.h>
#include <qsplitter.h>

#include <kapp.h>
#include <ktmainwindow.h>
#include <kmenubar.h>
#include <kstatusbar.h>

#include "SensorClient.h"
#include "MainMenu.h"

extern KApplication* Kapp;

class SensorAgent;

class TopLevel : public KTMainWindow, public SensorClient
{
	Q_OBJECT

public:
	TopLevel(const char *name = 0, int sfolder = 0);
	~TopLevel();

	void closeEvent(QCloseEvent*)
	{
		quitSlot();
	}

	virtual void answerReceived(int id, const QString& s);
	
protected:
	virtual void timerEvent(QTimerEvent*);

private:
	MainMenu* menubar;
	KStatusBar* statusbar;

	QSplitter* splitter;
	SensorBrowser* sb;
	Workspace* ws;
	int timerID;

	SensorAgent* localhost;

protected slots:

	void quitSlot();

};

#endif
