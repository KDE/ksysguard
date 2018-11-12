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

#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QColor>

class SensorModelEntry
{
  public:
    typedef QList<SensorModelEntry> List;

    void setId( int id );
    int id() const;

    void setHostName( const QString &hostName );
    QString hostName() const;

    void setSensorName( const QString &sensorName );
    QString sensorName() const;

    void setLabel( const QString &label );
    QString label() const;

    void setUnit( const QString &unit );
    QString unit() const;

    void setStatus( const QString &status );
    QString status() const;

    void setColor( const QColor &color );
    QColor color() const;

  private:
    int mId;
    QString mHostName;
    QString mSensorName;
    QString mLabel;
    QString mUnit;
    QString mStatus;
    QColor mColor;
};

class SensorModel : public QAbstractTableModel
{
  Q_OBJECT
  public:
    explicit SensorModel( QObject *parent = nullptr );

    void setSensors( const SensorModelEntry::List &sensors );
    SensorModelEntry::List sensors() const;

    void setSensor( const SensorModelEntry &sensor, const QModelIndex &index );
    void removeSensor( const QModelIndex &index );
    SensorModelEntry sensor( const QModelIndex &index ) const;

    void moveDownSensor(const QModelIndex &index);
    void moveUpSensor(const QModelIndex &index);
    void setHasLabel( bool hasLabel );

    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QList<int> order() const;
    QList<int> deleted() const;
    void clearDeleted();
    void resetOrder();

  private:
    SensorModelEntry::List mSensors;

    bool mHasLabel;
    /** The numbers of the sensors to be deleted.*/
    QList<int> mDeleted;
};

#endif
