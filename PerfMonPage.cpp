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

#include <kapp.h>

#include "PerfMonPage.moc"

PerfMonPage::PerfMonPage(QWidget* parent = 0, const char* name = 0)
	: QWidget(parent, name)
{
	cpubox = new QGroupBox(this, "_cpumon");
	CHECK_PTR(cpubox); 
	cpubox->setTitle(i18n("CPU load"));
	cpubox1 = new QGroupBox(this, "_cpumon1");
	CHECK_PTR(cpubox1); 
	cpubox1->setTitle(i18n("CPU load history"));
	cpu_cur = new QWidget(this, "cpu_child");
	CHECK_PTR(cpu_cur); 
	cpu_cur->setBackgroundColor(black);
	cpumon = new CpuMon(cpubox1, "cpumon", cpu_cur);
	CHECK_PTR(cpumon);

	membox = new QGroupBox(this, "_memmon");
	CHECK_PTR(membox);
	membox->setTitle(i18n("Memory"));
	membox1 = new QGroupBox(this, "_memhistory");
	CHECK_PTR(membox1);
	membox1->setTitle(i18n("Memory usage history"));
	mem_cur = new QWidget(this, "mem_child");
	CHECK_PTR(mem_cur);
	mem_cur->setBackgroundColor(black);
	memmon = new MemMon(membox1, "memmon", mem_cur);

#ifdef ADD_SWAPMON
   	swapbox = new QGroupBox(this, "_swapmon");
   	CHECK_PTR(swapbox);
   	swapbox->setTitle(i18n("Swap"));
   	swapbox1 = new QGroupBox(this, "_swaphistory");
   	CHECK_PTR(swapbox1);
   	swapbox1->setTitle(i18n("Swap history"));
   	swap_cur = new QWidget(this,"swap_child");
   	CHECK_PTR(swap_cur);
   	swap_cur->setBackgroundColor(black);
   	swapmon = new SwapMon(swapbox1, "swapmon", swap_cur);
#endif
}

void
PerfMonPage::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);

	int w = width();
	int h = height();
   
#ifdef ADD_SWAPMON
	cpubox->setGeometry(10, 10, 80, (h - 40) / 3);
	cpubox1->setGeometry(100, 10, w - 110, (h - 40) / 3);
	cpu_cur->setGeometry(20, 10 + 18, 60, (h - 118) / 3);
	cpumon->setGeometry(10, 18, w - 130, (h - 118) / 3);
	membox->setGeometry(10, (h + 20) / 3, 80, (h - 40) / 3);
	membox1->setGeometry(100, (h + 20) / 3, w - 110, (h - 40) / 3);
	mem_cur->setGeometry(20, (h + 20) / 3 + 18, 60,(h - 118) / 3);
	memmon->setGeometry(10, 18, w - 130, (h - 118) / 3);
	swapbox->setGeometry(10, (2 * h + 10) / 3, 80, (h - 40) / 3);
	swapbox1->setGeometry(100, (2 * h + 10) / 3, w - 110, (h - 40) / 3);
	swap_cur->setGeometry(20, (2 * h + 10) / 3 + 18, 60, (h - 118) / 3);
	swapmon->setGeometry(10, 18, w - 130, (h - 118) / 3);
#else
	cpubox->setGeometry(10, 10, 80, (h / 2) - 30);
	cpubox1->setGeometry(100, 10, w - 110, (h / 2) - 30);
	cpu_cur->setGeometry(20, 30, 60, (h / 2) - 60);
	cpumon->setGeometry(10, 20, cpubox1->width() - 20, cpubox1->height() - 30);

	membox->setGeometry(10, h / 2, 80, (h / 2) - 30);
	membox1->setGeometry(100, h / 2, w - 110, (h / 2) - 30);
	mem_cur->setGeometry(20, h / 2 + 20, 60, (h / 2) - 60);
	memmon->setGeometry(10, 20, membox1->width() - 20, membox1->height() - 30);
#endif
}
