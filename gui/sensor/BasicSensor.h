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

#ifndef BASICSENSOR_H_
#define BASICSENSOR_H_
#include <QList>
#include <QString>
#include <QListIterator>
#include <QColor>
#include <QtCore>

/**
 * BasicSensor class represent the most basic sensor required by any of our display.
 */
class BasicSensor {
public:
	BasicSensor(const QString argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor);
	BasicSensor(const QList<QString> argName, const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor);
	virtual ~BasicSensor();

	/* return first name in the list*/
	QString name() const;
	/* return first title in the list*/
	QString title() const;

	/* this is the list of sensor name, sensor can be associated with multiple names, for example mem/swap/free and mem/swap/used in the case of aggregate sensor*/
	QList<QString> nameList() const;
	/* title list associated with the name list, name(0) goes with title(0) */
	QList<QString> titleList() const;

	QString hostName() const;
	QColor color() const;
	QString type() const;
	QString regexpName() const;
	bool isInteger() const;
	bool isLocalHost() const;
	int dataSize() const;
	double data(int argIndex) const;
	QString unit() const;
	bool isOk() const;
	double lastValue() const;
	void setIsOk(const bool argValue);
	void setUnit(const QString argUnit);
	/* title have to be added in the same order as the name list was provided*/
	void addTitle(const QString argTitle);
	/* this is the theorethical maximum value that the sensor can have, for example as provided by ksysguardd*/
	double theorethicalMaxValue() const;
	void removeOldestValue(int argNumberToRemove = 1);

	/* last seen value by a client of this sensor, used to speed up drawing in some instance*/
	double lastSeenValue() const;
	double prevSeenValue() const;
	void updateLastSeenValue(double argLastSeenValue);

	/* this is implementation specific, for a basic sensor it simply override the previous value, i.e. a set*/
	virtual void putTheoreticalMaxValue(double argTheorethicalMaxValue);

	virtual void setColor(const QColor argColor);
	virtual void addData(const double argValue);
	virtual bool isAggregateSensor() const;

protected:
	double removeOneOldestValue();
private:
	void init(const QString argHostName, const QString argType, const QString argRegexpName, const QColor argSensorColor);

	QColor sensorColor;
	QList<QString> mNameList;
	QString mHostName;
	QString mType;
	QString mRegexpName;
	QList<QString> mTitleList;
	QString mUnit;
	QList<double> sensorData;
	double mTheorethicalMaxValue;
	double mLastSeenValue;
	double mPrevSeenValue;
	bool localHost;
	bool ok;
	bool integer;

};

inline QColor BasicSensor::color() const {
	return sensorColor;
}

inline double BasicSensor::data(int argIndex) const {
	return sensorData.at(argIndex);
}

inline int BasicSensor::dataSize() const {
	return sensorData.size();
}

inline double BasicSensor::theorethicalMaxValue() const {
	return mTheorethicalMaxValue;
}

inline double BasicSensor::lastValue() const {
	return sensorData.last();
}

inline double BasicSensor::lastSeenValue() const {
	return mLastSeenValue;
}

inline double BasicSensor::prevSeenValue() const {
	return mPrevSeenValue;
}

inline void BasicSensor::updateLastSeenValue(double argLastSeenValue) {
	mPrevSeenValue = mLastSeenValue;
	mLastSeenValue = argLastSeenValue;
}

inline bool BasicSensor::isInteger() const {
	return integer;
}
#endif /* BASICSENSOR_H_ */
