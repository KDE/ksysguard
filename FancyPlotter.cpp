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
	beamNames.setAutoDelete(TRUE);

	signalFrame = new QGroupBox(this, "signalFrame"); 
	signalFrame->setTitle(title);
	CHECK_PTR(signalFrame);

	plotter = new SignalPlotter(this, "signalPlotter", min, max);
	CHECK_PTR(plotter);
}

FancyPlotter::~FancyPlotter()
{
	delete plotter;
	delete signalFrame;
//	delete valuesFrame;
}

void
FancyPlotter::resizeEvent(QResizeEvent*)
{
	int w = width();
	int h = height();

#if 0
	QListIterator<QLabel> li(beamNames);
	QLabel* name;
	int nameWidth = 0;
	while ((name = ++li))
		if (nameWidth < name->sizeHint().width())
			nameWidth = name->sizeHint().width();

	if (w > nameWidth + 20 + 60 + 20)
	{
		signalFrame->move(nameWidth + 20, 5);
		signalFrame->resize(w - 10 - nameWidth - 20, h - 10);

		plotter->move(nameWidth + 25, 25);
		plotter->resize(w - 30 - nameWidth - 20, h - 40);
	}
	else
#endif
	{
		signalFrame->move(5, 5);
		signalFrame->resize(w - 10, h - 10);
		plotter->move(15, 25);
		plotter->resize(w - 30, h - 40);
	}
}
