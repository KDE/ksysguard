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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _SignalPlotter_h_
#define _SignalPlotter_h_

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qwidget.h>
#include <qstring.h>

#define	GRAPH_ORIGINAL		0
#define GRAPH_BASIC_POLY	1

class QColor;
class FancyPlotter;

class SignalPlotter : public QWidget
{
	Q_OBJECT

	friend class FancyPlotter;

public:
	SignalPlotter(QWidget* parent = 0, const char* name = 0);
	~SignalPlotter();

	bool addBeam(QColor col);
	void addSample(const QValueList<double>& samples);

	void removeBeam(uint idx)
	{
		beamColor.remove(beamColor.at(idx));
		beamData.remove(idx);
	}

	double getMin() const
	{
		return (autoRange ? 0 : minValue);
	}
	double getMax() const
	{
		return (autoRange ? 0 : maxValue);
	}

	void changeRange(int beam, double min, double max);

	void setTitle(const QString& t)
	{
		title = t;
	}

	void setSensorOk(bool ok)
	{
		if (ok != sensorOk)
		{
			sensorOk = ok;
			update();
		}
	}

protected:
	virtual void resizeEvent(QResizeEvent*);
	virtual void paintEvent(QPaintEvent*);

private:
	double minValue;
	double maxValue;
	bool autoRange;

	uint graphStyle;

	bool vLines;
	QColor vColor;
	uint vDistance;
  bool vScroll;
  uint vOffset;
	uint hScale;

	bool hLines;
	QColor hColor;
	uint hCount;

	bool labels;
	bool topBar;
	uint fontSize;

	QColor bColor;

	QPtrList<double> beamData;
	QValueList<QColor> beamColor;
	int samples;

	QPixmap errorIcon;
	bool sensorOk;
	QString title;
} ;

#endif
