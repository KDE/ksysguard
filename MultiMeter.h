/*
    KTop, the KDE Task Manager
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
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

#ifndef _MultiMeter_h_
#define _MultiMeter_h_

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlcdnumber.h>

class QColor;

class MultiMeter : public QWidget
{
	Q_OBJECT

public:
	MultiMeter(QWidget* parent = 0, const char* name = 0, int min = 0,
			   int max = 100);
	~MultiMeter()
	{
		delete gm;
	}

	bool addMeter(const char* name, QColor col);

	void updateValues(int v0 = 0, int v1 = 0, int v2 = 0, int v3 = 0,
					  int v4 = 0);

	virtual QSize sizeHint(void);

private:
	// the number of meters displayed in this multi meter
	int meters;
	int digits;

	QGridLayout* gm;
	QList<QLabel> meterLabel;
	QList<QLCDNumber> meterLcd;
} ;

#endif
