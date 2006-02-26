/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 John Tapsell <john.tapsell@kdemail.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef PROCESSMODEL_H_
#define PROCESSMODEL_H_

#include <kapplication.h>
#include <kiconloader.h>
#include <QPointer>
#include <QPixmap>
#include <QObject>
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QHash>
#include <QSet>

#include <Process.h>

/** For new data that comes in, this gives the type of it. 
  * If you want to add one, just chose a letter that isn't used yet
  * @see mColtype 
  */
/* These are used only internally.  Initially the columns come in as the set below, but we map to these types if we know them */
#define DATA_COLUMN_LOGIN 'L'
#define DATA_COLUMN_GID 'G'
#define DATA_COLUMN_PID 'P'
#define DATA_COLUMN_PPID 'Q'
#define DATA_COLUMN_UID 'U'
#define DATA_COLUMN_NAME 'N'
#define DATA_COLUMN_TRACERPID 'T'
/* These ones are known and used in ksysguardd so don't change unless you change there too*/
#define DATA_COLUMN_STATUS 'S'
#define DATA_COLUMN_OTHER_LONG 'd'
#define DATA_COLUMN_OTHER_PRETTY_LONG 'D' /*Printed as e.g 100,000,000 */
#define DATA_COLUMN_OTHER_PRETTY_FLOAT 'f'

extern KApplication* Kapp;

class ProcessModel : public QAbstractItemModel
{
	Q_OBJECT
		
public:
	ProcessModel(QObject* parent = 0);
	virtual ~ProcessModel() { }

	/* Functions for our Model for QAbstractItemModel*/
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	QModelIndex parent ( const QModelIndex & index ) const;
	
	bool hasChildren ( const QModelIndex & parent) const;
	/* Functions for setting the model */
	/** Set the untranslated heading names for the incomming data that will be sent in setData.
	 *  The column names we show to the user are based mostly on this information, translated if known, hidden if not necessary etc */
	bool setHeader(const QStringList &header, QList<char> coltype);
	/** This is called from outside every few seconds when we have a new answer.
	 *  It checks the new data against what we have currently, and tries to be efficent in merging in the new data.
	 */
	void setData(const QList<QStringList> &data);
	/** By telling the model whether the sensor it is recieving the data from is for the localhost,
	 *  then we can provide more information to the user.  For example, we can show the actual username
	 *  rather than just the userid.
	 */

	/** Set the heading names for the incomming data that will be sent in setXResData */
	bool setXResHeader(const QStringList &header, QList<char> coltype);
	/** Set the XRes data for a single process with the columns matching those given in setXResHeader.
	 *
	 *  XRes is an X server extension that returns information about an X process, such as the identifier (The window title),
	 *  the amount of memory the window is using for pixmaps, etc.
	 */
	void setXResData(const QStringList& data);
	
	/** If we are localhost, then we can offer additional help such as looking up information about a user, offering superuser
	 *  'kill' etc. */
	void setIsLocalhost(bool isLocalhost);

	/** The iconname is the name of the process or an aliases for it (like 'daemon' etc)
	 *  @return A QPixmap, that may or may not be null.
	 */
	QPixmap getIcon(const QString& iconname) const;
	


private:
	/** This returns a QModelIndex for the given process.  It has to look up the parent for this pid, find the offset this 
	 *  pid is from the parent, and return that.  It's not that slow, but does involve a couple of hash table lookups.
	 */
	QModelIndex getQModelIndex ( long long pid, int column) const;

	/** Insert the pid given, plus all its parents
	 */
	void insertOrChangeRows( long long pid ) ;
	/** Remove the given row from our internal model and notify the view, and do so for all the children of the given pid
	 */
	void removeRow( long long pid );
	/** Change the data for a process.
	 *  This basically copies new_pid_data across to current_pid_data, but also keeps track of what changed and then
	 *  emits dataChanged for the range that changed.  This makes the redrawing quicker.
	 */
	void changeProcess(long long pid) ;

	
	QHash<QString,Process::ProcessType> mProcessType;
	QStringList mHeader;
	QList<char> mColType;
	
	/** For a given process id, it returns a Process structure.
	 *  @see class Process
	 */
	QHash<long long, QPointer<Process> > mPidToProcess;

	/** Return a qt markup tooltip string for a local user.  It will have their full name etc.
	 *  This will be slow the first time, as it practically indirectly reads the whole of /etc/passwd
	 *  But the second time will be as fast as hash lookup as we cache the result
	 */
	inline QString getTooltipForUser(long long uid, long long gid) const;

	/** Return a username (as a QString) for a local user if it can, otherwise just their uid (as a long long).
	 *  This may have been given from the result of "ps" (but not necessarily).  If it's not found, then it
	 *  needs to find out the username from the uid. This will be slow the first time, as it practically indirectly reads the whole of /etc/passwd
	 *  But the second time will be as fast as hash lookup as we cache the result
	 */
	inline QVariant getUsernameForUser(long long uid) const;

		
	/** Cache for the icons to show next to the process.
	 *  Name is the process name, or an alias (like 'daemon' etc).
	 *  @see getIcon(const QString& iconname)
	 */
	mutable QHash<QString,QPixmap> mIconCache;
	
	KIconLoader mIcons;

	/** @see setIsLocalhost */
	bool mIsLocalhost;

	/** Set to true when something has gone really wrong with the model and we should just reload from disk */
	bool mNeedReset;
	/** A caching hash for tooltips for a user.
	 *  @see getTooltipForUser */
	mutable QHash<long long,QString> mUserTooltips;

	/** A caching hash for (QString) username for a user uid, or just their uid if it can't be found (as a long long)
	 *  @see getUsernameForUser */
	mutable QHash<long long, QVariant> mUserUsername;

	/** A set of all the pids we know about.  Stored in a set so we can easily compare them against new data sets*/
	QSet<long long> mPids;

	enum { HeadingUser, HeadingName, HeadingXIdentifier, HeadingOther };
	
	/** This are used when there is new data.  There are cleared as soon as the data is merged into the current model */
	QSet<long long> new_pids;
	QHash<long long, QStringList> newData;
	QHash<long long, long long> newPidToPpidMapping;

	/** A translated list of headings (column titles) in the order we want to display them. Used in headerData() */
	QStringList mHeadings;
	/** A list that matches up with headings and gives the type of each, using the enum HeadingUser, etc. Used in data() */
	QList<int> mHeadingsToType;
	/** For new data that comes in, this list matches up with that, and gives the type of each heading using a letter. */
	QList<char> mColtype; 

	int mPidColumn;
	int mPPidColumn;
	
	int mXResNumColumns;
	int mXResPidColumn;
	int mXResIdentifierColumn;
	int mXResPxmMemColumn;
	int mXResNumPxmColumn;
	
};

#endif

