/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2009 Sebastien Martel <sebastiendevel@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef SENSORDATAPROVIDER_H_
#define SENSORDATAPROVIDER_H_
#include <QList>
#include "BasicSensor.h"

class SensorDataProvider {
public:
	SensorDataProvider();
	virtual ~SensorDataProvider();
	void addSensor(BasicSensor* toAdd);
	void removeSensor(BasicSensor* toRemove);
	void reorderSensor(const QList<int> & newOrder);
	int sensorCount() const;
	BasicSensor* sensor(int index);
	BasicSensor* removeSensor(int index);
private:
	QList<BasicSensor*> mSensors;
};


inline int SensorDataProvider::sensorCount() const  {
	return mSensors.size();
}

inline void SensorDataProvider::addSensor(BasicSensor* toAdd)  {
	mSensors.append(toAdd);
}

inline void SensorDataProvider::removeSensor(BasicSensor* toRemove)  {
	mSensors.removeOne(toRemove);
}

inline BasicSensor* SensorDataProvider::sensor(int index)  {
	return mSensors.at(index);
}

inline BasicSensor* SensorDataProvider::removeSensor(int index)  {
	return mSensors.takeAt(index);
}

#endif /* SENSORDATAPROVIDER_H_ */
