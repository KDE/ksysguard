/*
    KTop, the KDE Task Manager and System Monitor
   
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <string.h>
#include <assert.h>
#include <kdebug.h>

#include <qpainter.h>
#include <qpixmap.h>

#include "BarGraph.moc"

BarGraph::BarGraph(QWidget* parent, const char* name, int min, int max)
	: QWidget(parent, name), minValue(min), maxValue(max)
{
	// paintEvent covers whole widget so we use no background to avoid flicker
	setBackgroundMode(NoBackground);

	bars = 0;
	autoRange = (min == max);

	// Anything smaller than this does not make sense.
	setMinimumSize(16, 16);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
							  QSizePolicy::Expanding, FALSE));
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
BarGraph::updateSamples(const QArray<long>& newSamples)
{
	samples = newSamples;
	repaint();
}

void
BarGraph::changeRange(long min, long max)
{
	if (min == max)
	{
		autoRange = TRUE;
	}
	else
	{
		minValue = min;
		maxValue = max;
		autoRange = FALSE;
	}
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

	pm.fill(black);
	/* Draw white line along the bottom and the right side of the
	 * widget to create a 3D like look. */
	p.setPen(QColor(colorGroup().light()));
	p.drawLine(0, h - 1, w - 1, h - 1);
	p.drawLine(w - 1, 0, w - 1, h - 1);

	p.setClipRect(1, 1, w - 2, h - 2);

	int barWidth = (w - 2) / bars;
	int barHeight;
	barHeight = h - 2 - fm.lineSpacing() - 2;
	for (int b = 0; b < bars; b++)
	{
		int topVal = (int) ((float) barHeight / maxValue * samples[b]);
		int limitVal = (int) ((float) barHeight / maxValue * limit);

		for (int i = 0; i < barHeight && i < topVal; i += 2)
		{
			if (limit && i > limitVal && samples[b] > limit)
				p.setPen(QColor(128 + (int) ((float) 128 / barHeight * i), 0,
								0));
			else
				p.setPen(QColor(0, 128 + (int) ((float) 128 / barHeight * i),
								0));
			p.drawLine(b * barWidth + 3, barHeight - i, (b + 1) * barWidth - 3,
					   barHeight - i);
		}
		if (limit && samples[b] > limit)
			p.setPen(QColor("red"));
		else
			p.setPen(QColor("green"));
		p.drawText(b * barWidth + 3, h - fm.lineSpacing() - 2,
				   barWidth - 2 * 3, fm.lineSpacing(), Qt::AlignCenter,
				   footers[b]);
	}
	
	p.end();
	bitBlt(this, 0, 0, &pm);
}
