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

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
    not commit any changes without consulting me first. Thanks!

*/

#include <qdragobject.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorManager.h>

#include "SensorBrowser.h"

class HostItem : public QListViewItem
{
  public:
    HostItem( SensorBrowser *parent, const QString &text, int id,
              KSGRD::SensorAgent *host)
      : QListViewItem( parent, text ), mInited( false ), mId( id ),
        mHost( host ), mParent( parent )
    {
      setExpandable( true );
    }
   
    void setOpen( bool open )
    {
      if ( open && !mInited ) {
        mInited = true;
        // request sensor list from host
        mHost->sendRequest( "monitors", mParent, mId );
      }

      QListViewItem::setOpen( open );
    }
   
  private:
    bool mInited;
    int mId;
    KSGRD::SensorAgent* mHost;
    SensorBrowser* mParent;
};

SensorBrowser::SensorBrowser( QWidget* parent, KSGRD::SensorManager* sm,
                              const char* name)
  : QListView( parent, name ), mSensorManager( sm )
{
  mHostInfoList.setAutoDelete(true);

  connect( mSensorManager, SIGNAL( update() ), SLOT( update() ) );
  connect( this, SIGNAL( selectionChanged( QListViewItem* ) ),
           SLOT( newItemSelected( QListViewItem* ) ) );

  addColumn( i18n( "Sensor Browser" ) );
  addColumn( i18n( "Sensor Type" ) );
  QToolTip::add( this, i18n( "Drag sensors to empty cells of a worksheet "
                             "or the panel applet." ) );
  setRootIsDecorated( true );

  mIconLoader = new KIconLoader();

  // The sensor browser can be completely hidden.
  setMinimumWidth( 1 );

  QWhatsThis::add( this, i18n( "The sensor browser lists the connected hosts and the sensors "
                               "that they provide. Click and drag sensors into drop zones "
                               "of a worksheet or the panel applet. A display will appear "
                               "that visualizes the "
                               "values provided by the sensor. Some sensor displays can "
                               "display values of multiple sensors. Simply drag other "
                               "sensors on to the display to add more sensors." ) );
}

SensorBrowser::~SensorBrowser()
{
  delete mIconLoader;
}

void SensorBrowser::disconnect()
{
  QPtrListIterator<HostInfo> it( mHostInfoList );

  for ( ; it.current(); ++it )
    if ( (*it)->listViewItem()->isSelected() ) {
      kdDebug(1215) << "Disconnecting " <<  (*it)->hostName() << endl;
      KSGRD::SensorMgr->disengage( (*it)->sensorAgent() );
    }
}

void SensorBrowser::hostReconfigured( const QString& )
{
  // TODO: not yet implemented.
}

void SensorBrowser::update()
{
  static int id = 0;

  KSGRD::SensorManagerIterator it( mSensorManager );

  mHostInfoList.clear();
  clear();

  KSGRD::SensorAgent* host;
  for ( int i = 0 ; ( host = it.current() ); ++it, ++i ) {
    QString hostName = mSensorManager->hostName( host );
    HostItem* lvi = new HostItem( this, hostName, id, host );

    QPixmap pix = mIconLoader->loadIcon( "computer", KIcon::Desktop, KIcon::SizeSmall );
    lvi->setPixmap( 0, pix );

    HostInfo* hostInfo = new HostInfo( id, host, hostName, lvi );
    mHostInfoList.append( hostInfo );
    ++id;
  }

  setMouseTracking( false );
}

void SensorBrowser::newItemSelected( QListViewItem *item )
{
  if ( item->pixmap( 0 ) )
    KMessageBox::information( this, i18n( "Drag sensors to empty fields in a worksheet" ),
                              QString::null, "ShowSBUseInfo" );
}

void SensorBrowser::answerReceived( int id, const QString &answer )
{
  /* An answer has the following format:

     cpu/idle integer
     cpu/sys  integer
     cpu/nice integer
     cpu/user integer
     ps       table
  */

  QPtrListIterator<HostInfo> it( mHostInfoList );

  /* Check if id is still valid. It can get obsolete by rapid calls
   * of update() or when the sensor died. */
  for ( ; it.current(); ++it )
    if ( (*it)->id() == id )
      break;

  if ( !it.current() )
    return;

  KSGRD::SensorTokenizer lines( answer, '\n' );

  for ( uint i = 0; i < lines.count(); ++i ) {
    if ( lines[ i ].isEmpty() )
      break;

    KSGRD::SensorTokenizer words( lines[ i ], '\t' );
    QString sensorName = words[ 0 ];
    QString sensorType = words[ 1 ];

    /* Calling update() a rapid sequence will create pending
     * requests that do not get erased by calling
     * clear(). Subsequent updates will receive the old pending
     * answers so we need to make sure that we register each
     * sensor only once. */
    if ( (*it)->isRegistered( sensorName ) )
      return;

    /* The sensor browser can display sensors in a hierachical order.
     * Sensors can be grouped into nodes by seperating the hierachical
     * nodes through slashes in the sensor name. E. g. cpu/user is
     * the sensor user in the cpu node. There is no limit for the
     * depth of nodes. */
    KSGRD::SensorTokenizer absolutePath( sensorName, '/' );

    QListViewItem* parent = (*it)->listViewItem();
    for ( uint j = 0; j < absolutePath.count(); ++j ) {
      // Localize the sensor name part by part.
      QString name = KSGRD::SensorMgr->translateSensorPath( absolutePath[ j ] );

      bool found = false;
      QListViewItem* sibling = parent->firstChild();
      while ( sibling && !found ) {
        if (sibling->text( 0 ) == name ) {
          // The node or sensor is already known.
          found = true;
        } else
          sibling = sibling->nextSibling();
      }
      if ( !found ) {
        QListViewItem* lvi = new QListViewItem( parent, name );
        if ( j == absolutePath.count() - 1 ) {
          QPixmap pix = mIconLoader->loadIcon( "ksysguardd", KIcon::Desktop,
                                               KIcon::SizeSmall );
          lvi->setPixmap( 0, pix );
          lvi->setText( 1, KSGRD::SensorMgr->translateSensorType( sensorType ) );

          // add sensor info to internal data structure
          (*it)->addSensor( lvi, sensorName, name, sensorType );
        } else
          parent = lvi;

        // The child indicator might need to be updated.
        repaintItem( parent );
      } else
        parent = sibling;
    }
  }

  repaintItem( (*it)->listViewItem() );
}

void SensorBrowser::viewportMouseMoveEvent( QMouseEvent *e )
{
  /* setMouseTracking(false) seems to be broken. With current Qt
   * mouse tracking cannot be turned off. So we have to check each event
   * whether the LMB is really pressed. */

  if ( !( e->state() & LeftButton ) )
    return;

  QListViewItem* item = itemAt( e->pos() );
  if ( !item ) // no item under cursor
    return;

  // Make sure that a sensor and not a node or hostname has been picked.
  QPtrListIterator<HostInfo> it( mHostInfoList );
  for ( ; it.current() && !(*it)->isRegistered( item ); ++it );
  if ( !it.current() )
    return;

  // Create text drag object as
  // "<hostname> <sensorname> <sensortype> <sensordescription>".
  // Only the description may contain blanks.
  mDragText = (*it)->hostName() + " " +
              (*it)->sensorName( item ) + " " +
              (*it)->sensorType( item ) + " " +
              (*it)->sensorDescription( item );

  QDragObject* dragObject = new QTextDrag( mDragText, this );
  dragObject->dragCopy();
}

QStringList SensorBrowser::listHosts()
{
  QStringList hostList;

  QPtrListIterator<HostInfo> it( mHostInfoList );
  for ( ; it.current(); ++it )
    hostList.append( (*it)->hostName() );

  return hostList;
}

QStringList SensorBrowser::listSensors( const QString &hostName )
{
  QPtrListIterator<HostInfo> it( mHostInfoList );
  for ( ; it.current(); ++it ) {
    if ( (*it)->hostName() == hostName ) {
      return (*it)->allSensorNames();
    }
  }

  return QStringList();
}

/**
 Helper classes
 */
SensorInfo::SensorInfo( QListViewItem *lvi, const QString &name,
                        const QString &desc, const QString &type )
  : mLvi( lvi ), mName( name ), mDesc( desc ), mType( type )
{
}

QListViewItem* SensorInfo::listViewItem() const
{
  return mLvi;
}

const QString& SensorInfo::name() const
{
  return mName;
}

const QString& SensorInfo::type() const
{
  return mType;
}

const QString& SensorInfo::description() const
{
  return mDesc;
}

HostInfo::HostInfo( int id, const KSGRD::SensorAgent *agent,
                    const QString &name, QListViewItem *lvi )
  : mId( id ), mSensorAgent( agent ), mHostName( name ), mLvi( lvi )
{
  mSensorList.setAutoDelete( true );
}

int HostInfo::id() const
{
  return ( mId );
}

const KSGRD::SensorAgent* HostInfo::sensorAgent() const
{
  return mSensorAgent;
}

const QString& HostInfo::hostName() const
{
  return mHostName;
}

QListViewItem* HostInfo::listViewItem() const
{
  return mLvi;
}

const QString& HostInfo::sensorName( const QListViewItem *lvi ) const
{
  QPtrListIterator<SensorInfo> it( mSensorList );
  for ( ; it.current() && (*it)->listViewItem() != lvi; ++it );

  Q_ASSERT( it.current() );
  return ( (*it)->name() );
}

QStringList HostInfo::allSensorNames() const
{
  QStringList list;

  QPtrListIterator<SensorInfo> it( mSensorList );
  for ( ; it.current(); ++it )
    list.append( it.current()->name() );

  return list;
}

const QString& HostInfo::sensorType( const QListViewItem *lvi ) const
{
  QPtrListIterator<SensorInfo> it( mSensorList );
  for ( ; it.current() && (*it)->listViewItem() != lvi; ++it );

  Q_ASSERT( it.current() );
  return ( (*it)->type() );
}

const QString& HostInfo::sensorDescription( const QListViewItem *lvi ) const
{
  QPtrListIterator<SensorInfo> it( mSensorList );
  for ( ; it.current() && (*it)->listViewItem() != lvi; ++it );

  Q_ASSERT( it.current() );
  return ( (*it)->description() );
}

void HostInfo::addSensor( QListViewItem *lvi, const QString& name,
                          const QString& desc, const QString& type )
{
  SensorInfo* info = new SensorInfo( lvi, name, desc, type );
  mSensorList.append( info );
}

bool HostInfo::isRegistered( const QString& name ) const
{
  QPtrListIterator<SensorInfo> it( mSensorList );
  for ( ; it.current(); ++it )
    if ( (*it)->name() == name )
      return true;

  return false;
}

bool HostInfo::isRegistered( QListViewItem *lvi ) const
{
  QPtrListIterator<SensorInfo> it( mSensorList );
  for ( ; it.current(); ++it )
    if ( (*it)->listViewItem() == lvi )
      return true;

  return false;
}

#include "SensorBrowser.moc"
