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

#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>
#include <QtGui/QColor>

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
  public:
    SensorModel( QObject *parent = 0 );

    void setSensors( const SensorModelEntry::List &sensors );
    SensorModelEntry::List sensors() const;

    void setSensor( const SensorModelEntry &sensor, const QModelIndex &index );
    void removeSensor( const QModelIndex &index );
    SensorModelEntry sensor( const QModelIndex &index ) const;

    void setHasLabel( bool hasLabel );

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
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
