/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
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
*/

// $Id$

#include <qgroupbox.h>

#include <kapp.h>

#include "FancyPlotter.moc"

FancyPlotter::FancyPlotter(QWidget* parent, const char* name,
						   const char* title, int min, int max)
	: QWidget(parent, name)
{
	meterFrame = new QGroupBox(this, "meterFrame"); 
	meterFrame->setTitle(title);
	CHECK_PTR(meterFrame);

	multiMeter = new MultiMeter(this, "multiMeter", min, max);
	CHECK_PTR(meterFrame);

	plotter = new SignalPlotter(this, "signalPlotter", min, max);
	CHECK_PTR(plotter);
}

FancyPlotter::~FancyPlotter()
{
	delete multiMeter;
	delete plotter;
	delete meterFrame;
}

void
FancyPlotter::resizeEvent(QResizeEvent*)
{
	int w = width();
	int h = height();

	meterFrame->move(5, 5);
	meterFrame->resize(w - 10, h - 10);

	int mmw;
	QSize mmSize = multiMeter->sizeHint();

	if ((w < 280) || (h < (mmSize.height() + 40)))
	{
		mmw = 0;
		multiMeter->hide();
	}
	else
	{
		mmw = 150;
		multiMeter->show();
		multiMeter->move(15, 25);
		multiMeter->resize(mmw, h - 40);
	}

	plotter->move(mmw + 25, 25);
	plotter->resize(w - mmw - 40, h - 40);
}
