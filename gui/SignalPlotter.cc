/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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
#include <math.h>

#include <qpainter.h>
#include <qpixmap.h>

#include <kiconloader.h>
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

	KIconLoader iconLoader;
	errorIcon = iconLoader.loadIcon("connect_creating", KIcon::Desktop,
									KIcon::SizeSmall);
	sensorOk = false;

	vLines = true;
	vColor = green;
	vDistance = 30;

	hLines = true;
	hColor = green;
	hCount = 5;

	labels = true;
	topBar = false;
	fontSize = 12;
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

	update();
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
	uint w = width();
	uint h = height();

	QPixmap pm(w, h);
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
	/* If the range too small we will force it to 1.0 since it looks
	 * a lot nicer. */
	if (range < 0.000001)
		range = 1.0;

	double minVal = minValue;
	if (autoRange)
	{
		if (minValue != 0.0)
		{
			double dim = pow(10, floor(log10(fabs(minValue)))) / 2;
			if (minValue < 0.0)
				minVal = dim * floor(minValue / dim);
			else
				minVal = dim * ceil(minValue / dim);
			range = maxValue - minVal;
			if (range < 0.000001)
				range = 1.0;
		}
		// Massage the range so that the grid shows some nice values.
		double step = range / hCount;
		double dim = pow(10, floor(log10(step))) / 2;
		range = dim * ceil(step / dim) * hCount;
	}
	double maxVal = minVal + range;

	int top = 0;
	if (topBar && h > (fontSize + 2 + hCount * 10))
	{
		/* Draw horizontal bar with current sensor values at top of display. */
		p.setPen(hColor);
		int x0 = w / 2;
		p.setFont(QFont(p.font().family(), fontSize));
		top = p.fontMetrics().height() + 2;
		h -= top;
		int h0 = top - 2;
		p.drawText(0, 0, x0, top - 2, Qt::AlignCenter, title);

		p.drawLine(x0 - 1, 1, x0 - 1, h0);
		p.drawLine(0, top - 1, w - 2, top - 1);

		double bias = -minVal;
		double scaleFac = (w - x0 - 2) / range;
		for (int b = 0; b < beams; b++)
		{
			int start = x0 + (int) (bias * scaleFac);
			int end = x0 + (int) ((bias += beamData[b][w - 3]) * scaleFac);
			/* If the rect is wider than 2 pixels we draw only the last
			 * pixels with the bright color. The rest is painted with
			 * a 50% darker color. */
			if (end - start > 1)
			{
				p.setPen(beamColor[b].dark(150));
				p.setBrush(beamColor[b].dark(150));
				p.drawRect(start, 1, end - start, h0);
				p.setPen(beamColor[b]);
				p.drawLine(end, 1, end, h0);
			}
			else if (start - end > 1)
			{
				p.setPen(beamColor[b].dark(150));
				p.setBrush(beamColor[b].dark(150));
				p.drawRect(end, 1, start - end, h0);
				p.setPen(beamColor[b]);
				p.drawLine(end, 1, end, h0);
			}
			else
			{
				p.setPen(beamColor[b]);
				p.drawLine(start, 1, start, h0);
			}
		}
	}

	/* Draw scope-like grid vertical lines */
	if (vLines && w > 60)
	{
		p.setPen(vColor);
		for (uint x = vDistance; x < (w - 2); x += vDistance)
			p.drawLine(w - x, top, w - x, h + top - 2);
	}

	/* Plot stacked values */
	double scaleFac = (h - 2) / range;
	for (int i = 0; i < samples; i++)
	{
		double bias = -minVal;
		for (int b = 0; b < beams; b++)
		{
			int start = top + h - 2 - (int) (bias * scaleFac);
			int end = top + h - 2 - (int) ((bias + beamData[b][i]) * scaleFac);
			bias += beamData[b][i];
			/* If the line is longer than 2 pixels we draw only the last
			 * 2 pixels with the bright color. The rest is painted with
			 * a 50% darker color. */
			if (end - start > 2)
			{
				p.setPen(beamColor[b].dark(150));
				p.drawLine(i + 1, start, i + 1, end - 1);
				p.setPen(beamColor[b]);
				p.drawLine(i + 1, end - 1, i + 1, end);
			}
			else if (start - end > 2)
			{
				p.setPen(beamColor[b].dark(150));
				p.drawLine(i + 1, start, i + 1, end + 1);
				p.setPen(beamColor[b]);
				p.drawLine(i + 1, end + 1, i + 1, end);
			}
			else
			{
				p.setPen(beamColor[b]);
				p.drawLine(i + 1, start, i + 1, end);
			}
		}
	}
	
	/* Draw horizontal lines and values. Lines are drawn when the
	 * height is greater than 10 times hCount + 1, values are shown
	 * when width is greater than 60 */
	if (hLines && h > (10 * (hCount + 1)))
	{
		p.setPen(hColor);
		p.setFont(QFont(p.font().family(), fontSize));
		QString val;
		for (uint y = 1; y < hCount; y++)
		{
			p.drawLine(0, top + y * (h / hCount), w - 2,
					   top + y * (h / hCount));
			if (labels && h > (fontSize + 1) * (hCount + 1) && w > 60)
			{
				val = QString("%1").arg(maxVal - y * (range / hCount));
				p.drawText(6, top + y * (h / hCount) - 1, val);
			}
		}
		if (labels && h > (fontSize + 1) * (hCount + 1) && w > 60)
		{
			val = QString("%1").arg(minVal);
			p.drawText(6, top + h - 2, val);
		}
	}

	if (!sensorOk)
		p.drawPixmap(2, 2, errorIcon);

	p.end();
	bitBlt(this, 0, 0, &pm);
}
