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

#include <qgroupbox.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qdom.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kdebug.h>

#include "SensorManager.h"
#include "FancyPlotterSettings.h"
#include "FancyPlotter.moc"

FancyPlotter::FancyPlotter(QWidget* parent, const char* name,
						   const QString& title, double min, double max)
	: SensorDisplay(parent, name)
{
	meterFrame = new QGroupBox(1, Qt::Vertical, title, this, "meterFrame"); 
	CHECK_PTR(meterFrame);
	if (!title.isEmpty())
		meterFrame->setTitle(title);

	beams = 0;
	flags = 0;

	plotter = new SignalPlotter(meterFrame, "signalPlotter", min, max);
	CHECK_PTR(plotter);

	setMinimumSize(sizeHint());

	/* All RMB clicks to the plotter widget will be handled by 
	 * SensorDisplay::eventFilter. */
	plotter->installEventFilter(this);

	modified = FALSE;
}

FancyPlotter::~FancyPlotter()
{
	delete plotter;
	delete meterFrame;
}

void
FancyPlotter::settings()
{
	fps = new FancyPlotterSettings(this, "FancyPlotterSettings", true);
	CHECK_PTR(fps);
	fps->title->setText(meterFrame->title());
	fps->minVal->setText(QString("%1").arg(plotter->getMin()));
	fps->maxVal->setText(QString("%1").arg(plotter->getMax()));
	connect(fps->applyButton, SIGNAL(clicked()),
			this, SLOT(applySettings()));

	if (fps->exec())
		applySettings();

	delete fps;
	fps = 0;
}

void
FancyPlotter::sensorError(bool err)
{
	if (err == sensorOk)
	{
		// this happens only when the sensorOk status needs to be changed.
		meterFrame->setEnabled(!err);
		sensorOk = !err;
	}
}

void
FancyPlotter::applySettings()
{
	meterFrame->setTitle(fps->title->text());
	plotter->changeRange(0, fps->minVal->text().toDouble(),
						 fps->maxVal->text().toDouble());

	modified = TRUE;
}

bool
FancyPlotter::addSensor(const QString& hostName, const QString& sensorName,
						const QString& title)
{
	static QColor cols[] = { blue, red, yellow, cyan, magenta };

	if ((unsigned) beams >= (sizeof(cols) / sizeof(QColor)))
		return (false);

	if (!plotter->addBeam(cols[beams]))
		return (false);

	registerSensor(hostName, sensorName, title);
	++beams;

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", beams + 100);

	return (true);
}

void
FancyPlotter::resizeEvent(QResizeEvent*)
{
	meterFrame->setGeometry(0, 0, width(), height());
}

QSize
FancyPlotter::sizeHint(void)
{
	return (meterFrame->sizeHint());
}

void
FancyPlotter::answerReceived(int id, const QString& answer)
{
	if (id < 5)
	{
		sampleBuf[id] = answer.toDouble();
		if (flags & (1 << id))
			qDebug("ERROR: FancyPlotter lost sample (%x, %d)", flags, beams);
		flags |= 1 << id;

		if (flags == (uint) ((1 << beams) - 1))
		{
			plotter->addSample(sampleBuf[0], sampleBuf[1], sampleBuf[2],
							   sampleBuf[3], sampleBuf[4]);
			flags = 0;
		}
	}
	else if (id > 100)
	{
		SensorFloatInfo info(answer);
		plotter->changeRange(id - 100, info.getMin(), info.getMax());
		timerOn();
	}
}

bool
FancyPlotter::load(QDomElement& domElem)
{
	modified = false;

	QString title = domElem.attribute("title");
	if (!title.isEmpty())
		meterFrame->setTitle(title);

	plotter->changeRange(0, domElem.attribute("min").toDouble(),
						 domElem.attribute("max").toDouble());

	QDomNodeList dnList = domElem.elementsByTagName("beam");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement el = dnList.item(i).toElement();
		addSensor(el.attribute("hostName"), el.attribute("sensorName"), "");
	}

	return (TRUE);
}

bool
FancyPlotter::save(QDomDocument& doc, QDomElement& display)
{
	display.setAttribute("title", meterFrame->title());
	display.setAttribute("min", plotter->getMin());
	display.setAttribute("max", plotter->getMax());

	for (int i = 0; i < beams; ++i)
	{
		QDomElement beam = doc.createElement("beam");
		display.appendChild(beam);
		beam.setAttribute("hostName", *hostNames.at(i));
		beam.setAttribute("sensorName", *sensorNames.at(i));
	}
	modified = FALSE;

	return (TRUE);
}
