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
#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <QBitmap>
#include <QFont>
#include <QIcon>
#include <QPixmap>
#include <QList>

#define HEADING_X_ICON_SIZE 16

#ifdef Q_WS_X11
#include <kwm.h>
#include <netwm.h>
#include <QtGui/QX11Info>
#include <X11/Xatom.h>
#include <kxerrorhandler.h>
#endif


#define GET_OWN_ID

#ifdef GET_OWN_ID
/* For getuid*/
#include <unistd.h>
#include <sys/types.h>
#endif

#include "ProcessModel.moc"
#include "ProcessModel.h"

#include "processcore/processes.h"



ProcessModel::ProcessModel(QObject* parent)
	: QAbstractItemModel(parent), mBlankPixmap(HEADING_X_ICON_SIZE,1)
{
	mBlankPixmap.fill(QColor(0,0,0,0));
	mSimple = true;
	mIsLocalhost = true;
	mMemTotal = -1;
	
	mShowChildTotals = true;
	mIsChangingLayout = false;

	setupProcesses();
	setupHeader();

	setupWindows();
}

void ProcessModel::windowRemoved(WId wid) {
#ifdef Q_WS_X11
	long long pid = mWIdToPid.value(wid, 0);
	kDebug() << "Window removed" << endl;
	if(pid <= 0) return;

	kDebug() << "Window removed  pid is "  << pid << endl;
	int count = mPidToWindowInfo.count(pid);
	QMultiHash<long long, WindowInfo>::iterator i = mPidToWindowInfo.find(pid);
	while (i != mPidToWindowInfo.end() && i.key() == pid) {
		kDebug() << "window has wid "  << wid << endl;
		if(i.value().wid == wid) {
			kDebug() << "Found a window "  << pid << endl;
			i = mPidToWindowInfo.erase(i);
//			break;
		} else
			i++;
	}
	kDebug() << "Count before: " << count << " and after:" << mPidToWindowInfo.count(pid) << endl;
	Q_ASSERT(count-1 == mPidToWindowInfo.count(pid) || count == mPidToWindowInfo.count(pid));
        KSysGuard::Process *process = mProcesses->getProcess(pid);
        if(!process) return;
	
	kDebug() << "updating "  << pid << endl;

	int row;
	if(mSimple)
		row = process->index;
	else
		row = process->parent->children.indexOf(process);
	QModelIndex index1 = createIndex(row, HeadingName, process);
	emit dataChanged(index1, index1);
	QModelIndex index2 = createIndex(row, HeadingXTitle, process);
	emit dataChanged(index2, index2);
#endif
}

void ProcessModel::setupWindows() {
#ifdef Q_WS_X11
	QList<WId>::ConstIterator it;
	for ( it = KWM::windows().begin(); it != KWM::windows().end(); ++it ) {
		windowAdded(*it);
	}

	connect( KWM::self(), SIGNAL( windowChanged (WId, unsigned int )), this, SLOT(windowChanged(WId, unsigned int)));
	connect( KWM::self(), SIGNAL( windowAdded (WId )), this, SLOT(windowAdded(WId)));
	connect( KWM::self(), SIGNAL( windowRemoved (WId )), this, SLOT(windowRemoved(WId)));
#endif
}

void ProcessModel::setupProcesses() {
	if(mProcesses)
		KSysGuard::Processes::returnInstance();

	mProcesses = KSysGuard::Processes::getInstance();  //For now, hard code in a local instance

        connect( mProcesses, SIGNAL( processChanged(KSysGuard::Process *, bool)), this, SLOT(processChanged(KSysGuard::Process *, bool)));
	connect( mProcesses, SIGNAL( beginAddProcess(KSysGuard::Process *)), this, SLOT(beginInsertRow( KSysGuard::Process *)));
        connect( mProcesses, SIGNAL( endAddProcess()), this, SLOT(endInsertRow()));
	connect( mProcesses, SIGNAL( beginRemoveProcess(KSysGuard::Process *)), this, SLOT(beginRemoveRow( KSysGuard::Process *)));
        connect( mProcesses, SIGNAL( endRemoveProcess()), this, SLOT(endRemoveRow()));
        connect( mProcesses, SIGNAL( beginMoveProcess(KSysGuard::Process *, KSysGuard::Process *)), this, 
			       SLOT( beginMoveProcess(KSysGuard::Process *, KSysGuard::Process *)));
        connect( mProcesses, SIGNAL( endMoveProcess()), this, SLOT(endMoveRow()));
	mMemTotal = mProcesses->totalPhysicalMemory();
	update();
}

#ifdef Q_WS_X11
void ProcessModel::windowChanged(WId wid, unsigned int properties)
{
	if(! (properties & NET::WMVisibleName || properties & NET::WMName || properties & NET::WMIcon || properties & NET::WMState)) return;
	windowAdded(wid);

}

void ProcessModel::windowAdded(WId wid)
{
	foreach(WindowInfo w, mPidToWindowInfo.values()) {
		if(w.wid == wid) return; //already added
	}
	//The name changed
	KXErrorHandler handler;
	NETWinInfo *info = new NETWinInfo( QX11Info::display(), wid, QX11Info::appRootWindow(), 
			NET::WMPid | NET::WMVisibleName | NET::WMName | NET::WMState );
	long unsigned state = info->state();
	if(/*state & NET::SkipTaskbar || state & NET::SkipPager || */state & NET::Hidden) return;

	if(handler.error( false ) )
		return;  //info is invalid - window just closed or something probably
	long long pid = info->pid();
        if(pid <= 0)
                return;

	WindowInfo w;
	w.icon = KWM::icon(wid, HEADING_X_ICON_SIZE, HEADING_X_ICON_SIZE, true);
	w.wid = wid;
	w.netWinInfo = info;
	mPidToWindowInfo.insertMulti(pid, w);
	mWIdToPid[wid] = pid;

	KSysGuard::Process *process = mProcesses->getProcess(pid);
        if(!process) return; //shouldn't really happen.. maybe race condition etc
	int row;
	if(mSimple)
		row = process->index;
	else
		row = process->parent->children.indexOf(process);
	QModelIndex index1 = createIndex(row, HeadingName, process);
	emit dataChanged(index1, index1);
	QModelIndex index2 = createIndex(row, HeadingXTitle, process);
	emit dataChanged(index2, index2);
}
#endif

void ProcessModel::update(int updateDurationMS) {
//	kDebug() << "update all processes: " << QTime::currentTime().toString("hh:mm:ss.zzz") << endl;
	mProcesses->updateAllProcesses(updateDurationMS);
	if(mIsChangingLayout) {
		mIsChangingLayout = false;
		emit layoutChanged();
	}
//	kDebug() << "finished:             " << QTime::currentTime().toString("hh:mm:ss.zzz") << endl;
}

QString ProcessModel::getStatusDescription(KSysGuard::Process::ProcessStatus status) const
{
	switch( status) {
		case KSysGuard::Process::Running:
			return i18n("- Process is doing some work");
		case KSysGuard::Process::Sleeping:
			return i18n("- Process is waiting for something to happen");
		case KSysGuard::Process::Stopped:
			return i18n("- Process has been stopped. It will not respond to user input at the moment");
		case KSysGuard::Process::Zombie:
			return i18n("- Process has finished and is now dead, but the parent process has not cleaned up");
		default:
			return QString();
	}
}

KSysGuard::Process *ProcessModel::getProcessAtIndex(int index) const
{
	Q_ASSERT(mSimple);
	return mProcesses->getAllProcesses().at(index);
}
int ProcessModel::rowCount(const QModelIndex &parent) const
{
	if(mSimple) {
		if(parent.isValid()) return 0; //In flat mode, none of the processes have children
		return mProcesses->getAllProcesses().count();
	}

	//Deal with the case that we are showing it as a tree
	KSysGuard::Process *process;
	if(parent.isValid()) {
		if(parent.column() != 0) return 0;  //For a treeview we say that only the first column has children
		process = reinterpret_cast< KSysGuard::Process * > (parent.internalPointer()); //when parent is invalid, it must be the root level which we set as 0
	} else {
		process = mProcesses->getProcess(0);
	}
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

	if(mSimple) {
		if(parent.isValid()) return 0; //In flat mode, none of the processes have children
		return !mProcesses->getAllProcesses().isEmpty();
	}

	//Deal with the case that we are showing it as a tree
	KSysGuard::Process *process;
	if(parent.isValid()) {
		if(parent.column() != 0) return false;  //For a treeview we say that only the first column has children
		process = reinterpret_cast< KSysGuard::Process * > (parent.internalPointer()); //when parent is invalid, it must be the root level which we set as 0
	} else {
		process = mProcesses->getProcess(0);
	}
	Q_ASSERT(process);
	bool has_children = !process->children.isEmpty();

	Q_ASSERT((rowCount(parent) > 0) == has_children);
	return has_children;
}

QModelIndex ProcessModel::index ( int row, int column, const QModelIndex & parent ) const
{
	if(row<0) return QModelIndex();
	if(column<0 || column >= mHeadings.count() ) return QModelIndex();

	if(mSimple) {
		if( parent.isValid()) return QModelIndex();
		if( mProcesses->getAllProcesses().count() <= row) return QModelIndex();
		return createIndex( row, column, mProcesses->getAllProcesses().at(row));
	}

	//Deal with the case that we are showing it as a tree
	KSysGuard::Process *parent_process = 0;
	
	if(parent.isValid()) //not valid for init, and init has ppid of 0
		parent_process = reinterpret_cast< KSysGuard::Process * > (parent.internalPointer());
	else
		parent_process = mProcesses->getProcess(0);
	Q_ASSERT(parent_process);

	if(parent_process->children.count() > row)
		return createIndex(row,column, parent_process->children[row]);
	else
	{
		return QModelIndex();
	}
}

void ProcessModel::processChanged(KSysGuard::Process *process, bool onlyCpuOrMem)
{
	//FIXME
	//There seem to be lots of bugs with qtreeview.  The idea here was to only update the actual rows
	//that need changing.  However this seems to actually repaint completely the wrong cell.  I can only
	//blame qt for this.
	//
	//So instead we will not call dataChanged here.  Instead after update, we will expand the first column.
	//For some unknown reason (another bug?) this causes all the rows to be repainted, which is
	//good enough for us

	return;

	int row;
	if(mSimple)
		row = process->index;
	else
		row = process->parent->children.indexOf(process);

	Q_ASSERT(row != -1);  //Something has gone very wrong
	if(!onlyCpuOrMem) {
		//Only the cpu usage changed, so only update that
		QModelIndex index = createIndex(row, HeadingCPUUsage, process);
		QModelIndex index2 = createIndex(row, HeadingSharedMemory, process);
		emit dataChanged(index, index2);
	} else {
		QModelIndex startIndex = createIndex(row, 0, process);
		QModelIndex endIndex = createIndex(row, mHeadings.count()-1, process);
		emit dataChanged(startIndex, endIndex);
	}
}

void ProcessModel::beginInsertRow( KSysGuard::Process *process)
{
	Q_ASSERT(process);
	if(mIsChangingLayout) {
		mIsChangingLayout = false;
		emit layoutChanged();
	}

	if(mSimple) {
		int row = mProcesses->getAllProcesses().count();
		beginInsertRows( QModelIndex(), row, row );
		return;
	}

	//Deal with the case that we are showing it as a tree
	int row = process->parent->children.count();
	QModelIndex parentModelIndex = getQModelIndex(process->parent, 0);

	//Only here can we actually change the model.  First notify the view/proxy models then modify
	beginInsertRows(parentModelIndex, row, row);
}
void ProcessModel::endInsertRow() {
	endInsertRows();
}
void ProcessModel::beginRemoveRow( KSysGuard::Process *process )
{
	if(mIsChangingLayout) {
		mIsChangingLayout = false;
		emit layoutChanged();
	}

	Q_ASSERT(process);
	Q_ASSERT(process->pid > 0);
	
	if(mSimple) {
		return beginRemoveRows(QModelIndex(), process->index, process->index);
	} else  {
		int row = process->parent->children.indexOf(process);
		if(row == -1) {
			kDebug(1215) << "A serious problem occurred in remove row." << endl;
			return;
		}

		return beginRemoveRows(getQModelIndex(process->parent,0), row, row);
	}
}
void ProcessModel::endRemoveRow() 
{
	endRemoveRows();
}


void ProcessModel::beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent)
{
	if(mSimple) return;  //We don't need to move processes when in simple mode
	if(!mIsChangingLayout) {
		emit layoutAboutToBeChanged ();
		mIsChangingLayout = true;
	}

	//FIXME
	int current_row = process->parent->children.indexOf(process);
	int new_row = new_parent->children.count();
	Q_ASSERT(current_row != -1);

	QList<QModelIndex> fromIndexes;
	QList<QModelIndex> toIndexes;
        for(int i=0; i < columnCount(); i++) {
		fromIndexes << createIndex(current_row, i, process);
		toIndexes << createIndex(new_row, i, process);
	}
	changePersistentIndexList(fromIndexes, toIndexes);
}
void ProcessModel::endMoveRow() 
{
}


QModelIndex ProcessModel::getQModelIndex( KSysGuard::Process *process, int column) const
{
	Q_ASSERT(process);
	int pid = process->pid;
	if(pid == 0) return QModelIndex(); //pid 0 is our fake process meaning the very root (never drawn).  To represent that, we return QModelIndex() which also means the top element
	int row = 0;
        if(mSimple) {
		row = process->index;
	} else {
		row = process->parent->children.indexOf(process);
	}
	Q_ASSERT(row != -1);
	return createIndex(row, column, process);
}

QModelIndex ProcessModel::parent ( const QModelIndex & index ) const
{
	if(!index.isValid()) return QModelIndex();
	KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
	Q_ASSERT(process);

        if(mSimple)
		return QModelIndex();
	else 
		return getQModelIndex(process->parent,0);
}

QVariant ProcessModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if(orientation != Qt::Horizontal)
		return QVariant();
	if(section < 0 || section >= mHeadings.count())
		return QVariant(); //is this needed?
	return mHeadings[section];
}
void ProcessModel::setSimpleMode(bool simple)
{ 
	if(mSimple == simple) return;

	if(!mIsChangingLayout) {
		emit layoutAboutToBeChanged ();
	}

	mSimple = simple;
	mIsChangingLayout = false;

	int flatrow;
	int treerow;
	QList<QModelIndex> flatIndexes;
	QList<QModelIndex> treeIndexes;
        foreach( KSysGuard::Process *process, mProcesses->getAllProcesses()) {
		flatrow = process->index;
		treerow = process->parent->children.indexOf(process);
		flatIndexes.clear();
		treeIndexes.clear();
	
	        for(int i=0; i < columnCount(); i++) {
			flatIndexes << createIndex(flatrow, i, process);
			treeIndexes << createIndex(treerow, i, process);
		}
		if(mSimple) //change from tree mode to flat mode
			changePersistentIndexList(treeIndexes, flatIndexes);
		else // change from flat mode to tree mode
			changePersistentIndexList(flatIndexes, treeIndexes);
	}
	emit layoutChanged();

}

bool ProcessModel::canUserLogin(long long uid ) const
{
	if(uid == 65534) {
		//nobody user
		return false;
	}

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

QString ProcessModel::getTooltipForUser(const KSysGuard::Process *ps) const
{
	QString userTooltip;
	if(!mIsLocalhost) {
		userTooltip = "<qt>";
		userTooltip += i18n("Login Name: %1<br/>", getUsernameForUser(ps->uid, true));
	} else {
		KUser user(ps->uid);
		if(!user.isValid())
			userTooltip = i18n("This user is not recognised for some reason");
		else {
			userTooltip = "<qt>";
			if(!user.fullName().isEmpty()) userTooltip += i18n("<b>%1</b><br/>", user.fullName());
			userTooltip += i18n("Login Name: %1 (uid: %2)<br/>", user.loginName(), ps->uid);
			if(!user.roomNumber().isEmpty()) userTooltip += i18n("  Room Number: %1<br/>", user.roomNumber());
			if(!user.workPhone().isEmpty()) userTooltip += i18n("  Work Phone: %1<br/>", user.workPhone());
		}
	}
	if( (ps->uid != ps->euid && ps->euid != -1) || 
                   (ps->uid != ps->suid && ps->suid != -1) || 
                   (ps->uid != ps->fsuid && ps->fsuid != -1)) {
		if(ps->euid != -1) 
			userTooltip += i18n("Effective User: %1<br/>", getUsernameForUser(ps->euid, true));
		if(ps->suid != -1)
			userTooltip += i18n("Setuid User: %1<br/>", getUsernameForUser(ps->suid, true));
		if(ps->fsuid != -1)
			userTooltip += i18n("File System User: %1<br/>", getUsernameForUser(ps->fsuid, true));
		userTooltip += "<br/>";
	}
	if(ps->gid != -1) {
		userTooltip += i18n("Group: %1", getGroupnameForGroup(ps->gid));
		if( (ps->gid != ps->egid && ps->egid != -1) || 
	                   (ps->gid != ps->sgid && ps->sgid != -1) || 
	                   (ps->gid != ps->fsgid && ps->fsgid != -1)) {
			if(ps->egid != -1) 
				userTooltip += i18n("<br/>Effective Group: %1", getGroupnameForGroup(ps->egid));
			if(ps->sgid != -1)
				userTooltip += i18n("<br/>Setuid Group: %1", getGroupnameForGroup(ps->sgid));
			if(ps->fsgid != -1)
				userTooltip += i18n("<br/>File System Group: %1", getGroupnameForGroup(ps->fsgid));
		}
	}
	return userTooltip;
}

QString ProcessModel::getStringForProcess(KSysGuard::Process *process) const {
	return i18nc("Short description of a process. PID, name, user", "%1: %2, owned by user %3", (long)(process->pid), process->name, getUsernameForUser(process->uid, false));
}

QString ProcessModel::getGroupnameForGroup(long long gid) const {
	if(mIsLocalhost) {
		QString groupname = KUserGroup(gid).name();
		if(!groupname.isEmpty())
			return i18n("%1 (gid: %2)", groupname, gid);
	}
	return QString::number(gid);
}

QString ProcessModel::getUsernameForUser(long long uid, bool withuid) const {
	QString &username = mUserUsername[uid];
	if(username.isNull()) {
		if(!mIsLocalhost) {
			username = ""; //empty, but not null
		} else {
			KUser user(uid);
			if(!user.isValid())
				username = "";
			else
				username = user.loginName();
		}
	}
	if(username.isEmpty()) 
		return QString::number(uid);
	if(withuid)
		return i18n("%1 (uid: %2)", username, (long int)uid);
	return username;
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
	//This function must be super duper ultra fast because it's called thousands of times every few second :(
	//I think it should be optomised for role first, hence the switch statement (fastest possible case)

	if (!index.isValid()) {
		return QVariant();
	}
	if (index.column() >= mHeadings.count()) {
		return QVariant(); 
	}

	switch (role){
	case Qt::DisplayRole: {
		KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
		switch(index.column()) {
		case HeadingName:
			return process->name;
		case HeadingUser:
			if(process->uid == process->euid)
				return getUsernameForUser(process->uid, false);
			else
				return getUsernameForUser(process->uid, false) + ", " + getUsernameForUser(process->euid, false);
		case HeadingNiceness:
			return process->niceLevel;
		case HeadingTty:
			return process->tty;
		case HeadingCPUUsage:
			{
				double total;
				if(mShowChildTotals && !mSimple) total = process->totalUserUsage + process->totalSysUsage;
				else total = process->userUsage + process->sysUsage;

				if(total < 1 && process->status != KSysGuard::Process::Sleeping && process->status != KSysGuard::Process::Running)
					return process->translatedStatus();  //tell the user when the process is a zombie or stopped
				if(total < 1)
					return "";

				return QString::number(total, 'f', (total>=1)?0:1) + '%';
			}
		case HeadingMemory:
			if(process->vmRSS == 0) return QVariant(QVariant::String);
			if(process->vmURSS == -1) {
				//If we don't have the URSS (the memory used by only the process, not the shared libraries)
				//then return the RSS (physical memory used by the process + shared library) as the next best thing
				return KGlobal::locale()->formatLong(process->vmRSS) + " k";
//				return KGlobal::locale()->formatByteSize(process->vmRSS * 1024);
			} else {
				return KGlobal::locale()->formatLong(process->vmURSS) + " k";
//				return KGlobal::locale()->formatByteSize(process->vmURSS * 1024);
			}
		case HeadingVmSize:
			if(process->vmSize == 0) return QVariant(QVariant::String);
			return KGlobal::locale()->formatLong(process->vmSize) + " k";
			return KGlobal::locale()->formatByteSize(process->vmSize * 1024);
		case HeadingSharedMemory:
			if(process->vmRSS - process->vmURSS <= 0 || process->vmURSS == -1) return QVariant(QVariant::String);
			return KGlobal::locale()->formatLong(process->vmRSS - process->vmURSS) + " k";
//			return KGlobal::locale()->formatByteSize( (process->vmRSS-process->vmURSS) * 1024);
		case HeadingCommand: 
			{
				return process->command;
// It would be nice to embolden the process name in command, but this requires that the itemdelegate to support html text
//				QString command = process->command;
//				command.replace(process->name, "<b>" + process->name + "</b>");
//				return "<qt>" + command;
			}
#ifdef Q_WS_X11
		case HeadingXTitle:
			{
				if(!mPidToWindowInfo.contains(process->pid)) return QVariant();
				WindowInfo w = mPidToWindowInfo.value(process->pid);
				if(!w.netWinInfo) return QVariant();
				const char *name = w.netWinInfo->visibleName();
				if( !name || name[0] == 0 )
					name = w.netWinInfo->name();
				if(name && name[0] != 0)
					return QString::fromUtf8(name);
				return QVariant();
			}
#endif
		default:
			return QVariant();
		}
		break;
	}
	case Qt::ToolTipRole: {
		KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
		QString tracer;
		if(process->tracerpid > 0) {
			KSysGuard::Process *process_tracer = mProcesses->getProcess(process->tracerpid);
			if(process_tracer) { //it is possible for this to be not the case in certain race conditions
				KSysGuard::Process *process_tracer = mProcesses->getProcess(process->tracerpid);
				tracer = i18nc("tooltip. name,pid ","This process is being debugged by %1 (%2)", process_tracer->name, (long int)process->tracerpid);
			}
		}
		switch(index.column()) {
		case HeadingName: {
			QString tooltip;
			/***  It would be nice to be able to show the icon in the tooltip, but Qt4 won't let us put
			 *    a picture in a tooltip :(
			   
			QIcon icon;
			if(mPidToWindowInfo.contains(process->pid)) {
				WId wid;
				wid = mPidToWindowInfo[process->pid].wid;
				icon = KWM::icon(wid);
			}
			if(icon.isValid()) {
				tooltip = i18n("<qt><table><tr><td>%1", icon);
			}
			*/
			if(process->parent_pid == 0)
				tooltip	= i18nc("name column tooltip. first item is the name","<qt><b>%1</b><br/>Process ID: %2<br/>Command: %3", process->name, (long int)process->pid, process->command);
			else
				tooltip	= i18nc("name column tooltip. first item is the name","<qt><b>%1</b><br/>Process ID: %2<br/>Parent's ID: %3<br/>Command: %4", process->name, (long int)process->pid, (long int)process->parent_pid, process->command);
			if(!process->tty.isEmpty())
				tooltip += i18n( "<br/>Running on: %1", QString(process->tty));
			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}

		case HeadingCommand: {
			QString tooltip =
				i18n("<qt>This process was run with the following command:<br/>%1", process->command);
			if(!process->tty.isEmpty())
				tooltip += i18n( "<br/><br/>Running on: %1", QString(process->tty));
		        if(tracer.isEmpty()) return tooltip;
			return tooltip + "<br/>" + tracer;
		}
		case HeadingUser: {
			if(!tracer.isEmpty())
				return getTooltipForUser(process) + "<br/>" + tracer;
			return getTooltipForUser(process);
		}
		case HeadingNiceness:
			return i18n("Nice level: %1 (%2)", process->niceLevel, process->niceLevelAsString() );
		case HeadingCPUUsage: {
			QString tooltip = ki18n("<qt>Process status: %1 %2<br/>"
						"User CPU usage: %3%<br/>System CPU usage: %4%")
						.subs(process->translatedStatus())
						.subs(getStatusDescription(process->status))
						.subs(process->userUsage)
						.subs(process->sysUsage)
						.toString();

			if(process->numChildren > 0) {
				tooltip += ki18n("<br/>Number of children: %1<br/>Total User CPU usage: %2%<br/>"
						"Total System CPU usage: %3%<br/>Total CPU usage: %4%")
						.subs(process->numChildren)
						.subs(process->totalUserUsage)
						.subs(process->totalSysUsage)
						.subs(process->totalUserUsage + process->totalSysUsage)
						.toString();
			}
			if(process->userTime > 0) 
				tooltip += ki18n("<br/><br/>CPU time spent running as user: %1 seconds")
						.subs(process->userTime / 100.0, 0, 'f', 1)
						.toString();
			if(process->sysTime > 0) 
				tooltip += ki18n("<br/>CPU time spent running in kernel: %1 seconds")
						.subs(process->sysTime / 100.0, 0, 'f', 1)
						.toString();
			if(process->niceLevel != 0)
				tooltip += i18n("<br/>Nice level: %1 (%2)", process->niceLevel, process->niceLevelAsString() );

			if(!tracer.isEmpty())
				return tooltip + "<br/>" + tracer;
			return tooltip;
		}
		case HeadingVmSize: {
			QString tooltip = i18n("<qt>This is the amount of virtual memory space that the process is using, included shared libraries, graphics memory, files on disk, and so on.  This number is almost meaningless.");
			return tooltip;
		}
		case HeadingMemory: {
			QString tooltip;
			if(process->vmURSS == -1)
				//We don't have information about the URSS, so just fallback to RSS
				tooltip = i18n("<qt>This is the amount of real physical memory that this process is using.  It does not include any swapped out memory, but does include the code size for shared libraries etc.<br/><br/>");
			else {
				tooltip = i18n("<qt>This is the amount of real physical memory that this process is using by itself.  It does not include any swapped out memory, nor the code size of its shared libraries.  This is often the most useful figure to judge the memory use of a program.<br/><br/>");
				if(mMemTotal > 0)
					tooltip += i18n("Memory usage: %1 out of %2  (%3 %)<br/>", KGlobal::locale()->formatByteSize(process->vmURSS * 1024), KGlobal::locale()->formatByteSize(mMemTotal*1024), process->vmURSS*100/mMemTotal);
				else
					tooltip += i18n("Memory usage: %1<br/>", KGlobal::locale()->formatByteSize(process->vmURSS * 1024));
			}
			if(mMemTotal > 0)
				tooltip += i18n("RSS Memory usage: %1 out of %2  (%3 %)", KGlobal::locale()->formatByteSize(process->vmRSS * 1024), KGlobal::locale()->formatByteSize(mMemTotal*1024), process->vmRSS*100/mMemTotal);
			else
				tooltip += i18n("RSS Memory usage: %1", KGlobal::locale()->formatByteSize(process->vmRSS * 1024));
			return tooltip;
		}
		case HeadingSharedMemory: {
			if(process->vmURSS == -1)
				return i18n("<qt>Your system does not seem to have this information for us to read, sorry.");
			QString tooltip = i18n("<qt>This is the amount of real physical memory that this process's shared libraries are using.  This memory is shared among all processes that use this library");
			if(mMemTotal >0)
				tooltip += i18n("<br/><br/>Shared library memory usage: %1 out of %2  (%3 %)", KGlobal::locale()->formatByteSize((process->vmRSS - process->vmURSS) * 1024), KGlobal::locale()->formatByteSize(mMemTotal*1024), (process->vmRSS-process->vmURSS)*100/mMemTotal);
			else
				tooltip += i18n("<br/><br/>Shared library memory usage: %1", KGlobal::locale()->formatByteSize((process->vmRSS - process->vmURSS) * 1024));

			return tooltip;
		}
		case HeadingXTitle: {
			QString tooltip;
			QList<WindowInfo> values = mPidToWindowInfo.values(process->pid);
			if(values.isEmpty()) return QVariant(QVariant::String);
			for(int i = 0; i< values.size(); i++) {
				WindowInfo w = values[i];
				if(w.netWinInfo) {
	                                const char *name = w.netWinInfo->visibleName();
	                                if( !name || name[0] == 0 )
		                                name = w.netWinInfo->name();
	        	                if(name && name[0] != 0) {
						if( i==0 && values.size()==1)
							return QString::fromUtf8(name);
		                                tooltip += "<li>" + QString::fromUtf8(name) + "</li>";
					}
				}
			}
			if(!tooltip.isEmpty())
				return "<qt><ul>" + tooltip + "</ul>";
			return QVariant(QVariant::String);
		}

		default:
			return QVariant(QVariant::String);
		}
	}
	case Qt::TextAlignmentRole:
		switch(index.column() ) {
			case HeadingUser:
			case HeadingCPUUsage:
				return QVariant(Qt::AlignCenter);
			case HeadingMemory:
			case HeadingSharedMemory:
			case HeadingVmSize:
				return QVariant(Qt::AlignRight);
		}
		return QVariant();
	case Qt::UserRole: {
		//We have a special understanding with the filter.  If it queries us as UserRole in column 0, return uid
		if(index.column() != 0) return QVariant();  //If we query with this role, then we want the raw UID for this.
		KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
		return process->uid;
	}

	case Qt::UserRole+1: {
		//We have a special understanding with the filter sort. This returns an int (in a qvariant) that can be sorted by
		KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
		Q_ASSERT(process);
		switch(index.column()) {
		case HeadingUser: {
			//Sorting by user will be the default and the most common.
			//We want to sort in the most useful way that we can. We need to return a number though.
			//This code is based on that sorting ascendingly should put the current user at the top
			//
			//First the user we are running as should be at the top.  We add 0 for this
			//Then any other users in the system.  We add 100,000,000 for this (remember it's ascendingly sorted)
			//Then at the bottom the 'system' processes.  We add 200,000,000 for this

			//One special exception is a traced process since that's probably important. We should put that at the top
			//
			//We subtract the uid to sort ascendingly by that, then subtract the cpu usage to sort by that, then finally subtract the memory
			if(process->tracerpid >0) return (long long)0;
			
			long long base = 0;
			long long memory = 0;
			if(process->vmURSS != -1) memory = process->vmURSS;
			else memory = process->vmRSS;
			if(process->uid == getuid())
				base = 0;
			else if(process->uid < 100 || !canUserLogin(process->uid))
				base = 200000000 - process->uid * 10000;
			else
				base = 100000000 - process->uid * 10000;
			int cpu;
			if(mSimple || !mShowChildTotals)
				cpu = process->userUsage + process->sysUsage;
			else
				cpu = process->totalUserUsage + process->totalSysUsage;
			if(cpu == 0 && process->status != KSysGuard::Process::Running && process->status != KSysGuard::Process::Sleeping) 
				cpu = 1;  //stopped or zombied processes should be near the top of the list
			bool hasWindow = mPidToWindowInfo.contains(process->pid);
			//However we can of course have lots of processes with the same user.  Next we sort by CPU.
			return (long long)(base - (cpu*100) -(hasWindow?50:0) - memory*100/mMemTotal);
		}
		case HeadingCPUUsage: {
			int cpu;
			if(mSimple || !mShowChildTotals)
				cpu = process->userUsage + process->sysUsage;
			else
				cpu = process->totalUserUsage + process->totalSysUsage;
			if(cpu == 0 && process->status != KSysGuard::Process::Running && process->status != KSysGuard::Process::Sleeping) 
				cpu = 1;  //stopped or zombied processes should be near the top of the list
			return -cpu;
		 }
		case HeadingMemory:
			if(process->vmURSS == -1) 
				return (long long)-process->vmRSS;
			else
				return (long long)-process->vmURSS;
		case HeadingVmSize:
			return (long long)-process->vmSize;
		case HeadingSharedMemory:
			if(process->vmURSS == -1) return (long long)0;
			return (long long)-(process->vmRSS - process->vmURSS);
		}
		return QVariant();
	}
	case Qt::UserRole+2: {
		//Return an int here to draw a percentage bar in the table
		if(index.column() == HeadingCPUUsage) {
			KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
			return (int)(process->userUsage + process->sysUsage);
		}else if(index.column() == HeadingMemory) {
                        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
			long long memory = 0;
			if(process->vmURSS != -1) memory = process->vmURSS;
			else memory = process->vmRSS;
			return (int)(memory*100/mMemTotal);
		} else if(index.column() == HeadingSharedMemory) {
                        KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
			if(process->vmURSS == -1) return 0;
			return (int)((process->vmRSS - process->vmURSS)*100/mMemTotal);
		}
		return 0;
	}
	case Qt::DecorationRole: {
		if(index.column() == HeadingName) {
			KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
			if(!mPidToWindowInfo.contains(process->pid)) return QIcon(mBlankPixmap);
			WindowInfo w = mPidToWindowInfo.value(process->pid);
			return w.icon;

		} else if (index.column() == HeadingCPUUsage) {
			KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
			if(process->status == KSysGuard::Process::Stopped || process->status == KSysGuard::Process::Zombie) {
        			QPixmap pix = KIconLoader::global()->loadIcon("button_cancel", K3Icon::Small,
			                K3Icon::SizeSmall, K3Icon::DefaultState,
				        0L, true);

			}
		}
		return QVariant();
	}
	case Qt::BackgroundRole: {
                if(index.column() != HeadingUser) return QVariant();
		KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
		if(process->tracerpid >0) {
			//It's being debugged, so probably important.  Let's mark it as such
			return QColor("yellow");
		}
		if(mIsLocalhost && process->uid == getuid()) { //own user
			return QColor(0, 208, 214, 50);
		}
		if(process->uid < 100 || !canUserLogin(process->uid))
			return QColor(218, 220,215, 50); //no color for system tasks
		//other users
		return QColor(2, 154, 54, 50);
	}
	case Qt::FontRole: {
		if(index.column() == HeadingCPUUsage) {
			KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (index.internalPointer());
			if(process->userUsage == 0) {
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

bool ProcessModel::isLocalhost() const
{
	return mIsLocalhost;
}


void ProcessModel::setupHeader() {
	QStringList headings;
	headings << i18nc("process heading", "Name");
	headings << i18nc("process heading", "User Name");
	headings << i18nc("process heading", "Tty");
	headings << i18nc("process heading", "Niceness");
	headings << i18nc("process heading", "CPU %");
	headings << i18nc("process heading", "Virtual Size");
	headings << i18nc("process heading", "Memory");
	headings << i18nc("process heading", "Shared Mem");
	headings << i18nc("process heading", "Command");
#ifdef Q_WS_X11
	headings << i18nc("process heading", "Title Name");
#endif

	beginInsertColumns(QModelIndex(), 0, headings.count()-1);
		mHeadings = headings;
	endInsertColumns();
}

KSysGuard::Process *ProcessModel::getProcess(long long pid) {
	return mProcesses->getProcess(pid);
}

void ProcessModel::setShowTotals(bool showTotals)  //slot
{
        if(showTotals == mShowChildTotals) return;
        mShowChildTotals = showTotals;

        QModelIndex index;
        foreach( KSysGuard::Process *process, mProcesses->getAllProcesses()) {
               if(process->numChildren > 0) {
                       int row;
		       if(mSimple)
		               row = process->index;
		       else
		               row = process->parent->children.indexOf(process);
                       index = createIndex(row, HeadingCPUUsage, process);
                       emit dataChanged(index, index);
               }
       }
}

