/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _BarGraph_h_
#define _BarGraph_h_

#include <qwidget.h>
#include <qvector.h>
#include <qarray.h>

class BarGraph : public QWidget
{
	Q_OBJECT

	friend class DancingBars;

public:
	BarGraph(QWidget* parent = 0, const char* name = 0);
	~BarGraph();

	bool addBar(const QString& footer);
	bool removeBar(uint idx);

	void updateSamples(const QArray<double>& newSamples);

	double getMin() const
	{
		return minValue;
	}
	double getMax() const
	{
		return maxValue;
	}
	void getLimits(double& l, bool& la, double& u, bool& ua) const
	{
		l = lowerLimit;
		la = lowerLimitActive;
		u = upperLimit;
		ua = upperLimitActive;
	}
	void setLimits(double l, bool la, double u, bool ua)
	{
		lowerLimit = l;
		lowerLimitActive = la;
		upperLimit = u;
		upperLimitActive = ua;
	}

	void changeRange(double min, double max);

	void setSensorOk(bool ok)
	{
		if (ok != sensorOk)
		{
			sensorOk = ok;
			update();
		}
	}

protected:
	virtual void paintEvent(QPaintEvent*);

private:
	double minValue;
	double maxValue;
	double lowerLimit;
	double lowerLimitActive;
	double upperLimit;
	bool upperLimitActive;
	bool autoRange;
	QArray<double> samples;
	QStringList footers;
	int bars;
	QColor normalColor;
	QColor alarmColor;
	QColor backgroundColor;
	int fontSize;

	QPixmap errorIcon;
	bool sensorOk;
} ;

#endif
