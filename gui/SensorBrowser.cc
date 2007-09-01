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

#include <QtGui/QDrag>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QMimeData>

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksgrd/SensorManager.h>

#include "SensorBrowser.h"
//#define SENSOR_MODEL_DO_TEST
//uncomment the above to test the model
#ifdef SENSOR_MODEL_DO_TEST
#include "modeltest.h"
#endif

SensorBrowserModel::SensorBrowserModel()
{
#ifdef SENSOR_MODEL_DO_TEST
  new ModelTest(this);
#endif
  mIdCount=1;
}
SensorBrowserModel::~SensorBrowserModel()
{

  qDeleteAll( mHostInfoMap );
  mHostInfoMap.clear();
  qDeleteAll( mSensorInfoMap );
  mSensorInfoMap.clear();
}

int SensorBrowserModel::columnCount( const QModelIndex &) const { //virtual 
  return 1;
}

QVariant SensorBrowserModel::data( const QModelIndex & index, int role) const { //virtual
  if(!index.isValid()) return QVariant();
  switch(role) {
    case Qt::DisplayRole: {
      if(index.column()==0) {
        uint id = index.internalId();
        if(mSensorInfoMap.contains(id)) {
	    Q_ASSERT(mSensorInfoMap.value(id));
	    SensorInfo *sensorInfo = mSensorInfoMap.value(id);
	    return sensorInfo->description() + " (" +sensorInfo->type() +')' ;
        }
        if(mTreeNodeNames.contains(id)) return mTreeNodeNames.value(id);
        if(mHostInfoMap.contains(id)) {
	    Q_ASSERT(mHostInfoMap.value(id));
	    return mHostInfoMap.value(id)->hostName();
        }
      }
      return QString();
    }
    case Qt::DecorationRole: {
      if(index.column() == 0 && mHostInfoMap.contains(index.internalId())) {
        return KIcon("system");
      } else 
        return QIcon(); //work around for drawing bug
      break;
    }
  } //switch
  return QVariant();
}

QVariant SensorBrowserModel::headerData ( int section, Qt::Orientation , int role) const { //virtual
	if(role != Qt::DisplayRole) return QVariant();
	if(section==0) return i18n("Sensor Browser");
	return QVariant();
}

QModelIndex SensorBrowserModel::index ( int row, int column, const QModelIndex & parent) const { //virtual
  if(column != 0) return QModelIndex();
  QList<int> ids;
  if(!parent.isValid()) {
    ids = mHostInfoMap.keys();
  }
  else {
    ids = mTreeMap.value(parent.internalId());
  }
  if( row >= ids.size() || row< 0) {
    return QModelIndex();
  }
  QModelIndex index = createIndex(row, column, ids[row]);
  Q_ASSERT(index.isValid());
  return index;
}

QStringList SensorBrowserModel::listHosts() const
{
  QStringList hostList;

  QMapIterator<int, HostInfo*> it( mHostInfoMap );
  while ( it.hasNext() ) {
    it.next();
    Q_ASSERT(it.value());
    hostList.append( it.value()->hostName() );
  }

  return hostList;
}

QStringList SensorBrowserModel::listSensors( const QString &hostName ) const
{
  QMapIterator<int, HostInfo*> it( mHostInfoMap );
  while ( it.hasNext() ) {
    it.next();
    Q_ASSERT(it.value());
    if ( it.value()->hostName() == hostName ) {
      Q_ASSERT(mSensorInfoMap.contains(it.key()));
      return listSensors( it.key() );
    }
  }
  return QStringList();
}

QStringList SensorBrowserModel::listSensors( int parentId) const
{
  SensorInfo *sensor=mSensorInfoMap.value(parentId);
  if(sensor) return QStringList(sensor->name());

  QStringList childSensors;
  QList<int> children = mTreeMap.value(parentId);
  for(int i=0; i < children.size(); i++) {
    childSensors+= listSensors(children[i]); 
  }
  return childSensors; 
}
SensorInfo *SensorBrowserModel::getSensorInfo(QModelIndex index) const
{
  if(!index.isValid()) return NULL;
  return mSensorInfoMap.value(index.internalId());
}

int SensorBrowserModel::makeSensor(HostInfo *hostInfo, int parentId, const QString &sensorName, const QString &name, const QString &sensorType) {
//sensorName is the full version.  e.g.  mem/free
//name is the short version. e.g. free
//sensortype is e.g. Integer
  QList<int> children = mTreeMap.value(parentId);
  for(int i=0; i<children.size(); i++)
    if(mSensorInfoMap.contains(children[i])) {
      Q_ASSERT(mSensorInfoMap.value(children[i]));
      if(mSensorInfoMap.value(children[i])->name() == sensorName) 
        return children[i];
    }

  QModelIndex parentModelIndex;
  if(hostInfo->id() == parentId) {
    parentModelIndex = createIndex(mHostInfoMap.keys().indexOf(parentId), 0 , parentId);
  } else {
    int parentsParentId = mParentsTreeMap.value(parentId);
    parentModelIndex = createIndex(mTreeMap.value(parentsParentId).indexOf(parentId), 0, parentId);
  }
  Q_ASSERT(parentModelIndex.isValid());
  QList<int> &parentTreemap = mTreeMap[parentId];
  SensorInfo *sensorInfo = new SensorInfo(hostInfo, sensorName, name, sensorType);
  beginInsertRows( parentModelIndex , parentTreemap.size(), parentTreemap.size() );
    parentTreemap << mIdCount;
    mParentsTreeMap.insert( mIdCount, parentId );
    mSensorInfoMap.insert(mIdCount, sensorInfo);
    mHostSensorsMap[hostInfo->id()].insert(sensorName, true);
    mIdCount++;
  endInsertRows();
  return mIdCount-1;  //NOTE mIdCount is next available number. Se we use it, then increment it, but return the number of the one that we use  
}
int SensorBrowserModel::makeTreeBranch(int parentId, const QString &name) {
  QList<int> children = mTreeMap.value(parentId);
  for(int i=0; i<children.size(); i++)
    if(mTreeNodeNames.value(children[i]) == name) return children[i];

  QModelIndex parentModelIndex;
  if(mHostInfoMap.contains(parentId)) {
    parentModelIndex = createIndex(mHostInfoMap.keys().indexOf(parentId), 0 , parentId);
  } else {
    int parentsParentId = mParentsTreeMap.value(parentId);
    parentModelIndex = createIndex(mTreeMap.value(parentsParentId).indexOf(parentId), 0, parentId);
  }
  Q_ASSERT(parentModelIndex.isValid());
  QList<int> &parentTreemap = mTreeMap[parentId];
  beginInsertRows( parentModelIndex , parentTreemap.size(), parentTreemap.size() );
    parentTreemap << mIdCount;
    mParentsTreeMap.insert( mIdCount, parentId );
    mTreeMap[mIdCount];  //create with empty qlist
    mTreeNodeNames.insert(mIdCount, name);
    mIdCount++;
  endInsertRows();
  
  return mIdCount-1;
}

void SensorBrowserModel::answerReceived( int hostId,  const QList<QByteArray>&answer )
{
  /* An answer has the following example format:

     cpu/system/idle integer
     cpu/system/sys  integer
     cpu/system/nice integer
     cpu/system/user integer
     ps       table
  */

  HostInfo *hostInfo = getHostInfo(hostId);
  if(!hostInfo) {
    kDebug(1215) << "SensorBrowserModel::answerReceived with invalid hostId " << hostId ;
    return;
  }  
  for ( int i = 0; i < answer.count(); ++i ) {
    if ( answer[ i ].isEmpty() )
      break;

    QList<QByteArray> words = answer[ i ].split('\t');
    QString sensorName = QString::fromUtf8(words[ 0 ]);
    QString sensorType = QString::fromUtf8(words[ 1 ]);

    if ( hasSensor(hostId, sensorName))
      break;
    if(sensorName.isEmpty()) break;

    if(sensorType == "string") continue;

    /* The sensor browser can display sensors in a hierarchical order.
     * Sensors can be grouped into nodes by seperating the hierarchical
     * nodes through slashes in the sensor name. E. g. cpu/system/user is
     * the sensor user in the cpu node. There is no limit for the
     * depth of nodes. */
    int currentNodeId = hostId;  //Start from the host branch and work our way down the tree
    QStringList absolutePath = sensorName.split( '/' );
    for ( int j = 0; j < absolutePath.count()-1; ++j ) {
      // Localize the sensor name part by part.
      QString name = KSGRD::SensorMgr->translateSensorPath( absolutePath[ j ] );
      currentNodeId = makeTreeBranch(currentNodeId, name);
    }
    QString name = KSGRD::SensorMgr->translateSensorPath( absolutePath[ absolutePath.size()-1] );
    makeSensor(hostInfo, currentNodeId, sensorName, name, sensorType);
  }
  emit sensorsAddedToHost( createIndex( mHostInfoMap.keys().indexOf(hostId), 0, hostId ) )  ;
}

QModelIndex SensorBrowserModel::parent ( const QModelIndex & index ) const { //virtual
  if(!index.isValid() || index.column() != 0)
    return QModelIndex();
  if(mHostInfoMap.contains(index.internalId())) return QModelIndex();
  if(!mParentsTreeMap.contains(index.internalId())) {
	  kDebug(1215) << "Something is wrong with the model.  Doesn't contain " << index.internalId();
	  return QModelIndex();
  }
  int parentId = mParentsTreeMap.value(index.internalId());

  QModelIndex parentModelIndex;
  if(mHostInfoMap.contains(parentId)) {
    parentModelIndex = createIndex(mHostInfoMap.keys().indexOf(parentId), 0 , parentId);
  } else {
    int parentsParentId = mParentsTreeMap.value(parentId);
    parentModelIndex = createIndex(mTreeMap.value(parentsParentId).indexOf(parentId), 0, parentId);
  }
  Q_ASSERT(parentModelIndex.isValid());
  return parentModelIndex;
}
int SensorBrowserModel::rowCount ( const QModelIndex & parent ) const {  //virtual
  if(!parent.isValid()) return mHostInfoMap.size();
  if(parent.column() != 0) return 0;
  return mTreeMap.value(parent.internalId()).size();
}
Qt::ItemFlags SensorBrowserModel::flags ( const QModelIndex & index ) const {  //virtual
  if(!index.isValid()) return 0;
  if(mSensorInfoMap.contains(index.internalId())) return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  else return Qt::ItemIsEnabled;
}

SensorBrowserWidget::SensorBrowserWidget( QWidget* parent, KSGRD::SensorManager* sm )
  : QTreeView( parent ), mSensorManager( sm )
{
  connect( mSensorManager, SIGNAL( update() ), SLOT( update() ) );
  setModel(&mSensorBrowserModel);

  this->setToolTip( i18n( "Drag sensors to empty cells of a worksheet "
                             "or the panel applet." ) );
//  setRootIsDecorated( false );
  setDragDropMode(QAbstractItemView::DragOnly);

  //setMinimumWidth( 1 );

  this->setWhatsThis( i18n( "The sensor browser lists the connected hosts and the sensors "
                               "that they provide. Click and drag sensors into drop zones "
                               "of a worksheet or the panel applet. A display will appear "
                               "that visualizes the "
                               "values provided by the sensor. Some sensor displays can "
                               "display values of multiple sensors. Simply drag other "
                               "sensors on to the display to add more sensors." ) );
  connect( &mSensorBrowserModel, SIGNAL(sensorsAddedToHost(const QModelIndex&)), this, SLOT(expand(const QModelIndex&)));
  update();
}

SensorBrowserWidget::~SensorBrowserWidget()
{
}

void SensorBrowserWidget::disconnect()
{
  QModelIndexList indexlist = selectionModel()->selectedRows();
  for(int i=0; i < indexlist.size(); i++)
  {
    mSensorBrowserModel.disconnectHost(indexlist.value(i).internalId());
  }
}

void SensorBrowserWidget::hostReconfigured( const QString& )
{
  // TODO: not yet implemented.
}

void SensorBrowserModel::clear() {
  qDeleteAll(mHostInfoMap);
  mHostInfoMap.clear();

}

void SensorBrowserModel::disconnectHost(uint id)
{
  disconnectHost(mHostInfoMap.value(id));
}
void SensorBrowserModel::disconnectHost(const HostInfo *hostInfo)
{
  KSGRD::SensorMgr->requestDisengage( hostInfo->sensorAgent() );
}
void SensorBrowserModel::disconnectHost(const QString &hostname)
{
  QMapIterator<int, HostInfo*> it( mHostInfoMap );
  while ( it.hasNext() ) {
    it.next();
    if(it.value()->hostName() == hostname) {
      disconnectHost(it.value());
      return;
    }
  }
}

void SensorBrowserModel::addHost(KSGRD::SensorAgent *sensorAgent, const QString &hostName)
{
  beginInsertRows( QModelIndex() , mHostInfoMap.size(), mHostInfoMap.size() );
    HostInfo* hostInfo = new HostInfo( mIdCount, sensorAgent, hostName);
    mHostInfoMap.insert(mIdCount, hostInfo);
    mTreeMap.insert(mIdCount, QList<int>());
    mHostSensorsMap.insert(mIdCount, QHash<QString, bool>());
    mIdCount++;
  endInsertRows();
  hostInfo->sensorAgent()->sendRequest( "monitors", this, mIdCount-1 );
}

void SensorBrowserWidget::update()
{
  mSensorBrowserModel.clear();

  KSGRD::SensorManagerIterator it( mSensorManager );
  while ( it.hasNext() ) {
    KSGRD::SensorAgent* sensorAgent = it.next().value();
    QString hostName = mSensorManager->hostName( sensorAgent );
    mSensorBrowserModel.addHost(sensorAgent, hostName);
  }
}
QMimeData * SensorBrowserModel::mimeData ( const QModelIndexList & indexes ) const { //virtual 
  QMimeData *mimeData = new QMimeData();
  if(indexes.size() != 1) return mimeData;
  SensorInfo *sensor = getSensorInfo(indexes[0]);
  if(!sensor) return mimeData;
  // Create text drag object as
  // "<hostname> <sensorname> <sensortype> <sensordescription>".
  // Only the description may contain blanks.
  Q_ASSERT(sensor);
  Q_ASSERT(sensor->hostInfo());
  QString mDragText = sensor->hostInfo()->hostName() + ' ' +
                      sensor->name() + ' ' +
                      sensor->type()+ ' ' +
	              sensor->description();


  mimeData->setText( mDragText );
  return mimeData;
}

SensorInfo::SensorInfo( HostInfo *hostInfo, const QString &name,
                        const QString &desc, const QString &type )
  : mName( name ), mDesc( desc ), mType( type ), mHostInfo( hostInfo )
{
  Q_ASSERT(mHostInfo);
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

HostInfo *SensorInfo::hostInfo() const
{
  return mHostInfo;
}



#include "SensorBrowser.moc"
