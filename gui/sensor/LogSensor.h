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

#ifndef LOGSENSOR_H_
#define LOGSENSOR_H_

#include "BasicSensor.h"

class LogSensor: public BasicSensor {

public:
    LogSensor(QString name, QString hostName);
    virtual ~LogSensor();

    void setFileName( const QString& name );
    QString fileName() const;

    void setUpperLimitActive( bool value );
    bool upperLimitActive() const;

    void setLowerLimitActive( bool value );
    bool lowerLimitActive() const;

    void setUpperLimit( double value );
    double upperLimit() const;

    void setLowerLimit( double value );
    double lowerLimit() const;

    bool limitReached() const;
    void setLimitReached(bool limit);

  private:

    QString mFileName;

    bool mLowerLimitActive;
    bool mUpperLimitActive;

    double mLowerLimit;
    double mUpperLimit;

    bool mLimitReached;
};

#endif /* LOGSENSOR_H_ */
