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

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
    not commit any changes without consulting me first. Thanks!

*/

#include <QtGui/QDrag>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorManager.h>

#include "SensorBrowser.h"

class HostItem : public Q3ListViewItem
{
  public:
    HostItem( SensorBrowser *parent, const QString &text, int id,
              KSGRD::SensorAgent *host)
      : Q3ListViewItem( parent, text ), mInited( false ), mId( id ),
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

      Q3ListViewItem::setOpen( open );
    }

  private:
    bool mInited;
    int mId;
    KSGRD::SensorAgent* mHost;
    SensorBrowser* mParent;
};

SensorBrowser::SensorBrowser( QWidget* parent, KSGRD::SensorManager* sm )
  : K3ListView( parent ), mSensorManager( sm )
{
  connect( mSensorManager, SIGNAL( update() ), SLOT( update() ) );
  connect( this, SIGNAL( clicked( Q3ListViewItem* ) ),
           SLOT( newItemSelected( Q3ListViewItem* ) ) );
  connect( this, SIGNAL( returnPressed( Q3ListViewItem* ) ),
           SLOT( newItemSelected( Q3ListViewItem* ) ) );

  addColumn( i18n( "Sensor Browser" ) );
  setFullWidth( true );

  this->setToolTip( i18n( "Drag sensors to empty cells of a worksheet "
                             "or the panel applet." ) );
  setRootIsDecorated( true );

  mIconLoader = new KIconLoader();

  // The sensor browser can be completely hidden.
  setMinimumWidth( 1 );

  this->setWhatsThis( i18n( "The sensor browser lists the connected hosts and the sensors "
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

  qDeleteAll( mHostInfoMap );
  mHostInfoMap.clear();
}

void SensorBrowser::disconnect()
{
  QMapIterator<int, HostInfo*> it( mHostInfoMap );

  while ( it.hasNext() ) {
    it.next();
    if ( it.value()->listViewItem()->isSelected() ) {
      kDebug(1215) << "Disconnecting " <<  it.value()->hostName() << endl;
      KSGRD::SensorMgr->requestDisengage( it.value()->sensorAgent() );
    }
  }
}

void SensorBrowser::hostReconfigured( const QString& )
{
  // TODO: not yet implemented.
}

void SensorBrowser::update()
{
  static int id = 0;

  qDeleteAll( mHostInfoMap );
  mHostInfoMap.clear();
  clear();

  KSGRD::SensorManagerIterator it( mSensorManager );
  while ( it.hasNext() ) {
    KSGRD::SensorAgent* host = it.next().value();
    QString hostName = mSensorManager->hostName( host );
    HostItem* lvi = new HostItem( this, hostName, id, host );

    QPixmap pix = mIconLoader->loadIcon( "computer", K3Icon::Desktop, K3Icon::SizeSmall );
    lvi->setPixmap( 0, pix );

    HostInfo* hostInfo = new HostInfo( id, host, hostName, lvi );
    mHostInfoMap.insert( id, hostInfo );
    ++id;
  }

  setMouseTracking( false );
}

void SensorBrowser::newItemSelected( Q3ListViewItem *item )
{
  static bool showAnnoyingPopup = true;
  if ( item && item->pixmap( 0 ) && showAnnoyingPopup)
  {
    showAnnoyingPopup = false;
    KMessageBox::information( this, i18n( "Drag sensors to empty fields in a worksheet." ),
                              QString(), "ShowSBUseInfo" );
  }
}

void SensorBrowser::answerReceived( int id, const QStringList &answer )
{
  /* An answer has the following format:

     cpu/idle integer
     cpu/sys  integer
     cpu/nice integer
     cpu/user integer
     ps       table
  */

  if ( !mHostInfoMap.contains( id ) )
    return;

  HostInfo *info = mHostInfoMap[ id ];

  for ( int i = 0; i < answer.count(); ++i ) {
    if ( answer[ i ].isEmpty() )
      break;

    KSGRD::SensorTokenizer words( answer[ i ], '\t' );
    QString sensorName = words[ 0 ];
    QString sensorType = words[ 1 ];

    /* Calling update() a rapid sequence will create pending
     * requests that do not get erased by calling
     * clear(). Subsequent updates will receive the old pending
     * answers so we need to make sure that we register each
     * sensor only once. */
    if ( info->isRegistered( sensorName ) )
      return;

    /* The sensor browser can display sensors in a hierachical order.
     * Sensors can be grouped into nodes by seperating the hierachical
     * nodes through slashes in the sensor name. E. g. cpu/user is
     * the sensor user in the cpu node. There is no limit for the
     * depth of nodes. */
    KSGRD::SensorTokenizer absolutePath( sensorName, '/' );

    Q3ListViewItem* parent = info->listViewItem();
    for ( uint j = 0; j < absolutePath.count(); ++j ) {
      // Localize the sensor name part by part.
      QString name = KSGRD::SensorMgr->translateSensorPath( absolutePath[ j ] );

      bool found = false;
      Q3ListViewItem* sibling = parent->firstChild();
      while ( sibling && !found ) {
        if (sibling->text( 0 ) == name ) {
          // The node or sensor is already known.
          found = true;
        } else
          sibling = sibling->nextSibling();
      }
      if ( !found ) {
        Q3ListViewItem* lvi = new Q3ListViewItem( parent, name );
        if ( j == absolutePath.count() - 1 ) {
          QPixmap pix = mIconLoader->loadIcon( "ksysguardd", K3Icon::Desktop,
                                               K3Icon::SizeSmall );
          lvi->setPixmap( 0, pix );

          // add sensor info to internal data structure
          info->addSensor( lvi, sensorName, name, sensorType );
        } else
          parent = lvi;

        // The child indicator might need to be updated.
        repaintItem( parent );
      } else
        parent = sibling;
    }
  }

  repaintItem( info->listViewItem() );
}

void SensorBrowser::viewportMouseMoveEvent( QMouseEvent *e )
{
  /* setMouseTracking(false) seems to be broken. With current Qt
   * mouse tracking cannot be turned off. So we have to check each event
   * whether the LMB is really pressed. */

  if ( !( e->buttons() & Qt::LeftButton ) )
    return;

  Q3ListViewItem* item = itemAt( e->pos() );
  if ( !item ) // no item under cursor
    return;

  // Make sure that a sensor and not a node or hostname has been picked.
  HostInfo *info = 0;

  QMapIterator<int, HostInfo*> it( mHostInfoMap );
  while ( it.hasNext() ) {
    it.next();
    if ( it.value()->isRegistered( item ) ) {
      info = it.value();
      break;
    }
  }

  if ( !info )
    return;

  // Create text drag object as
  // "<hostname> <sensorname> <sensortype> <sensordescription>".
  // Only the description may contain blanks.
  mDragText = info->hostName() + ' ' +
              info->sensorName( item ) + ' ' +
              info->sensorType( item ) + ' ' +
              info->sensorDescription( item );

  QDrag *drag = new QDrag( this );
  QMimeData *mimeData = new QMimeData;

  mimeData->setText( mDragText );
  drag->setMimeData( mimeData );

  /**
   * Create nice drag pixmap
   */
  QLabel *label = new QLabel( this );
  label->setMargin( 3 );
  label->setText( QString( "<html style=\"font-size:small\"><b>%1</b>&nbsp;&nbsp;&nbsp;(%2)</html>" )
                         .arg( info->sensorDescription( item ), info->sensorName( item ) ) );
  label->resize( label->sizeHint() );

  QPixmap pixmap = QPixmap::grabWidget( label );
  delete label;

  QPainter painter;
  painter.begin( &pixmap );
  painter.setPen( Qt::gray );
  painter.drawRect( 0, 0, pixmap.width() - 1, pixmap.height() - 1 );
  painter.end();

  drag->setPixmap( pixmap );

  drag->start();
}

QStringList SensorBrowser::listHosts()
{
  QStringList hostList;

  QMapIterator<int, HostInfo*> it( mHostInfoMap );
  while ( it.hasNext() ) {
    it.next();
    hostList.append( it.value()->hostName() );
  }

  return hostList;
}

QStringList SensorBrowser::listSensors( const QString &hostName )
{
  QMapIterator<int, HostInfo*> it( mHostInfoMap );
  while ( it.hasNext() ) {
    it.next();
    if ( it.value()->hostName() == hostName ) {
      return it.value()->allSensorNames();
    }
  }

  return QStringList();
}

/**
 Helper classes
 */
SensorInfo::SensorInfo( Q3ListViewItem *lvi, const QString &name,
                        const QString &desc, const QString &type )
  : mLvi( lvi ), mName( name ), mDesc( desc ), mType( type )
{
}

SensorInfo::~SensorInfo()
{
}

Q3ListViewItem* SensorInfo::listViewItem() const
{
  return mLvi;
}

QString SensorInfo::name() const
{
  return mName;
}

QString SensorInfo::type() const
{
  return mType;
}

QString SensorInfo::description() const
{
  return mDesc;
}

HostInfo::HostInfo( int id, const KSGRD::SensorAgent *agent,
                    const QString &name, Q3ListViewItem *lvi )
  : mId( id ), mSensorAgent( agent ), mHostName( name ), mLvi( lvi )
{
}

HostInfo::~HostInfo()
{
  qDeleteAll( mSensorList );
  mSensorList.clear();
}

int HostInfo::id() const
{
  return mId;
}

const KSGRD::SensorAgent* HostInfo::sensorAgent() const
{
  return mSensorAgent;
}

QString HostInfo::hostName() const
{
  return mHostName;
}

Q3ListViewItem* HostInfo::listViewItem() const
{
  return mLvi;
}

QString HostInfo::sensorName( const Q3ListViewItem *lvi ) const
{
  const SensorInfo *info = findInfo( lvi );
  if ( !info )
    return QString();

  return info->name();
}

QStringList HostInfo::allSensorNames() const
{
  QStringList list;

  QListIterator<SensorInfo*> it( mSensorList );
  while ( it.hasNext() )
    list.append( it.next()->name() );

  return list;
}

QString HostInfo::sensorType( const Q3ListViewItem *lvi ) const
{
  const SensorInfo *info = findInfo( lvi );
  if ( !info )
    return QString();

  return info->type();
}

QString HostInfo::sensorDescription( const Q3ListViewItem *lvi ) const
{
  const SensorInfo *info = findInfo( lvi );
  if ( !info )
    return QString();

  return info->description();
}

void HostInfo::addSensor( Q3ListViewItem *lvi, const QString& name,
                          const QString& desc, const QString& type )
{
  SensorInfo* info = new SensorInfo( lvi, name, desc, type );
  mSensorList.append( info );
}

bool HostInfo::isRegistered( const QString& name ) const
{
  QListIterator<SensorInfo*> it( mSensorList );
  while ( it.hasNext() ) {
    if ( it.next()->name() == name )
      return true;
  }

  return false;
}

bool HostInfo::isRegistered( const Q3ListViewItem *lvi ) const
{
  return ( findInfo( lvi ) != 0 );
}

SensorInfo* HostInfo::findInfo( const Q3ListViewItem *item ) const
{
  QListIterator<SensorInfo*> it( mSensorList );
  while ( it.hasNext() ) {
    SensorInfo *info = it.next();
    if ( info->listViewItem() == item )
      return info;
  }

  return 0;
}

#include "SensorBrowser.moc"
