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

#ifndef _PerfMonPage_h_
#define _PerfMonPage_h_

#include <qwidget.h>
#include <qgroupbox.h>

#include "cpu.h"
#include "memory.h"

class PerfMonPage : public QWidget
{
	Q_OBJECT

public:
	PerfMonPage(QWidget* parent = 0, const char* name = 0);
	~PerfMonPage()
	{
		delete cpumon;
		delete cpu_cur;
		delete cpubox;
		delete cpubox1;

		delete memmon;
		delete mem_cur;
		delete membox;
		delete membox1;

#ifdef ADD_SWAPMON
		delete swapmon;
		delete swap_cur;
		delete swapbox;
		delete swapbox1;
#endif
	}

	virtual void resizeEvent(QResizeEvent* ev);

private:
	CpuMon* cpumon;
	QWidget* cpu_cur;
	QGroupBox* cpubox;
	QGroupBox* cpubox1;

	MemMon* memmon;
	QWidget* mem_cur;
	QGroupBox* membox;
	QGroupBox* membox1;

#ifdef ADD_SWAPMON
	SwapMon* swapmon;
	QWidget* swap_cur;	
	QGroupBox* swapbox;
	QGroupBox* swapbox1;
#endif
} ;

#endif
