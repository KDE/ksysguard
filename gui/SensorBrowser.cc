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

#include <klocale.h>

#include "SensorBrowser.h"
#include "SensorManager.h"
#include "SensorBrowser.moc"

SensorBrowser::SensorBrowser(QWidget* parent, SensorManager* sm,
							 const char* name) :
	QListView(parent, name), sensorManager(sm)
{
	connect(sm, SIGNAL(update(void)), this, SLOT(update(void)));

	addColumn(i18n("Sensor Browser"));
	setRootIsDecorated(TRUE);
}

void
SensorBrowser::update()
{
	SensorManagerIterator it(sensorManager);

	sensors.clear();
	for (int i = 0 ; it.current(); ++it, ++i)
	{
		sensors.append(new QListViewItem(this,
										 sensorManager->
										 getHostName(it.current())));
		(*it).sendRequest("monitors", this, i);
	}
}

void
SensorBrowser::answerReceived(int id, const QString& s)
{
	SensorLinesTokenizer tok(s);

	for (unsigned int i = 0; i < tok.numberOfTokens(); ++i)
		new QListViewItem(sensors.at(id), tok[i]);
}
