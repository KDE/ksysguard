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

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "SensorManager.h"
#include "FancyPlotter.moc"

static const int FrameMargin = 0;
static const int Margin = 5;
static const int HeadHeight = 0;

FancyPlotter::FancyPlotter(QWidget* parent, const char* name,
						   const char* title, int min, int max)
	: SensorDisplay(parent, name)
{
	meterFrame = new QGroupBox(this, "meterFrame"); 
	CHECK_PTR(meterFrame);
	meterFrame->setTitle(title);

	beams = 0;

	multiMeter = new MultiMeter(this, "multiMeter", min, max);
	CHECK_PTR(multiMeter);

	plotter = new SignalPlotter(this, "signalPlotter", min, max);
	CHECK_PTR(plotter);

	setMinimumSize(sizeHint());
}

FancyPlotter::~FancyPlotter()
{
	delete multiMeter;
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

	if (!multiMeter->addMeter(title, cols[beams]))
		return (false);

	registerSensor(hostName, sensorName);
	++beams;

	/* To differentiate between answers from value requests and info
	 * requests we add 100 to the beam index for info requests. */
	sendRequest(hostName, sensorName + "?", beams + 100);

	return (true);
}

void
FancyPlotter::resizeEvent(QResizeEvent*)
{
	int w = width();
	int h = height();

	meterFrame->setGeometry(0, 0, width(), height());
	QRect meter = meterFrame->contentsRect();
	meter.setX(meter.x() + Margin);
	meter.setY(meter.y() + HeadHeight + Margin);
	meter.setWidth(meter.width() - Margin);
	meter.setHeight(meter.height() - Margin);

	int mmw;
	QSize mmSize = multiMeter->sizeHint();

	if ((w < 280) || (h < (mmSize.height() + (2 * Margin + HeadHeight))))
	{
		mmw = 0;
		multiMeter->hide();
		plotter->setGeometry(meter);
	}
	else
	{
		mmw = 150;
		multiMeter->show();
		multiMeter->move(Margin, Margin + HeadHeight);
		multiMeter->resize(mmw, h - 40);
		plotter->move(mmw + 2 * Margin, Margin + HeadHeight);
		plotter->resize(w - mmw - (3 * Margin),
						h - (2 * Margin + HeadHeight));
	}
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
		/* TODO: set unit in multi meter
		 * unit = info.getUnit() */
	}

	if (id == beams - 1)
	{
		multiMeter->updateValues(s[0], s[1], s[2], s[3], s[4]);
		plotter->addSample(s[0], s[1], s[2], s[3], s[4]);
	}
}
