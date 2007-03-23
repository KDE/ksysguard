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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <qapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include "SensorShellAgent.h"
#include "SensorSocketAgent.h"

#include "SensorManager.h"

using namespace KSGRD;

class AgentEvent : public QEvent
{
  public:
    AgentEvent( const SensorAgent *agent )
      : QEvent( QEvent::User ), mAgent( agent )
    {
    }

    const SensorAgent *agent() const
    {
      return mAgent;
    }

  private:
    const SensorAgent *mAgent;
};

SensorManager::MessageEvent::MessageEvent( const QString &message )
  : QEvent( QEvent::User ), mMessage( message )
{
}

QString SensorManager::MessageEvent::message() const
{
  return mMessage;
}

SensorManager* KSGRD::SensorMgr;

SensorManager::SensorManager()
{
  // Fill the sensor description dictionary.
  mDict.insert( QLatin1String( "cpu" ), i18n( "CPU Load" ) );
  mDict.insert( QLatin1String( "idle" ), i18n( "Idling" ) );
  mDict.insert( QLatin1String( "nice" ), i18n( "Nice Load" ) );
  mDict.insert( QLatin1String( "user" ), i18n( "User Load" ) );
  mDict.insert( QLatin1String( "sys" ), i18n( "System Load" ) );
  mDict.insert( QLatin1String( "wait" ), i18n( "Waiting" ) );
  mDict.insert( QLatin1String( "TotalLoad" ), i18n( "Total Load" ) );
  mDict.insert( QLatin1String( "mem" ), i18n( "Memory" ) );
  mDict.insert( QLatin1String( "physical" ), i18n( "Physical Memory" ) );
  mDict.insert( QLatin1String( "swap" ), i18n( "Swap Memory" ) );
  mDict.insert( QLatin1String( "cached" ), i18n( "Cached Memory" ) );
  mDict.insert( QLatin1String( "buf" ), i18n( "Buffered Memory" ) );
  mDict.insert( QLatin1String( "used" ), i18n( "Used Memory" ) );
  mDict.insert( QLatin1String( "application" ), i18n( "Application Memory" ) );
  mDict.insert( QLatin1String( "free" ), i18n( "Free Memory" ) );
  mDict.insert( QLatin1String( "pscount" ), i18n( "Process Count" ) );
  mDict.insert( QLatin1String( "ps" ), i18n( "Process Controller" ) );
  mDict.insert( QLatin1String( "disk" ), i18n( "Disk Throughput" ) );
  mDict.insert( QLatin1String( "load" ), i18nc( "CPU Load", "Load" ) );
  mDict.insert( QLatin1String( "total" ), i18n( "Total Accesses" ) );
  mDict.insert( QLatin1String( "rio" ), i18n( "Read Accesses" ) );
  mDict.insert( QLatin1String( "wio" ), i18n( "Write Accesses" ) );
  mDict.insert( QLatin1String( "rblk" ), i18n( "Read Data" ) );
  mDict.insert( QLatin1String( "wblk" ), i18n( "Write Data" ) );
  mDict.insert( QLatin1String( "pageIn" ), i18n( "Pages In" ) );
  mDict.insert( QLatin1String( "pageOut" ), i18n( "Pages Out" ) );
  mDict.insert( QLatin1String( "context" ), i18n( "Context Switches" ) );
  mDict.insert( QLatin1String( "network" ), i18n( "Network" ) );
  mDict.insert( QLatin1String( "interfaces" ), i18n( "Interfaces" ) );
  mDict.insert( QLatin1String( "receiver" ), i18n( "Receiver" ) );
  mDict.insert( QLatin1String( "transmitter" ), i18n( "Transmitter" ) );
  mDict.insert( QLatin1String( "data" ), i18n( "Data" ) );
  mDict.insert( QLatin1String( "compressed" ), i18n( "Compressed Packets" ) );
  mDict.insert( QLatin1String( "drops" ), i18n( "Dropped Packets" ) );
  mDict.insert( QLatin1String( "errors" ), i18n( "Errors" ) );
  mDict.insert( QLatin1String( "fifo" ), i18n( "FIFO Overruns" ) );
  mDict.insert( QLatin1String( "frame" ), i18n( "Frame Errors" ) );
  mDict.insert( QLatin1String( "multicast" ), i18n( "Multicast" ) );
  mDict.insert( QLatin1String( "packets" ), i18n( "Packets" ) );
  mDict.insert( QLatin1String( "carrier" ), i18n( "Carrier" ) );
  mDict.insert( QLatin1String( "collisions" ), i18n( "Collisions" ) );
  mDict.insert( QLatin1String( "sockets" ), i18n( "Sockets" ) );
  mDict.insert( QLatin1String( "count" ), i18n( "Total Number" ) );
  mDict.insert( QLatin1String( "list" ), i18n( "Table" ) );
  mDict.insert( QLatin1String( "apm" ), i18n( "Advanced Power Management" ) );
  mDict.insert( QLatin1String( "acpi" ), i18n( "ACPI" ) );
  mDict.insert( QLatin1String( "thermal_zone" ), i18n( "Thermal Zone" ) );
  mDict.insert( QLatin1String( "temperature" ), i18n( "Temperature" ) );
  mDict.insert( QLatin1String( "fan" ), i18n( "Fan" ) );
  mDict.insert( QLatin1String( "state" ), i18n( "State" ) );
  mDict.insert( QLatin1String( "battery" ), i18n( "Battery" ) );
  mDict.insert( QLatin1String( "batterycharge" ), i18n( "Battery Charge" ) );
  mDict.insert( QLatin1String( "batteryusage" ), i18n( "Battery Usage" ) );
  mDict.insert( QLatin1String( "remainingtime" ), i18n( "Remaining Time" ) );
  mDict.insert( QLatin1String( "interrupts" ), i18n( "Interrupts" ) );
  mDict.insert( QLatin1String( "loadavg1" ), i18n( "Load Average (1 min)" ) );
  mDict.insert( QLatin1String( "loadavg5" ), i18n( "Load Average (5 min)" ) );
  mDict.insert( QLatin1String( "loadavg15" ), i18n( "Load Average (15 min)" ) );
  mDict.insert( QLatin1String( "clock" ), i18n( "Clock Frequency" ) );
  mDict.insert( QLatin1String( "lmsensors" ), i18n( "Hardware Sensors" ) );
  mDict.insert( QLatin1String( "partitions" ), i18n( "Partition Usage" ) );
  mDict.insert( QLatin1String( "usedspace" ), i18n( "Used Space" ) );
  mDict.insert( QLatin1String( "freespace" ), i18n( "Free Space" ) );
  mDict.insert( QLatin1String( "filllevel" ), i18n( "Fill Level" ) );
  mDict.insert( QLatin1String( "system" ), i18n( "System" ) );
  mDict.insert( QLatin1String( "uptime" ), i18n( "Uptime" ) );
  mDict.insert( QLatin1String( "SoftRaid" ), i18n( "Linux Soft Raid (md)" ) );

  for ( int i = 0; i < 32; i++ ) {
    mDict.insert( QLatin1String( "cpu" ) + QString::number( i ), i18n( "CPU %1", i+1 ) );
    mDict.insert( QLatin1String( "disk" ) + QString::number( i ), i18n( "Disk %1", i+1 ) );
  }

  for ( int i = 1; i < 6; i++) {
    mDict.insert( QLatin1String( "fan" ) + QString::number( i ), i18n( "Fan %1", i ) );
    mDict.insert( QLatin1String( "temp" ) + QString::number( i ), i18n( "Temperature %1", i ) );
  }

  mDict.insert( QLatin1String( "int00" ), i18n( "Total" ) );

  QString num;
  for ( int i = 1; i < 25; i++ ) {
    num.sprintf( "%.2d", i );
    mDict.insert( QLatin1String( "int" ) + num, ki18n( "Int %1" ).subs( i - 1, 3 ).toString() );
  }

  // TODO: translated descriptions not yet implemented.

  mUnits.insert( QLatin1String( "1/s" ), i18nc( "the unit 1 per second", "1/s" ) );
  mUnits.insert( QLatin1String( "kBytes" ), i18n( "kBytes" ) );
  mUnits.insert( QLatin1String( "min" ), i18nc( "the unit minutes", "min" ) );
  mUnits.insert( QLatin1String( "MHz" ), i18nc( "the frequency unit", "MHz" ) );
  mUnits.insert( QLatin1String( "%" ), i18nc( "a percentage", "%" ) );

  mTypes.insert( QLatin1String( "integer" ), i18n( "Integer Value" ) );
  mTypes.insert( QLatin1String( "float" ), i18n( "Floating Point Value" ) );
  mTypes.insert( QLatin1String( "table" ), i18n( "Process Controller" ) );
  mTypes.insert( QLatin1String( "listview" ), i18n( "Table" ) );

  mBroadcaster = 0;

}

SensorManager::~SensorManager()
{
}

bool SensorManager::engage( const QString &hostName, const QString &shell,
                            const QString &command, int port )
{
  if ( !mAgents.contains( hostName ) ) {
    SensorAgent *agent = 0;

    if ( port == -1 )
      agent = new SensorShellAgent( this );
    else
      agent = new SensorSocketAgent( this );

    if ( !agent->start( hostName.toAscii(), shell, command, port ) ) {
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
  /* When a sensor agent becomes disfunctional it calls this function
   * to request that it is being removed from the SensorManager. It must
   * not call disengage() directly since it would trigger ~SensorAgent()
   * while we are still in a SensorAgent member function.
   * So we have to post an event which is later caught by
   * SensorManger::customEvent(). */
  AgentEvent* event = new AgentEvent( agent );
  qApp->postEvent( this, event );
}

bool SensorManager::disengage( const SensorAgent *agent )
{
  const QString key = mAgents.key( const_cast<SensorAgent*>( agent ) );
  if ( !key.isEmpty() ) {
    mAgents.remove( key );

    emit update();
    return true;
  }

  return false;
}

bool SensorManager::isConnected( const QString &hostName )
{
  return mAgents.contains( hostName );
}
bool SensorManager::disengage( const QString &hostName )
{
  if ( mAgents.contains( hostName ) ) {
    mAgents.remove( hostName );

    emit update();
    return true;
  }

  return false;
}

bool SensorManager::resynchronize( const QString &hostName )
{
  const SensorAgent *agent = mAgents.value( hostName );

  if ( !agent )
    return false;

  QString shell, command;
  int port;
  hostInfo( hostName, shell, command, port );

  disengage( hostName );

  kDebug (1215) << "Re-synchronizing connection to " << hostName << endl;

  return engage( hostName, shell, command );
}

void SensorManager::hostLost( const SensorAgent *agent )
{
  emit hostConnectionLost( agent->hostName() );

  notify( i18n( "Connection to %1 has been lost.", agent->hostName() ) );
}

void SensorManager::notify( const QString &msg ) const
{
  /* This function relays text messages to the toplevel widget that
   * displays the message in a pop-up box. It must be used for objects
   * that might have been deleted before the pop-up box is closed. */
  if ( mBroadcaster ) {
    MessageEvent *event = new MessageEvent( msg );
    qApp->postEvent( mBroadcaster, event );
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
    disengage( static_cast<AgentEvent*>( event )->agent() );
    return true;
  }

  return false;
}

bool SensorManager::sendRequest( const QString &hostName, const QString &req,
                                 SensorClient *client, int id )
{
  SensorAgent *agent = mAgents.value( hostName );
  if ( !agent && hostName == "localhost") {
    //we should always be able to reconnect to localhost
    engage("localhost", "", "ksysguardd", -1);
    agent = mAgents.value( hostName );
  }
  if ( agent ) {
    agent->sendRequest( req, client, id );
    return true;
  }

  return false;
}

const QString SensorManager::hostName( const SensorAgent *agent ) const
{
  return mAgents.key( const_cast<SensorAgent*>( agent ) );
}

bool SensorManager::hostInfo( const QString &hostName, QString &shell,
                              QString &command, int &port )
{
  const SensorAgent *agent = mAgents.value( hostName );
  if ( agent ) {
    agent->hostInfo( shell, command, port );
    return true;
  }

  return false;
}

QString SensorManager::translateUnit( const QString &unit ) const
{
  if ( !unit.isEmpty() && mUnits.contains( unit ) )
    return mUnits[ unit ];
  else
    return unit;
}

QString SensorManager::translateSensorPath( const QString &path ) const
{
  if ( !path.isEmpty() && mDict.contains( path ) )
    return mDict[ path ];
  else
    return path;
}

QString SensorManager::translateSensorType( const QString &type ) const
{
  if ( !type.isEmpty() && mTypes.contains( type ) )
    return mTypes[ type ];
  else
    return type;
}

QString SensorManager::translateSensor( const QString &sensor ) const
{
  QString token, out;
  int start = 0, end = 0;
  for ( ; ; ) {
    end = sensor.indexOf( '/', start );
    if ( end > 0 )
      out += translateSensorPath( sensor.mid( start, end - start ) ) + '/';
    else {
      out += translateSensorPath( sensor.right( sensor.length() - start ) );
      break;
    }
    start = end + 1;
  }

  return out;
}

void SensorManager::readProperties( const KConfigGroup& cfg )
{
  mHostList = cfg.readEntry( "HostList" ,QStringList());
  mCommandList = cfg.readEntry( "CommandList",QStringList() );
}

void
SensorManager::saveProperties( KConfigGroup &cfg )
{
  cfg.writeEntry( "HostList", mHostList );
  cfg.writeEntry( "CommandList", mCommandList );
}

void SensorManager::disconnectClient( SensorClient *client )
{
  QHashIterator<QString, SensorAgent*> it( mAgents );

  while ( it.hasNext() )
    it.next().value()->disconnectClient( client );
}

#include "SensorManager.moc"
