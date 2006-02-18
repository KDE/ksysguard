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
#include <QBitmap>

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
	mAliases.insert("init", "penguin");
	/* kernel stuff */
	mAliases.insert("bdflush", "kernel");
	mAliases.insert("dhcpcd", "kernel");
	mAliases.insert("kapm-idled", "kernel");
	mAliases.insert("keventd", "kernel");
	mAliases.insert("khubd", "kernel");
	mAliases.insert("klogd", "kernel");
	mAliases.insert("kreclaimd", "kernel");
	mAliases.insert("kreiserfsd", "kernel");
	mAliases.insert("ksoftirqd_CPU0", "kernel");
	mAliases.insert("ksoftirqd_CPU1", "kernel");
	mAliases.insert("ksoftirqd_CPU2", "kernel");
	mAliases.insert("ksoftirqd_CPU3", "kernel");
	mAliases.insert("ksoftirqd_CPU4", "kernel");
	mAliases.insert("ksoftirqd_CPU5", "kernel");
	mAliases.insert("ksoftirqd_CPU6", "kernel");
	mAliases.insert("ksoftirqd_CPU7", "kernel");
	mAliases.insert("kswapd", "kernel");
	mAliases.insert("kupdated", "kernel");
	mAliases.insert("mdrecoveryd", "kernel");
	mAliases.insert("scsi_eh_0", "kernel");
	mAliases.insert("scsi_eh_1", "kernel");
	mAliases.insert("scsi_eh_2", "kernel");
	mAliases.insert("scsi_eh_3", "kernel");
	mAliases.insert("scsi_eh_4", "kernel");
	mAliases.insert("scsi_eh_5", "kernel");
	mAliases.insert("scsi_eh_6", "kernel");
	mAliases.insert("scsi_eh_7", "kernel");
	/* daemon and other service providers */
	mAliases.insert("artsd", "daemon");
	mAliases.insert("atd", "daemon");
	mAliases.insert("automount", "daemon");
	mAliases.insert("cardmgr", "daemon");
	mAliases.insert("cron", "daemon");
	mAliases.insert("cupsd", "daemon");
	mAliases.insert("in.identd", "daemon");
	mAliases.insert("lpd", "daemon");
	mAliases.insert("mingetty", "daemon");
	mAliases.insert("nscd", "daemon");
	mAliases.insert("portmap", "daemon");
	mAliases.insert("rpc.statd", "daemon");
	mAliases.insert("rpciod", "daemon");
	mAliases.insert("sendmail", "daemon");
	mAliases.insert("sshd", "daemon");
	mAliases.insert("syslogd", "daemon");
	mAliases.insert("usbmgr", "daemon");
	mAliases.insert("wwwoffled", "daemon");
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
			if(role == Qt::ToolTipRole) {
				QString tooltip = "<qt>";
				if(!user.fullName().isEmpty()) tooltip += i18n("<b>%1</b><br/>").arg(user.fullName());
				tooltip += i18n("Login Name: %1<br/>").arg(user.loginName());
				if(!user.roomNumber().isEmpty()) tooltip += i18n("Room Number: %1<br/>").arg(user.roomNumber());
				if(!user.workPhone().isEmpty()) tooltip += i18n("Work Phone: %1<br/>").arg(user.workPhone());
				tooltip += i18n("User ID: %1</qt>").arg(uid);
				return tooltip;
			}
			if(role == Qt::DisplayRole)
				return user.loginName();
		}
	}
	if(index.column() == PROCESS_NAME) {
		//The first column - the process name (e.g.  'bash' or 'mozilla')
		if (role == Qt::DisplayRole) {
			return mData.at(index.row()).at(PROCESS_NAME);
		} else if(role == Qt::DecorationRole) {
			QString name = mData.at(index.row()).at(PROCESS_NAME);
			QString iconname = mAliases[name];
			if(iconname.isEmpty()) iconname = name;
			//so iconname tries to guess as what icon to use.
			return getIcon(iconname); //return this pixmap now
		}
		//unsupported role
		return QVariant();
	}
	
	if (role != Qt::DisplayRole)
		return QVariant();
	QString value = mData.at(index.row()).at(index.column());
	QString type = mColType.at(index.column());
	
	if(type == "d") return value.toLongLong();
	if(type == "f") return value.toDouble();
	if(type == "D") return value.toDouble();
	if(type == "S") {
		//This indicates it's a process status
		(void)I18N_NOOP2("process status", "running");
		(void)I18N_NOOP2("process status", "sleeping");
		(void)I18N_NOOP2("process status", "disk sleep");
		(void)I18N_NOOP2("process status", "zombie");
		(void)I18N_NOOP2("process status", "stopped");
		(void)I18N_NOOP2("process status", "paging");
		(void)I18N_NOOP2("process status", "idle");
		
		return i18n("process status", value.latin1());
	}
	
	else return value;



	
}

QPixmap ProcessModel::getIcon(const QString&iconname) const {
		
	/* Get icon from icon list that might be appropriate for a process
	 * with this name. */
	QPixmap pix = mIconCache[iconname];
	if (pix.isNull())
	{
		pix = mIcons.loadIcon(iconname, KIcon::Small,
						 KIcon::SizeSmall, KIcon::DefaultState,
						 0L, true);
		if (pix.isNull() || !pix.mask())
		    pix = mIcons.loadIcon("unknownapp", KIcon::User,
							 KIcon::SizeSmall);

		if (pix.width() != 16 || pix.height() != 16)
		{
			/* I guess this isn't needed too often. The KIconLoader should
			 * scale the pixmaps already appropriately. Since I got a bug
			 * report claiming that it doesn't work with GNOME apps I've
			 * added this safeguard. */
			QImage img;
			img = pix;
			img.smoothScale(16, 16);
			pix = img;
		}
		/* We copy the icon into a 24x16 pixmap to add a 4 pixel margin on
		 * the left and right side. In tree view mode we use the original
		 * icon. */
		QPixmap icon(24, 16);
/*		if (!treeViewEnabled)
		{
			icon.fill();
			bitBlt(&icon, 4, 0, &pix, 0, 0, pix.width(), pix.height());
			QBitmap mask(24, 16, true);
			bitBlt(&mask, 4, 0, &pix.mask(), 0, 0, pix.width(), pix.height());
			icon.setMask(mask);
			pix = icon;
		}*/
		mIconCache.insert(iconname,pix);
	}
	return pix;
}

void ProcessModel::setIsLocalhost(bool isLocalhost)
{
	mIsLocalhost = isLocalhost;
}


