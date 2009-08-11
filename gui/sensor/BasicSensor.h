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
    BasicSensor(const QString &name, const QString &hostName, const QString &type, const QString &regexpName);
    BasicSensor(const QList<QString> &name, const QString &hostName, const QString &type, const QString &regexpName);
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

    QString type() const;
    QString regexpName() const;
    bool isInteger() const;
    bool isLocalHost() const;

    bool isOk() const;

    void setOk(bool value);

    /* titles have to be added in the same order as the name list was provided */
    void addTitle(const QString &title);

    virtual bool isAggregateSensor() const;

private:
    void init(const QString &hostName, const QString &type, const QString &regexpName);


    QList<QString> mNameList;
    QString mHostName;
    QString mType;
    QString mRegexpName;
    QList<QString> mTitleList;
    bool mInteger;
    bool mOk;
    bool mLocalHost;
};

inline bool BasicSensor::isInteger() const {
    return mInteger;
}
#endif /* BASICSENSOR_H_ */
