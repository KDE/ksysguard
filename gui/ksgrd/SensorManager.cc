/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

*/

#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include "HostConnector.h"
#include "SensorShellAgent.h"
#include "SensorSocketAgent.h"

#include "SensorManager.h"

using namespace KSGRD;

SensorManager* KSGRD::SensorMgr;

SensorManager::SensorManager()
{
  mAgents.setAutoDelete( true );
  mDict.setAutoDelete( true );

  // Fill the sensor description dictionary.
  mDict.insert( "cpu", new QString( i18n( "CPU Load" ) ) );
  mDict.insert( "idle", new QString( i18n( "Idle Load" ) ) );
  mDict.insert( "sys", new QString( i18n( "System Load" ) ) );
  mDict.insert( "nice", new QString( i18n( "Nice Load" ) ) );
  mDict.insert( "user", new QString( i18n( "User Load" ) ) );
  mDict.insert( "mem", new QString( i18n( "Memory" ) ) );
  mDict.insert( "physical", new QString( i18n( "Physical Memory" ) ) );
  mDict.insert( "swap", new QString( i18n( "Swap Memory" ) ) );
  mDict.insert( "cached", new QString( i18n( "Cached Memory" ) ) );
  mDict.insert( "buf", new QString( i18n( "Buffered Memory" ) ) );
  mDict.insert( "used", new QString( i18n( "Used Memory" ) ) );
  mDict.insert( "application", new QString( i18n( "Application Memory" ) ) );
  mDict.insert( "free", new QString( i18n( "Free Memory" ) ) );
  mDict.insert( "pscount", new QString( i18n( "Process Count" ) ) );
  mDict.insert( "ps", new QString( i18n( "Process Controller" ) ) );
  mDict.insert( "disk", new QString( i18n( "Disk Throughput" ) ) );
  mDict.insert( "load", new QString( i18n( "CPU Load", "Load" ) ) );
  mDict.insert( "total", new QString( i18n( "Total Accesses" ) ) );
  mDict.insert( "rio", new QString( i18n( "Read Accesses" ) ) );
  mDict.insert( "wio", new QString( i18n( "Write Accesses" ) ) );
  mDict.insert( "rblk", new QString( i18n( "Read Data" ) ) );
  mDict.insert( "wblk", new QString( i18n( "Write Data" ) ) );
  mDict.insert( "pageIn", new QString( i18n( "Pages In" ) ) );
  mDict.insert( "pageOut", new QString( i18n( "Pages Out" ) ) );
  mDict.insert( "context", new QString( i18n( "Context Switches" ) ) );
  mDict.insert( "network", new QString( i18n( "Network" ) ) );
  mDict.insert( "interfaces", new QString( i18n( "Interfaces" ) ) );
  mDict.insert( "receiver", new QString( i18n( "Receiver" ) ) );
  mDict.insert( "transmitter", new QString( i18n( "Transmitter" ) ) );
  mDict.insert( "data", new QString( i18n( "Data" ) ) );
  mDict.insert( "compressed", new QString( i18n( "Compressed Packets" ) ) );
  mDict.insert( "drops", new QString( i18n( "Dropped Packets" ) ) );
  mDict.insert( "errors", new QString( i18n( "Errors" ) ) );
  mDict.insert( "fifo", new QString( i18n( "FIFO Overruns" ) ) );
  mDict.insert( "frame", new QString( i18n( "Frame Errors" ) ) );
  mDict.insert( "multicast", new QString( i18n( "Multicast" ) ) );
  mDict.insert( "packets", new QString( i18n( "Packets" ) ) );
  mDict.insert( "carrier", new QString( i18n( "Carrier" ) ) );
  mDict.insert( "collisions", new QString( i18n( "Collisions" ) ) );
  mDict.insert( "sockets", new QString( i18n( "Sockets" ) ) );
  mDict.insert( "count", new QString( i18n( "Total Number" ) ) );
  mDict.insert( "list", new QString( i18n( "Table" ) ) );
  mDict.insert( "apm", new QString( i18n( "Advanced Power Management" ) ) );
  mDict.insert( "acpi", new QString( i18n( "ACPI" ) ) );
  mDict.insert( "thermal_zone", new QString( i18n( "Thermal Zone" ) ) );
  mDict.insert( "temperature", new QString( i18n( "Temperature" ) ) );
  mDict.insert( "fan", new QString( i18n( "Fan" ) ) );
  mDict.insert( "state", new QString( i18n( "State" ) ) );
  mDict.insert( "battery", new QString( i18n( "Battery" ) ) );
  mDict.insert( "batterycharge", new QString( i18n( "Battery Charge" ) ) );
  mDict.insert( "batteryusage", new QString( i18n( "Battery Usage" ) ) );
  mDict.insert( "remainingtime", new QString( i18n( "Remaining Time" ) ) );
  mDict.insert( "interrupts", new QString( i18n( "Interrupts" ) ) );
  mDict.insert( "loadavg1", new QString( i18n( "Load Average (1 min)" ) ) );
  mDict.insert( "loadavg5", new QString( i18n( "Load Average (5 min)" ) ) );
  mDict.insert( "loadavg15", new QString( i18n( "Load Average (15 min)" ) ) );
  mDict.insert( "clock", new QString( i18n( "Clock Frequency" ) ) );
  mDict.insert( "lmsensors", new QString( i18n( "Hardware Sensors" ) ) );
  mDict.insert( "partitions", new QString( i18n( "Partition Usage" ) ) );
  mDict.insert( "usedspace", new QString( i18n( "Used Space" ) ) );
  mDict.insert( "freespace", new QString( i18n( "Free Space" ) ) );
  mDict.insert( "filllevel", new QString( i18n( "Fill Level" ) ) );

  for ( int i = 0; i < 32; i++ ) {
    mDict.insert( "cpu" + QString::number( i ),
                 new QString( QString( i18n( "CPU%1" ) ).arg( i ) ) );
    mDict.insert( "disk" + QString::number( i ),
                 new QString( QString( i18n( "Disk%1" ) ).arg( i ) ) );
  }

  for ( int i = 0; i < 6; i++) {
    mDict.insert( "fan" + QString::number( i ),
                 new QString( QString( i18n( "Fan%1" ) ).arg( i ) ) );
    mDict.insert( "temp" + QString::number( i ),
                 new QString( QString( i18n( "Temperature%1" ) ).arg( i ) ) );
  }

  mDict.insert( "int00", new QString( i18n( "Total" ) ) );

  QString num;
  for ( int i = 1; i < 25; i++ ) {
    num.sprintf( "%.2d", i );
		mDict.insert( "int" + num,
                 new QString( QString( i18n( "Int%1" ) ).arg( i - 1, 3 ) ) );
	}

  mDescriptions.setAutoDelete( true );
  // TODO: translated descriptions not yet implemented.

  mUnits.setAutoDelete( true );
  mUnits.insert( "1/s", new QString( i18n( "the unit 1 per second", "1/s" ) ) );
  mUnits.insert( "kBytes", new QString( i18n( "kBytes" ) ) );
  mUnits.insert( "min", new QString( i18n( "the unit minutes", "min" ) ) );
  mUnits.insert( "MHz", new QString( i18n( "the frequency unit", "MHz" ) ) );

  mTypes.setAutoDelete( true );
  mTypes.insert( "integer", new QString( i18n( "Integer Value" ) ) );
  mTypes.insert( "float", new QString( i18n( "Floating Point Value" ) ) );
  mTypes.insert( "table", new QString( i18n( "Process Controller" ) ) );
  mTypes.insert( "listview", new QString( i18n( "Table" ) ) );

  mBroadcaster = 0;

  mHostConnector = new HostConnector( 0 );
}

SensorManager::~SensorManager()
{
  delete mHostConnector;
}

bool SensorManager::engageHost( const QString &hostName )
{
  bool retVal = true;

  if ( hostName.isEmpty() || mAgents.find( hostName ) == 0 ) {
    mHostConnector->setCurrentHostName( hostName );

    if ( mHostConnector->exec() ) {
      QString shell = "";
      QString command = "";
      int port = -1;

      /* Check which radio button is selected and set parameters
       * appropriately. */
      if ( mHostConnector->useSsh() )
        shell = "ssh";
      else if ( mHostConnector->useRsh() )
        shell = "rsh";
      else if ( mHostConnector->useDaemon() )
        port = mHostConnector->port();
      else
        command = mHostConnector->currentCommand();

      if ( hostName.isEmpty() )
        retVal = engage( mHostConnector->currentHostName(), shell,
                         command, port );
      else
        retVal = engage( hostName, shell, command, port );
    }
  }

  return retVal;
}

bool SensorManager::engage( const QString &hostName, const QString &shell,
                            const QString &command, int port )
{
  SensorAgent *agent;

  if ( ( agent = mAgents.find( hostName ) ) == 0 ) {
    if ( port == -1 )
      agent = new SensorShellAgent( this );
    else
      agent = new SensorSocketAgent( this );

    if ( !agent->start( hostName.ascii(), shell, command, port ) ) {
      delete agent;
      return false;
    }

    mAgents.insert( hostName, agent );
    connect( agent, SIGNAL( reconfigure( const SensorAgent* ) ),
             SLOT( reconfigure( const SensorAgent* ) ) );

		emit update();
    return true;
  }

  return false;
}

void SensorManager::requestDisengage( const SensorAgent *agent )
{
	/* When a sensor agent becomes disfunctional it calles this function
	 * to request that it is being removed from the SensorManager. It must
	 * not call disengage() directly since it would trigger ~SensorAgent()
	 * while we are still in a SensorAgent member function.
	 * So we have to post an event which is later caught by
	 * SensorManger::customEvent(). */
  QCustomEvent* event = new QCustomEvent( QEvent::User, (void*)agent );
  kapp->postEvent( this, event );
}

bool SensorManager::disengage( const SensorAgent *agent )
{
  QDictIterator<SensorAgent> it( mAgents );

  for ( ; it.current(); ++it )
    if ( it.current() == agent ) {
      mAgents.remove( it.currentKey() );
      emit update();
      return true;
    }

  return false;
}

bool SensorManager::disengage( const QString &hostName )
{
  SensorAgent *agent;
  if ( ( agent = mAgents.find( hostName ) ) != 0 ) {
    mAgents.remove( hostName );
    emit update();
    return true;
  }

  return false;
}

bool SensorManager::resynchronize( const QString &hostName )
{
  SensorAgent *agent;

  if ( ( agent = mAgents.find( hostName ) ) == 0 )
    return false;

  QString shell, command;
  int port;
  hostInfo( hostName, shell, command, port );

  disengage( hostName );

	kdDebug (1215) << "Re-synchronizing connection to " << hostName << endl;

  return engage( hostName, shell, command );
}

void SensorManager::hostLost( const SensorAgent *agent )
{
  emit hostConnectionLost( agent->hostName() );

  if ( mBroadcaster ) {
    QCustomEvent *event = new QCustomEvent( QEvent::User );
    event->setData( new QString( i18n( "Connection to %1 has been lost." )
                    .arg( agent->hostName() ) ) );
    kapp->postEvent( mBroadcaster, event );
  }
}

void SensorManager::notify( const QString &msg ) const
{
  /* This function relays text messages to the toplevel widget that
   * displays the message in a pop-up box. It must be used for objects
   * that might have been deleted before the pop-up box is closed. */
  if ( mBroadcaster ) {
    QCustomEvent *event = new QCustomEvent( QEvent::User );
    event->setData( new QString( msg ) );
    kapp->postEvent( mBroadcaster, event );
  }
}

void SensorManager::setBroadcaster( QWidget *wdg )
{
  mBroadcaster = wdg;
}

void SensorManager::reconfigure( const SensorAgent* )
{
  emit update();
}

bool SensorManager::event( QEvent *event )
{
  if ( event->type() == QEvent::User ) {
    disengage( (const SensorAgent*)((QCustomEvent*)event)->data() );
    return true;
  }

  return false;
}

bool SensorManager::sendRequest( const QString &hostName, const QString &req,
                                 SensorClient *client, int id )
{
  SensorAgent *agent;
  if ( ( agent = mAgents.find( hostName ) ) != 0 ) {
    agent->sendRequest( req, client, id );
    return true;
  }

  return false;
}

const QString SensorManager::hostName( const SensorAgent *agent) const
{
  QDictIterator<SensorAgent> it( mAgents );
	
  while ( it.current() ) {
    if ( it.current() == agent )
      return it.currentKey();
    ++it;
  }

  return QString::null;
}

bool SensorManager::hostInfo( const QString &hostName, QString &shell,
                              QString &command, int &port )
{
  SensorAgent *agent;
  if ( ( agent = mAgents.find( hostName ) ) != 0 ) {
    agent->hostInfo( shell, command, port );
    return true;
  }

  return false;
}

const QString &SensorManager::translateUnit( const QString &unit ) const
{
  if ( !unit.isEmpty() && mUnits[ unit ] )
    return *mUnits[ unit ];
  else
    return unit;
}

const QString &SensorManager::translateSensorPath( const QString &path ) const
{
  if ( !path.isEmpty() && mDict[ path ] )
    return *mDict[ path ];
  else
    return path;
}
	
const QString &SensorManager::translateSensorType( const QString &type ) const
{
  if ( !type.isEmpty() && mTypes[ type ] )
    return *mTypes[ type ];
  else
    return type;
}

QString SensorManager::translateSensor( const QString &sensor ) const
{
  QString token, out;
  int start = 0, end = 0;
  for ( ; ; ) {
    end = sensor.find( '/', start );
    if ( end > 0 )
      out += translateSensorPath( sensor.mid( start, end - start ) ) + "/";
    else {
      out += translateSensorPath( sensor.right( sensor.length() - start ) );
      break;
    }
    start = end + 1;
  }

  return out;
}

void SensorManager::readProperties( KConfig *cfg )
{
  mHostConnector->setHostNames( cfg->readListEntry( "HostList" ) );
  mHostConnector->setCommands( cfg->readListEntry( "CommandList" ) );
}

void
SensorManager::saveProperties( KConfig *cfg )
{
  cfg->writeEntry( "HostList", mHostConnector->hostNames() );
  cfg->writeEntry( "CommandList", mHostConnector->commands() );
}

void SensorManager::disconnectClient( SensorClient *client )
{
  QDictIterator<SensorAgent> it( mAgents );

  for ( ; it.current(); ++it)
    it.current()->disconnectClient( client );
}

#include "SensorManager.moc"
