/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms version 2 of of the GNU General Public
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



#include <assert.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kuser.h>

#include <ksgrd/SensorManager.h>

#include "ProcessModel.moc"
#include "ProcessModel.h"
#include "SignalIDs.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kpushbutton.h>

ProcessModel::ProcessModel(QObject* parent)
	: QAbstractItemModel(parent)
{
	mIsLocalhost = false; //this really shouldn't matter, because setIsLocalhost should be called before setData()
}

void ProcessModel::setData(const QList<QStringList> &data)
{
	mData = data;
	mPidList.clear();
/*	for(int i = 0; i < mData.size(); i++) {
		int pid = mData.at(i).at(2).toInt();
		mPidList.insert(pid,i);
	}*/
	emit reset();
}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
	if(parent.isValid()) return 0;
	return mData.count();
}

int ProcessModel::columnCount ( const QModelIndex & parent ) const
{
	if(parent.isValid()) return 0;
	return mHeader.count();
}

QModelIndex ProcessModel::index ( int row, int column, const QModelIndex & parent ) const
{
	if(parent.isValid() || !hasIndex(row, column, parent))
		return QModelIndex();
	if(mData.at(row).count() < 2) {
		kDebug() << "Bad data at " << row << "," << column << ". '" << mData.at(row).join(" ") << "'" << endl;
		return QModelIndex();
	}
	return createIndex(row,column, mData.at(row).at(PROCESS_PID).toInt());
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
	return QModelIndex();
}

QVariant ProcessModel::headerData(int section, Qt::Orientation orientation,
		                                         int role) const
{
	(void)I18N_NOOP2("process headings", "Name");
	(void)I18N_NOOP2("process headings", "PID");
	(void)I18N_NOOP2("process headings", "PPID");
	(void)I18N_NOOP2("process headings", "UID");
	(void)I18N_NOOP2("process headings", "GID");
	(void)I18N_NOOP2("process headings", "Status");

	(void)I18N_NOOP2("process headings", "User%");
	(void)I18N_NOOP2("process headings", "System%");
	(void)I18N_NOOP2("process headings", "Nice");
	(void)I18N_NOOP2("process headings", "VmSize");
	(void)I18N_NOOP2("process headings", "VmRss");
	(void)I18N_NOOP2("process headings", "Login");
	(void)I18N_NOOP2("process headings", "Command");

	if (role != Qt::DisplayRole)
		return QVariant(); //error
	if(section < 0 || section >= mHeader.count()) return QVariant(); //error

	return i18n("process headings", mHeader.at(section).utf8()); //translate the header if possible
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (index.parent().isValid())
		return QVariant();
	if (index.row() >= mData.count())
		return QVariant();
	if (index.column() >= mData.at(index.row()).count())
		return QVariant();
	
	if(index.column() == PROCESS_UID) {
		
		long long uid = mData.at(index.row()).at(PROCESS_UID).toLong();
		if(role == Qt::UserRole) return uid; //If we query with this role, then we want the raw UID for this.
		if(!mIsLocalhost) {
			if(role == Qt::DisplayRole) return uid;
			else return QVariant();
		}
		//since we are on localhost, we can give more useful information about the username
		KUser user(uid);
		if(!user.isValid()) {
			if( role == Qt::ToolTipRole)
				return i18n("This user is not recognised for some reason");
			if( role == Qt::DisplayRole)
			       	return uid;
		} else {
			if(role == Qt::ToolTipRole)
				return i18n("<qt>Full name: <i>%1</i></qt>").arg(user.fullName());
			if(role == Qt::DisplayRole)
				return user.loginName();
		}
	}
	
	if (role != Qt::DisplayRole)
		return QVariant();
	QString value = mData.at(index.row()).at(index.column());
	QString type = mColType.at(index.column());
	
	if(type == "d") return value.toLongLong();
	if(type == "f") return value.toDouble();
	if(type == "D") return value.toDouble();
	else return value;
}


void ProcessModel::setIsLocalhost(bool isLocalhost)
{
	mIsLocalhost = isLocalhost;
}


