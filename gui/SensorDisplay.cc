/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

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

#include <qpopupmenu.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>

#include "SensorDisplay.h"
#include "SensorDisplay.moc"
#include "SensorManager.h"

SensorDisplay::SensorDisplay(QWidget* parent, const char* name) :
	QWidget(parent, name)
{
	// default interval is 2 seconds.
	timerInterval = 2000;
	timerId = NONE;
	timerOn();
	sensorOk = TRUE;

	QWhatsThis::add(this, i18n(
		"This is a sensor display. To customize a sensor display click "
		"and hold the right mouse button on either the frame or the "
		"display box and select the 'Properties' entry from the popup "
		"menu. Select 'Remove' to delete the display from the work "
		"sheet."));
}

SensorDisplay::~SensorDisplay()
{
	killTimer(timerId);
}

void
SensorDisplay::registerSensor(const QString& hostName,
							  const QString& sensorName,
							  const QString& sensorDescription)
{
	hostNames.append(hostName);
	sensorNames.append(sensorName);
	sensorDescriptions.append(sensorDescription);
}

void
SensorDisplay::timerEvent(QTimerEvent*)
{
	QStringList::Iterator hnIt;
	QStringList::Iterator snIt;

	hnIt = hostNames.begin();
	snIt = sensorNames.begin();
	for (int i = 0; hnIt != hostNames.end(); ++hnIt, ++snIt, ++i)
	{
		sendRequest(*hnIt, *snIt, i);
	}
}

bool
SensorDisplay::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::MouseButtonPress &&
		((QMouseEvent*) e)->button() == RightButton)
	{
		QPopupMenu pm;
		if (hasSettingsDialog())
			pm.insertItem(i18n("&Properties"), 1);
		pm.insertItem(i18n("&Remove Display"), 2);
		switch (pm.exec(QCursor::pos()))
		{
		case 1:
			this->settings();
			break;
		case 2:
			QCustomEvent* ev = new QCustomEvent(QEvent::User);
			ev->setData(this);
			kapp->postEvent(parent(), ev);
			break;
		}
		return (TRUE);
	}
	return QWidget::eventFilter(o, e);
}

void
SensorDisplay::sendRequest(const QString& hostName, const QString& cmd,
						   int id)
{
	if (!SensorMgr->sendRequest(hostName, cmd,
								(SensorClient*) this, id))
		sensorError(true);
}

void
SensorDisplay::sensorError(bool err)
{
	if (err == sensorOk)
	{
		if (err)
		{
			/* The sensor has been lost. The default implementation
			 * simply removes the display from the worksheet. This
			 * function should be overloaded to signal the error
			 * condition in the display but keep the display in the
			 * worksheet. */
			QCustomEvent* ev = new QCustomEvent(QEvent::User);
			ev->setData(this);
			kapp->postEvent(parent(), ev);
		}
		sensorOk = !err;
	}
}

bool
SensorDisplay::loadSensor(QDomElement& /*dolElement*/)
{
	// TODO: not sure whether it makes sense to have this function.
	return (false);
}

bool
SensorDisplay::saveSensor(QDomDocument& /*doc*/, QDomElement& /*sensor*/)
{
	// TODO: not sure whether it makes sense to have this function.
	return (false);
}

void
SensorDisplay::collectHosts(QValueList<QString>& list)
{
	QStringList::Iterator hnIt;

	for (hnIt = hostNames.begin(); hnIt != hostNames.end(); ++hnIt)
		if (!list.contains(*hnIt))
			list.append(*hnIt);
}
