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

#ifndef _StyleEngine_h_
#define _StyleEngine_h_

#include "qobject.h"
#include "qcolor.h"
#include "qlist.h"

class KConfig;
class QListBoxItem;
class StyleSettings;

class StyleEngine : public QObject
{
	Q_OBJECT

public:
	StyleEngine();
	~StyleEngine();

	void readProperties(KConfig* cfg);
	void saveProperties(KConfig* cfg);

	const QColor& getFgColor1() const
	{
		return fgColor1;
	}
	const QColor& getFgColor2() const
	{
		return fgColor2;
	}
	const QColor& getAlarmColor() const
	{
		return alarmColor;
	}
	const QColor& getBackgroundColor() const
	{
		return backgroundColor;
	}
	uint getFontSize() const
	{
		return fontSize;
	}
	const QColor& getSensorColor(uint i)
	{
		static QColor dummy;
		if (i < sensorColors.count())
			return *(sensorColors.at(i));
		else
			return dummy;
	}
	const uint getSensorColorCount() const
	{
		return sensorColors.count();
	}

public slots:
	void configure();
	void editColor();
	void selectionChanged(QListBoxItem*);
	void applyToWorksheet()
	{
		apply();
		emit applyStyleToWorksheet();
	}

signals:
	void applyStyleToWorksheet();

private:
	void apply();

	StyleSettings* ss;

	QColor fgColor1;
	QColor fgColor2;
	QColor alarmColor;
	QColor backgroundColor;
	uint fontSize;
	QList<QColor> sensorColors;
} ;

extern StyleEngine* Style;

#endif
