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

#ifndef _SignalPlotter_h_
#define _SignalPlotter_h_

#include <qwidget.h>
#include <qarray.h>
#include <qbrush.h>

class QColor;

#define MAXBEAMS 5

class SignalPlotter : public QWidget
{
	Q_OBJECT

public:
	SignalPlotter(QWidget* parent = 0, const char* name = 0, double min = 0,
				  double max = 100);
	~SignalPlotter();

	bool addBeam(QColor col);
	void addSample(double s0, double s1 = 0, double s2 = 0, double s3 = 0,
				   double s4 = 0);

	double getMin() const
	{
		return (autoRange ? 0 : minValue);
	}
	double getMax() const
	{
		return (autoRange ? 0 : maxValue);
	}

	void changeRange(int beam, double min, double max);

	void setLowPass(bool lp)
	{
		lowPass = lp;
	}

protected:
	virtual void resizeEvent(QResizeEvent*);
	virtual void paintEvent(QPaintEvent*);

private:
	void calcRange();

	double minValue;
	double maxValue;
	bool autoRange;
	bool lowPass;
	double* beamData[MAXBEAMS];
	QColor beamColor[MAXBEAMS];
	int beams;
	int samples;
} ;

#endif
