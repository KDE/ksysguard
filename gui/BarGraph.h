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

#ifndef _BarGraph_h_
#define _BarGraph_h_

#include <qwidget.h>
#include <qvector.h>
#include <qarray.h>

class BarGraph : public QWidget
{
	Q_OBJECT

public:
	BarGraph(QWidget* parent = 0, const char* name = 0, int min = 0,
				  int max = 100);
	~BarGraph();

	bool addBar(const QString& footer);
	void updateSamples(const QArray<long>& newSamples);

	long getMin() const
	{
		return (autoRange ? 0 : minValue);
	}
	long getMax() const
	{
		return (autoRange ? 0 : maxValue);
	}
	void getLimits(long& l, bool& la, long& u, bool& ua) const
	{
		l = lowerLimit;
		la = lowerLimitActive;
		u = upperLimit;
		ua = upperLimitActive;
	}
	void setLimits(long l, bool la, long u, bool ua)
	{
		lowerLimit = l;
		lowerLimitActive = la;
		upperLimit = u;
		upperLimitActive = ua;
	}

	void changeRange(long min, long max);

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
	long minValue;
	long maxValue;
	long lowerLimit;
	bool lowerLimitActive;
	long upperLimit;
	bool upperLimitActive;
	bool autoRange;
	QArray<long> samples;
	QStringList footers;
	int bars;

	QPixmap errorIcon;
	bool sensorOk;
} ;

#endif
