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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!
*/

// $Id$

#ifndef _PerfMonPage_h_
#define _PerfMonPage_h_

#include <qwidget.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include "FancyPlotter.h"
#include "OSStatus.h"

class PerfMonPage : public QWidget
{
	Q_OBJECT

public:
	PerfMonPage(QWidget* parent = 0, const char* name = 0);
	~PerfMonPage()
	{
		killTimer(timerID);
		delete cpuload;
		delete memory;
		delete gm;
	}

	virtual void timerEvent(QTimerEvent*);

private:
	OSStatus stat;
	int timerID;

	int noCpus;

	QGridLayout* gm;
	FancyPlotter* cpuload;
	FancyPlotter* memory;
	QList<FancyPlotter> cpu;
} ;

#endif
