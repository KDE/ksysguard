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

#include <qevent.h>
#include <qdragobject.h>

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

	// Fill the sensor description dictionary.
	dict.insert("cpuidle", new QString(i18n("Idle Load")));
	dict.insert("cpusys", new QString(i18n("System Load")));
	dict.insert("cpunice", new QString(i18n("Nice Load")));
	dict.insert("cpuuser", new QString(i18n("User Load")));
	dict.insert("memswap", new QString(i18n("Swap Memory")));
	dict.insert("memcached", new QString(i18n("Cached Memory")));
	dict.insert("membuf", new QString(i18n("Buffered Memory")));
	dict.insert("memused", new QString(i18n("Used Memory")));
	dict.insert("memfree", new QString(i18n("Free Memory")));
	dict.insert("pscount", new QString(i18n("Process Count")));
	dict.insert("ps", new QString(i18n("Process Controller")));

	for (int i = 0; i < 32; i++)
	{
		dict.insert("cpu" + QString::number(i) + "idle",
					new QString(QString(i18n("CPU%1 Idle Load")).arg(i)));
		dict.insert("cpu" + QString::number(i) + "sys",
					new QString(QString(i18n("CPU%1 System Load")).arg(i)));
		dict.insert("cpu" + QString::number(i) + "nice",
					new QString(QString(i18n("CPU%1 Nice Load")).arg(i)));
		dict.insert("cpu" + QString::number(i) + "user",
					new QString(QString(i18n("CPU%1 User Load")).arg(i)));
	}

	// The sensor browser can be completely hidden.
	setMinimumWidth(1);
}

void
SensorBrowser::disconnect()
{
	QListIterator<HostInfo> it(hostInfos);

	for (; it.current(); ++it)
		if ((*it)->getLVI()->isSelected())
		{
			debug("Disconnecting %s", (*it)->getHostName().ascii());
			SensorMgr->disengage((*it)->getSensorAgent());
		}
}

void
SensorBrowser::update()
{
	SensorManagerIterator it(sensorManager);

	hostInfos.clear();
	clear();

	SensorAgent* host;
	for (int i = 0 ; (host = it.current()); ++it, ++i)
	{
		QString hostName = sensorManager->getHostName(host);
		QListViewItem* lvi = new QListViewItem(this, hostName);
		CHECK_PTR(lvi);

		HostInfo* hostInfo = new HostInfo(host, hostName, lvi);
		CHECK_PTR(hostInfo);
		hostInfos.append(hostInfo);

		// request sensor list from host
		host->sendRequest("monitors", this, i);
	}
	setMouseTracking(FALSE);
}

void
SensorBrowser::answerReceived(int id, const QString& s)
{
	/* An answer has the following format:

	   cpuidle	integer
	   cpusys 	integer
	   cpunice	integer
	   cpuuser	integer
	   ps	table
	*/

	SensorTokenizer lines(s, '\n');

	for (unsigned int i = 0; i < lines.numberOfTokens(); ++i)
	{
		SensorTokenizer words(lines[i], '\t');

		QString sensorName = words[0];
		QString sensorType = words[1];

		/* Calling update() a rapid sequence will create pending 
		 * requests that do not get erased by calling clear(). Subsequent
		 * updates will receive the old pending answers so we need to make
		 * sure that we register each sensor only once. */
		if (hostInfos.at(id)->isRegistered(sensorName))
			return;

		// retrieve localized description from dictionary
		QString sensorDescription;
		if (!dict[sensorName])
			sensorDescription = sensorName;
		else
			sensorDescription = *(dict[sensorName]);

		QListViewItem* lvi = new QListViewItem(hostInfos.at(id)->getLVI(),
											   sensorDescription);
		CHECK_PTR(lvi);

		// add sensor info to internal data structure
		hostInfos.at(id)->addSensor(lvi, sensorName, sensorDescription,
									sensorType);
	}
	// The child indicator might need to be updated.
	repaintItem(hostInfos.at(id)->getLVI());
}

void
SensorBrowser::viewportMouseMoveEvent(QMouseEvent* ev)
{
	/* setMouseTracking(FALSE) seems to be broken. With current Qt
	 * mouse tracking cannot be turned off. So we have to check each event
	 * whether the LMB is really pressed. */

	if (!(ev->state() & LeftButton))
		return;

	QListViewItem* item = itemAt(ev->pos());
	if (!item)
		return;		// no item under cursor

	QListViewItem* parent = item->parent();
	if (!parent)
		return;		// item is not a sensor name

	// find the host info record that belongs to the LVI
	QListIterator<HostInfo> it(hostInfos);
	for ( ; it.current() && (*it)->getLVI() != parent; ++it)
		;
	assert(it.current());

	// Create text drag object as "<hostname> <sensorname> <sensortype>".
	dragText = (*it)->getHostName() + " "
		+ (*it)->getSensorName(item) + " "
		+ (*it)->getSensorType(item);

	QDragObject* dObj = new QTextDrag(dragText, this);
	CHECK_PTR(dObj);
	dObj->dragCopy();
}
