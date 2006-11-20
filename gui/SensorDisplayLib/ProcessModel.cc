/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 John Tapsell <john.tapsell@kde.org>

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



#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
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

ProcessModel::ProcessModel(QObject* parent)
	: QAbstractItemModel(parent)
{
	mSimple = true;
	mIsLocalhost = false; //this default really shouldn't matter, because setIsLocalhost should be called before setData()
	mPidToProcess[0] = new Process();  //Add a fake process for process '0', the parent for init.  This lets us remove checks everywhere for init process
	
	mShowChildTotals = true;

	mPidColumn = -1;

	//Translatable strings for the status
	(void)I18N_NOOP2("process status", "running");
	(void)I18N_NOOP2("process status", "sleeping");
	(void)I18N_NOOP2("process status", "disk sleep");
	(void)I18N_NOOP2("process status", "zombie");
	(void)I18N_NOOP2("process status", "stopped");
	(void)I18N_NOOP2("process status", "paging");
	(void)I18N_NOOP2("process status", "idle");
	mStatusDescription["running"] = i18n("- Process is doing some work");
	mStatusDescription["sleeping"] = i18n("- Process is waiting for something to happen");
	mStatusDescription["stopped"] = i18n("- Process has been stopped. It will not respond to user input at the moment");	
	mStatusDescription["zombie"] = i18n("- Process has finished and is now dead, but the parent has not noticed yet");

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

bool ProcessModel::setData(const QList<QStringList> &data)
{

	if(mPidColumn == -1) {
		kDebug(1215) << "We have received a setData()  before we know about our headings." << endl;
		return false;
	}
	// We can set this from anywhere to basically say something has gone wrong, and that the code is buggy, so reset and get all the data again
	// It shouldn't happen, but just-in-case.  From 2006-05-13 I have not seen any resets for a while, just for reference.
	mNeedReset = false;

	//We have our new data, and our current data.
	//First we pull all the information from data so it's in the same format as the existing data.
	//Then we delete all those that have stopped running
	//Then insert all the new ones
	//Then finally update the rest that might have changed

	for(long i = 0; i < data.size(); i++) {
		QStringList new_pid_data = data.at(i);
		if(new_pid_data.count() <= mPidColumn || new_pid_data.count() <= mPPidColumn) {
			kDebug(1215) << "Something wrong with the ps data coming from ksysguardd daemon.  Ignoring it." << endl;
			kDebug(1215) << new_pid_data.join(",") << endl;
			return false; //returning false will trigger a sensor error
		}
		long long pid = new_pid_data.at(mPidColumn).toLongLong();
		long long ppid = 0;
		if(mPPidColumn >= 0 && !mSimple)
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
		kDebug(1215) << "After merging the new process data, an internal discrancy was found. Fail safe reseting view." << endl;
		kDebug(1215) << "We were told there were " << data.count() << " processes, but after merging we know about " << mPids.count() << endl;
		mNeedReset = true;
	}
	if(mNeedReset) {
		//This shouldn't happen, but good to have a fall back incase it does :)
		//fixme - save selection
		kDebug(1215) << "HAD TO RESET!" << endl;
		mPidToProcess.clear();
		mPidToProcess[0] = new Process();  //Add a fake process for process '0', the parent for init.  This lets us remove checks everywhere for init process
		mPids.clear();
		reset();
		mNeedReset = false;
		emit layoutChanged();
	}
	return true;
}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
	if(!parent.isValid()) return (mPidToProcess.count() == 1 )?0:1;
	if(parent.column() != 0) return 0;  //For a treeview we say that only the first column has children
	Process *process = reinterpret_cast< Process * > (parent.internalPointer()); //when parent is invalid, it must be the root level which we set as 0
	Q_ASSERT(process);
	int num_rows = process->children.count();
	return num_rows;
}

int ProcessModel::columnCount ( const QModelIndex & ) const
{
	return mHeadings.count();
}

bool ProcessModel::hasChildren ( const QModelIndex & parent = QModelIndex() ) const
{
	int num_children = rowCount(parent);
	bool has_children = num_children > 0;
	return has_children;
}

QModelIndex ProcessModel::index ( int row, int column, const QModelIndex & parent ) const
{
	if(row<0) return QModelIndex();
	if(column<0) return QModelIndex();
	Process *parent_process = 0;
	
	if(parent.isValid()) //not valid for init, and init has ppid of 0
		parent_process = reinterpret_cast< Process * > (parent.internalPointer());
	else
		parent_process = mPidToProcess[0];
	Q_ASSERT(parent_process);

	// Using QList<long long>& instead of QList<long long> creates an internal error in gcc 3.3.1
	if(parent_process->children.count() > row)
		return createIndex(row,column, parent_process->children[row]);
	else
	{
		return QModelIndex();
	}
}

void ProcessModel::updateProcessTotals(Process *process, double sysChange, double userChange, int numChildrenChange)
{
	
	if(process == NULL || process->pid == 0) return;
	Q_ASSERT(process);

	for(;;) {
		//update the pid to that of the parent
		process->totalSysUsage += sysChange;
		process->totalUserUsage += userChange;
		process->numChildren += numChildrenChange;
		//Okay we have finished updating the process.  Now change to the parent process

		//If we are init, then our ppid is 0, so leave our row as 0.  If we are not, then we need to find our parent process
		//and find which child item we are in that
		if(process->parent_pid == 0) {
			if(mCPUHeading != -1 && mShowChildTotals && !mSimple) {
				QModelIndex index = createIndex(0, mCPUHeading, process);
				emit dataChanged(index, index);
			}
			return;
		}
		Process *parent_process = process->parent;
		Q_ASSERT(parent_process);
		//Get the row in this parent where the child process is
		int row = parent_process->children.indexOf(process);
		Q_ASSERT(row != -1); //something has gone really wrong
	
		//Update the process we modified
		if(mCPUHeading != -1 && mShowChildTotals && !mSimple) {
			QModelIndex index = createIndex(row, mCPUHeading, process);
			emit dataChanged(index, index);
		}

		//Switch to the parent, and repeat
		process = parent_process;
	}
}

//only to be called if the (new) parent will not change under us!
void ProcessModel::changeProcess(long long pid)
{
	Q_ASSERT(pid != 0);
	long long new_ppid = newPidToPpidMapping[pid];
	//This is called from insertOrChangeRows and after the parent is checked, so we know the (new) parent won't change under us

	if(new_ppid != mPidToProcess[pid]->parent_pid) {
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

	//We know the pid is the same (obviously), and ppid.  So check name (and update processType?),uid and data.  children will take care
	//of themselves as they are inserted later.

	Process *process = mPidToProcess[pid];

	const QStringList &newDataRow = newData[pid];

	//Use i for the index in the new data, and j for the index for the process->data structure that we are copying into
	int j = 0;
	bool changed = false;
	QString loginName;
	double sysUsageChange = 0;
	double userUsageChange = 0;
	for (int i = 0; i < newDataRow.count(); i++)
	{  //At the moment the data is just a string, so turn it into a list of variants instead and insert the variants into our new process

		QVariant value;
		switch(mColType.at(i)) {

			case DataColumnUserUsage: {
				float usage = newDataRow[i].toFloat();
				if(process->userUsage != usage) {
					userUsageChange = usage - process->userUsage;
					process->userUsage = usage;
					changed = true;
				}
			} break;
			case DataColumnSystemUsage: {
				float usage = newDataRow[i].toFloat();
				if(process->sysUsage != usage) {
					sysUsageChange = usage - process->sysUsage;
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
				kDebug(1215) << "Uncaught column: " << value << endl;
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

	//Now all the data has been changed for this process.
	//If nothing changed, no need to update the display etc
	if(!changed)
		return;

	//If the user or sys values changed, then we need to update their and their parents totals
	if(sysUsageChange != 0 || userUsageChange != 0) {

		process->totalSysUsage += sysUsageChange;
		process->totalUserUsage += userUsageChange;
		updateProcessTotals(process->parent, sysUsageChange, userUsageChange, 0);
	}

	int row = process->parent->children.indexOf(process);
	Q_ASSERT(row != -1);  //Something has gone very wrong
	QModelIndex startIndex = createIndex(row, 0, process);
	QModelIndex endIndex = createIndex(row, mHeadings.count()-1, process);
	emit dataChanged(startIndex, endIndex);

}

void ProcessModel::insertOrChangeRows( long long pid)
{
	if(!new_pids.contains(pid)) {
		kDebug(1215) << "Internal problem with data structure.  A loop perhaps?" << endl;
		mNeedReset = true;
		return;
	}
	Q_ASSERT(pid != 0);

	long long ppid = newPidToPpidMapping[pid];

	if(ppid != 0 && new_pids.contains(ppid))  //If we haven't inserted/changed the parent yet, do that first!
		insertOrChangeRows(ppid);   //by the nature of recursion, we know that _this_ parent will have its parents checked and so on
	//so now all parents are safe
	if(mPidToProcess.contains(pid)) {
		changeProcess(pid);  //we are changing, no need for insert
		return;
	}

	new_pids.remove(pid); //we will deal with this pid for certain, so remove it from the list
	//We are inserting a new process

	//This process may have children, however we are now guaranteed that:
	// a) If the children are new, then they will be inserted after the parent because in this function we recursively check the parent(s) first.
	// b) If the children already exist (a bit weird, but possible if a new process is created, then an existing one is reparented to it)
	//    then in changed() it will call this function to recursively insert its parents

	Process *parent = mPidToProcess[ppid];
	if(!parent) {
		kDebug(1215) << "Internal problem with data structure.  Possibly a race condition hit.  We were told there is process " << pid << " with parent " << ppid << ", but we can't find the process structure for that parent process." << endl;
		mNeedReset = true;
		return;
	}

	int row = parent->children.count();
	QModelIndex parentModel = getQModelIndex(parent, 0);

	const QStringList &newDataRow = newData[pid];
	Q_ASSERT(parent);
	Process *new_process = new Process(pid, ppid, parent);
	Q_CHECK_PTR(new_process);

	QList<QVariant> &data = new_process->data;
	QString loginName;
	for (int i = 0; i < mColType.size() && i < newDataRow.count(); i++)
	{  //At the moment the data is just a string, so turn it into a list of variants instead and insert the variants into our new process
		switch(mColType.at(i)) {
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
	//Update the totals
	new_process->totalSysUsage += new_process->sysUsage;
	new_process->totalUserUsage += new_process->userUsage;
	updateProcessTotals(parent, new_process->sysUsage, new_process->userUsage, 1);
	
	//Only here can we actually change the model.  First notify the view/proxy models then modify

	beginInsertRows(parentModel, row, row);
		mPidToProcess[new_process->pid] = new_process;
		parent->children << new_process;  //add ourselves to the parent
		mPids << new_process->pid;
	endInsertRows();
}
void ProcessModel::removeRow( long long pid )
{
	if(pid <= 0) return; //init has parent pid 0
	if(!mPidToProcess.contains(pid)) {
		return; //we may have already deleted for some reason?
	}

	Process *process = mPidToProcess[pid];

	{
		QList<Process *> *children = &(process->children); //remove all the children now
		foreach(Process *child, *children) {
			if(child == process) {
				kDebug(1215) << "A process is its own child? Something has gone wrong.  Reseting model" << endl;
				mNeedReset = true;
				return;
			}
			removeRow(child->pid);
		}
		children = NULL;
	}


	int row = process->parent->children.indexOf(process);
	QModelIndex parentModel = getQModelIndex(process->parent, 0);
	if(row == -1) {
		kDebug(1215) << "A serious problem occurred in remove row." << endl;
		return;
	}

	//Subtract the current totals from the parent processes
/*	if(process->sysUsage != 0 || process->userUsage != 0) {
#ifndef QT_NO_DEBUG
		if(process->totalSysUsage != process->sysUsage) qDebug("Internal error with consistency with keeping track of sys totals");
		if(process->totalUserUsage != process->userUsage) qDebug("Internal error with consistency with keeping track of user totals");
#endif
		updateProcessTotals(process->parent, -process->sysUsage, -process->userUsage, -1);
	}*/


	//so no more children left, we are free to delete now
	beginRemoveRows(parentModel, row, row);
		mPidToProcess.remove(pid);
		process->parent->children.removeAll(process);  //remove ourselves from the parent
		mPids.remove(pid);
	endRemoveRows();

	delete process;
	process = NULL;  //okay, now we aren't pointing to Process or any of its structures at all now. Should be safe to remove now.
}

QModelIndex ProcessModel::getQModelIndex( Process *process, int column) const
{
	Q_ASSERT(process);
	int pid = process->pid;
	if(pid == 0) return QModelIndex(); //pid 0 is our fake process meaning the very root.  To represent that, we return QModelIndex() which also means the top element
	int row = 0;
	if(process->parent) {
		row = process->parent->children.indexOf(process);
		Q_ASSERT(row != -1);
	}
	return createIndex(row, column, process);
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
	if(!index.isValid()) return QModelIndex();
	Process *process = reinterpret_cast< Process * > (index.internalPointer());
	Q_ASSERT(process);

	return getQModelIndex(process->parent,0);
}

QVariant ProcessModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant(); //error
	if(orientation != Qt::Horizontal)
		return QVariant();
	if(section < 0 || section >= mHeadings.count())
		return QVariant(); //TODO: Find out why this is needed
	return mHeadings[section];
}

bool ProcessModel::canUserLogin(long long uid ) const
{
	if(!mIsLocalhost) return true; //We only deal with localhost.  Just always return true for non localhost

	int canLogin = mUidCanLogin.value(uid, -1); //Returns 0 if we cannot login, 1 if we can, and the default is -1 meaning we don't know
	if(canLogin != -1) return canLogin; //We know whether they can log in

	//We got the default, -1, so we don't know.  Look it up
	
	KUser user(uid);
	if(!user.isValid()) { 
		//for some reason the user isn't recognised.  This might happen under certain security situations.
		//Just return true to be safe
		mUidCanLogin[uid] = 1;
		return true;
	}
	QString shell = user.shell();
	if(shell == "/bin/false" )  //FIXME - add in any other shells it could be for false
	{
		mUidCanLogin[uid] = 0;
		return false;
	}
	mUidCanLogin[uid] = 1;
	return true;
}

QString ProcessModel::getTooltipForUser(long long uid, long long gid) const
{
	QString &userTooltip = mUserTooltips[uid];
	if(userTooltip.isEmpty()) {
		if(!mIsLocalhost) {
			QVariant username = getUsernameForUser(uid);
			userTooltip = "<qt>";
			userTooltip += i18n("Login Name: %1<br/>", username.toString());
			userTooltip += i18n("User ID: %1", (long int)uid);
		} else {
			KUser user(uid);
			if(!user.isValid())
				userTooltip = i18n("This user is not recognised for some reason");
			else {
				userTooltip = "<qt>";
				if(!user.fullName().isEmpty()) userTooltip += i18n("<b>%1</b><br/>", user.fullName());
				userTooltip += i18n("Login Name: %1<br/>", user.loginName());
				if(!user.roomNumber().isEmpty()) userTooltip += i18n("Room Number: %1<br/>", user.roomNumber());
				if(!user.workPhone().isEmpty()) userTooltip += i18n("Work Phone: %1<br/>", user.workPhone());
				userTooltip += i18n("User ID: %1", (long int)uid);
			}
		}
	}
	if(gid != -1) {
		if(!mIsLocalhost)
			return userTooltip + i18n("<br/>Group ID: %1", (long int)gid);
		QString groupname = KUserGroup(gid).name();
		if(groupname.isEmpty())
			return userTooltip + i18n("<br/>Group ID: %1", (long int)gid);
		return userTooltip +  i18n("<br/>Group Name: %1", groupname)+ i18n("<br/>Group ID: %1", (long int)gid);
	}
	return userTooltip;
}

QString ProcessModel::getStringForProcess(Process *process) const {
	return i18nc("Short description of a process. PID, name", "%1: %2", (long)(process->pid), process->name);
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

	if (!index.isValid()) {
		return QVariant();
	}
	if (index.column() >= mHeadingsToType.count()) {
		return QVariant(); 
	}

	switch (role){
	case Qt::DisplayRole: {
		Process *process = reinterpret_cast< Process * > (index.internalPointer());
		int headingType = mHeadingsToType[index.column()];
		switch(headingType) {
		case HeadingName:
			return process->name;
		case HeadingUser:
			return getUsernameForUser(process->uid);
		case HeadingXIdentifier:
			return process->xResIdentifier;
		case HeadingXMemory:
			if(process->xResMemOtherBytes + process->xResPxmMemBytes == 0) return QVariant(QVariant::String);
			return KGlobal::locale()->formatByteSize(process->xResMemOtherBytes + process->xResPxmMemBytes);
		case HeadingCPUUsage:
			{
				double total;
				if(mShowChildTotals && !mSimple) total = process->totalUserUsage + process->totalSysUsage;
				else total = process->userUsage + process->sysUsage;

				if(total <= 0.1)
					return "";
				if(total > 100) total = 100;

				return QString::number(total, 'f', 2) + '%';
			}
		case HeadingRSSMemory:
			if(process->vmRSS == 0) return QVariant(QVariant::String);
			return KGlobal::locale()->formatByteSize(process->vmRSS * 1024);
		case HeadingMemory:
			if(process->vmSize == 0) return QVariant(QVariant::String);
			return KGlobal::locale()->formatByteSize(process->vmSize * 1024);
		case HeadingCommand:
			return process->command;
		default:
			return process->data.at(headingType-HeadingOther);
		}
		break;
	}
	case Qt::ToolTipRole: {
		Process *process = reinterpret_cast< Process * > (index.internalPointer());
		QString tracer;
		if(process->tracerpid > 0) {
			if(mPidToProcess.contains(process->tracerpid)) { //it is possible for this to be not the case in certain race conditions
				Process *process_tracer = mPidToProcess[process->tracerpid];
				tracer = i18nc("tooltip. name,pid ","This process is being debugged by %1 (%2)", process_tracer->name, (long int)process->tracerpid);
			}
		}
		switch(mHeadingsToType[index.column()]) {
		case HeadingName: {
			QString tooltip;
			if(process->parent_pid == 0)
				tooltip	= i18nc("name column tooltip. first item is the name","<qt><b>%1</b><br/>Process ID: %2<br/>Command: %3", process->name, (long int)process->pid, process->command);
			else
				tooltip	= i18nc("name column tooltip. first item is the name","<qt><b>%1</b><br/>Process ID: %2<br/>Parent's ID: %3<br/>Command: %4", process->name, (long int)process->pid, (long int)process->parent_pid, process->command);

			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}

		case HeadingCommand: {
			QString tooltip =
				i18n("<qt>This process was run with the following command:<br/>%1", process->command);
		        if(tracer.isEmpty()) return tooltip;
			return tooltip + "<br/>" + tracer;
		}
		case HeadingUser: {
			if(!tracer.isEmpty())
				return getTooltipForUser(process->uid, process->gid) + "<br/>" + tracer;
			return getTooltipForUser(process->uid, process->gid);
		}
		case HeadingXMemory: {
			QString tooltip;
			if(process->xResMemOtherBytes + process->xResPxmMemBytes == 0 && process->xResIdentifier.isEmpty())
				tooltip = i18n("<qt>This is a program that does not have graphical user interface.");
			else 
				tooltip = i18n("<qt>Number of pixmaps: %1<br/>Amount of memory used by pixmaps: %2<br/>Other X server memory used: %3", process->xResNumPxm, KGlobal::locale()->formatByteSize(process->xResPxmMemBytes), KGlobal::locale()->formatByteSize(process->xResMemOtherBytes));
			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}
		case HeadingCPUUsage: {
			QString tooltip = ki18n("<qt>Process status: %1 %2<br/>"
						"User CPU usage: %3%<br/>System CPU usage: %4%")
						.subs(i18nc("process status", process->status))
						.subs(mStatusDescription.value(process->status, ""))
						.subs(process->userUsage, 0, 'f', 2)
						.subs(process->sysUsage, 0, 'f', 2)
						.toString();
			if(process->numChildren > 0) {
				tooltip += ki18n("<br/>Number of children: %1<br/>Total User CPU usage: %2%<br/>"
						"Total System CPU usage: %3%<br/>Total CPU usage: %4%")
						.subs(process->numChildren)
						.subs(process->totalUserUsage, 0, 'f', 2)
						.subs(process->totalSysUsage, 0, 'f', 2)
						.subs(process->totalUserUsage + process->totalSysUsage, 0, 'f', 2)
						.toString();
			}

			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}
		default:
			tracer.isEmpty(); return tracer;
			return QVariant(QVariant::String);
		}
	}
	case Qt::UserRole: {
		//We have a special understanding with the filter.  If it queries us as UserRole in column 0, return uid
		if(index.column() != 0) return QVariant();  //If we query with this role, then we want the raw UID for this.
		Process *process = reinterpret_cast< Process * > (index.internalPointer());
		return process->uid;
	}

	case Qt::UserRole+1: {
		//We have a special understanding with the filter sort. This returns an int (in a qvariant) that can be sorted by
		Process *process = reinterpret_cast< Process * > (index.internalPointer());
		int headingType = mHeadingsToType[index.column()];
		switch(headingType) {
		case HeadingUser: {
			//Sorting by user will be the default and the most common.
			//We want to sort in the most useful way that we can. We need to return a number though.
			//First the user we are running as should be at the top.  We add 2,000,000 for this
			//Then any other users in the system.  We add 1,000,000 for this,
			//Then at the bottom the 'system' processes.  We add 0 for this

			//One special exception is a traced process since that's probably important. We should put that at the top
			if(process->tracerpid >0) return (long long)99999999;
			
			long long base = 0;
			if(process->uid == getuid())
				base = 2000000;
			else if(process->uid < 100 || !canUserLogin(process->uid))
				base = 0;
			else
				base = 1000000;

			//However we can of course have lots of processes with the same user.  Next we sort by CPU.
			return base + (long long)((process->totalUserUsage + process->totalSysUsage)*100);
		}
		case HeadingXMemory:
			return (long long)(process->xResMemOtherBytes + process->xResPxmMemBytes);
		case HeadingCPUUsage:
			return (long long)process->totalUserUsage + process->totalSysUsage;
		case HeadingRSSMemory:
			return (long long)process->vmRSS;
		case HeadingMemory:
			return (long long)process->vmSize;
		}

	}
	case Qt::DecorationRole: {
		if(mSimple) return QVariant();
		if(mHeadingsToType[index.column()] == HeadingName) {
			Process *process = reinterpret_cast< Process * > (index.internalPointer());
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
		} else if (mHeadingsToType[index.column()] == HeadingCPUUsage) {
			Process *process = reinterpret_cast< Process * > (index.internalPointer());
			if(process->status == "stopped" || process->status == "zombie") {
				return getIcon("button_cancel");
			}
		}
		return QVariant();
	}
	case Qt::BackgroundRole: {
		if(!mIsLocalhost) return QVariant();
		if(mSimple) return QVariant();  //Simple mode means no colors 
		Process *process = reinterpret_cast< Process * > (index.internalPointer());
		if(process->tracerpid >0) {
			//It's being debugged, so probably important.  Let's mark it as such
			return QColor("yellow");
		}
		if(process->uid == getuid()) {
			return QColor(216,212,255);
		}
		if(process->uid < 100 || !canUserLogin(process->uid))
			return QColor("gainsboro");
		return QColor("mediumaquamarine");
	}
	case Qt::FontRole: {
		if(index.column() == mCPUHeading) {
			Process *process = reinterpret_cast< Process * > (index.internalPointer());
			if(process->userUsage + process->sysUsage <= 0.1) {
				QFont font;
				font.setItalic(true);
				return font;
			}
		}
		return QVariant();
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
		pix = mIcons.loadIcon(iconname, K3Icon::Small,
						 K3Icon::SizeSmall, K3Icon::DefaultState,
						 0L, true);
		if (pix.isNull() || !pix.mask())
			pix = SmallIcon("unknownapp");

		if (pix.width() != 16 || pix.height() != 16)
		{
			/* I guess this isn't needed too often. The KIconLoader should
			 * scale the pixmaps already appropriately. Since I got a bug
			 * report claiming that it doesn't work with GNOME apps I've
			 * added this safeguard. */
			QImage img = pix.toImage();
			img.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			pix = QPixmap::fromImage( img );
		}
		/* We copy the icon into a 24x16 pixmap to add a 4 pixel margin on
		 * the left and right side. In tree view mode we use the original
		 * icon. */
/*		QPixmap icon(24, 16);
		if (!treeViewEnabled)
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
bool ProcessModel::setHeader(const QStringList &header, const QByteArray &coltype_) {
	//Argument examples:
	//header: (Name,PID,PPID,UID,GID,Status,User%,System%,Nice,VmSize,VmRss,Login,TracerPID,Command)
	//coltype_: sddddSffdDDsds

	mPidColumn = -1;  //We need to be able to access the pid directly, so remember which column it will be in
	mPPidColumn = -1; //Same, although this may not always be found :/
	mCPUHeading = -1;
	QStringList headings;
	QList<int> headingsToType;
	int num_of_others = 0; //Number of headings found that we don't know about.  Will match the index in process->data[index]
	QByteArray coltype;
	for(int i = 0; i < header.count(); i++) {
		bool other = false;
		if(header[i] == "Login") {
			coltype += DataColumnLogin;
		} else if(header[i] == "GID") {
			coltype += DataColumnGid;
		} else if(header[i] == "PID") {
			coltype += DataColumnPid;
			mPidColumn = i;
		} else if(header[i] == "PPID") {
			coltype += DataColumnPPid;
			mPPidColumn = i;
		} else if(header[i] == "UID") {
			headings.prepend(i18nc("process heading", "User Name"));   //The heading for the top of the qtreeview
			headingsToType.prepend(HeadingUser);
			coltype += DataColumnUid;
		} else if(header[i] == "Name") {
			coltype += DataColumnName;
		} else if(header[i] == "TracerPID") {
			coltype += DataColumnTracerPid;
		} else if(header[i] == "User%") {
			coltype += DataColumnUserUsage;
			headings << i18nc("process heading", "CPU %");
			headingsToType << HeadingCPUUsage;
			mCPUHeading = headingsToType.size();
		} else if(header[i] == "System%") {
			coltype += DataColumnSystemUsage;
		} else if(header[i] == "Nice") {
			coltype += DataColumnNice;
		} else if(header[i] == "VmSize") {
			coltype += DataColumnVmSize;
			headings << i18nc("process heading", "Virtual Size");
			headingsToType << HeadingMemory;
		} else if(header[i] == "VmRss") {
			coltype += DataColumnVmRss;
			headings << i18nc("process heading", "Memory");
			headingsToType << HeadingRSSMemory;
		} else if(header[i] == "Command") {
			coltype += DataColumnCommand;
			headings << i18nc("process heading", "Command");
			headingsToType << HeadingCommand;
		} else if(coltype_[i] == DATA_COLUMN_STATUS) {
			coltype += DataColumnStatus;
		} else if(coltype_[i] == DATA_COLUMN_LONG) {
			coltype += DataColumnOtherLong;
			other = true;
		} else if(coltype_[i] == DATA_COLUMN_PRETTY_LONG) {
			coltype += DataColumnOtherPrettyLong;
			other = true;
		} else if(coltype_[i] == DATA_COLUMN_PRETTY_FLOAT) {
			coltype += DataColumnOtherPrettyFloat;
			other = true;
		} else {
			coltype += DataColumnError;
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

	headings.prepend(i18nc("process heading", "Name"));
	headingsToType.prepend(HeadingName);

	kDebug() << "Adding " << header.count() << " columns: " << header <<endl;
	beginInsertColumns(QModelIndex(), 0, header.count()-1);
		mHeadingsToType = headingsToType;
		mColType = coltype;
		mHeadings = headings;
	endInsertColumns();
	reset();

	Q_ASSERT(mHeadingsToType.size() == mHeadings.size());

	return true;
}
bool ProcessModel::setXResHeader(const QStringList &header, const QByteArray &)
{
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

	beginInsertColumns(QModelIndex(), mHeadings.count(), mHeadings.count() + ((insertXMemory && insertXIdentifier)?1:0));
		if(insertXMemory) {
			mHeadings << i18nc("process heading", "Graphics Memory");
			mHeadingsToType << HeadingXMemory;
		}
		if(insertXIdentifier) {
			mHeadings << i18nc("process heading", "Application");
			mHeadingsToType << HeadingXIdentifier;
		}
	endInsertColumns();
	return true;
}
void ProcessModel::setXResData(long long pid, const QStringList& data)
{
	if(data.count() < mXResNumColumns) {
		kDebug(1215) << "Invalid data in setXResData. Not enough columns: " << data.join(",") << endl;
		return;
	}
	if(mXResPidColumn == -1) {
		kDebug(1215) << "XRes data received when we still don't know which column the XPid is in" << endl;
		return;
	}
	
	Process *process = mPidToProcess[pid];
	if(!process) {
		kDebug(1215) << "XRes Data for process with PID=" << pid << ",  which we don't know about" << endl;
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
	Process *parent_process = process->parent;
	Q_ASSERT(parent_process);

	int row = parent_process->children.indexOf(process);
	Q_ASSERT(row != -1);

	QModelIndex startIndex = createIndex(row, 0, process);
	QModelIndex endIndex = createIndex(row, mHeadings.count()-1, process);
	emit dataChanged(startIndex, endIndex);
}

void ProcessModel::setShowTotals(int totals)  //slot
{
	mShowChildTotals = totals;

	QModelIndex index;
	Process *process;
	
	QList<Process *> processes = mPidToProcess.values();
	for(int i = 0; i < processes.size(); i++) {
		process = processes.at(i);
		if (!process) {
			kDebug(1215) << "One of the processes in our QHash, mPidToProcess is invalid. We found it at index " << i << " after converting to a QList. QList<Process *> processes = mPidToProcess.values(). processes.size() was " << processes.size() << "." << endl;
		}
		else if(process->numChildren > 0) {
			int row = process->parent->children.indexOf(process);
			index = createIndex(row, mCPUHeading, process);
			emit dataChanged(index, index);
		}
	}
}
