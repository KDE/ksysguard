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
*/

// $Id$

#ifndef _FancyPlotter_h_
#define _FancyPlotter_h_

#include <qwidget.h>
#include <qlabel.h>

#include "SignalPlotter.h"
#include "MultiMeter.h"

class QGroupBox;

class FancyPlotter : public QWidget
{
	Q_OBJECT

public:
	FancyPlotter(QWidget* parent = 0, const char* name = 0,
				 const char* title = 0, int min = 0,
				 int max = 100);
	~FancyPlotter();

	bool addBeam(const char* name, QColor col)
	{
		return (plotter->addBeam(col) && multiMeter->addMeter(name, col));
	}

	void addSample(int s0, int s1 = 0, int s2 = 0, int s3 = 0, int s4 = 0)
	{
		multiMeter->updateValues(s0, s1, s2, s3, s4);
		plotter->addSample(s0, s1, s2, s3, s4);
	}

	void setLowPass(bool lp)
	{
		plotter->setLowPass(lp);
	}

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	QGroupBox* meterFrame;

	MultiMeter* multiMeter;

	SignalPlotter* plotter;
} ;

#endif
