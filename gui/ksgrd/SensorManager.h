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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id$
*/

#ifndef KSG_SENSORMANAGER_H
#define KSG_SENSORMANAGER_H

#include <kconfig.h>

#include <qdict.h>
#include <qobject.h>

#include <SensorAgent.h>

class HostConnector;

namespace KSGRD {

class SensorManagerIterator;

/**
  The SensorManager handles all interaction with the connected
  hosts. Connections to a specific hosts are handled by
  SensorAgents. Use engage() to establish a connection and
  disengage() to terminate the connection. If you don't know if a
  certain host is already connected use engageHost(). If there is no
  connection yet or the hostname is empty, a dialog will be shown to
  enter the connections details.
 */
class SensorManager : public QObject
{
  Q_OBJECT

  friend class SensorManagerIterator;

  public:
    SensorManager();
    ~SensorManager();

    bool engageHost( const QString &hostName );
    bool engage( const QString &hostName, const QString &shell = "ssh",
                 const QString &command = "", int port = -1 );

    void requestDisengage( const SensorAgent *agent );
    bool disengage( const SensorAgent *agent );
    bool disengage( const QString &hostName );
    bool resynchronize( const QString &hostName );
    void hostLost( const SensorAgent *agent );
    void notify( const QString &msg ) const;

    void setBroadcaster( QWidget *wdg );

    virtual bool event( QEvent *event );

    bool sendRequest( const QString &hostName, const QString &request,
                      SensorClient *client, int id = 0 );

    const QString hostName( const SensorAgent *sensor ) const;
    bool hostInfo( const QString &host, QString &shell,
                   QString &command, int &port );

    const QString& translateUnit( const QString &unit ) const;
    const QString& translateSensorPath( const QString &path ) const;
    const QString& translateSensorType( const QString &type ) const;
    QString translateSensor(const QString& u) const;

    void readProperties( KConfig *cfg );
    void saveProperties( KConfig *cfg );

    void disconnectClient( SensorClient *client );
	
  public slots:
    void reconfigure( const SensorAgent *agent );

  signals:
    void update();
    void hostConnectionLost( const QString &hostName );

  protected:
    QDict<SensorAgent> mAgents;

  private:
    /**
      These dictionary stores the localized versions of the sensor
      descriptions and units.
     */
    QDict<QString> mDescriptions;
    QDict<QString> mUnits;
    QDict<QString> mDict;
    QDict<QString> mTypes;

    QWidget* mBroadcaster;

    HostConnector* mHostConnector;
};

extern SensorManager* SensorMgr;

class SensorManagerIterator : public QDictIterator<SensorAgent>
{
  public:
    SensorManagerIterator( const SensorManager *sm )
      : QDictIterator<SensorAgent>( sm->mAgents ) { }

    ~SensorManagerIterator() { }
};

}

#endif
