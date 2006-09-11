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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_SENSORBROWSER_H
#define KSG_SENSORBROWSER_H

//Added by qt3to4:
#include <QMouseEvent>
#include <Q3PtrList>

#include <k3listview.h>
#include <ksgrd/SensorClient.h>

class QMouseEvent;
class KIconLoader;

namespace KSGRD {
class SensorManager;
class SensorAgent;
}

class SensorInfo;
class HostInfo;

/**
 * The SensorBrowser is the graphical front-end of the SensorManager. It
 * displays the currently available hosts and their sensors.
 */
class SensorBrowser : public K3ListView, public KSGRD::SensorClient
{
  Q_OBJECT

  public:
    SensorBrowser( QWidget* parent, KSGRD::SensorManager* sm );
    ~SensorBrowser();

    QStringList listHosts();
    QStringList listSensors( const QString &hostName );

  public Q_SLOTS:
    void disconnect();
    void hostReconfigured( const QString &hostName );
    void update();
    void newItemSelected( Q3ListViewItem *item );

  protected:
    virtual void viewportMouseMoveEvent( QMouseEvent* );

  private:
    void answerReceived( int id, const QStringList& );

    KIconLoader* mIconLoader;
    KSGRD::SensorManager* mSensorManager;

    QMap<int, HostInfo*> mHostInfoMap;
    QString mDragText;
};

/**
 Helper classes
 */
class SensorInfo
{
  public:
    SensorInfo( Q3ListViewItem *lvi, const QString &name,
                const QString &description, const QString &type );
    ~SensorInfo();

    /**
      Returns a pointer to the list view item of the sensor.
     */
    Q3ListViewItem* listViewItem() const;

    /**
      Returns the name of the sensor.
     */
    QString name() const;

    /**
      Returns the description of the sensor.
     */
    QString description() const;

    /**
      Returns the type of the sensor.
     */
    QString type() const;

  private:
    Q3ListViewItem* mLvi;
    QString mName;
    QString mDesc;
    QString mType;
};

class HostInfo
{
  public:
    HostInfo( int id, const KSGRD::SensorAgent *agent, const QString &name,
              Q3ListViewItem *lvi );
    ~HostInfo();

    /**
      Returns the unique id of the host.
     */
    int id() const;

    /**
      Returns a pointer to the sensor agent of the host.
     */
    const KSGRD::SensorAgent* sensorAgent() const;

    /**
      Returns the name of the host.
     */
    QString hostName() const;

    /**
      Returns the a pointer to the list view item of the host.
     */
    Q3ListViewItem* listViewItem() const;

    /**
      Returns the sensor name of a special list view item.
     */
    QString sensorName( const Q3ListViewItem *lvi ) const;

    /**
      Returns all sensor names of the host.
     */
    QStringList allSensorNames() const;

    /**
      Returns the type of a special list view item.
     */
    QString sensorType( const Q3ListViewItem *lvi ) const;

    /**
      Returns the description of a special list view item.
     */
    QString sensorDescription( const Q3ListViewItem *lvi ) const;

    /**
      Adds a new Sensor to the host.

      @param lvi  The list view item.
      @param name The sensor name.
      @param desc A description.
      @param type The type of the sensor.
     */
    void addSensor( Q3ListViewItem *lvi, const QString& name,
                    const QString& desc, const QString& type );

    /**
      Returns whether the sensor with @ref name
      is registered at the host.
     */
    bool isRegistered( const QString& name ) const;

    /**
      Returns whether the sensor with @ref lvi
      is registered at the host.
     */
    bool isRegistered( const Q3ListViewItem *lvi ) const;

  private:
    SensorInfo* findInfo( const Q3ListViewItem *item ) const;

    int mId;

    const KSGRD::SensorAgent* mSensorAgent;
    const QString mHostName;
    Q3ListViewItem* mLvi;

    QList<SensorInfo*> mSensorList;
};

#endif
