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

#include <math.h>
#include <stdlib.h>

#include <qgroupbox.h>
#include <qlcdnumber.h>
#include <qdom.h>
#include <qtextstream.h>

#include "MultiMeter.moc"

static const int FrameMargin = 0;
static const int Margin = 5;
static const int HeadHeight = 10;

MultiMeter::MultiMeter(QWidget* parent, const char* name,
					   const QString& t, int, int)
	: SensorDisplay(parent, name)
{
	frame = new QGroupBox(this, "meterFrame");
	CHECK_PTR(frame);

	setTitle(t, unit);

	lcd = new QLCDNumber(this, "meterLCD");
	CHECK_PTR(lcd);
//	digits = (int) log10(QMAX(abs(minVal), abs(maxVal))) + 1;
}

bool
MultiMeter::addSensor(const QString& hostName, const QString& sensorName,
					  const QString&)
{
	registerSensor(hostName, sensorName);

	/* To differentiate between answers from value requests and info
	 * requests we use 100 for info requests. */
	sendRequest(hostName, sensorName + "?", 100);

	return (TRUE);
}

void
MultiMeter::answerReceived(int id, const QString& answer)
{
	if (id == 100)
	{
		SensorIntegerInfo info(answer);
		setTitle(title, info.getUnit());
	}
	else
		lcd->display(answer.toInt());
}

void
MultiMeter::resizeEvent(QResizeEvent*)
{
	frame->setGeometry(0, 0, width(), height());
 
	QRect box = frame->contentsRect();
	box.setX(Margin);
	box.setY(HeadHeight + Margin);
	box.setWidth(width() - 2 * Margin);
	box.setHeight(height() - HeadHeight - 2 * Margin);
	lcd->setGeometry(box);
}

void
MultiMeter::setTitle(const QString& t, const QString& u)
{
	title = t;
	unit = u;
	QString titleWithUnit = title;

	if (!unit.isEmpty())
		titleWithUnit = title + " [" + unit + "]";

	frame->setTitle(titleWithUnit);
}

bool
MultiMeter::load(QDomElement& domElem)
{
	title = domElem.attribute("title");
	setTitle(title, unit);

	return (TRUE);
}

bool
MultiMeter::save(QDomDocument&, QDomElement& display)
{
	display.setAttribute("title", title);

	return (TRUE);
}
