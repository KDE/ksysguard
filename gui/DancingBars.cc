/*
    KTop, the KDE Task Manager and System Monitor
   
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qgroupbox.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdom.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kdebug.h>

#include "SensorManager.h"
#include "DancingBarsSettings.h"
#include "DancingBars.moc"

DancingBars::DancingBars(QWidget* parent, const char* name,
						 const QString& title, int min, int max)
	: SensorDisplay(parent, name)
{
	if (!title.isEmpty())
		frame->setTitle(title);

	bars = 0;
	flags = 0;

	plotter = new BarGraph(frame, "signalPlotter", min, max);
	CHECK_PTR(plotter);

	setMinimumSize(sizeHint());

	/* All RMB clicks to the plotter widget will be handled by 
	 * SensorDisplay::eventFilter. */
	plotter->installEventFilter(this);

	modified = false;
}

DancingBars::~DancingBars()
{
}

void
DancingBars::sensorError(int sensorId, bool err)
{
	if (sensorId >= (int) sensors.count() || sensorId < 0)
		return;

	if (err == sensors.at(sensorId)->ok)
	{
		// this happens only when the sensorOk status needs to be changed.
		sensors.at(sensorId)->ok = !err;
	}
	bool ok = true;
	for (uint i = 0; i < sensors.count(); ++i)
		if (!sensors.at(i)->ok)
		{
			ok = false;
			break;
		}
	plotter->setSensorOk(ok);
}

void
DancingBars::settings()
{
	dbs = new DancingBarsSettings(this, "DancingBarsSettings", true);
	CHECK_PTR(dbs);

	dbs->title->setText(frame->title());
	dbs->title->setFocus();
	dbs->maximumValue->setValue(plotter->getMax());
	long l, u;
	bool la, ua;
	plotter->getLimits(l, la, u, ua);
	if (ua)
		dbs->enableAlarm->setChecked(true);
	dbs->limit->setValue(u);

	connect(dbs->applyButton, SIGNAL(clicked()),
			this, SLOT(applySettings()));

	if (dbs->exec())
		applySettings();

	delete dbs;
	dbs = 0;
}

void
DancingBars::applySettings()
{
	frame->setTitle(dbs->title->text());
	plotter->changeRange(plotter->getMin(),
						 dbs->maximumValue->text().toLong());
	if (dbs->enableAlarm->isChecked())
		plotter->setLimits(0, false, dbs->limit->text().toLong(), true);
	else
		plotter->setLimits(0, false, 0, false);

	repaint();
	modified = true;
}

bool
DancingBars::addSensor(const QString& hostName, const QString& sensorName,
					   const QString& title)
{
	if (bars >= 32)
		return (false);

	if (!plotter->addBar(title))
		return (false);

	registerSensor(new SensorProperties(hostName, sensorName, title));
	++bars;
	sampleBuf.resize(bars);

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", bars + 100);

	return (true);
}

void
DancingBars::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(0, 0, width(), height());
}

QSize
DancingBars::sizeHint(void)
{
	return (frame->sizeHint());
}

void
DancingBars::answerReceived(int id, const QString& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);

	if (id <= 100)
	{
		sampleBuf[id] = answer.toLong();
		if (flags & (1 << id))
		{
			kdDebug() << "ERROR: DancingBars lost sample (" << flags
					  << ", " << bars << ")" << endl;
			sensorError(id, true);
		}
		flags |= 1 << id;

		if (flags == (uint) ((1 << bars) - 1))
		{
			plotter->updateSamples(sampleBuf);
			flags = 0;
		}
	}
	else if (id > 100)
	{
		SensorIntegerInfo info(answer);
		if (id == 100)
		{
			plotter->changeRange(info.getMin(), info.getMax());
		}
		timerOn();
	}
}

bool
DancingBars::createFromDOM(QDomElement& domElem)
{
	modified = false;

	frame->setTitle(domElem.attribute("title"));

	plotter->changeRange(domElem.attribute("min").toLong(),
						 domElem.attribute("max").toLong());
	plotter->setLimits(domElem.attribute("lowlimit").toLong(),
					   domElem.attribute("lowlimitactive").toInt(),
					   domElem.attribute("uplimit").toLong(),
					   domElem.attribute("uplimitactive").toInt());

	QDomNodeList dnList = domElem.elementsByTagName("beam");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement el = dnList.item(i).toElement();
		addSensor(el.attribute("hostName"), el.attribute("sensorName"),
				  el.attribute("sensorDescr"));
	}

	return (true);
}

bool
DancingBars::addToDOM(QDomDocument& doc, QDomElement& display, bool save)
{
	display.setAttribute("title", frame->title());
	display.setAttribute("min", (int) plotter->getMin());
	display.setAttribute("max", (int) plotter->getMax());
	long l, u;
	bool la, ua;
	plotter->getLimits(l, la, u, ua);
	display.setAttribute("lowlimit", (int) l);
	display.setAttribute("lowlimitactive", la);
	display.setAttribute("uplimit", (int) u);
	display.setAttribute("uplimitactive", ua);

	for (int i = 0; i < bars; ++i)
	{
		QDomElement beam = doc.createElement("beam");
		display.appendChild(beam);
		beam.setAttribute("hostName", sensors.at(i)->hostName);
		beam.setAttribute("sensorName", sensors.at(i)->name);
		beam.setAttribute("sensorDescr", sensors.at(i)->description);
	}

	if (save)
		modified = false;

	return (true);
}
