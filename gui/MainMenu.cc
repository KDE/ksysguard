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

#include <assert.h>

#include <qstring.h>

#include <kapp.h>
#include <khelpmenu.h>
#include <klocale.h>

#include "../version.h"
#include "MainMenu.moc"

extern KApplication* Kapp;

MainMenu* MainMenuBar = 0;

MainMenu::MainMenu(QWidget* parent, const char* name) :
	KMenuBar(parent, name)
{
	assert(MainMenuBar == 0);
	MainMenuBar = this;

	// 'File' submenu
	file = new QPopupMenu();
	CHECK_PTR(file);
	file->insertItem(i18n("New Worksheet"), MENU_ID_NEW_WORKSHEET, -1);
	file->insertItem(i18n("Delete Worksheet"), MENU_ID_DELETE_WORKSHEET, -1);
	file->insertSeparator(-1);
	file->insertItem(i18n("Load Worksheet"), MENU_ID_LOAD_WORKSHEET, -1);
	file->insertItem(i18n("Save Worksheet"), MENU_ID_SAVE_WORKSHEET, -1);
	file->insertSeparator(-1);
	file->insertItem(i18n("Quit"), MENU_ID_QUIT, -1);
	connect(file, SIGNAL(activated(int)), this, SLOT(handler(int)));

	// 'Sensor' submenu
	sensor = new QPopupMenu();
	CHECK_PTR(sensor);
	sensor->insertItem(i18n("Connect Host"), MENU_ID_CONNECT_HOST, -1);
	sensor->insertItem(i18n("Disconnect Host"), MENU_ID_DISCONNECT_HOST, -1);
	connect(sensor, SIGNAL(activated(int)), this, SLOT(handler(int)));

	// 'Help' submenu
	QString about;
	about = i18n("KDE Task Manager and Perfomance Monitor  Version %1\n\n"
				 "Copyright:\n"
				 "1996 : A. Sanda <alex@darkstar.ping.at>\n"
				 "1997 : Ralf Mueller <ralf@bj-ig.de>\n"
				 "1997-1998 : Bernd Johannes Wuebben <wuebben@kde.org>\n"
				 "1998 : Nicolas Leclercq <nicknet@planete.net>\n"
				 "1999 : Chris Schlaeger <cs@kde.org>\n")
		.arg(KTOP_VERSION);
    help = new KHelpMenu(this, about);
	CHECK_PTR(help);

	// register submenues
	setLineWidth(1);
	insertItem(i18n("&File"), file, 2, -1);
	insertItem(i18n("&Sensor"), sensor, -1, -1);

	insertSeparator(-1);
	insertItem(i18n("&Help"), help->menu(), 2, -1);
}

void 
MainMenu::handler(int id)
{
	switch(id)
	{
	case MENU_ID_QUIT:
		emit(quit());
		break;

	case MENU_ID_NEW_WORKSHEET:
		emit(newWorkSheet());
		break;

	case MENU_ID_DELETE_WORKSHEET:
		emit(deleteWorkSheet());
		break;

	default:
		break;
	}
}
