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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!
*/

// $Id$

#include <assert.h>

#include <qstring.h>

#include <kapp.h>

#include "version.h"
#include <klocale.h>
#include <kstdaccel.h>
#include "MainMenu.moc"

extern KApplication* Kapp;

MainMenu* MainMenuBar = 0;

MainMenu::MainMenu(QWidget* parent, const char* name) :
	KMenuBar(parent, name)
{
	KStdAccel accel;

	assert(MainMenuBar == 0);
	MainMenuBar = this;

	// 'File' submenu
	file = new QPopupMenu();
	file->insertItem(i18n("&Quit"), this, SLOT(slotQuit()), accel.quit());
	connect(file, SIGNAL(activated(int)), this, SLOT(handler(int)));

	// 'Help' submenu
	QString about;
	about = i18n("KDE Task Manager (KTop) Version %1\n\n"
					   "Copyright:\n"
					   "1996 : A. Sanda <alex@darkstar.ping.at>\n"
					   "1997 : Ralf Mueller <ralf@bj-ig.de>\n"
					   "1997-98 : Bernd Johannes Wuebben <wuebben@kde.org>\n"
					   "1998 : Nicolas Leclercq <nicknet@planete.net>\n"
					   "1999 : Chris Schlaeger <cs@kde.org>\n")
				  .arg(KTOP_VERSION);
	help = Kapp->helpMenu(true, about);

	// 'Refresh Rate' submenu
	refresh = new QPopupMenu();
	refresh->setCheckable(true);
	refresh->insertItem(i18n("Manual Refresh"), MENU_ID_REFRESH_MANUAL, -1);
	refresh->insertItem(i18n("Slow Refresh"), MENU_ID_REFRESH_SLOW, -1);
	refresh->insertItem(i18n("Medium Refresh"), MENU_ID_REFRESH_MEDIUM, -1);
	refresh->insertItem(i18n("Fast Refresh"), MENU_ID_REFRESH_FAST, -1);
	connect(refresh, SIGNAL(activated(int)), this, SLOT(handler(int)));

	// 'Process' submenu
	process = new ProcessMenu();
	process->setItemEnabled(MENU_ID_MENU_PROCESS, false);
	connect(process, SIGNAL(requestUpdate(void)),
			this, SLOT(requestUpdateSlot(void)));

	// register submenues
	setLineWidth(1);
	insertItem(i18n("&File"), file, 2, -1);
	insertItem(i18n("&Refresh Rate"), refresh, MENU_ID_MENU_REFRESH, -1);
	insertItem(i18n("&Process"), process, MENU_ID_MENU_PROCESS, -1);

	insertSeparator(-1);
	insertItem(i18n("&Help"), help, 2, -1);
}

void 
MainMenu::handler(int id)
{
	switch(id)
	{
	case MENU_ID_REFRESH_MANUAL:
	case MENU_ID_REFRESH_SLOW:
	case MENU_ID_REFRESH_MEDIUM:
	case MENU_ID_REFRESH_FAST:
		emit(setRefreshRate(id - MENU_ID_REFRESH_MANUAL));
		break;

	default:
		break;
	}
}

void
MainMenu::checkRefreshRate(int rate)
{
	// uncheck old and check new rate
	for (int i = MENU_ID_REFRESH_MANUAL; i <= MENU_ID_REFRESH_FAST; i++)
		refresh->setItemChecked(i, rate == i - MENU_ID_REFRESH_MANUAL);
}
