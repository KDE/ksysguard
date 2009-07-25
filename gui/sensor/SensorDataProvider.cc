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
#include "SensorDataProvider.h"
#include "BasicSensor.h"

SensorDataProvider::SensorDataProvider() {


}

SensorDataProvider::~SensorDataProvider() {
	qDeleteAll(mSensors);
}



void SensorDataProvider::reorderSensor(const QList<int> & newOrder) {
    int newOrderSize = newOrder.size();
    if (newOrderSize <= mSensors.count()) {
        for (int newIndex = 0; newIndex < newOrderSize; ++newIndex) {
            int oldIndex = newOrder.at(newIndex);
            mSensors.swap(newIndex,oldIndex);
        }
    }
}


#include "SensorDataProvider.moc"
