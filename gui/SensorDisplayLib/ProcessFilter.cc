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
	if(source_parent.isValid()) {
		return true; //no such thing at the moment
	}
	//We need the uid for this, so we have a special understanding with the model.
	//We query the first row with Qt:UserRole, and it gives us the uid.  Nasty but works.
	QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);
	bool ok;
	long uid = sourceModel()->data(source_index, Qt::UserRole).toInt(&ok);
	if(!ok) {
		kDebug() << "Serious error with data.  The UID is not a number? Maybe 'ps' is not returning the data correctly.  UID is: " << source_parent.child(source_row,0).data().toString() << endl;
		return true;
	}
	switch(mFilter) {
	case PROCESS_FILTER_ALL:
		break;
        case PROCESS_FILTER_SYSTEM:
                if(uid >= 100) return false;
		break;
        case PROCESS_FILTER_USER:
		if(uid < 100) return false;
        case PROCESS_FILTER_OWN:
        default:
                if(uid != (long) getuid()) return false;
        }
	return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void ProcessFilter::setFilter(int index) {
	mFilter = index; 
	clear();//Tell the proxy view to refresh all its information
}
#include "ProcessFilter.moc"
