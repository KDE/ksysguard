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

#include <qpainter.h>
#include <qpixmap.h>

#include "SignalPlotter.moc"

static inline int
min(int a, int b)
{
	return (a < b ? a : b);
}

SignalPlotter::SignalPlotter(QWidget* parent, const char* name, int min,
							 int max)
	: QWidget(parent, name), minValue(min), maxValue(max)
{
	// paintEvent covers whole widget so we use no background to avoid flicker
	setBackgroundMode(NoBackground);

	beams = samples = 0;
	lowPass = FALSE;
	autoRange = (min == max);

	// Anything smaller than this does not make sense.
	setMinimumSize(16, 16);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
							  QSizePolicy::Expanding, FALSE));
}

SignalPlotter::~SignalPlotter()
{
	for (int i = 0; i < beams; i++)
		delete [] beamData[i];
}

bool
SignalPlotter::addBeam(QColor col)
{
	if (beams == MAXBEAMS)
		return (false);

	beamData[beams] = new long[samples];
	memset(beamData[beams], 0, sizeof(long) * samples);
	beamColor[beams] = col;
	beams++;

	return (true);
}

void
SignalPlotter::addSample(int s0, int s1, int s2, int s3, int s4)
{
	/* To avoid unecessary calls to the fairly expensive calcRange
	 * function we only do a recalc if the value to be dropped is
	 * an extreme value. */
	bool recalc = false;
	for (int i = 0; i < beams; i++)
		if (beamData[i][0] <= minValue ||
			beamData[i][0] >= maxValue)
		{
			recalc = true;
		}

	// Shift data buffers one sample down.
	for (int i = 0; i < beams; i++)
		memmove(beamData[i], beamData[i] + 1, (samples - 1) * sizeof(int));

	if (lowPass)
	{
		/* We use an FIR type low-pass filter to make the display look
		 * a little nicer without becoming too inaccurate. */
		if (beams > 0)
			beamData[0][samples - 1] = (beamData[0][samples - 2] + s0) / 2;
		if (beams > 1)
			beamData[1][samples - 1] = (beamData[1][samples - 2] + s1) / 2;
		if (beams > 2)
			beamData[2][samples - 1] = (beamData[2][samples - 2] + s2) / 2;
		if (beams > 3)
			beamData[3][samples - 1] = (beamData[3][samples - 2] + s3) / 2;
		if (beams > 4)
			beamData[4][samples - 1] = (beamData[4][samples - 2] + s4) / 2;
	}
	else
	{
		if (beams > 0)
			beamData[0][samples - 1] = s0;
		if (beams > 1)
			beamData[1][samples - 1] = s1;
		if (beams > 2)
			beamData[2][samples - 1] = s2;
		if (beams > 3)
			beamData[3][samples - 1] = s3;
		if (beams > 4)
			beamData[4][samples - 1] = s4;
	}
	for (int i = 0; i < beams; i++)
		if (beamData[i][samples - 1] <= minValue ||
			beamData[i][samples - 1] >= maxValue)
		{
			recalc = true;
		}

	if (autoRange && recalc)
		calcRange();

	repaint();
}

void
SignalPlotter::changeRange(int beam, long min, long max)
{
	if (beam < 0 || beam >= MAXBEAMS)
	{
		qDebug("SignalPlotter::changeRange: beam index out of range");
		return;
	}

	// Only the first beam affects range calculation.
	if (beam > 1)
		return;

	if (min == max)
	{
		autoRange = TRUE;
		calcRange();
	}
	else
	{
		minValue = min;
		maxValue = max;
		autoRange = FALSE;
	}
}

void
SignalPlotter::calcRange()
{
	minValue = 0;
	maxValue = 0;

	for (int i = 0; i < samples; i++)
	{
		for (int b = 0; b < beams; b++)
		{
			if (beamData[b][i] < minValue)
				minValue = beamData[b][i];
			if (beamData[b][i] > maxValue)
				maxValue = beamData[b][i];
		}
	}
}

void
SignalPlotter::resizeEvent(QResizeEvent*)
{
	assert(width() > 2);

	/* Since the data buffers for the beams are equal in size to the
	 * width of the widget minus 2 we have to enlarge or shrink the
	 * buffers accordingly when a resize occures. To have a nicer
	 * display we try to keep as much data as possible. Data that is
	 * lost due to shrinking the buffers cannot be recovered on
	 * enlarging though. */
	long* tmp[MAXBEAMS];

	// overlap between the old and the new buffers.
	int overlap = min(samples, width() - 2);

	for (int i = 0; i < beams; i++)
	{
		tmp[i] = new long[width() - 2];

		// initialize new part of the new buffer
		if (width() - 2 > overlap)
			memset(tmp[i], 0, sizeof(int) * (width() - 2 - overlap));

		// copy overlap from old buffer to new buffer
		memcpy(tmp[i] + ((width() - 2) - overlap),
			   beamData[i] + (samples - overlap),
			   overlap * sizeof(int));

		// discard old buffer
		delete [] beamData[i];

		beamData[i] = tmp[i];
	}
	samples = width() - 2;
}

void 
SignalPlotter::paintEvent(QPaintEvent*)
{
	int w = width();
	int h = height();

	QPixmap pm(width(), height());
	QPainter p;
	p.begin(&pm, this);

	pm.fill(black);
	/* Draw white line along the bottom and the right side of the
	 * widget to create a 3D like look. */
	p.setPen(QColor(colorGroup().light()));
	p.drawLine(0, h - 1, w - 1, h - 1);
	p.drawLine(w - 1, 0, w - 1, h - 1);

	p.setClipRect(1, 1, w - 2, h - 2);
	int range = maxValue - minValue;
	/* It makes no sense to have a range that is smaller than the 20
	 * in pixels. So we force the range to at least be 20. */
	if (range < 20)
		range = 20;
	int maxVal = minValue + range;

	/* Draw scope-like grid */
	if (w > 60)
	{
		p.setPen(QColor("green"));
		for (int x = 30; x < (w - 2); x += 30)
			p.drawLine(x, 0, x, height() - 1);
	}

	// Scale painter to value coordinates
	p.scale(1.0f, (float) h / range);
	for (int i = 0; i < samples; i++)
	{
		int bias = 0;
		for (int b = 0; b < beams; b++)
		{
			if (beamData[b][i] > 0)
			{
				p.setPen(beamColor[b]);
				p.drawLine(i + 1, range - bias - 1,
						   i + 1, range - (bias + beamData[b][i]) - 1);
				bias += beamData[b][i];
			}
		}
	}
	
	// Scale back to pixel coordinates
	p.scale(1.0f, (float) range / h);
	// draw horizontal lines and values
	if (h > 60)
	{
		p.setPen(QColor("green"));
		p.setFont(QFont("lucidatypewriter", 12));
		QString val;
		for (int y = 1; y < 5; y++)
		{
			p.drawLine(0, y * (height() / 5), w - 2, y * (height() / 5));
			val = QString("%1").arg(maxVal - y * (range / 5));
			p.drawText(6, y * (height() / 5) - 1, val);
		}
		val = QString("%1").arg(minValue);
		p.drawText(6, height() - 2, val);
	}

	p.end();
	bitBlt(this, 0, 0, &pm);
}
