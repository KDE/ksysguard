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

#include <qdom.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qframe.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtextstream.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include "DancingBars.h"
#include "FancyPlotter.h"
#include "KSysGuardApplet.moc"
#include "KSysGuardAppletSettings.h"
#include "MultiMeter.h"
#include "SensorManager.h"
#include "StyleEngine.h"

extern "C"
{
	KPanelApplet* init(QWidget *parent, const QString& configFile)
	{
		KGlobal::locale()->insertCatalogue("ksysguard");
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
	ksgas = 0;

	SensorMgr = new SensorManager();
	Q_CHECK_PTR(SensorMgr);

	Style = new StyleEngine();
	Q_CHECK_PTR(Style);

	dockCnt = 1;
	docks = new QWidget*[dockCnt];
	Q_CHECK_PTR(docks);

	docks[0] = new QFrame(this);
	Q_CHECK_PTR(docks[0]);
	((QFrame*)docks[0])->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	QToolTip::add(docks[0],
				  i18n("Drag sensors from the KDE System Guard into "
					   "this cell."));
	updateInterval = 2;
	sizeRatio = 1.0;

	load();

	setAcceptDrops(TRUE);

	KMessageBox::information(
		this, i18n("Drag sensors from the sensor-tree of ksysguard to the empty fields in the applet"),
		QString::null, "ShowSBUseInfo");
}

KSysGuardApplet::~KSysGuardApplet()
{
	save();

	if (ksgas)
		delete ksgas;

	delete Style;
	delete SensorMgr;
}

int
KSysGuardApplet::widthForHeight(int h) const
{
	return ((int) (h * sizeRatio) * dockCnt);
}

int
KSysGuardApplet::heightForWidth(int w) const
{
	return ((int) (w * sizeRatio) * dockCnt);
}

void
KSysGuardApplet::resizeEvent(QResizeEvent*)
{
	layout();
}

void
KSysGuardApplet::preferences()
{
	ksgas = new KSysGuardAppletSettings(
		this, "KSysGuardAppletSettings", true);
	Q_CHECK_PTR(ksgas);
																
	connect(ksgas->applyButton, SIGNAL(clicked()),
			this, SLOT(applySettings()));

	ksgas->dockCnt->setValue(dockCnt);
	ksgas->ratio->setValue(sizeRatio * 100.0);
	ksgas->interval->setValue(updateInterval);
	if (ksgas->exec())
		applySettings();

	delete ksgas;
	ksgas = 0;

	save();
}

void
KSysGuardApplet::applySettings()
{
	updateInterval = ksgas->interval->text().toUInt();
	sizeRatio = ksgas->ratio->text().toDouble() / 100.0;
	resizeDocks(ksgas->dockCnt->text().toUInt());

	for (uint i = 0; i < dockCnt; ++i)
		if (!docks[i]->isA("QFrame"))
			((SensorDisplay*) docks[i])->setUpdateInterval(updateInterval);

	save();
}

void
KSysGuardApplet::layout()
{
	if (orientation() == Horizontal)
	{
		int h = height();
		int w = (int) (h * sizeRatio);
		for (uint i = 0; i < dockCnt; ++i)
			if (docks[i])
				docks[i]->setGeometry(i * w, 0, w, h);
	}
	else
	{
		int w = width();
		int h = (int) (w * sizeRatio);
		for (uint i = 0; i < dockCnt; ++i)
			if (docks[i])
				docks[i]->setGeometry(0, i * h, w, h);
	}
}

int
KSysGuardApplet::findDock(const QPoint& p)
{
	if (orientation() == Horizontal)
		return (p.x() / (int) (height() * sizeRatio));
	else
		return (p.y() / (int) (width() * sizeRatio));
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
		if (docks[dock]->isA("QFrame"))
		{
			if (sensorType == "integer")
			{
				QPopupMenu popup;
				QWidget *wdg;

				popup.insertItem(i18n("Select a display type"), 0);
				popup.setItemEnabled(0, false);
				popup.insertSeparator();
				popup.insertItem(i18n("&Signal Plotter"), 1);
				popup.insertItem(i18n("&Multimeter"), 2);
				popup.insertItem(i18n("&Dancing Bars"), 3);
				switch (popup.exec(QCursor::pos()))
				{
					case 1:
						wdg = new FancyPlotter(this, "FancyPlotter", sensorDescr, 100.0, 100.0, true);
						Q_CHECK_PTR(wdg);
						break;

					case 2:
						wdg = new MultiMeter(this, "MultiMeter", sensorDescr, 100.0, 100.0, true);
						Q_CHECK_PTR(wdg);
						break;

					case 3:
						wdg = new DancingBars(this, "DancingBars", sensorDescr, 100.0, 100.0, true);
						Q_CHECK_PTR(wdg);
						break;
				}

				if (wdg)
				{
					delete docks[dock];
					docks[dock] = wdg;
					layout();
					docks[dock]->show();
				}
			 }
			 else
			 {
				KMessageBox::sorry(
					this,
					i18n("The KSysGuard applet does not support displaying of\n"
						 "this type of sensor. Please choose another sensor."));
			 }
		}
		((SensorDisplay*) docks[dock])->
			addSensor(hostName, sensorName, sensorType, sensorDescr);
	}

	save();
}

void
KSysGuardApplet::customEvent(QCustomEvent* ev)
{
	if (ev->type() == QEvent::User)
	{
		// SensorDisplays send out this event if they want to be removed.
		removeDisplay((SensorDisplay*) ev->data());
		save();
	}
}

void
KSysGuardApplet::removeDisplay(SensorDisplay* sd)
{
	for (uint i = 0; i < dockCnt; ++i)
		if (sd == docks[i])
		{
			delete docks[i];

			docks[i] = new QFrame(this);
			Q_CHECK_PTR(docks[i]);
			((QFrame*) docks[i])->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
			QToolTip::add(docks[i],
						  i18n("Drag sensors from the KDE System Guard into "
							   "this cell."));
			layout();
			if (isVisible())
				docks[i]->show();
			return;
		}
}

void
KSysGuardApplet::resizeDocks(uint newDockCnt)
{
	/* This function alters the number of available docks. The number of
	 * docks can be increased or decreased. We try to preserve existing
	 * docks if possible. */

	if (newDockCnt == dockCnt)
	{
		emit updateLayout();
		return;
	}

	// Create and initialize new dock array.
	QWidget** tmp = new QWidget*[newDockCnt];
	Q_CHECK_PTR(tmp);

	uint i;
	// Copy old docks into new.
	for (i = 0; (i < newDockCnt) && (i < dockCnt); ++i)
		tmp[i] = docks[i];
	// Destruct old docks that are not needed anymore.
	for (i = newDockCnt; i < dockCnt; ++i)
		if (docks[i])
			delete docks[i];
	
	for (i = dockCnt; i < newDockCnt; ++i)
	{
		tmp[i] = new QFrame(this);
		Q_CHECK_PTR(tmp[i]);
		((QFrame*) tmp[i])->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
		QToolTip::add(tmp[i],
					  i18n("Drag sensors from the KDE System Guard into "
						   "this cell."));
		if (isVisible())
			tmp[i]->show();
	}
	// Destruct old dock.
	delete docks;

	docks = tmp;
	dockCnt = newDockCnt;

	emit updateLayout();
}

bool
KSysGuardApplet::load()
{
	KStandardDirs* kstd = KGlobal::dirs();
	kstd->addResourceType("data", "share/apps/ksysguard");
	QString fileName = kstd->findResource("data", "KSysGuardApplet.xml");

	QFile file(fileName);
	if (!file.open(IO_ReadOnly))
	{
		KMessageBox::sorry(this, i18n("Can't open the file %1.")
						   .arg(fileName));
		return (FALSE);
	}

	// Parse the XML file.
	QDomDocument doc;
	// Read in file and check for a valid XML header.
	if (!doc.setContent(&file))
	{
		KMessageBox::sorry(
			this,
			i18n("The file %1 does not contain valid XML.").arg(fileName));
		return (FALSE);
	}
	// Check for proper document type.
	if (doc.doctype().name() != "KSysGuardApplet")
	{
		KMessageBox::sorry(
			this,
			i18n("The file %1 does not contain a valid applet\n"
				 "definition, which must have a document type\n"
				 "'KSysGuardApplet'.").arg(fileName));
		return (FALSE);
	}

	QDomElement element = doc.documentElement();
	bool ok;
	uint d = element.attribute("dockCnt").toUInt(&ok);
	if (!ok)
		d = 1;
	sizeRatio = element.attribute("sizeRatio").toDouble(&ok);
	if (!ok)
		sizeRatio = 1.0;
	updateInterval = element.attribute("interval").toUInt(&ok);
	if (!ok)
		updateInterval = 2;
	resizeDocks(d);

	/* Load lists of hosts that are needed for the work sheet and try
	 * to establish a connection. */
	QDomNodeList dnList = element.elementsByTagName("host");
	uint i;
	for (i = 0; i < dnList.count(); ++i)
	{
		QDomElement element = dnList.item(i).toElement();
		bool ok;
		int port = element.attribute("port").toInt(&ok);
		if (!ok)
			port = -1;
		SensorMgr->engage(element.attribute("name"),
						  element.attribute("shell"),
						  element.attribute("command"), port);
	}

	// Load the displays and place them into the work sheet.
	dnList = element.elementsByTagName("display");
	for (i = 0; i < dnList.count(); ++i)
	{
		QDomElement element = dnList.item(i).toElement();
		uint dock = element.attribute("dock").toUInt();
		if (i >= dockCnt)
		{
			kdDebug () << "Dock number " << i << " out of range "
					   << dockCnt << endl;
			return (FALSE);
		}

		QString classType = element.attribute("class");
		SensorDisplay* newDisplay;
		if (classType == "FancyPlotter")
			newDisplay = new FancyPlotter(this, "FancyPlotter", "Dummy", 100.0, 100.0, true);
		else if (classType == "MultiMeter")
			newDisplay = new MultiMeter(this, "MultiMeter", "Dummy", 100.0, 100.0, true);
		else if (classType == "DancingBars")
			newDisplay = new DancingBars(this, "DancingBars", "Dummy", 100.0, 100.0, true);
		else
		{
			KMessageBox::sorry(
				this,
				i18n("The KSysGuard applet does not support displaying of\n"
					 "this type of sensor. Please choose another sensor."));
			return (FALSE);
		}
		Q_CHECK_PTR(newDisplay);

		newDisplay->setUpdateInterval(updateInterval);
		// load display specific settings
		if (!newDisplay->createFromDOM(element))
			return (FALSE);

		delete docks[dock];
		docks[dock] = newDisplay;
	}
	return (TRUE);
}

bool
KSysGuardApplet::save()
{
	// Parse the XML file.
	QDomDocument doc("KSysGuardApplet");
	doc.appendChild(doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\""));

	// save work sheet information
	QDomElement ws = doc.createElement("WorkSheet");
	doc.appendChild(ws);
	ws.setAttribute("dockCnt", dockCnt);
	ws.setAttribute("sizeRatio", sizeRatio);
	ws.setAttribute("interval", updateInterval);

	QStringList hosts;
	uint i;
	for (i = 0; i < dockCnt; ++i)
		if (!docks[i]->isA("QFrame"))
			((SensorDisplay*) docks[i])->collectHosts(hosts);

	// save host information (name, shell, etc.)
	QStringList::Iterator it;
	for (it = hosts.begin(); it != hosts.end(); ++it)
	{
		QString shell, command;
		int port;

		if (SensorMgr->getHostInfo(*it, shell, command, port))
		{
			QDomElement host = doc.createElement("host");
			ws.appendChild(host);
			host.setAttribute("name", *it);
			host.setAttribute("shell", shell);
			host.setAttribute("command", command);
			host.setAttribute("port", port);
		}
	}
	
	for (i = 0; i < dockCnt; ++i)
		if (!docks[i]->isA("QFrame"))
		{
			QDomElement display = doc.createElement("display");
			ws.appendChild(display);
			display.setAttribute("dock", i);
			display.setAttribute("class", docks[i]->className());

			((SensorDisplay*) docks[i])->addToDOM(doc, display);
		}	

	KStandardDirs* kstd = KGlobal::dirs();
	kstd->addResourceType("data", "share/apps/ksysguard");
	QString fileName = kstd->saveLocation("data", "ksysguard");
	fileName += "/KSysGuardApplet.xml";

	QFile file(fileName);
	if (!file.open(IO_WriteOnly))
	{
		KMessageBox::sorry(this, i18n("Can't save file %1")
						   .arg(fileName));
		return (FALSE);
	}
	QTextStream s(&file);
	s.setEncoding(QTextStream::UnicodeUTF8);
	s << doc;
	file.close();

	return (TRUE);
}
