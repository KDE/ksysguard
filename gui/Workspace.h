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

#ifndef _Workspace_h_
#define _Workspace_h_

#include <qtabwidget.h>
#include <qstring.h>
#include <qlist.h>

class WorkSheet;

class Workspace : public QTabWidget
{
	Q_OBJECT
public:
	Workspace(QWidget* parent, const char* name = 0);
	~Workspace() { }

	void addSheet(const QString& title, int columns = 1);

public slots:
	void newWorkSheet();
	void deleteWorkSheet();

private:
	int tabCount;
} ;

#endif
