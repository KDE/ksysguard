/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QPixmap>

#include <KLocalizedString>

#include "SensorModel.h"

void SensorModelEntry::setId( int id )
{
  mId = id;
}

int SensorModelEntry::id() const
{
  return mId;
}

void SensorModelEntry::setHostName( const QString &hostName )
{
  mHostName = hostName;
}

QString SensorModelEntry::hostName() const
{
  return mHostName;
}

void SensorModelEntry::setSensorName( const QString &sensorName )
{
  mSensorName = sensorName;
}

QString SensorModelEntry::sensorName() const
{
  return mSensorName;
}

void SensorModelEntry::setLabel( const QString &label )
{
  mLabel = label;
}

QString SensorModelEntry::label() const
{
  return mLabel;
}

void SensorModelEntry::setUnit( const QString &unit )
{
  mUnit = unit;
}

QString SensorModelEntry::unit() const
{
  return mUnit;
}

void SensorModelEntry::setStatus( const QString &status )
{
  mStatus = status;
}

QString SensorModelEntry::status() const
{
  return mStatus;
}

void SensorModelEntry::setColor( const QColor &color )
{
  mColor = color;
}

QColor SensorModelEntry::color() const
{
  return mColor;
}

SensorModel::SensorModel( QObject *parent )
  : QAbstractTableModel( parent ), mHasLabel( false )
{
}

int SensorModel::columnCount( const QModelIndex& ) const
{
  if ( mHasLabel )
    return 5;
  else
    return 4;
}

int SensorModel::rowCount( const QModelIndex& ) const
{
  return mSensors.count();
}

QVariant SensorModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() >= mSensors.count() || index.row() < 0 )
    return QVariant();

  SensorModelEntry sensor = mSensors[ index.row() ];

  if ( role == Qt::DisplayRole ) {
    switch ( index.column() ) {
      case HostName:
        return sensor.hostName();
      case SensorName:
        return sensor.sensorName();
      case Unit:
        return sensor.unit();
      case Status:
        return sensor.status();
      case Label:
        return sensor.label();
    }
  } else if ( role == Qt::DecorationRole ) {
    if ( index.column() == SensorName ) {
      if ( sensor.color().isValid() ) {
        QPixmap pm( 12, 12 );
        pm.fill( sensor.color() );

        return pm;
      }
    }
  }

  return QVariant();
}

QVariant SensorModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Vertical )
    return QVariant();

  if ( role == Qt::DisplayRole ) {
    switch ( section ) {
      case HostName:
        return i18n( "Host" );
      case SensorName:
        return i18n( "Sensor" );
      case Unit:
        return i18n( "Unit" );
      case Status:
        return i18n( "Status" );
      case Label:
        return i18n( "Label" );
      default:
        return QVariant();
    }
  }

  return QVariant();
}

void SensorModel::setSensors( const SensorModelEntry::List &sensors )
{
  mSensors = sensors;

  Q_EMIT layoutChanged();
}

SensorModelEntry::List SensorModel::sensors() const
{
  return mSensors;
}

void SensorModel::setSensor( const SensorModelEntry &sensor, const QModelIndex &sindex )
{
  if ( !sindex.isValid() )
    return;

  int row = sindex.row();
  if ( row < 0 || row >= mSensors.count() )
    return;

  mSensors[row] = sensor;

  Q_EMIT dataChanged( index(row,0), index(row, columnCount()-1));
}

void SensorModel::removeSensor( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  if ( index.row() < 0 || index.row() >= mSensors.count() )
    return;

  beginRemoveRows( QModelIndex(), index.row(), index.row());
    int id = mSensors[index.row() ].id();
    mDeleted.append(id);

    mSensors.removeAt( index.row() );
    for(int i = 0; i < mSensors.count(); i++) {
      if(mSensors[i].id() > id) 
        mSensors[i].setId(mSensors[i].id()-1);
    }
  endRemoveRows();

}

void SensorModel::moveDownSensor(const QModelIndex &sindex)
{
  int row = sindex.row();
  if(row >= mSensors.count()) return;
  mSensors.move(row, row+1);
  
  for( int i = 0; i < columnCount(); i++)
    changePersistentIndex(index(row, i), index(row+1, i));
 
  Q_EMIT dataChanged(sindex, index(row+1, columnCount()-1));
}
void SensorModel::moveUpSensor(const QModelIndex &sindex)
{
  int row = sindex.row();
  if(row <= 0) return;
  mSensors.move(row, row-1);
  for( int i = 0; i < columnCount(); i++)
    changePersistentIndex(index(row, i), index(row-1, i));
  Q_EMIT dataChanged(sindex, index(row-1, columnCount()-1));
}
QList<int> SensorModel::deleted() const
{
  return mDeleted;
}

void SensorModel::clearDeleted()
{
  mDeleted.clear();
}
QList<int> SensorModel::order() const
{
  QList<int> newOrder;
  for(int i = 0; i < mSensors.count(); i++)
  {
    newOrder.append(mSensors[i].id());
  }
  return newOrder;

}
void SensorModel::resetOrder() {
  //Renumber the items 3, 2, 1, 0  etc
  for(int i = 0; i < mSensors.count(); i++)
  {
    mSensors[i].setId(i);
  }
  beginResetModel();
  endResetModel();
}

SensorModelEntry SensorModel::sensor( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.row() >= mSensors.count() || index.row() < 0 )
    return SensorModelEntry();

  return mSensors[ index.row() ];
}

void SensorModel::setHasLabel( bool hasLabel )
{
  mHasLabel = hasLabel;
}




