/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    
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
	Please do not commit any changes without consulting me
	first. Thanks!

	$Id$
*/

#include <assert.h>
#include <math.h>
#include <string.h>

#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>

#include <ksgrd/StyleEngine.h>

#include "SignalPlotter.moc"

static inline int
min(int a, int b)
{
	return (a < b ? a : b);
}

SignalPlotter::SignalPlotter(QWidget* parent, const char* name)
	: QWidget(parent, name)
{
	// Auto deletion does not work for pointer to arrays.
	beamData.setAutoDelete(FALSE);

	// paintEvent covers whole widget so we use no background to avoid flicker
	setBackgroundMode(NoBackground);

	samples = 0;
	minValue = maxValue = 0.0;
	autoRange = false;

	graphStyle = GRAPH_ORIGINAL;

	// Anything smaller than this does not make sense.
	setMinimumSize(16, 16);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
							  QSizePolicy::Expanding, false));

	vLines = true;
	vColor = KSGRD::Style->firstForegroundColor();
	vDistance = 30;
	vScroll = true;
	vOffset = 0;
	hScale = 1;

	hLines = true;
	hColor = KSGRD::Style->secondForegroundColor();
	hCount = 5;

	labels = true;
	topBar = false;
	fontSize = KSGRD::Style->fontSize();

	bColor = KSGRD::Style->backgroundColor();
}

SignalPlotter::~SignalPlotter()
{
	for (double* p = beamData.first(); p; p = beamData.next())
		delete [] p;
}

bool
SignalPlotter::addBeam(QColor col)
{
	double* d = new double[samples];
	memset(d, 0, sizeof(double) * samples);
	beamData.append(d);
	beamColor.append(col);

	return (true);
}

void
SignalPlotter::addSample(const QValueList<double>& sampleBuf)
{
	if (beamData.count() != sampleBuf.count())
		return;

	double* d;
	if (autoRange)
	{
		double sum = 0;
		for (d = beamData.first(); d; d = beamData.next())
		{
			sum += d[0];
			if (sum < minValue)
				minValue = sum;
			if (sum > maxValue)
				maxValue = sum;
		}
	}

	/* If the vertical lines are scrolling, increment the offset
	 * so they move with the data. The vOffset / hScale confusion
	 * is because v refers to Vertical Lines, and h to the horizontal
	 * distance between the vertical lines. */
	if (vScroll)
	{
		vOffset = (vOffset + hScale) % vDistance;
	}

	// Shift data buffers one sample down and insert new samples.
	QValueList<double>::ConstIterator s;
	for (d = beamData.first(), s = sampleBuf.begin(); d;
		 d = beamData.next(), ++s)
	{
		memmove(d, d + 1, (samples - 1) * sizeof(double));
		d[samples - 1] = *s;
	}

	update();
}

void
SignalPlotter::changeRange(int beam, double min, double max)
{
	// Only the first beam affects range calculation.
	if (beam > 1)
		return;

	minValue = min;
	maxValue = max;
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

	/* Determine new number of samples first.
	 *  +0.5 to ensure rounding up
	 *  +2 for extra data points so there is
	 *     1) no wasted space and
	 *     2) no loss of precision when drawing the first data point. */
	uint newSampleNum = static_cast<uint>(((width() - 2) / hScale) + 2.5);

	// overlap between the old and the new buffers.
	int overlap = min(samples, newSampleNum);

	for (uint i = 0; i < beamData.count(); ++i)
	{
		double* nd = new double[newSampleNum];

		// initialize new part of the new buffer
		if (newSampleNum > overlap)
			memset(nd, 0, sizeof(double) * (newSampleNum - overlap));

		// copy overlap from old buffer to new buffer
		memcpy(nd + (newSampleNum - overlap),
			   beamData.at(i) + (samples - overlap),
			   overlap * sizeof(double));

		beamData.remove(i);
		beamData.insert(i, nd);
	}
	samples = newSampleNum;
}

void
SignalPlotter::paintEvent(QPaintEvent*)
{
	uint w = width();
	uint h = height();

	/* Do not do repaints when the widget is not yet setup properly. */
	if (w <= 2)
		return;
	
	QPixmap pm(w, h);
	QPainter p;
	p.begin(&pm, this);

	pm.fill(bColor);
	/* Draw white line along the bottom and the right side of the
	 * widget to create a 3D like look. */
	p.setPen(QColor(colorGroup().light()));
	p.drawLine(0, h - 1, w - 1, h - 1);
	p.drawLine(w - 1, 0, w - 1, h - 1);

	p.setClipRect(1, 1, w - 2, h - 2);
	double range = maxValue - minValue;
	/* If the range is too small we will force it to 1.0 since it
	 * looks a lot nicer. */
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
		top = p.fontMetrics().height();
		h -= top;
		int h0 = top - 2;
		p.drawText(0, 0, x0, top - 2, Qt::AlignCenter, title);

		p.drawLine(x0 - 1, 1, x0 - 1, h0);
		p.drawLine(0, top - 1, w - 2, top - 1);

		double bias = -minVal;
		double scaleFac = (w - x0 - 2) / range;
		QValueList<QColor>::Iterator col;
		col = beamColor.begin();
		for (double* d = beamData.first(); d; d = beamData.next(), ++col)
		{
			int start = x0 + (int) (bias * scaleFac);
			int end = x0 + (int) ((bias += d[w - 3]) * scaleFac);
			/* If the rect is wider than 2 pixels we draw only the last
			 * pixels with the bright color. The rest is painted with
			 * a 50% darker color. */
			if (end - start > 1)
			{
				p.setPen((*col).dark(150));
				p.setBrush((*col).dark(150));
				p.drawRect(start, 1, end - start, h0);
				p.setPen(*col);
				p.drawLine(end, 1, end, h0);
			}
			else if (start - end > 1)
			{
				p.setPen((*col).dark(150));
				p.setBrush((*col).dark(150));
				p.drawRect(end, 1, start - end, h0);
				p.setPen(*col);
				p.drawLine(end, 1, end, h0);
			}
			else
			{
				p.setPen(*col);
				p.drawLine(start, 1, start, h0);
			}
		}
	}

	/* Draw scope-like grid vertical lines */
	if (vLines && w > 60)
	{
		p.setPen(vColor);
		for (uint x = vOffset; x < (w - 2); x += vDistance)
			p.drawLine(w - x, top, w - x, h + top - 2);
	}

	/* In autoRange mode we determine the range and plot the values in
	 * one go. This is more efficently than running through the
	 * buffers twice but we do react on recently discarded samples as
	 * well as new samples one plot too late. So the range is not
	 * correct if the recently discarded samples are larger or smaller
	 * than the current extreme values. But we can probably live with
	 * this. */
	if (autoRange)
		minValue = maxValue = 0.0;
	/* Plot stacked values */
	double scaleFac = (h - 2) / range;
	if (graphStyle == GRAPH_ORIGINAL)
	{
		int xPos = 0;
		for (int i = 0; i < samples; i++, xPos += hScale)
		{
			double bias = -minVal;
			QValueList<QColor>::Iterator col;
			col = beamColor.begin();
			double sum = 0.0;
			for (double* d = beamData.first(); d; d = beamData.next(), ++col)
			{
				if (autoRange)
				{
					sum += d[i];
					if (sum < minValue)
						minValue = sum;
					if (sum > maxValue)
						maxValue = sum;
				}
				int start = top + h - 2 - (int) (bias * scaleFac);
				int end = top + h - 2 - (int) ((bias + d[i]) * scaleFac);
				bias += d[i];
				/* If the line is longer than 2 pixels we draw only the last
				 * 2 pixels with the bright color. The rest is painted with
				 * a 50% darker color. */
				if (end - start > 2)
				{
					p.fillRect(xPos, start, hScale, end - start - 1,
							   (*col).dark(150));
					p.fillRect(xPos, end - 1, hScale, 2, *col);
				}
				else if (start - end > 2)
				{
					p.fillRect(xPos, start, hScale, end - start + 1,
							   (*col).dark(150));
					p.fillRect(xPos, end + 1, hScale, 2, *col);
				}
				else
				{
					p.fillRect(xPos, start, hScale, end - start, *col);
				}
			}
		}
	}
	else
	{
		int *prevVals = new int[beamData.count()];
		int hack[4];
		int x1 = w - ((samples + 1) * hScale);

		for (int i = 0; i < samples; i++)
		{
			QValueList<QColor>::Iterator col;
			col = beamColor.begin();
			double sum = 0.0;
			int y = top + h - 2;
			int oldY = top + h;
			int oldPrevY = oldY;
			int height = 0;
			int j = 0;
			int jMax = beamData.count() - 1;
			x1 += hScale;
			int x2 = x1 + hScale;

			for (double* d = beamData.first(); d; d = beamData.next(), ++col,
				 j++)
			{
				if (autoRange)
				{
					sum += d[i];
					if (sum < minValue)
						minValue = sum;
					if (sum > maxValue)
						maxValue = sum;
				}
				height = (int) ((d[i] - minVal) * scaleFac);
				y -= height;
				/* If the line is longer than 2 pixels we draw only the last
				 * 2 pixels with the bright color. The rest is painted with
				 * a 50% darker color. */

				QPen lastPen = QPen(p.pen());
				p.setPen((*col).dark(150));
				p.setBrush((*col).dark(150));
				QPointArray pa(4);
				int prevY = (i == 0) ? y : prevVals[j];
				pa.putPoints(0, 1, x1, prevY);
				pa.putPoints(1, 1, x2, y);
				pa.putPoints(2, 1, x2, oldY);
				pa.putPoints(3, 1, x1, oldPrevY);
				p.drawPolygon(pa);
				p.setPen(lastPen);
				if (jMax == 0)
				{
					// draw as normal, no deferred drawing req'd.
					p.setPen(*col);
					p.drawLine(x1, prevY, x2, y);
				}
				else if (j == jMax)
				{
					// draw previous values and current values
					p.drawLine(hack[0], hack[1], hack[2], hack[3]);
					p.setPen(*col);
					p.drawLine(x1, prevY, x2, y);
				}
				else if (j == 0)
				{
					// save values only
					hack[0] = x1;
					hack[1] = prevY;
					hack[2] = x2;
					hack[3] = y;
					p.setPen(*col);
				}
				else
				{
					p.drawLine(hack[0], hack[1], hack[2], hack[3]);
					hack[0] = x1;
					hack[1] = prevY;
					hack[2] = x2;
					hack[3] = y;
					p.setPen(*col);
				}
				prevVals[j] = y;
				oldY = y;
				oldPrevY = prevY;
			}
		}

		delete[] prevVals;
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

	p.end();
	bitBlt(this, 0, 0, &pm);
}
