/*
    KSysGuard, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <qdom.h>

#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>

#include "SensorDisplay.h"
#include "SensorDisplay.moc"
#include "SensorManager.h"

SensorDisplay::SensorDisplay(QWidget* parent, const char* name) :
	QWidget(parent, name)
{
	sensors.setAutoDelete(true);

	// default interval is 2 seconds.
	timerInterval = 2000;
	timerId = NONE;
	timerOn();
	QWhatsThis::add(this, "dummy");

	frame = new QGroupBox(1, Qt::Vertical, "", this, "displayFrame"); 
	CHECK_PTR(frame);

	/* Let's call updateWhatsThis() in case the derived class does not do
	 * this. */
	updateWhatsThis();
	setFocusPolicy(QWidget::StrongFocus);
}

SensorDisplay::~SensorDisplay()
{
	killTimer(timerId);
}

void
SensorDisplay::registerSensor(SensorProperties* sp)
{
	sensors.append(sp);
}

void
SensorDisplay::unregisterSensor(uint idx)
{
	sensors.remove(idx);
}

void
SensorDisplay::timerEvent(QTimerEvent*)
{
	int i = 0;
	for (SensorProperties* s = sensors.first(); s; s = sensors.next(), ++i)
		sendRequest(s->hostName, s->name, i);
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
	else if (e->type() == QEvent::MouseButtonRelease &&
			 ((QMouseEvent*) e)->button() == LeftButton)
	{
		setFocus();
	}
	return QWidget::eventFilter(o, e);
}

void
SensorDisplay::sendRequest(const QString& hostName, const QString& cmd,
						   int id)
{
	if (!SensorMgr->sendRequest(hostName, cmd,
								(SensorClient*) this, id))
		sensorError(id, true);
}

void
SensorDisplay::sensorError(int sensorId, bool err)
{
	if (sensorId >= (int) sensors.count() || sensorId < 0)
		return;

	if (err == sensors.at(sensorId)->ok)
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
		sensors.at(sensorId)->ok = !err;
	}
}

void
SensorDisplay::updateWhatsThis()
{
	QWhatsThis::add(this, QString(i18n(
		"<qt><p>This is a sensor display. To customize a sensor display click "
		"and hold the right mouse button on either the frame or the "
		"display box and select the <i>Properties</i> entry from the popup "
		"menu. Select <i>Remove</i> to delete the display from the work "
		"sheet.</p>%1</qt>")).arg(additionalWhatsThis()));
}

void
SensorDisplay::collectHosts(QValueList<QString>& list)
{
	for (SensorProperties* s = sensors.first(); s; s = sensors.next())
		if (!list.contains(s->hostName))
			list.append(s->hostName);
}

QColor
SensorDisplay::restoreColorFromDOM(QDomElement& de, const QString& attr,
								   const QColor& fallback)
{
	bool ok;
	uint c = de.attribute(attr).toUInt(&ok);
	if (!ok)
		return (fallback);
	else
		return (QColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF));
}

void
SensorDisplay::addColorToDOM(QDomElement& de, const QString& attr,
							 const QColor& col)
{
	int r, g, b;
	col.rgb(&r, &g, &b);
	de.setAttribute(attr, (r << 16) | (g << 8) | b);
}
