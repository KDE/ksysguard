/*
    KSysGuard, the KDE System Guard
   
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

	KSysGuard is currently maintained by Chris Schlaeger
	<cs@kde.org>. Please do not commit any changes without consulting
	me first. Thanks!

	$Id$
*/

#include <qdragobject.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "SensorManager.h"
#include "FancyPlotter.h"
#include "KSysGuardApplet.moc"

extern "C"
{
	KPanelApplet* init(QWidget *parent, const QString& configFile)
	{
		KGlobal::locale()->insertCatalogue("ksysguardapplet");
		return new KSysGuardApplet(configFile, KPanelApplet::Normal,
								   KPanelApplet::Preferences, parent,
								   "ksysguardapplet");
    }
}

KSysGuardApplet::KSysGuardApplet(const QString& configFile, Type t,
								 int actions, QWidget *parent,
								 const char *name)
    : KPanelApplet(configFile, t, actions, parent, name)
{
	SensorMgr = new SensorManager();
	CHECK_PTR(SensorMgr);
	SensorMgr->engage("localhost", "", "ksysguardd");

	dockCnt = 2;
	docks = new SensorDisplay*[dockCnt];
	docks[0] = new FancyPlotter(this, "LoadMeter", "Load", 100.0, 100.0, true);
	docks[0]->addSensor("localhost", "cpu/user", "Load");
	docks[0]->addSensor("localhost", "cpu/sys", "Load");
	docks[0]->addSensor("localhost", "cpu/nice", "Load");

	docks[1] = new FancyPlotter(this, "Memory", "Memory", 100.0, 100.0, true);
	docks[1]->addSensor("localhost", "mem/physical/application", "Memory");
	docks[1]->addSensor("localhost", "mem/physical/buf", "Memory");
	docks[1]->addSensor("localhost", "mem/physical/cached", "Memory");

	setAcceptDrops(TRUE);
	show();
}

KSysGuardApplet::~KSysGuardApplet()
{
	delete SensorMgr;
}

int
KSysGuardApplet::widthForHeight(int h) const
{
	return (h * dockCnt);
}

int
KSysGuardApplet::heightForWidth(int w) const
{
	return (w * dockCnt);
}

void
KSysGuardApplet::resizeEvent(QResizeEvent*)
{
	layout();
}

void
KSysGuardApplet::preferences()
{
	kdDebug() << "Preferences" << endl;
}

void
KSysGuardApplet::layout()
{
	int w = width();
	int h = height();

	if (orientation() == Horizontal)
		for (uint i = 0; i < dockCnt; ++i)
			docks[i]->setGeometry(i * h, 0, h, h);
	else
		for (uint i = 0; i < dockCnt; ++i)
			docks[i]->setGeometry(0, i * w, w, w);
}

int
KSysGuardApplet::findDock(const QPoint& p)
{
	if (orientation() == Horizontal)
		return (p.x() / height());
	else
		return (p.y() / width());
}

void
KSysGuardApplet::dragEnterEvent(QDragEnterEvent* ev)
{
    ev->accept(QTextDrag::canDecode(ev));
}

void
KSysGuardApplet::dropEvent(QDropEvent* ev)
{
	QString dObj;

	if (QTextDrag::decode(ev, dObj))
	{
		// The host name, sensor name and type are seperated by a ' '.
		QString hostName = dObj.left(dObj.find(' '));
		dObj.remove(0, dObj.find(' ') + 1);
		QString sensorName = dObj.left(dObj.find(' '));
		dObj.remove(0, dObj.find(' ') + 1);
		QString sensorType = dObj.left(dObj.find(' '));
		dObj.remove(0, dObj.find(' ') + 1);
		QString sensorDescr = dObj;

		if (hostName.isEmpty() || sensorName.isEmpty() ||
			sensorType.isEmpty())
		{
			return;
		}

		int dock = findDock(ev->pos());
		if (docks[dock] == 0)
		{
			docks[dock] = new FancyPlotter(this, "FancyPlotter",
										   sensorDescr);
			layout();
			kdDebug() << "New display added" << endl;

		}
		docks[dock]->addSensor(hostName, sensorName, sensorDescr);
	}
}

void
KSysGuardApplet::customEvent(QCustomEvent* ev)
{
	if (ev->type() == QEvent::User)
	{
		// SensorDisplays send out this event if they want to be removed.
		removeDisplay((SensorDisplay*) ev->data());
		delete ev;
	}
}

void
KSysGuardApplet::removeDisplay(SensorDisplay* sd)
{
	for (uint i = 0; i < dockCnt; ++i)
		if (sd == docks[i])
		{
			delete docks[i];
			docks[i] = 0;
			return;
		}
}
