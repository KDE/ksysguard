/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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
*/

// $Id$

#include <stdlib.h>

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "PerfMonPage.moc"

PerfMonPage::PerfMonPage(QWidget* parent, const char* name)
	: QWidget(parent, name)
{
	noCpus = stat.getCpuCount();

	/*
	 * On SMP systems we call it a system load meter. On single CPU systems
	 * it's just a CPU load meter.
	 */
	if (noCpus == 1)
		cpuload = new FancyPlotter(this, "cpuload_meter",
								   i18n("CPU Load History"), 0, 100);
	else
		cpuload = new FancyPlotter(this, "cpuload_meter",
								   i18n("System Load History"), 0, 100);

	cpuload->setLowPass(TRUE);
	cpuload->addBeam(i18n("User (%)"), blue);
	cpuload->addBeam(i18n("Nice (%)"), yellow);
	cpuload->addBeam(i18n("System (%)"), red);

	int physical, swap, dum;
	stat.getMemoryInfo(physical, dum, dum, dum, dum);
	stat.getSwapInfo(swap, dum);

	memory = new FancyPlotter(this, "memory_meter",
							  i18n("Memory Usage History"),
							  0, physical + swap);
	memory->addBeam(i18n("Program (kB)"), blue);
	memory->addBeam(i18n("Buffer (kB)"), green);
	memory->addBeam(i18n("Cache (kB)"), yellow);
	memory->addBeam(i18n("Swap (kB)"), red);

	if (noCpus == 1)
	{
		/*
		 * On single CPU systems the performance meter features a memory
		 * plotter underneath a CPU load plotter. The layout is 2 rows and
		 * 1 column.
		 */
		gm = new QGridLayout(this, 2, 1);

		gm->addWidget(cpuload, 0, 0);
		gm->addWidget(memory, 1, 0);

		gm->setRowStretch(0, 1);
		gm->setRowStretch(1, 1);
	}
	else
	{
		/*
		 * On SMP systems the performance meter features a memory plotter
		 * to the right of the system load plotter. Underneath are the load
		 * plotter for each CPU. The layout is 1 + (noCpus/2) rows and 2
		 * columns.
		 */
		gm = new QGridLayout(this, 1 + (noCpus / 2), 2);
		gm->addWidget(cpuload, 0, 0);
		gm->addWidget(memory, 0, 1);

		// all rows and columns have the same size
		gm->setColStretch(0, 1);
		gm->setColStretch(1, 1);
		for (int row = 0; row < 1 + (noCpus / 2); row++)
			gm->setRowStretch(row, 1);

		for (int c = 0; c < noCpus; c++)
		{
			QString name;
			name = QString("cpu%1_meter").arg(c);
			QString label;
			label = i18n("CPU%1 Load History").arg(c);

			FancyPlotter* p = new FancyPlotter(this, name, label, 0, 100);
			p->setLowPass(TRUE);
			p->addBeam(i18n("User (%)"), blue);
			p->addBeam(i18n("Nice (%)"), yellow);
			p->addBeam(i18n("System (%)"), red);

			cpu.append(p);
			gm->addWidget(p, 1 + (c / 2), c % 2);
		}
	}

	gm->activate();

	setMinimumSize(sizeHint());

    timerID = startTimer(2000);
	printf("PerfMonPage: %d, %d\n", sizeHint().width(), sizeHint().height());
}

void
PerfMonPage::timerEvent(QTimerEvent*)
{
	int user, sys, nice, idle;
	if (!stat.getCpuLoad(user, sys, nice, idle))
	{
		KMessageBox::error(this, stat.getErrMessage());
		abort();
	}
	cpuload->addSample(user, nice, sys);

	int dum, used, buffer, cache, stotal, sfree;
	if (!stat.getMemoryInfo(dum, dum, used, buffer, cache) ||
		!stat.getSwapInfo(stotal, sfree))
	{
		KMessageBox::error(this, stat.getErrMessage());
		abort();
	}
	memory->addSample(used - (buffer + cache), buffer, cache, stotal - sfree);

	if (noCpus > 1)
		for (int i = 0; i < noCpus; i++)
		{
			if (!stat.getCpuXLoad(i, user, sys, nice, idle))
			{
				KMessageBox::error(this, stat.getErrMessage());
				abort();
			}
			cpu.at(i)->addSample(user, nice, sys);
		}
}
