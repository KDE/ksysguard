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
//	mData.clear();
//	mPidToPpidMapping.clear();
//	mPpidToPidMapping.clear();
	bool needreset = false;
	for(long i = 0; i < data.size(); i++) {
		//Note that init always exists, and has pid 1.  It's ppid is 0. No process will have pid 0
		QStringList pid_data = data.at(i);
		long long pid = pid_data.at(PROCESS_PID).toLongLong();
		long long ppid = pid_data.at(PROCESS_PPID).toLongLong();
		QStringList &old_pid_data = mData[pid];
	        if(old_pid_data.isEmpty()) {
			needreset = true;
			old_pid_data = pid_data;
		}
		long long &old_ppid = mPidToPpidMapping[pid];
		if(old_ppid != ppid) {
			needreset = true;
			old_ppid = ppid;
		}
		if(!mPpidToPidMapping[ppid].contains(pid))
		{
			mPpidToPidMapping[ppid] << pid;
		}
	}
	if(needreset) {
		emit layoutChanged();
		reset();
	}
}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
	if(parent.isValid()) {
		return mPpidToPidMapping[parent.internalId()/*pid*/].count();
	} else {
	//	Q_ASSERT(mPpidToPidMapping[0].count() == 1);
		return mPpidToPidMapping[0].count();
	}
}

int ProcessModel::columnCount ( const QModelIndex & parent ) const
{
	if(parent.isValid()) return 0;
	return mHeader.count();
}

bool ProcessModel::hasChildren ( const QModelIndex & parent = QModelIndex() ) const
{
	return rowCount(parent) > 0;
}

QModelIndex ProcessModel::index ( int row, int column, const QModelIndex & parent ) const
{	
	if(row<0) return QModelIndex();
	long long ppid = 0;
	if(parent.isValid()) //not valid for init, and init has ppid of 0
	{
		ppid = parent.internalId()/*pid*/;
	}
	const QList<long long> &siblings = mPpidToPidMapping[ppid];
	if(siblings.count() > row)
		return createIndex(row,column, siblings[row]);
	else
		return QModelIndex();
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
	if(!index.isValid()) return QModelIndex();
	if(index.internalId() == 0) return QModelIndex();
	long long parent_pid = mPidToPpidMapping[ index.internalId()/*pid*/ ];
	if(parent_pid == 0) return QModelIndex();
	long long parent_ppid = mPidToPpidMapping[ parent_pid ];
	
	//note init will have ppid 0 which won't exist
	long row = mPpidToPidMapping[parent_ppid].indexOf(parent_pid); //Our row is where we are in the list of siblings with the same parent
	
	if(row == -1) // not found.  probably because ppid == 0 so we are the init process, but might be due to some wierd error
		return QModelIndex();
	return createIndex(row,0,parent_pid);	
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
	long long pid = index.internalId();
	const QStringList &process_data = mData[pid];
	if (index.column() >= process_data.count())
		return QVariant();
	
	if(index.column() == PROCESS_UID) {
		
		long long uid = process_data.at(PROCESS_UID).toLong();
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
			return process_data.at(PROCESS_NAME);
		} else if(role == Qt::DecorationRole) {
			QString name = process_data.at(PROCESS_NAME);
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
	QString value = process_data.at(index.column());
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


