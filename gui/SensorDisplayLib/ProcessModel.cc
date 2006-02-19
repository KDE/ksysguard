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
	//We have our new data, and our current data.
	//First we pull all the information from data so it's in the same format as the existing data.
	//Then we delete all those that have stopped running
	//Then insert all the new ones
	//Then finally update the rest that might have changed
	
	QHash<long long, QStringList> newData;
	QHash<long long, QList<long long> > newPpidToPidMapping;
	QHash<long long, long long> newPidToPpidMapping;

	QSet<long long> new_pids;
	for(long i = 0; i < data.size(); i++) {
		QStringList new_pid_data = data.at(i);
		long long pid = new_pid_data.at(PROCESS_PID).toLongLong();
		long long ppid = new_pid_data.at(PROCESS_PPID).toLongLong();
		new_pids << pid;
		newData[pid] = data[i];
		newPidToPpidMapping[pid] = ppid;
		newPpidToPidMapping[ppid] << pid;
	}

	//so we have a set of new_pids and a set of the current mPids
	QSet<long long> pids_to_delete = mPids;
	pids_to_delete.subtract(new_pids);
	//By deleting, we may delete children that haven't actually died, so do the delete first, then insert which will reinsert the children
	foreach(long long pid, pids_to_delete)
		removeRow(pid);
	QSet<long long> pids_to_insert = new_pids;
	pids_to_insert.subtract(mPids);
	foreach(long long pid, pids_to_insert)	
		insertRow(pid, newData, newPidToPpidMapping);
	//now only changing what is there should be needed.

	Q_ASSERT(mPids.count() == new_pids.count());
	
	//This code will cause a reset if there are any processes reparented or any other wierd cases
	
	bool needreset = false;
	if(data.count() != mData.count()) {
		//A new processes has been added/removed.
		//This still works 1 process was added and another removed, because although it won't be caught here
		//it will be caught in the next bit when it finds a new process.
		kDebug() << "mData and data contain different number of processes!!" << endl;
		needreset = true;
	}
	for(long i = 0; i < data.size() && !needreset; i++) {	
		//Note that init always exists, and has pid 1.  It's ppid is 0. No process will have pid 0
		QStringList new_pid_data = data.at(i);
		long long pid = new_pid_data.at(PROCESS_PID).toLongLong();
		long long ppid = new_pid_data.at(PROCESS_PPID).toLongLong();
		QStringList &current_pid_data = mData[pid];
	        if(current_pid_data.isEmpty()) {
			//new item.  something has gone wrong.  reset back to sane value.  This shouldn't ever happen
			needreset = true;
			break;
		} else {
			changeProcess(pid, new_pid_data, current_pid_data);
			long long old_ppid = mPidToPpidMapping[pid];
			if(old_ppid != ppid) {
				//reparented? reset!
				needreset = true;
				break;
			}
			if(!mPpidToPidMapping[ppid].contains(pid)) {
				//we already existed, but the parent didn't know about us?  probably a reparent or something.
				//reset!
				needreset = true;
				break;
			}
		}
	}
	if(needreset) {
		//fixme - save selection
		kDebug() << "HAD TO RESET!" << endl;
		mPidToPpidMapping = newPidToPpidMapping;
		mPpidToPidMapping = newPpidToPidMapping;
		mData = newData;
		mPids = new_pids;
		reset();
		emit layoutChanged();
	}
}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
	if(parent.isValid()) {
		return mPpidToPidMapping[parent.internalId()/*pid*/].count();
	} else {
	//	Q_ASSERT(mPpidToPidMapping[0].count() <= 1);  Just incase something goes wrong, we want to cope gracefully and not crash, so commented out
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


void ProcessModel::changeProcess(const long long &pid, const QStringList &new_pid_data, QStringList &current_pid_data) 
{
	Q_ASSERT(pid != 0);
	Q_ASSERT(new_pid_data.count() == current_pid_data.count());
	int first_column_changed = -1;
	int last_column_changed=-1;
	for(int i = 0; i<current_pid_data.count(); i++) {
		if(current_pid_data[i] != new_pid_data[i]) {
			current_pid_data[i] = new_pid_data[i]; //copy across now
			if(first_column_changed == -1) first_column_changed = i;
			last_column_changed = i;
		}
	}
	//Now all the data has been changed for this process.
	if(last_column_changed == -1)
		return; 

	//Tell the view we changed something
	long long ppid = mPidToPpidMapping[ pid ];
	int row = mPpidToPidMapping[ppid].indexOf(pid);
	if(row == -1) //something has gone really wrong
		return;
	QModelIndex startIndex = createIndex(row, first_column_changed, pid);
	QModelIndex endIndex = createIndex(row, last_column_changed, pid);
	emit dataChanged(startIndex, endIndex);
}

void ProcessModel::insertRow( const long long &pid, const QHash<long long, QStringList> &newData, const QHash<long long, long long> &newPidToPpidMapping)
{
	if(pid <= 0) return; //init has parent pid 0
	if(mPids.contains(pid)) return; //we may have already inserted.  any modification of the data will be done later
	long long ppid = newPidToPpidMapping[pid];
	insertRow(ppid, newData, newPidToPpidMapping);
	int row = 0;
	QModelIndex parentModel;
	if(ppid != 0) {
		row = mPpidToPidMapping[ppid].count();
		parentModel = getQModelIndex(ppid, 0);
	}	
	beginInsertRows(parentModel, row, row);
	mData[pid] << newData[pid];
	mPpidToPidMapping[ppid] << pid;
	mPidToPpidMapping[pid] = ppid;
	mPids << pid;
	endInsertRows();
}
void ProcessModel::removeRow( const long long &pid ) 
{
	if(!mPidToPpidMapping.contains(pid)) return;
	long long ppid = mPidToPpidMapping[ pid ];
	int row = mPpidToPidMapping[ppid].indexOf(pid);
	if(row == -1) return;
	QModelIndex parentModel = getQModelIndex(ppid, 0);
	QList<long long> children = mPpidToPidMapping[pid];
	//we should remove all the children first, recursively.
	for(int i =0; i < children.size(); i++) removeRow(children[i]);
	//so no more children left!
	beginRemoveRows(parentModel, row, row);
	mPpidToPidMapping[ppid].removeAll(pid);
	mPidToPpidMapping.remove(pid);
	mPids.remove(pid);
	endRemoveRows();
}

QModelIndex ProcessModel::getQModelIndex ( const long long &pid, int column) const
{
	if(pid == 0) return QModelIndex();
	long long ppid = mPidToPpidMapping[ pid ];
	int row = mPpidToPidMapping[ppid].indexOf(pid);
	if(row == -1) //something has gone really wrong
		return QModelIndex();
	return createIndex(row, 0, pid);
	
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
	if(!index.isValid()) return QModelIndex();
	if(index.internalId() == 0) return QModelIndex();
	long long parent_pid = mPidToPpidMapping[ index.internalId()/*pid*/ ];
	return getQModelIndex(parent_pid,0);
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

QString ProcessModel::getTooltipForUser(const long long &localhost_uid) const {
	QString &userTooltip = mUserTooltips[localhost_uid];
	if(userTooltip.isEmpty()) {
		KUser user(localhost_uid);
		if(!user.isValid())
			userTooltip = i18n("This user is not recognised for some reason");
		else
			userTooltip = "<qt>";
			if(!user.fullName().isEmpty()) userTooltip += i18n("<b>%1</b><br/>").arg(user.fullName());
			userTooltip += i18n("Login Name: %1<br/>").arg(user.loginName());
			if(!user.roomNumber().isEmpty()) userTooltip += i18n("Room Number: %1<br/>").arg(user.roomNumber());
			if(!user.workPhone().isEmpty()) userTooltip += i18n("Work Phone: %1<br/>").arg(user.workPhone());
			userTooltip += i18n("User ID: %1</qt>").arg(localhost_uid);
	}
	return userTooltip;
}

QVariant ProcessModel::getUsernameForUser(const long long &localhost_uid) const {
	QVariant &username = mUserUsername[localhost_uid];
	if(!username.isValid()) {
		KUser user(localhost_uid);
		if(!user.isValid())
			username = localhost_uid;
		username = user.loginName();
	}
	return username;
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
	//This function must be super duper ultra fast because it's called thousands of times every few second :(
	//I think it should be optomised for role first, hence the switch statement (fastest possible case)
	//but at the cost of duplicate code.  To avoid the typing, I use a #define.  Please forgive me.
#define PROCESS_MODEL_DATA_INITIALISATION \
	long long pid = index.internalId();\
	const QStringList &process_data = mData[pid];\
	if (index.column() >= process_data.count()) \
	     return QVariant();
		     
	if (!index.isValid())
		return QVariant();
	switch (role){
	case Qt::DisplayRole: {
		PROCESS_MODEL_DATA_INITIALISATION
		switch(index.column()) {
		case PROCESS_UID: {
			long long uid = process_data.at(PROCESS_UID).toLongLong();
			if(!mIsLocalhost)
				return uid;
			//since we are on localhost, we can give more useful information about the username
			return getUsernameForUser(uid);
		}
		case PROCESS_NAME:
			return process_data.at(PROCESS_NAME);
		default:
			QString value = process_data.at(index.column());
			switch(mColType.at(index.column()))
			{
				case 'd': return value.toLongLong();
				case 'D': 
				case 'f': return value.toDouble();
				case 'S': {
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
				default:
					  return value;
			}
				
			
		}
		break;
	}
	case Qt::ToolTipRole: {
		if(!mIsLocalhost) return QVariant();//you might have to move this into the switch if you add more tooltips
		PROCESS_MODEL_DATA_INITIALISATION
		switch(index.column()) {
		case PROCESS_UID: {
			long long uid = process_data.at(PROCESS_UID).toLongLong();
			//since we are on localhost, we can give more useful information about the username
			return getTooltipForUser(uid);	
		}
		default:
			return QVariant();
		}
	}
	case Qt::UserRole: {
		if(index.column() != PROCESS_UID) return QVariant();  //If we query with this role, then we want the raw UID for this.
		PROCESS_MODEL_DATA_INITIALISATION
		return process_data.at(PROCESS_UID).toLongLong();
	}
	case Qt::DecorationRole: {
		if(index.column() != PROCESS_NAME) return QVariant(); //you might have to change this into a switch if you add more decorations
		PROCESS_MODEL_DATA_INITIALISATION
		QString name = process_data.at(PROCESS_NAME);
		QString iconname = mAliases[name];
		if(iconname.isEmpty()) iconname = name;
		//so iconname tries to guess as what icon to use.
		return getIcon(iconname); //return this pixmap now
	}
	default: //This is a very very common case, so the route to this must be very minimal
		return QVariant();
	}

	return QVariant(); //never get here, but make compilier happy
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


