/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 John Tapsell <john.tapsell@kdemail.net>

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

*/

#ifndef PROCESSMODEL_H_
#define PROCESSMODEL_H_

#include <kapplication.h>
#include <kiconloader.h>
#include <QPixmap>
#include <QObject>
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QHash>

#define PROCESS_NAME 0
#define PROCESS_PID 1
#define PROCESS_PPID 2
#define PROCESS_UID 3


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
	
	bool hasChildren ( const QModelIndex & parent) const;
	/* Functions for setting the model */
	void setHeader(const QStringList &header) {
		beginInsertColumns(QModelIndex(), 0, header.count()-1);
		mHeader = header;
		endInsertColumns();
	}
	void setColType(const QStringList &coltype) {mColType = coltype;}
	void setData(const QList<QStringList> &data);
	/** By telling the model whether the sensor it is recieving the data from is for the localhost,
	 *  then we can provide more information to the user.  For example, we can show the actual username
	 *  rather than just the userid.
	 */
	void setIsLocalhost(bool isLocalhost);

	/** The iconname is the name of the process or an aliases for it (like 'daemon' etc)
	 *  @return A QPixmap, that may or may not be null.
	 */
	QPixmap getIcon(const QString& iconname) const;
private:
	QHash<QString,QString> mAliases;
	QStringList mHeader;
	QStringList mColType;
	QHash<long long, QStringList> mData;
	
	/** For a given process id, it returns all the children that have this pid,
	 *  sorted in order of pid.
	 */
	QHash<long long, QList<long long> > mPpidToPidMapping;
	/** For a given process id, it returns the parent process id.
	 */
	QHash<long long, long long> mPidToPpidMapping;
	
	/** Cache for the icons to show next to the process.
	 *  Name is the process name, or an alias (like 'daemon' etc).
	 *  @see getIcon(const QString& iconname)
	 */
	mutable QHash<QString,QPixmap> mIconCache;
	
	KIconLoader mIcons;

	/** @see setIsLocalhost */
	bool mIsLocalhost;
};

#endif

