/*
    KSysGuard, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

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

#include <string.h>
#include <assert.h>
#include <kdebug.h>

#include <qpainter.h>
#include <qpixmap.h>

#include <kiconloader.h>

#include "BarGraph.moc"

BarGraph::BarGraph(QWidget* parent, const char* name)
	: QWidget(parent, name)
{
	// paintEvent covers whole widget so we use no background to avoid flicker
	setBackgroundMode(NoBackground);

	bars = 0;
	minValue = 0.0;
	maxValue = 100.0;
	lowerLimit = upperLimit = 0.0;
	lowerLimitActive = upperLimitActive = false;

	normalColor = QColor(20, 255, 20);
	alarmColor = QColor(255, 20, 20);
	backgroundColor = QColor(0, 0, 0);

	// Anything smaller than this does not make sense.
	setMinimumSize(16, 16);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
							  QSizePolicy::Expanding, FALSE));

	KIconLoader iconLoader;
	errorIcon = iconLoader.loadIcon("connect_creating", KIcon::Desktop,
									KIcon::SizeSmall);
	sensorOk = false;
}

BarGraph::~BarGraph()
{
}

bool
BarGraph::addBar(const QString& footer)
{
	samples.resize(++bars);
	footers.append(footer);

	return (true);
}

void
BarGraph::updateSamples(const QArray<double>& newSamples)
{
	samples = newSamples;
	update();
}

void
BarGraph::changeRange(double min, double max)
{
	minValue = min;
	maxValue = max;
}

void 
BarGraph::paintEvent(QPaintEvent*)
{
	int w = width();
	int h = height();

	QPixmap pm(w, h);
	QPainter p;
	p.begin(&pm, this);
	p.setFont(QFont("lucidatypewriter", 12));
	QFontMetrics fm(QFont("lucidatypewriter", 12));

	pm.fill(backgroundColor);
	/* Draw white line along the bottom and the right side of the
	 * widget to create a 3D like look. */
	p.setPen(QColor(colorGroup().light()));
	p.drawLine(0, h - 1, w - 1, h - 1);
	p.drawLine(w - 1, 0, w - 1, h - 1);

	p.setClipRect(1, 1, w - 2, h - 2);

	int barWidth = (w - 2) / bars;
	int b;
	/* Labels are only printed underneath the bars if the labels for all
	 * bars are smaller than the bar width. If a single label does not fit
	 * no label is shown. */
	bool showLabels = true;
	for (b = 0; b < bars; b++)
		if (fm.width(footers[b]) > barWidth)
			showLabels = false;

	int barHeight;
	if (showLabels)
		barHeight = h - 2 - fm.lineSpacing() - 2;
	else
		barHeight = h - 2;
	for (int b = 0; b < bars; b++)
	{
		int topVal = (int) ((float) barHeight / maxValue
							* (samples[b] - minValue));
		/* TODO: This widget does not handle negative values properly. */
		if (topVal < 0)
			topVal = 0;

		for (int i = 0; i < barHeight && i < topVal; i += 2)
		{
			if ((upperLimitActive && samples[b] > upperLimit) ||
				(lowerLimitActive && samples[b] < lowerLimit))
				p.setPen(alarmColor.light((int) 30 +
										  (70.0 / (barHeight + 1) * i)));
			else
				p.setPen(normalColor.light((int) 30 +
										   (70.0 / (barHeight + 1) * i)));
			p.drawLine(b * barWidth + 3, barHeight - i, (b + 1) * barWidth - 3,
					   barHeight - i);
		}
		if ((upperLimitActive && samples[b] > upperLimit) ||
			(lowerLimitActive && samples[b] < lowerLimit))
			p.setPen(alarmColor);
		else
			p.setPen(normalColor);
		if (showLabels)
			p.drawText(b * barWidth + 3, h - fm.lineSpacing() - 2,
					   barWidth - 2 * 3, fm.lineSpacing(), Qt::AlignCenter,
					   footers[b]);
	}
	
	if (!sensorOk)
		p.drawPixmap(2, 2, errorIcon);

	p.end();
	bitBlt(this, 0, 0, &pm);
}
