/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef PROCESSMODEL_H_
#define PROCESSMODEL_H_

#include <QObject>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QHash>

extern KApplication* Kapp;

class ProcessModel : public QAbstractItemModel
{
	Q_OBJECT
		
public:
	ProcessModel(QObject* parent = 0);
	virtual ~ProcessModel() { }

	/* Functions for our Model for QAbstractItemModel*/
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	QModelIndex parent ( const QModelIndex & index ) const;

	/* Functions for setting the model */
	void setHeader(const QStringList &header) {
		beginInsertColumns(QModelIndex(), 0, header.count()-1);
		mHeader = header;
		endInsertColumns();
	}
	void setColType(const QStringList &coltype) {mColType = coltype;}
	void setData(const QList<QStringList> &data);

	class Filter : public QSortFilterProxyModel
	{
	public:
		Filter(QObject *parent=0) : QSortFilterProxyModel(parent) {}
		virtual ~Filter() {}
	protected:
		virtual bool filterAcceptsRow( int source_row, const QModelIndex & source_parent ) {
			return false;
			if(source_row>10) return false;
			return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		}
	};

private:
	QStringList mHeader;
	QStringList mColType;
	QList<QStringList> mData;
	QHash<int,int> mPidList;
};

#endif
