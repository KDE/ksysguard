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

#include <kdebug.h>

#include "SignalPlotter.moc"

static inline int
min(int a, int b)
{
	return (a < b ? a : b);
}

SignalPlotter::SignalPlotter(QWidget* parent, const char* name, double min,
							 double max)
	: QWidget(parent, name), minValue(min), maxValue(max)
{
	// paintEvent covers whole widget so we use no background to avoid flicker
	setBackgroundMode(NoBackground);

	beams = samples = 0;
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

	beamData[beams] = new double[samples];
	memset(beamData[beams], 0, sizeof(double) * samples);
	beamColor[beams] = col;
	beams++;

	return (true);
}

void
SignalPlotter::addSample(double s0, double s1, double s2, double s3, double s4)
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
		memmove(beamData[i], beamData[i] + 1, (samples - 1) * sizeof(double));

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
SignalPlotter::changeRange(int beam, double min, double max)
{
	if (beam < 0 || beam >= MAXBEAMS)
	{
		kdDebug() << "SignalPlotter::changeRange: beam index out of range"
				  << endl;
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
	double* tmp[MAXBEAMS];

	// overlap between the old and the new buffers.
	int overlap = min(samples, width() - 2);

	for (int i = 0; i < beams; i++)
	{
		tmp[i] = new double[width() - 2];

		// initialize new part of the new buffer
		if (width() - 2 > overlap)
			memset(tmp[i], 0, sizeof(double) * (width() - 2 - overlap));

		// copy overlap from old buffer to new buffer
		memcpy(tmp[i] + ((width() - 2) - overlap),
			   beamData[i] + (samples - overlap),
			   overlap * sizeof(double));

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
	double range = maxValue - minValue;
	/* We enforce a minimum range of 1.0 to avoid division by 0 errors. */
	if (range < 1.0)
		range = 1.0;

	if (autoRange)
	{
		/* Massage the range so that the grid shows some nice values. The
		 * lowest printed value should only have 2 non-zero digits. */
		double step = range / 5.0;
		if (step >= 100)
		{
			int shift = 0;
			while (step >= 100)
			{
				shift++;
				step /= 10;
			}
			if (((double) ((int) step)) != step)
				step = ((int) step) + 1.0;
			else
				step = (int) step;
			while (--shift >= 0)
				step *= 10;
		}
		else
		{
			int shift = 0;
			while (step < 10)
			{
				shift++;
				step *= 10;
			}
			if (((double) ((int) step)) != step)
				step = ((int) step) + 1.0;
			else
				step = (int) step;
			while (--shift >= 0)
				step /= 10;
		}
		range = 5.0 * step;
	}
	double maxVal = minValue + range;

	/* Draw scope-like grid vertical lines */
	if (w > 60)
	{
		p.setPen(QColor("green"));
		for (int x = 30; x < (w - 2); x += 30)
			p.drawLine(x, 0, x, height() - 2);
	}

	/* Plot stacked values */
	for (int i = 0; i < samples; i++)
	{
		double bias = 0;
		double scaleFac = (h - 2) / range;
		for (int b = 0; b < beams; b++)
		{
			if (beamData[b][i] > 0)
			{
				p.setPen(beamColor[b]);
				p.drawLine(i + 1, h - 2 - (int) (bias * scaleFac),
						   i + 1, h - 2 -
						   (int) ((bias + beamData[b][i]) * scaleFac));
				bias += beamData[b][i];
			}
		}
	}
	
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
