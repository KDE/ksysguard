/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <string.h>
#include <assert.h>

#include <qpainter.h>

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
    setBackgroundColor(black);

	beams = 0;
	lowPass = FALSE;
	for (int i = 0; i < MAXBEAMS; i++)
		beamData[i] = 0;
}

SignalPlotter::~SignalPlotter()
{
	for (int i = 0; i < MAXBEAMS; i++)
		delete [] beamData[i];
}

bool
SignalPlotter::addBeam(QColor col)
{
	assert(width() > 2);

	if (beams == MAXBEAMS)
		return (false);

	beamData[beams] = new int[width() - 2];
	memset(beamData[beams], 0, sizeof(int) * (width() - 2));
	beamColor[beams] = col;
	beams++;

	return (true);
}

void
SignalPlotter::addSample(int s0, int s1, int s2, int s3, int s4)
{
	assert(width() > 3);

	/*
	 * Shift data buffers one sample down.
	 */
	for (int i = 0; i < beams; i++)
		memmove(beamData[i], beamData[i] + 1, (width() - 3) * sizeof(int));

	if (lowPass)
	{
		/*
		 * We use an FIR type low-pass filter to make the display look a little
		 * nicer without becoming too inaccurate.
		 */
		if (beams > 0)
			beamData[0][width() - 3] = (beamData[0][width() - 4] + s0) / 2;
		if (beams > 1)
			beamData[1][width() - 3] = (beamData[1][width() - 4] + s1) / 2;
		if (beams > 2)
			beamData[2][width() - 3] = (beamData[2][width() - 4] + s2) / 2;
		if (beams > 3)
			beamData[3][width() - 3] = (beamData[3][width() - 4] + s3) / 2;
		if (beams > 4)
			beamData[4][width() - 3] = (beamData[4][width() - 4] + s4) / 2;
	}
	else
	{
		if (beams > 0)
			beamData[0][width() - 3] = s0;
		if (beams > 1)
			beamData[1][width() - 3] = s1;
		if (beams > 2)
			beamData[2][width() - 3] = s2;
		if (beams > 3)
			beamData[3][width() - 3] = s3;
		if (beams > 4)
			beamData[4][width() - 3] = s4;
	}

	repaint();
}

void
SignalPlotter::resizeEvent(QResizeEvent* ev)
{
	assert(width() > 2);

	/*
	 * Since the data buffers for the beams are equal in size to the width
	 * of the widget minus 2 we have to enlarge or shrink the buffers
	 * accordingly when a resize occures. To have a nices display we try to
	 * keep as much data as possible. Data that is lost due to shrinking the
	 * buffers cannot be recovered on enlarging though.
	 */
	int* tmp[MAXBEAMS];

	// overlap between the old and the new buffers.
	int overlap = min(ev->oldSize().width() - 2, width() - 2);

	for (int i = 0; i < beams; i++)
	{
		tmp[i] = new int[width() - 2];

		// initialize new part of the new buffer
		if (width() - 2 > overlap)
			memset(tmp[i], 0, sizeof(int) * (width() - 2 - overlap));

		// copy overlap from old buffer to new buffer
		memcpy(tmp[i] + ((width() - 2) - overlap),
			   beamData[i] + ((ev->oldSize().width() - 2) - overlap),
			   overlap * sizeof(int));

		// discard old buffer
		delete [] beamData[i];

		beamData[i] = tmp[i];
	}
}

void 
SignalPlotter::paintEvent(QPaintEvent*)
{
	assert(width() > 2);

	int w = width();
	int h = height();

	QPainter p;
	p.begin(this);

	/*
	 * Draw white line along the bottom and the right side of the widget to
	 * create a 3D like look.
	 */
	p.setPen(QColor("white"));
	p.drawLine(0, h - 1, w - 1, h - 1);
	p.drawLine(w - 1, 0, w - 1, h - 1);

	p.setViewport(1, 1, w - 2, h - 2);
	int range = maxValue - minValue;
	p.scale(1.0f, (float) (h - 2) / range);

	p.setPen(QColor("darkgreen"));
	if (w > 60)
		for (int x = 30; x < (w - 3); x += 30)
			p.drawLine(x, 0, x, range - 1);
	if (h > 60)
		for (int y = 1; y < 5; y++)
			p.drawLine(0, y * (range / 5), w - 2, y * (range / 5));

	for (int i = 0; i < (w - 2); i++)
	{
		int bias = 0;
		for (int b = 0; b < beams; b++)
		{
			if (beamData[b][i] > 0)
			{
				p.setPen(beamColor[b]);
				p.drawLine(i, range - bias - 1,
						   i, range - (bias + beamData[b][i]) - 1);
				bias += beamData[b][i];
			}
		}
	}
	p.end();
}
