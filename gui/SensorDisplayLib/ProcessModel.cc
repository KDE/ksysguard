/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 John Tapsell <john.tapsell@kdemail.net>

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
	mPidToProcess[0] = new Process();  //Add a fake process for process '0', the parent for init.  This lets us remove checks everywhere for init process
	mPidColumn = -1;

	//Translatable strings for the status
	(void)I18N_NOOP2("process status", "running"); 
	(void)I18N_NOOP2("process status", "sleeping");
	(void)I18N_NOOP2("process status", "disk sleep");
	(void)I18N_NOOP2("process status", "zombie");
	(void)I18N_NOOP2("process status", "stopped");
	(void)I18N_NOOP2("process status", "paging");
	(void)I18N_NOOP2("process status", "idle");

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
	mProcessType.insert("xntpd", Process::Daemon);
	mProcessType.insert("ypbind", Process::Daemon);
	 /* kde applications */
	mProcessType.insert("appletproxy", Process::Kdeapp);
	mProcessType.insert("dcopserver", Process::Kdeapp);
	mProcessType.insert("kcookiejar", Process::Kdeapp);
	mProcessType.insert("kde", Process::Kdeapp);
	mProcessType.insert("kded", Process::Kdeapp);
	mProcessType.insert("kdeinit", Process::Kdeapp);
	mProcessType.insert("kdesktop", Process::Kdeapp);
	mProcessType.insert("kdesud", Process::Kdeapp);
	mProcessType.insert("kdm", Process::Kdeapp);
	mProcessType.insert("khotkeys", Process::Kdeapp);
	mProcessType.insert("kio_file", Process::Kdeapp);
	mProcessType.insert("kio_uiserver", Process::Kdeapp);
	mProcessType.insert("klauncher", Process::Kdeapp);
	mProcessType.insert("ksmserver", Process::Kdeapp);
	mProcessType.insert("kwrapper", Process::Kdeapp);
	mProcessType.insert("kwrited", Process::Kdeapp);
	mProcessType.insert("kxmlrpcd", Process::Kdeapp);
	mProcessType.insert("startkde", Process::Kdeapp);
	 /* other processes */
	mProcessType.insert("bash", Process::Shell);
	mProcessType.insert("cat", Process::Tools);
	mProcessType.insert("egrep", Process::Tools);
	mProcessType.insert("emacs", Process::Wordprocessing);
	mProcessType.insert("fgrep", Process::Tools);
	mProcessType.insert("find", Process::Tools);
	mProcessType.insert("grep", Process::Tools);
	mProcessType.insert("ksh", Process::Shell);
	mProcessType.insert("screen", Process::Term);
	mProcessType.insert("sh", Process::Shell);
	mProcessType.insert("sort", Process::Tools);
	mProcessType.insert("ssh", Process::Shell);
	mProcessType.insert("su", Process::Tools);
	mProcessType.insert("tcsh", Process::Shell);
	mProcessType.insert("tee", Process::Tools);
	mProcessType.insert("vi", Process::Wordprocessing);
	mProcessType.insert("vim", Process::Wordprocessing);
	
}
void ProcessModel::setData(const QList<QStringList> &data)
{

	if(mPidColumn == -1) {
		kDebug(1215) << "We have recieved a setData()  before we know about our headings." << endl;
		return;
	}
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
		if(new_pid_data.count() <= mPidColumn || new_pid_data.count() <= mPPidColumn) {
			kDebug() << "Something wrong with the ps data comming from ksysguardd daemon.  Ignoring it." << endl;
			kDebug() << new_pid_data.join(",") << endl;
			return;
		}
		long long pid = new_pid_data.at(mPidColumn).toLongLong();
		long long ppid = 0;
		if(mPPidColumn >= 0)
			ppid = new_pid_data.at(mPPidColumn).toLongLong();
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
	return mPidToProcess[ppid]->children_pids.count();
}

int ProcessModel::columnCount ( const QModelIndex & parent ) const
{
	if(parent.isValid()) return 0;
	return mHeadings.count();
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
	const QList<long long> siblings = mPidToProcess[ppid]->children_pids;
	if(siblings.count() > row)
		return createIndex(row,column, siblings[row]);
	else
		return QModelIndex();
}


//only to be called if the (new) parent will not change under us!
void ProcessModel::changeProcess(long long pid)
{
	Q_ASSERT(pid != 0);
	long long ppid = mPidToProcess[pid]->parent_pid;
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

	QPointer<Process> process = mPidToProcess[pid];

	const QStringList &newDataRow = newData[pid];

	//Use i for the index in the new data, and j for the index for the process->data structure that we are copying into
	int j = 0;
	bool changed = false;
	QString loginName;
	for (int i = 0; i < newDataRow.count(); i++)
	{  //At the moment the data is just a string, so turn it into a list of variants instead and insert the variants into our new process
		
		QVariant value;
		switch(mColType[i]) {

			case DataColumnUserUsage: {
				float usage = newDataRow[i].toFloat();
				if(process->userUsage != usage) {
					process->userUsage = usage;
					changed = true;
				}
			} break;
			case DataColumnSystemUsage: {
				float usage = newDataRow[i].toFloat();
				if(process->sysUsage != usage) {
					process->sysUsage = usage;
					changed = true;
				}
			} break;
			case DataColumnNice: {
				int nice = newDataRow[i].toInt();
				if(process->nice != nice) {
					process->nice = nice;
					changed = true;
				}
			} break;
			case DataColumnVmSize: {
				int vmsize = newDataRow[i].toInt();
				if(process->vmSize != vmsize) {
					process->vmSize = vmsize;
					changed = true;
				}
			} break;

			case DataColumnVmRss: {
				int vmrss = newDataRow[i].toInt();
				if(process->vmRSS != vmrss) {
					process->vmRSS = vmrss;
					changed = true;
				}
			} break;

			case DataColumnCommand: {
				if(process->command != newDataRow[i]) {
					process->command = newDataRow[i];
					changed = true;
				}
			} break;
			case DataColumnName: {
				QString name = newDataRow[i];
				if(process->name != name) {
					process->name = name;
					process->processType = (mProcessType.contains(name))?mProcessType[name]:Process::Other;
					changed = true;
				}
			} break;
			case DataColumnUid: {
				long long uid = newDataRow[i].toLongLong();
				if(process->uid != uid) {
					process->uid = uid;
					changed = true;
				}
			} break;
			case DataColumnLogin: loginName = newDataRow[i]; break; //we might not know the uid yet, so remember the login name then at the end modify mUserUsername
			case DataColumnGid: {
				long long gid = newDataRow[i].toLongLong();
				if(process->gid != gid) {
					process->gid = gid;
					changed = true;
				} 
			} break;
			case DataColumnTracerPid: {
				long long tracerpid = newDataRow[i].toLongLong();
				if(process->tracerpid != tracerpid) {
					process->tracerpid = tracerpid;
					changed = true;
				}
			} break;
			case DataColumnPid: break; //Already dealt with
			case DataColumnPPid: break; //Already dealt with
			case DataColumnOtherLong: value = newDataRow[i].toLongLong(); break;
			case DataColumnOtherPrettyLong: value = KGlobal::locale()->formatNumber( newDataRow[i].toDouble(),0 ); break;
			case DataColumnOtherPrettyFloat: value = KGlobal::locale()->formatNumber( newDataRow[i].toDouble() ); break;
			case DataColumnStatus: {
				QByteArray status = newDataRow[i].toLatin1(); 
				if(process->status != status) {
					process->status = status;
					changed = true;
				}
				break;  
		        }
			default: {
				value = newDataRow[i];
				kDebug() << "Un caught column: " << value << endl;
				break;
			}
		}
		if(value.isValid()) {
			if(value != process->data[j]) {
				process->data[j] = value;
				changed = true;
			}
			j++;
		}
	}
	if(process->uid != -1)
		mUserUsername[process->uid] = loginName;
	
	//Now all the data has been changed for this process->
	if(!changed)
		return; 

	int row = mPidToProcess[ process->parent_pid ]->children_pids.indexOf(process->pid);
	if(row == -1) //something has gone really wrong
		return;
	QModelIndex startIndex = createIndex(row, 0, pid);
	QModelIndex endIndex = createIndex(row, mHeadings.count()-1, pid);
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
	

	QList<long long> &parents_children = mPidToProcess[ppid]->children_pids;
	int row = parents_children.count();
	QModelIndex parentModel = getQModelIndex(ppid, 0);

	const QStringList &newDataRow = newData[pid];

	QPointer<Process> new_process = new Process(pid, ppid);
	QList<QVariant> &data = new_process->data;
	QString loginName;
	for (int i = 0; i < mColType.count(); i++)
	{  //At the moment the data is just a string, so turn it into a list of variants instead and insert the variants into our new process
		switch(mColType[i]) {
			case DataColumnLogin: loginName = newDataRow[i]; break; //we might not know the uid yet, so remember the login name then at the end modify mUserUsername
			case DataColumnName:
				new_process->name = newDataRow[i];
				new_process->processType = (mProcessType.contains(new_process->name))?mProcessType[new_process->name]:Process::Other;
				break;
			case DataColumnGid: new_process->gid = newDataRow[i].toLongLong(); break;
			case DataColumnPid: break;  //Already dealt with
			case DataColumnPPid: break; //Already dealt with
			case DataColumnUid: new_process->uid = newDataRow[i].toLongLong(); break;
			case DataColumnTracerPid: new_process->tracerpid = newDataRow[i].toLongLong(); break;
			case DataColumnUserUsage: new_process->userUsage = newDataRow[i].toFloat(); break;
			case DataColumnSystemUsage: new_process->sysUsage = newDataRow[i].toFloat(); break;
			case DataColumnNice: new_process->nice = newDataRow[i].toInt(); break;
			case DataColumnVmSize: new_process->vmSize = newDataRow[i].toLong(); break;
			case DataColumnVmRss: new_process->vmRSS = newDataRow[i].toLong(); break;
			case DataColumnCommand: new_process->command = newDataRow[i]; break;
			case DataColumnStatus: new_process->status = newDataRow[i].toLatin1(); break;
			case DataColumnOtherLong: data << newDataRow[i].toLongLong(); break;
			case DataColumnOtherPrettyLong:  data << KGlobal::locale()->formatNumber( newDataRow[i].toDouble(),0 ); break;
			case DataColumnOtherPrettyFloat: data << KGlobal::locale()->formatNumber( newDataRow[i].toDouble() ); break;
			case DataColumnError:
			default:
				data << newDataRow[i]; break;
		}
	}
	if(new_process->uid != -1)
		mUserUsername[new_process->uid] = loginName;
	//Only here can we actually change the model.  First notify the view/proxy models then modify
	
//	kDebug() << "inserting " << pid << "(" << new_process->pid << "] at "<< row << " in parent " << new_process->parent_pid <<endl;
	beginInsertRows(parentModel, row, row);
		mPidToProcess[new_process->pid] = new_process;
		parents_children << new_process->pid;  //add ourselves to the parent
		mPids << new_process->pid;
	endInsertRows();
}
void ProcessModel::removeRow( long long pid ) 
{
//	kDebug() << "Removing " << pid << endl;
	if(pid <= 0) return; //init has parent pid 0
	if(!mPidToProcess.contains(pid)) {
		return; //we may have already deleted for some reason?
	}

	QPointer<Process> process = mPidToProcess[pid];	

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
	QList<long long> &parents_children = mPidToProcess[ppid]->children_pids;
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
	long long ppid = mPidToProcess[pid]->parent_pid;
	int row = 0;
	if(ppid != 0) {
		row = mPidToProcess[ppid]->children_pids.indexOf(pid);
		if(row == -1) //something has gone really wrong
			return QModelIndex();
	}
	return createIndex(row, column, pid);
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
	if(!index.isValid()) return QModelIndex();
	if(index.internalId() == 0) return QModelIndex();
	
	long long parent_pid = mPidToProcess[ index.internalId()/*pid*/ ]->parent_pid;
	return getQModelIndex(parent_pid,0);
}

QVariant ProcessModel::headerData(int section, Qt::Orientation orientation,
		                                         int role) const
{

	if (role != Qt::DisplayRole)
		return QVariant(); //error
	if(section < 0 || section >= mHeadings.count()) return QVariant(); //error
	return mHeadings[section];
}

QString ProcessModel::getTooltipForUser(long long uid, long long gid) const {
	QString &userTooltip = mUserTooltips[uid];
	if(userTooltip.isEmpty()) {
		if(!mIsLocalhost) {
			QVariant username = getUsernameForUser(uid);
			userTooltip = "<qt>";
			userTooltip += i18n("Login Name: %1<br/>").arg(username.toString());
			userTooltip += i18n("User ID: %1").arg(uid);
		} else {
			KUser user(uid);
			if(!user.isValid())
				userTooltip = i18n("This user is not recognised for some reason");
			else {
				userTooltip = "<qt>";
				if(!user.fullName().isEmpty()) userTooltip += i18n("<b>%1</b><br/>").arg(user.fullName());
				userTooltip += i18n("Login Name: %1<br/>").arg(user.loginName());
				if(!user.roomNumber().isEmpty()) userTooltip += i18n("Room Number: %1<br/>").arg(user.roomNumber());
				if(!user.workPhone().isEmpty()) userTooltip += i18n("Work Phone: %1<br/>").arg(user.workPhone());
				userTooltip += i18n("User ID: %1").arg(uid);
			}
		}
	}
	if(gid != -1) {
		if(!mIsLocalhost)
			return userTooltip + i18n("<br/>Group ID: %1").arg(gid);
		QString groupname = KUserGroup(gid).name();
		if(groupname.isEmpty())
			return userTooltip + i18n("<br/>Group ID: %1").arg(gid);
		return userTooltip +  i18n("<br/>Group Name: %1").arg(groupname)+ i18n("<br/>Group ID: %1").arg(gid);
	}
	return userTooltip;
}

QVariant ProcessModel::getUsernameForUser(long long uid) const {
	QVariant &username = mUserUsername[uid];
	if(!username.isValid()) {
		if(mIsLocalhost) {
			username = uid;
		} else {		
			KUser user(uid);
			if(!user.isValid())
				username = uid;
			username = user.loginName();
		}
	}
	return username;
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
	//This function must be super duper ultra fast because it's called thousands of times every few second :(
	//I think it should be optomised for role first, hence the switch statement (fastest possible case)
		     
	if (!index.isValid())
		return QVariant();
	Q_ASSERT(index.column() < mHeadingsToType.count());
	switch (role){
	case Qt::DisplayRole: {
		QPointer<Process> process = mPidToProcess[index.internalId()/*pid*/];
		int headingType = mHeadingsToType[index.column()];
		switch(headingType) {
		case HeadingName:
			return process->name;
		case HeadingUser:
			return getUsernameForUser(process->uid);
		case HeadingXIdentifier:
			return process->xResIdentifier;
		case HeadingXMemory:
			if(process->xResMemOtherBytes == 0 && process->xResPxmMemBytes == 0) return QVariant();
			return QString::number((process->xResMemOtherBytes + process->xResPxmMemBytes )/ 1024.0, 'f', 0);
		case HeadingCPUUsage:
			return QString::number((process->userUsage > process->sysUsage)?process->userUsage:process->sysUsage, 'f', 2) + "%";
		case HeadingRSSMemory:
			return QString::number(process->vmRSS);
		case HeadingMemory:
			return QString::number(process->vmSize);
		default:
			return process->data.at(headingType-HeadingOther);
		}
		break;
	}
	case Qt::ToolTipRole: {
		QPointer<Process> process = mPidToProcess[index.internalId()/*pid*/];
		QString tracer;
		if(process->tracerpid > 0) {
			if(mPidToProcess.contains(process->tracerpid)) { //it is possible for this to be not the case in certain race conditions
				QPointer<Process> process_tracer = mPidToProcess[process->tracerpid];
				tracer = i18n("tooltip. name,pid ","This process is being debugged by %1 (%2)").arg(process_tracer->name).arg(process->tracerpid);
			}
		}
		switch(mHeadingsToType[index.column()]) {
		case HeadingName: {
			QString tooltip = i18n("name column tooltip. first item is the name","<qt><b>%1</b><br/>Process ID: %2<br/>Parent's ID: %3").arg(process->name).arg(process->pid).arg(process->parent_pid);
			
			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}
		case HeadingUser: {
			if(!tracer.isEmpty())
				return getTooltipForUser(process->uid, process->gid) + "<br/>" + tracer;
			return getTooltipForUser(process->uid, process->gid);
		}
		case HeadingXMemory: {
			QString tooltip = i18n("<qt>Number of pixmaps: %1<br/>Amount of memory used by pixmaps: %2 KiB<br/>Other X server memory used: %3 KiB");
#warning Use klocale::formatByteString  when kdelibs trunk is synced 
			tooltip = tooltip.arg(process->xResNumPxm).arg(process->xResPxmMemBytes/1024.0).arg(process->xResMemOtherBytes/1024.0);
			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}
		case HeadingCPUUsage: {
			QString tooltip = i18n("<qt>User CPU usage: %1%<br/>System CPU usage: %2%").arg(process->userUsage).arg(process->sysUsage);
			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}
		default:
			tracer.isEmpty(); return tracer;
			return QVariant();
		}
	}
	case Qt::UserRole: {
		//We have a special understanding with the filter.  If it queries us as UserRole in column 0, return uid
		if(index.column() != 0) return QVariant();  //If we query with this role, then we want the raw UID for this.
		return mPidToProcess[index.internalId()/*pid*/]->uid;
	}
	case Qt::DecorationRole: {
		if(mHeadingsToType[index.column()] != HeadingName) return QVariant(); //you might have to change this into a switch if you add more decorations
		QPointer<Process> process = mPidToProcess[index.internalId()/*pid*/];
		switch (process->processType){
			case Process::Init:
				return getIcon("penguin");
			case Process::Daemon:
				return getIcon("daemon");
			case Process::Kernel:
				return getIcon("kernel");
			case Process::Kdeapp:
				return getIcon("kdeapp");
			case Process::Tools:
				return getIcon("tools");
			case Process::Shell:
				return getIcon("shell");
			case Process::Wordprocessing:
				return getIcon("wordprocessing");
			case Process::Term:
				return getIcon("openterm");
			case Process::Invalid:
				return QVariant();
//			case Process::Other:
			default:
				//so iconname tries to guess as what icon to use.
				return getIcon(process->name);
		}
		return QVariant(); //keep compilier happy
	}
	case Qt::BackgroundColorRole: {
		if(!mIsLocalhost) return QVariant();
		QPointer<Process> process = mPidToProcess[index.internalId()/*pid*/];
		if(process->tracerpid >0) {
			//It's being debugged, so probably important.  Let's mark it as such
			return QColor("yellow");
		}
		if(process->uid == getuid()) {
			return QColor("red"); 
		}
		if(process->uid < 100)
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
		    pix = SmallIcon("unknownapp");

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

/** Set the untranslated heading names for the model */
bool ProcessModel::setHeader(const QStringList &header, const QList<char> &coltype_) {
	mPidColumn = -1;  //We need to be able to access the pid directly, so remember which column it will be in
	mPPidColumn = -1; //Same, although this may not always be found :/
	QStringList headings;
	QList<int> headingsToType;
	int num_of_others = 0; //Number of headings found that we dont know about.  Will match the index in process->data[index]
	QList<char> coltype;
	for(int i = 0; i < header.count(); i++) {
		bool other = false;
		if(header[i] == "Login") {
			coltype << DataColumnLogin;
		} else if(header[i] == "GID") {
			coltype << DataColumnGid;
		} else if(header[i] == "PID") {
			coltype << DataColumnPid;
			mPidColumn = i;
		} else if(header[i] == "PPID") {
			coltype << DataColumnPPid;
			mPPidColumn = i;
		} else if(header[i] == "UID") {
			headings.prepend(i18n("process heading", "User"));   //The heading for the top of the qtreeview
			headingsToType.prepend(HeadingUser);
			coltype << DataColumnUid;
		} else if(header[i] == "Name") {
			coltype << DataColumnName;
		} else if(header[i] == "TracerPID") {
			coltype << DataColumnTracerPid;
		} else if(header[i] == "User%") {
			coltype << DataColumnUserUsage;
			headings << i18n("process heading", "CPU %");
			headingsToType << HeadingCPUUsage;
		} else if(header[i] == "System%") {
			coltype << DataColumnSystemUsage;
		} else if(header[i] == "Nice") {
			coltype << DataColumnNice;
		} else if(header[i] == "VmSize") {
			coltype << DataColumnVmSize;
			headings << i18n("process heading", "Memory");
			headingsToType << HeadingMemory;
		} else if(header[i] == "VmRss") {
			coltype << DataColumnVmRss;	
			headings << i18n("process heading", "RSS Memory");
			headingsToType << HeadingRSSMemory;
		} else if(header[i] == "Command") {
			coltype << DataColumnCommand;
		} else if(coltype_[i] == DATA_COLUMN_STATUS) {
			coltype << DataColumnStatus;
		} else if(coltype_[i] == DATA_COLUMN_LONG) {
			coltype << DataColumnOtherLong;
			other = true;
		} else if(coltype_[i] == DATA_COLUMN_PRETTY_LONG) {
			coltype << DataColumnOtherPrettyLong;
			other = true;
		} else if(coltype_[i] == DATA_COLUMN_PRETTY_FLOAT) {
			coltype << DataColumnOtherPrettyFloat;
			other = true;
		} else {
			coltype << DataColumnError;
		}	
		if(other) { //If we don't know about this column, just automatically add it
			headings << header[i];
			headingsToType << (HeadingOther + num_of_others++);
		}
	}
	if(mPidColumn == -1 || !coltype.contains(DataColumnName)) {
		kDebug(1215) << "Data from daemon for 'ps' is missing pid or name. Bad data." << endl;
		return false;
	}
	
	headings.prepend(i18n("process heading", "Name"));
	headingsToType.prepend(HeadingName);
	
	beginInsertColumns(QModelIndex(), 0, header.count()-1);
		mHeadingsToType = headingsToType;
		mColType = coltype;
		mHeadings = headings;
	endInsertColumns();
	return true;
}
bool ProcessModel::setXResHeader(const QStringList &header, const QList<char> &)
{
	kDebug() << "SetXResHeader" << header.join(",") << endl;
		
	mXResPidColumn = -1;
	mXResIdentifierColumn = -1;
	mXResNumPxmColumn = -1;
	mXResMemOtherColumn = -1;
	mXResNumColumns = header.count();
	if(mXResNumColumns < 4) return false;
	for(int i = 0; i < mXResNumColumns; i++) {
		if(header[i] == "XPid")
			mXResPidColumn = i;
		else if(header[i] == "XIdentifier")
			mXResIdentifierColumn = i;
		else if(header[i] == "XPxmMem")
			mXResPxmMemColumn = i;
		else if(header[i] == "XNumPxm")
			mXResNumPxmColumn = i;
		else if(header[i] == "XMemOther")
			mXResMemOtherColumn = i;
	}
	bool insertXIdentifier =
	  mXResIdentifierColumn != -1 &&
	  !mHeadingsToType.contains(HeadingXIdentifier);  //we can end up inserting twice without this check if we add then remove the sensor or something
	bool insertXMemory = (mXResMemOtherColumn != -1 &&
		   mXResPxmMemColumn != -1 && 
		   mXResNumPxmColumn != -1 && 
		   !mHeadingsToType.contains(HeadingXMemory));
	if(!insertXIdentifier && !insertXMemory) return true; //nothing to do - already inserted
		
	beginInsertColumns(QModelIndex(), mHeadings.count()-1, mHeadings.count() + (insertXMemory && insertXIdentifier)?1:0);
	
		if(insertXIdentifier) {
			mHeadings << i18n("process heading", "Application");
			mHeadingsToType << HeadingXIdentifier;
		}
		if(insertXMemory) {
			mHeadings << i18n("process heading", "X Server Memory");
			mHeadingsToType << HeadingXMemory;
		}
	endInsertColumns();
	return true;
}
void ProcessModel::setXResData(const QStringList& data)
{
	if(data.count() < mXResNumColumns) {
		kDebug() << "Invalid data in setXResData. Not enough columns: " << data.join(",") << endl;
		return;
	}
	if(mXResPidColumn == -1) {
		kDebug() << "XRes data recieved when we still don't know which column the XPid is in" << endl;
		return;
	}
		

	long long pid = data[mXResPidColumn].toLongLong();
	QPointer<Process> process = mPidToProcess[pid];
	if(process.isNull()) {
		kDebug() << "XRes Data for process '" << data[mXResPidColumn] << "'(int) which we don't know about" << endl;
		return;
	}
 	bool changed = false;
	if(mXResIdentifierColumn != -1)
		if(process->xResIdentifier != data[mXResIdentifierColumn]) {
		  changed = true;
		  process->xResIdentifier = data[mXResIdentifierColumn];
		}	
	if(mXResPxmMemColumn != -1) {
		long long pxmMem = data[mXResPxmMemColumn].toLongLong();
		if(process->xResPxmMemBytes != pxmMem) {
		  changed = true;
		  process->xResPxmMemBytes = pxmMem;
		}
	}
	if(mXResNumPxmColumn != -1) {
		int numPxm =  data[mXResNumPxmColumn].toInt();
		if(process->xResNumPxm != numPxm) {
		  changed = true;
		  process->xResNumPxm = numPxm;
		}
	}
	if(mXResMemOtherColumn != -1) {
		long long memOther = data[mXResMemOtherColumn].toInt();
		if(process->xResMemOtherBytes != memOther) {
		  changed = true;
		 process->xResMemOtherBytes = memOther;
		}
	}
	if(!changed)
		return;
	QPointer<Process> parent_process = mPidToProcess[ process->parent_pid ];
	if(parent_process.isNull()) return;
	int row = parent_process->children_pids.indexOf(process->pid);
	if(row == -1) //something has gone really wrong
		return;
        QModelIndex startIndex = createIndex(row, 0, pid);
	QModelIndex endIndex = createIndex(row, mHeadings.count()-1, pid);
        emit dataChanged(startIndex, endIndex);

}


