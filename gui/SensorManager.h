/*
    KTop, the KDE Task Manager
   
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

	$Id$
*/

#ifndef _SensorManager_h_
#define _SensorManager_h_

#include <qobject.h>
#include <qdict.h>

#include "SensorAgent.h"

class SensorManagerIterator;

class SensorManager : public QObject
{
	Q_OBJECT

	friend SensorManagerIterator;

public:
	SensorManager();
	~SensorManager();

	SensorAgent* engage(const QString& hostname, const QString& shell = "ssh",
						const QString& command = "");
	void disengage(const SensorAgent* sensor);

	const QString getHostName(const SensorAgent* sensor) const;

signals:
	void update();

protected:
	QDict<SensorAgent> sensors;
} ;

extern SensorManager* SensorMgr;

class SensorManagerIterator : public QDictIterator<SensorAgent>
{
public:
	SensorManagerIterator(const SensorManager* sm) :
		QDictIterator<SensorAgent>(sm->sensors) { }
	~SensorManagerIterator() { }
} ;

#endif
