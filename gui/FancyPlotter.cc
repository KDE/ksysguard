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
#include <qdom.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "SensorManager.h"
#include "FancyPlotter.moc"

static const int FrameMargin = 0;
static const int Margin = 5;
static const int HeadHeight = 10;

FancyPlotter::FancyPlotter(QWidget* parent, const char* name,
						   const char* title, int min, int max)
	: SensorDisplay(parent, name)
{
	sensorNames.setAutoDelete(true);

	meterFrame = new QGroupBox(this, "meterFrame"); 
	CHECK_PTR(meterFrame);
	meterFrame->setTitle(title);

	beams = 0;

	plotter = new SignalPlotter(this, "signalPlotter", min, max);
	CHECK_PTR(plotter);

	setMinimumSize(sizeHint());
}

FancyPlotter::~FancyPlotter()
{
	delete plotter;
	delete meterFrame;
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

	registerSensor(hostName, sensorName);
	++beams;

	if (!title.isEmpty())
		meterFrame->setTitle(title);

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", beams + 100);

	return (true);
}

void
FancyPlotter::resizeEvent(QResizeEvent*)
{
	meterFrame->setGeometry(0, 0, width(), height());
	QRect meter = meterFrame->contentsRect();
	meter.setX(Margin);
	meter.setY(HeadHeight + Margin);
	meter.setWidth(width() - 2 * Margin);
	meter.setHeight(height() - HeadHeight - 2 * Margin);

	plotter->setGeometry(meter);
}

QSize
FancyPlotter::sizeHint(void)
{
	QSize psize = plotter->minimumSize();
	return (QSize(psize.width() + Margin * 2,
				  psize.height() + Margin * 2 + HeadHeight));
}

void
FancyPlotter::answerReceived(int id, const QString& answer)
{
	static long s[5];

	if (id < 5)
		s[id] = answer.toLong();
	else if (id > 100)
	{
		SensorIntegerInfo info(answer);
		plotter->changeRange(id - 100, info.getMin(), info.getMax());
	}

	if (id == beams - 1)
	{
		plotter->addSample(s[0], s[1], s[2], s[3], s[4]);
	}
}

bool
FancyPlotter::load(QDomElement& domElem)
{
	QString title = domElem.attribute("title");
	if (!title.isEmpty())
		meterFrame->setTitle(title);
	QDomNodeList dnList = domElem.elementsByTagName("beam");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement el = dnList.item(i).toElement();
		addSensor(el.attribute("hostName"), el.attribute("sensorName"), "");
	}

	return (TRUE);
}

bool
FancyPlotter::save(QTextStream& s)
{
	s << "title=\"" << meterFrame->title() << "\">\n";

	for (int i = 0; i < beams; ++i)
	{
		s << "<beam hostName=\"" << *hostNames.at(i) << "\" "
		  << "sensorName=\"" << *sensorNames.at(i) << "\"/>\n";
	}
	return (TRUE);
}
