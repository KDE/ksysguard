/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
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
*/

// $Id$

#include <math.h>

#include <qcolor.h>

#include "MultiMeter.moc"

static const int GMBorder = 2;
static const int LCDHeight = 21;

MultiMeter::MultiMeter(QWidget* parent, const char* name, int minVal,
					   int maxVal)
	: QWidget(parent, name)
{
	meters = 0;
	digits = (int) log10(QMAX(abs(minVal), abs(maxVal))) + 1;
	if (minVal < 0)
		digits++;

	meterLabel.setAutoDelete(true);

	gm = new QGridLayout(this, 1, 2);
}

bool
MultiMeter::addMeter(const char* name, QColor col)
{
	delete gm;

	gm = new QGridLayout(this, ++meters, 2, GMBorder);

	QLabel* lab = new QLabel(name, this);
	meterLabel.append(lab);

	QLCDNumber* lcd = new QLCDNumber(this);
	lcd->setMaximumHeight(LCDHeight);
	lcd->setMinimumHeight(LCDHeight);
	lcd->setMaximumWidth(60);
	lcd->setBackgroundColor(col);
	lcd->setNumDigits(digits);
	meterLcd.append(lcd);

	for (int i = 0; i < meters; i++)
	{
		gm->addWidget(meterLabel.at(i), i, 0);
		gm->addWidget(meterLcd.at(i), i, 1);
		gm->setRowStretch(i, 1);
	}

	gm->activate();

	return (true);
}

void
MultiMeter::updateValues(int v0, int v1, int v2, int v3, int v4)
{
	if (meters > 0)
		meterLcd.at(0)->display(v0);
	if (meters > 1)
		meterLcd.at(1)->display(v1);
	if (meters > 2)
		meterLcd.at(2)->display(v2);
	if (meters > 3)
		meterLcd.at(3)->display(v3);
	if (meters > 4)
		meterLcd.at(4)->display(v4);
}

QSize
MultiMeter::sizeHint(void)
{
	int maxLab = 0;
	int maxLcd = 0;

	for (int i = 0; i < meters; i++)
	{
		if (meterLabel.at(i)->width() > maxLab)
			maxLab = meterLabel.at(i)->width();
		if (meterLcd.at(i)->width() > maxLcd)
			maxLcd = meterLcd.at(i)->width();
	}

	QSize hint;
	hint.setWidth(maxLab + maxLcd + 3 * GMBorder);
	hint.setHeight(GMBorder + meters * (GMBorder + LCDHeight));

	return (hint);
}
