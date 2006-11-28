/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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

#include <QtGui/QPixmap>

#include <klocale.h>

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
      case 0:
        return QString::number(sensor.id()) + " " +  sensor.hostName();
        break;
      case 1:
        return sensor.sensorName();
        break;
      case 2:
        return sensor.unit();
        break;
      case 3:
        return sensor.status();
        break;
      case 4:
        return sensor.label();
        break;
    }
  } else if ( role == Qt::DecorationRole ) {
    if ( index.column() == 1 ) {
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
      case 0:
        return i18n( "Host" );
        break;
      case 1:
        return i18n( "Sensor" );
        break;
      case 2:
        return i18n( "Unit" );
        break;
      case 3:
        return i18n( "Status" );
        break;
      case 4:
        return i18n( "Label" );
        break;
      default:
        return QVariant();
    }
  }

  return QVariant();
}

void SensorModel::setSensors( const SensorModelEntry::List &sensors )
{
  mSensors = sensors;

  emit layoutChanged();
}

SensorModelEntry::List SensorModel::sensors() const
{
  return mSensors;
}

void SensorModel::setSensor( const SensorModelEntry &sensor, const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  if ( index.row() < 0 || index.row() >= mSensors.count() )
    return;

  mSensors[ index.row() ] = sensor;

  emit dataChanged( index, index );
}

void SensorModel::removeSensor( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  if ( index.row() < 0 || index.row() >= mSensors.count() )
    return;

  beginRemoveRows( QModelIndex(), index.row(), index.row());
    mDeleted.append( mSensors[index.row() ].id());
    mSensors.removeAt( index.row() );
  endRemoveRows();
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
  reset();
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



#include "SensorModel.moc"
