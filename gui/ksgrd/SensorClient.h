/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#ifndef KSG_SENSORCLIENT_H
#define KSG_SENSORCLIENT_H

#include <qptrlist.h>
#include <qstring.h>

namespace KSGRD {

/**
  Every object that should act as a client to a sensor must inherit from
  this class. A pointer to the client object is passed as SensorClient*
  to the SensorAgent. When the requested information is available or a
  problem occurred one of the member functions is called.
 */
class SensorClient
{
  public:
    SensorClient() { }
    virtual ~SensorClient() { }

    /**
      This function is called whenever the information form the sensor has
      been received by the sensor agent. This function must be reimplemented
      by the sensor client to receive and process this information.
     */
    virtual void answerReceived( int, const QString& ) { }

    /**
      In case of an unexpected fatal problem with the sensor the sensor
      agent will call this function to notify the client about it.
     */
    virtual void sensorLost( int ) { }
};

/**
  Every object that has a SensorClient as a child must inherit from
  this class to support the advanced update interval settings.
 */
class SensorBoard
{
  public:
    SensorBoard() { }
    virtual ~SensorBoard() { }

    void updateInterval( int interval ) { mUpdateInterval = interval; }

    int updateInterval() { return mUpdateInterval; }

  private:
    int mUpdateInterval;
};

/**
  The following classes are utility classes that provide a
  convenient way to retrieve pieces of information from the sensor
  answers. For each type of answer there is a separate class.
 */
class SensorTokenizer
{
  public:
    SensorTokenizer( const QString &info, QChar separator )
    {
      mTokens = QStringList::split( separator, info );
    }

    ~SensorTokenizer() { }

    const QString& operator[]( unsigned idx )
	  {
      return mTokens[ idx ];
    }

    uint count()
    {
      return mTokens.count();
    }

  private:
    QStringList mTokens;
};

/**
  An integer info contains 4 fields seperated by TABS, a description
  (name), the minimum and the maximum values and the unit.
  e.g. Swap Memory	0	133885952	KB
 */
class SensorIntegerInfo : public SensorTokenizer
{
  public:
    SensorIntegerInfo( const QString &info )
      : SensorTokenizer( info, '\t' ) { }

    ~SensorIntegerInfo() { }

    const QString &name()
    {
      return (*this)[ 0 ];
    }

    long min()
    {
      return (*this)[ 1 ].toLong();
    }

    long max()
    {
      return (*this)[ 2 ].toLong();
    }

    const QString &unit()
    {
      return (*this)[ 3 ];
    }
};

/**
  An float info contains 4 fields seperated by TABS, a description
  (name), the minimum and the maximum values and the unit.
  e.g. CPU Voltage 0.0	5.0	V
 */
class SensorFloatInfo : public SensorTokenizer
{
  public:
    SensorFloatInfo( const QString &info )
      : SensorTokenizer( info, '\t' ) { }

    ~SensorFloatInfo() { }

    const QString &name()
    {
      return (*this)[ 0 ];
    }

    double min()
    {
      return (*this)[ 1 ].toDouble();
    }

    double max()
    {
      return (*this)[ 2 ].toDouble();
    }

    const QString &unit()
    {
      return (*this)[ 3 ];
    }
};

/**
  A PS line consists of information about a process. Each piece of 
  information is seperated by a TAB. The first 4 fields are process name,
  PID, PPID and real user ID. Those fields are mandatory.
 */
class SensorPSLine : public SensorTokenizer
{
  public:
    SensorPSLine( const QString &line )
      : SensorTokenizer( line, '\t' ) { }

    ~SensorPSLine() { }

    const QString& name()
    {
      return (*this)[ 0 ];
    }

    long pid()
    {
      return (*this)[ 1 ].toLong();
    }

    long ppid()
    {
      return (*this)[ 2 ].toLong();
    }

    long uid()
    {
      return (*this)[ 3 ].toLong();
    }
};

}

#endif
