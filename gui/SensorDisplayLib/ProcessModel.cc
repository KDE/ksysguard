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

#define GET_OWN_ID

#ifdef GET_OWN_ID
/* For getuid*/
#include <unistd.h>
#include <sys/types.h>
#endif

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
	mIsLocalhost = false; //this default really shouldn't matter, because setIsLocalhost should be called before setData()
	mPidToProcess[0] = Process();  //Add a fake process for process '0', the parent for init.  This lets us remove checks everywhere for init process

	//Translatable strings for the status
	(void)I18N_NOOP2("process status", "running"); 
	(void)I18N_NOOP2("process status", "sleeping");
	(void)I18N_NOOP2("process status", "disk sleep");
	(void)I18N_NOOP2("process status", "zombie");
	(void)I18N_NOOP2("process status", "stopped");
	(void)I18N_NOOP2("process status", "paging");
	(void)I18N_NOOP2("process status", "idle");

	//Translatable strings for the column headings
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

	mProcessType.insert("init", Process::Init);
	/* kernel stuff */
	mProcessType.insert("bdflush", Process::Kernel);
	mProcessType.insert("dhcpcd", Process::Kernel);
	mProcessType.insert("kapm-idled", Process::Kernel);
	mProcessType.insert("keventd", Process::Kernel);
	mProcessType.insert("khubd", Process::Kernel);
	mProcessType.insert("klogd", Process::Kernel);
	mProcessType.insert("kreclaimd", Process::Kernel);
	mProcessType.insert("kreiserfsd", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU0", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU1", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU2", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU3", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU4", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU5", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU6", Process::Kernel);
	mProcessType.insert("ksoftirqd_CPU7", Process::Kernel);
	mProcessType.insert("kswapd", Process::Kernel);
	mProcessType.insert("kupdated", Process::Kernel);
	mProcessType.insert("mdrecoveryd", Process::Kernel);
	mProcessType.insert("scsi_eh_0", Process::Kernel);
	mProcessType.insert("scsi_eh_1", Process::Kernel);
	mProcessType.insert("scsi_eh_2", Process::Kernel);
	mProcessType.insert("scsi_eh_3", Process::Kernel);
	mProcessType.insert("scsi_eh_4", Process::Kernel);
	mProcessType.insert("scsi_eh_5", Process::Kernel);
	mProcessType.insert("scsi_eh_6", Process::Kernel);
	mProcessType.insert("scsi_eh_7", Process::Kernel);
	/* daemon and other service providers */
	mProcessType.insert("artsd", Process::Daemon);
	mProcessType.insert("atd", Process::Daemon);
	mProcessType.insert("automount", Process::Daemon);
	mProcessType.insert("cardmgr", Process::Daemon);
	mProcessType.insert("cron", Process::Daemon);
	mProcessType.insert("cupsd", Process::Daemon);
	mProcessType.insert("in.identd", Process::Daemon);
	mProcessType.insert("lpd", Process::Daemon);
	mProcessType.insert("mingetty", Process::Daemon);
	mProcessType.insert("nscd", Process::Daemon);
	mProcessType.insert("portmap", Process::Daemon);
	mProcessType.insert("rpc.statd", Process::Daemon);
	mProcessType.insert("rpciod", Process::Daemon);
	mProcessType.insert("sendmail", Process::Daemon);
	mProcessType.insert("sshd", Process::Daemon);
	mProcessType.insert("syslogd", Process::Daemon);
	mProcessType.insert("usbmgr", Process::Daemon);
	mProcessType.insert("wwwoffled", Process::Daemon);
}
void ProcessModel::setData(const QList<QStringList> &data)
{
	// We can set this from anywhere to basically say something has gone wrong, and the code is buggy, so reset and get all the data again
	// It shouldn't happen, but just-in-case
	mNeedReset = false;
	
	//We have our new data, and our current data.
	//First we pull all the information from data so it's in the same format as the existing data.
	//Then we delete all those that have stopped running
	//Then insert all the new ones
	//Then finally update the rest that might have changed
	
	for(long i = 0; i < data.size(); i++) {
		QStringList new_pid_data = data.at(i);
		long long pid = new_pid_data.at(PROCESS_PID).toLongLong();
		long long ppid = new_pid_data.at(PROCESS_PPID).toLongLong();
		new_pids << pid;
		newData[pid] = data[i];
		newPidToPpidMapping[pid] = ppid;
	}

	//so we have a set of new_pids and a set of the current mPids
	QSet<long long> pids_to_delete = mPids;
	pids_to_delete.subtract(new_pids);
	//By deleting, we may delete children that haven't actually died, so do the delete first, then insert which will reinsert the children
	foreach(long long pid, pids_to_delete)
		removeRow(pid);

	QSet<long long>::const_iterator i = new_pids.begin();
	while( i != new_pids.end()) {
		insertOrChangeRows(*i);
		//this will be automatically removed, so just set i back to the start of whatever is left
		i = new_pids.begin();
	}

	//now only changing what is there should be needed.
	if(mPids.count() != data.count()) {
		kDebug() << "After merging the new process data, an internal discrancy was found. Fail safe reseting view." << endl;
		kDebug() << "We were told there were " << data.count() << " processes, but after merging we know about " << mPids.count() << endl;
		mNeedReset = true;
	}
	if(mNeedReset) {
		//This shouldn't happen, but good to have a fall back incase it does :)
		//fixme - save selection
		kDebug() << "HAD TO RESET!" << endl;
		mPidToProcess.clear();
		mPids.clear();
		reset();
		mNeedReset = false;
		emit layoutChanged();
	}
}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
	long long ppid = (parent.isValid())?parent.internalId():0; //when parent is invalid, it must be the root level which we set as 0
	return mPidToProcess[ppid].children_pids.count();
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
		ppid = parent.internalId()/*pid*/;
	
        // Using QList<long long>& instead of QList<long long> creates an internal error in gcc 3.3.1
	const QList<long long> siblings = mPidToProcess[ppid].children_pids;
	if(siblings.count() > row)
		return createIndex(row,column, siblings[row]);
	else
		return QModelIndex();
}


//only to be called if the (new) parent will not change under us!
void ProcessModel::changeProcess(long long pid)
{
	Q_ASSERT(pid != 0);
	long long ppid = mPidToProcess[pid].parent_pid;
	long long new_ppid = newPidToPpidMapping[pid];
	//This is called from insertOrChangeRows and after the parent is checked, so we know the (new) parent won't change under us
	
	if(new_ppid != ppid) {
		//Process has reparented.  Delete and reinsert
		//again, we know that the new parent won't change under us
		removeRow(pid);
		//removes this process, but also all its children.
		//But this is okay because we always parse the parents before the children
		//so the children will just get readded when parsed.

		//However we may have been called from insertOrChange when inserting the first child who wants its parent
		//We can't return with still no parent, so we have to insert the parent
		insertOrChangeRows(pid); //This will also remove the pid from new_pids so we don't need to do it here
		//now we can return
		return;
	}
	new_pids.remove(pid); //we will now deal with this pid for certain, so remove it from the list

	//We know the pid is the same (obviously), and ppid.  So check name (and update processType?),uid and data.  children_pids will take care
	//of themselves as they are inserted later.

	Process &process = mPidToProcess[pid];

	int first_column_changed = -1;
	int last_column_changed=-1;
	const QStringList &newDataRow = newData[pid];
	QString name = newDataRow[PROCESS_NAME];
	long long uid = newDataRow[PROCESS_UID].toLongLong();

	if(process.name != name) {
		process.name = name;
		process.processType = (mProcessType.contains(name))?mProcessType[name]:Process::Other;
		last_column_changed = first_column_changed == PROCESS_NAME;	
	}
	if(process.uid != uid) {
		process.uid = uid;
		if(first_column_changed == -1)
			first_column_changed = PROCESS_UID;
		last_column_changed = PROCESS_UID;
	}
	

	for (int i = PROCESS_DATASTART; i < newDataRow.count(); i++)
	{  //At the moment the data is just a string, so turn it into a list of variants instead and insert the variants into our new process
		QVariant value;
		switch(mColType[i]) {
			case 'd': value = newDataRow[i].toLongLong(); break;
			case 'D': 
			case 'f': value = newDataRow[i].toDouble(); break;
			case 'S': value = i18n("process status", newDataRow[i].latin1()); break;  //This indicates it's a process status.  See the I18N_NOOP2 strings in the constructor
			default:
				value = newDataRow[i]; break;
		}
		if(value != process.data[i-PROCESS_DATASTART]) {
			process.data[i-PROCESS_DATASTART] = value;
			if(first_column_changed == -1)
				first_column_changed = i;
			last_column_changed = i;
		}
	}

	//Now all the data has been changed for this process.
	if(last_column_changed == -1)
		return; 

	int row = mPidToProcess[ process.parent_pid ].children_pids.indexOf(process.pid);
	if(row == -1) //something has gone really wrong
		return;
	QModelIndex startIndex = createIndex(row, first_column_changed, pid);
	QModelIndex endIndex = createIndex(row, last_column_changed, pid);
	emit dataChanged(startIndex, endIndex);
}

void ProcessModel::insertOrChangeRows( long long pid)
{
	if(!new_pids.contains(pid)) {
		kDebug() << "Internal problem with data structure.  A loop perhaps?" << endl;
		mNeedReset = true;
		return;
	}
	Q_ASSERT(pid != 0);

	long long ppid = newPidToPpidMapping[pid];

	if(ppid != 0 && new_pids.contains(ppid))  //If we haven't inserted/changed the parent yet, do that first!
		insertOrChangeRows(ppid);   //by the nature of recursion, we know that _this_ parent will have its parents checked and so on
	//so now all parents are safe
	if(mPidToProcess.contains(pid)) {
//		kDebug() << "Changing " << pid << endl;
		changeProcess(pid);  //we are changing, no need for insert
		return;
	}

	new_pids.remove(pid); //we will deal with this pid for certain, so remove it from the list
//	kDebug() << "Inserting " << pid << endl;
	//We are inserting a new process
	
	//This process may have children, however we are now guaranteed that:
	// a) If the children are new, then they will be inserted after the parent because in this function we recursively check the parent(s) first.
	// b) If the children already exist (a bit wierd, but possible if a new process is created, then an existing one is reparented to it)
	//    then in changed() it will call this function to recursively insert its parents
	

	QList<long long> &parents_children = mPidToProcess[ppid].children_pids;
	int row = parents_children.count();
	QModelIndex parentModel = getQModelIndex(ppid, 0);

	const QStringList &newDataRow = newData[pid];
	QString name = newDataRow[PROCESS_NAME];
	long long uid = newDataRow[PROCESS_UID].toLongLong();
	Process::ProcessType processType = (mProcessType.contains(name))?mProcessType[name]:Process::Other;

	Process new_process(name, pid, ppid, uid, processType);
	QList<QVariant> &data = new_process.data;
	
	for (int i = PROCESS_DATASTART; i < newDataRow.count(); i++)
	{  //At the moment the data is just a string, so turn it into a list of variants instead and insert the variants into our new process
		switch(mColType[i]) {
			case 'd': data << newDataRow[i].toLongLong(); break;
			case 'D': 
			case 'f': data << newDataRow[i].toDouble(); break;
			case 'S': data << i18n("process status", newDataRow[i].latin1()); break;  //This indicates it's a process status.  See the I18N_NOOP2 strings in the constructor
			default:
				data << newDataRow[i]; break;
		}
	}
	//Only here can we actually change the model.  First notify the view/proxy models then modify
	
//	kDebug() << "inserting " << pid << "(" << new_process.pid << "] at "<< row << " in parent " << new_process.parent_pid <<endl;
	beginInsertRows(parentModel, row, row);
		mPidToProcess[new_process.pid] = new_process;
		parents_children << new_process.pid;  //add ourselves to the parent
		mPids << new_process.pid;
	endInsertRows();
}
void ProcessModel::removeRow( long long pid ) 
{
//	kDebug() << "Removing " << pid << endl;
	if(pid <= 0) return; //init has parent pid 0
	if(!mPidToProcess.contains(pid)) {
		kDebug() << "Trying to remove a process (" << pid << ") that already has been removed." << endl;
//		mNeedReset = true;
		return; //we may have already deleted for some reason?
	}

	Process *process = &mPidToProcess[pid];	

	{
		QList<long long> *children = &(process->children_pids); //remove all the children now
		foreach(long long child_pid, *children) {
			if(child_pid == pid) {
				kDebug() << "A process is its own child? Something has gone wrong.  Reseting model" << endl;
				mNeedReset = true;
				return;
			}
			removeRow(child_pid);
		}
		children = NULL;
	}

	
	long long ppid = process->parent_pid;
	QList<long long> &parents_children = mPidToProcess[ppid].children_pids;
	int row = parents_children.indexOf(pid);
	QModelIndex parentModel = getQModelIndex(ppid, 0);
	if(row == -1) return;
	else {
	}
	process = NULL;  //okay, now we aren't pointing to Process or any of its structures at all now. Should be safe to remove now.

	//so no more children left, we are free to delete now
	beginRemoveRows(parentModel, row, row);
		mPidToProcess.remove(pid);
		parents_children.remove(pid);  //remove ourselves from the parent
		mPids.remove(pid);
	endRemoveRows();
}

QModelIndex ProcessModel::getQModelIndex ( long long pid, int column) const
{
	if(pid == 0) return QModelIndex();
	long long ppid = mPidToProcess[pid].parent_pid;
	int row = 0;
	if(ppid != 0) {
		row = mPidToProcess[ppid].children_pids.indexOf(pid);
		if(row == -1) //something has gone really wrong
			return QModelIndex();
	}
	return createIndex(row, column, pid);
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
	if(!index.isValid()) return QModelIndex();
	if(index.internalId() == 0) return QModelIndex();
	
	long long parent_pid = mPidToProcess[ index.internalId()/*pid*/ ].parent_pid;
	return getQModelIndex(parent_pid,0);
}

QVariant ProcessModel::headerData(int section, Qt::Orientation orientation,
		                                         int role) const
{

	if (role != Qt::DisplayRole)
		return QVariant(); //error
	if(section < 0 || section >= mHeader.count()) return QVariant(); //error

	return i18n("process headings", mHeader.at(section).utf8()); //translate the header if possible
}

QString ProcessModel::getTooltipForUser(long long localhost_uid) const {
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

QVariant ProcessModel::getUsernameForUser(long long localhost_uid) const {
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
		     
	if (!index.isValid())
		return QVariant();
	switch (role){
	case Qt::DisplayRole: {
		const Process &process = mPidToProcess[index.internalId()/*pid*/];
		switch(index.column()) {
		case PROCESS_NAME:
			return process.name;
		case PROCESS_PID:
			return process.pid;
		case PROCESS_PPID:
			return process.parent_pid;
		case PROCESS_UID:
			if(!mIsLocalhost)
				return process.uid;
			//since we are on localhost, we can give more useful information about the username
			return getUsernameForUser(process.uid);
		default:
			return process.data.at(index.column() - PROCESS_DATASTART);
		}
		break;
	}
	case Qt::ToolTipRole: {
		if(!mIsLocalhost) return QVariant();//you might have to move this into the switch if you add more tooltips
		const Process &process = mPidToProcess[index.internalId()/*pid*/];
		switch(index.column()) {
		case PROCESS_UID: {
			long long uid = process.uid;
			//since we are on localhost, we can give more useful information about the username
			return getTooltipForUser(uid);	
		}
		default:
			return QVariant();
		}
	}
	case Qt::UserRole: {
		if(index.column() != PROCESS_UID) return QVariant();  //If we query with this role, then we want the raw UID for this.
		const Process &process = mPidToProcess[index.internalId()/*pid*/];
		return process.uid;
	}
	case Qt::DecorationRole: {
		if(index.column() != PROCESS_NAME) return QVariant(); //you might have to change this into a switch if you add more decorations
		const Process &process = mPidToProcess[index.internalId()/*pid*/];
		switch (process.processType){
			case Process::Init:
				return getIcon("penguin");
			case Process::Daemon:
				return getIcon("daemon");
			case Process::Kernel:
				return getIcon("kernel");
			case Process::Invalid:
				return QVariant();
			default:
				//so iconname tries to guess as what icon to use.
				return getIcon(process.name);
		}
		return QVariant(); //keep compilier happy
	}
	case Qt::BackgroundColorRole: {
		if(!mIsLocalhost) return QVariant();
		const Process &process = mPidToProcess[index.internalId()/*pid*/];
		if(process.uid == getuid()) {
			return QColor("red"); 
		}
		if(process.uid < 100)
			return QColor("gainsboro");
		return QColor("mediumaquamarine");
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


