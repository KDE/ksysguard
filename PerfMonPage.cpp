/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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

#include <stdlib.h>

#include <qmessagebox.h>

#include <kapp.h>

#include "PerfMonPage.moc"

PerfMonPage::PerfMonPage(QWidget* parent = 0, const char* name = 0)
	: QWidget(parent, name)
{
	cpu = new FancyPlotter(this, "cpu_meter", i18n("CPU Load History"),
						   0, 100);
	cpu->setLowPass(TRUE);
	cpu->addBeam(i18n("User"), blue);
	cpu->addBeam(i18n("System"), red);
	cpu->addBeam(i18n("Nice"), yellow);

	int physical, swap, dum;
	stat.getMemoryInfo(physical, dum, dum, dum, dum);
	stat.getSwapInfo(swap, dum);

	memory = new FancyPlotter(this, "memory_meter",
							  i18n("Memory Usage History"),
							  0, physical + swap);
	memory->addBeam(i18n("Used"), blue);
	memory->addBeam(i18n("Buffer"), green);
	memory->addBeam(i18n("Cache"), yellow);
	memory->addBeam(i18n("Swap"), red);

    timerID = startTimer(2000);
}

void
PerfMonPage::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);
	cpu->resize(width(), height() / 2);

	memory->move(0, height() / 2);
	memory->resize(width(), height() / 2);
}

void
PerfMonPage::timerEvent(QTimerEvent*)
{
	int user, sys, nice, idle;
	if (!stat.getCpuLoad(user, sys, nice, idle))
	{
		QMessageBox::critical(this, "Task Manager", stat.getErrMessage(),
							  0, 0);
		abort();
	}
	cpu->addSample(user, sys, nice);


	int dum, used, buffer, cache, stotal, sfree;
	if (!stat.getMemoryInfo(dum, dum, used, buffer, cache) ||
		!stat.getSwapInfo(stotal, sfree))
	{
		QMessageBox::critical(this, "Task Manager", stat.getErrMessage(),
							  0, 0);
		abort();
	}

	memory->addSample(used - (buffer + cache), buffer, cache, stotal - sfree);
}
