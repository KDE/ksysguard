/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _SensorClient_h_
#define _SensorClient_h_

#include <qstring.h>

/**
 * Every object that should act as a client to a sensor must inherit from
 * this class. A pointer to the client object is passed as SensorClient*
 * to the SensorAgent. When the requested information is available or a
 * problem occured one of the member functions is called.
 */
class SensorClient
{
public:
	SensorClient() { }
	virtual ~SensorClient() { }

	/**
	 * This function is called whenever the information form the sensor has
	 * been received by the sensor agent. This function must be reimplemented
	 * by the sensor client to receive and process this information.
	 */
	virtual void answerReceived(int id, const QString& s) { }

	/**
	 * In case of an unexpected fatal problem with the sensor the sensor
	 * agent will call this function to notify the client about it.
	 */
	virtual void sensorLost() { }
} ;

/**
 * The following classes are utility classes that provide a
 * convenient way to retrieve pieces of information from the sensor
 * answers. For each type of answer there is a separate class.
 */

/**
 * A monitor info contains 4 fields seperated by TABS, a description
 * (name), the minimum and the maximum values and the unit.
 * e.g. Swap Memory	0	133885952	KB
 */
class SensorMonitorInfo
{
public:
	SensorMonitorInfo(const QString& info)
	{
		QString s = info;
		name = s.left(s.find('\t'));
		// skip name
		s = s.remove(0, s.find('\t') + 1);
		min = s.left(s.find('\t')).toLong();
		// skip minimal value
		s = s.remove(0, s.find('\t') + 1);
		max = s.left(s.find('\t')).toLong();
		// skip maximum value
		s = s.remove(0, s.find('\t') + 1);
		unit = s;
	}
	~SensorMonitorInfo() { }

	const QString& getName()
	{
		return (name);
	}
	long getMin()
	{
		return (min);
	}
	long getMax()
	{
		return (max);
	}
	const QString& getUnit()
	{
		return (unit);
	}

private:
	QString name;
	long min;
	long max;
	QString unit;
} ;

class SensorLinesTokenizer
{
public:
	SensorLinesTokenizer(const QString& info)
	{
		tokens.setAutoDelete(TRUE);

		QString s = info;
		while (s.length() > 0)
		{
			int newline;

			if ((newline = s.find('\n')) < 0)
			{
				tokens.append(new QString(s));
				break;
			}
			else
			{
				tokens.append(new QString(s.left(newline)));
				s = s.remove(0, newline + 1);
			}
		}
	}
	~SensorLinesTokenizer() { }

	const QString& operator[](int idx) { return *(tokens.at(idx)); }
	unsigned numberOfTokens() { return (tokens.count()); }

private:
	QList<QString> tokens;
} ;

#endif
