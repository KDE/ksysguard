/*
    KSysGuard, the KDE System Guard

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


/* For getuid() */
#include <unistd.h>
#include <sys/types.h>

#include <QVariant>

#include <kdebug.h>

#include "ProcessModel.h"
#include "ProcessFilter.h"

bool ProcessFilter::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const
{
	//We need the uid for this, so we have a special understanding with the model.
	//We query the first row with Qt:UserRole, and it gives us the uid.  Nasty but works.
	if(mFilter == PROCESS_FILTER_ALL && filterRegExp().isEmpty()) return true; //Shortcut for common case 
	
	Process *parent_process;
	ProcessModel *model = static_cast<ProcessModel *>(sourceModel());
	if(source_parent.isValid()) {
		parent_process = reinterpret_cast<Process *>(source_parent.internalPointer());
	} else {
		parent_process = model->getProcess(0); //Get our 'special' process which should have the root init child
	}
        Q_ASSERT(parent_process);
	if(source_row >= parent_process->children.size()) {
		kDebug() << "Serious error with data.  Source row requested for a non existant row. Requested " << source_row << " of " << parent_process->children.size() << " for " << parent_process->pid << endl;
		return true;
	}
	const Process *process = parent_process->children.at(source_row);
	Q_ASSERT(process);
	long uid = process->uid;
	
	bool accepted = true;
	switch(mFilter) {
	case PROCESS_FILTER_ALL:
		break;
        case PROCESS_FILTER_SYSTEM:
                if(uid >= 100 && model->canUserLogin(uid)) accepted = false;
		break;
        case PROCESS_FILTER_USER:
		if(uid < 100 || !model->canUserLogin(uid)) accepted = false;
		break;
        case PROCESS_FILTER_OWN:
        default:
                if(uid != (long) getuid()) accepted = false;
        }

	if(accepted) { 
		if(filterRegExp().isEmpty()) return true;
		
		//Allow the user to search by PID
		if(QString::number(process->pid).contains(filterRegExp())) return true;

		//None of our tests have rejected it.  Pass it on to qsortfilterproxymodel's filter	
		if(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
			return true;
	}


	//We did not accept this row at all.  However one of our children might be accepted, so accept this row if our children are accepted.
	QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);
	for(int i = 0 ; i < sourceModel()->rowCount(source_index); i++) {
		if(filterAcceptsRow(i, source_index)) return true;
	}
	return false;
}

bool ProcessFilter::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	if(right.isValid() && left.isValid()) {
		Q_ASSERT(left.model());
		Q_ASSERT(right.model());
		QVariant l = (left.model() ? left.model()->data(left, Qt::UserRole+1) : QVariant());
		QVariant r = (right.model() ? right.model()->data(right, Qt::UserRole+1) : QVariant());
		if(l.isValid() && r.isValid() && !l.isNull() && !r.isNull())
			return l.toLongLong() < r.toLongLong();
	}
	return QSortFilterProxyModel::lessThan(left,right);
}


void ProcessFilter::setFilter(int index) {
	mFilter = index; 
	filterChanged();//Tell the proxy view to refresh all its information
}
#include "ProcessFilter.moc"
