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
#include <kuser.h>
#include <QPixmap>
#include <QObject>
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QHash>
#include <QSet>

#include <Process.h>

/* These are known and used in ksysguardd so don't change unless you change there too */
#define DATA_COLUMN_STATUS 'S'
#define DATA_COLUMN_LONG 'd'
#define DATA_COLUMN_PRETTY_LONG 'D' /*Printed as e.g 100,000,000 */
#define DATA_COLUMN_PRETTY_FLOAT 'f'

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
	/** Set the untranslated heading names for the incoming data that will be sent in setData.
	 *  The column names we show to the user are based mostly on this information, translated if known, hidden if not necessary etc */
	bool setHeader(const QStringList &header, const QByteArray &coltype);
	/** This is called from outside every few seconds when we have a new answer.
	 *  It checks the new data against what we have currently, and tries to be efficient in merging in the new data.
	 *  @returns Whether the operation was successful.  Returns false if there was a problem with the data/sensor
	 */
	bool setData(const QList<QStringList> &data);
	/** By telling the model whether the sensor it is recieving the data from is for the localhost,
	 *  then we can provide more information to the user.  For example, we can show the actual username
	 *  rather than just the userid.
	 */

	/** Set the heading names for the incoming data that will be sent in setXResData */
	bool setXResHeader(const QStringList &header, const QByteArray& coltype);
	/** Set the XRes data for a single process with the columns matching those given in setXResHeader.
	 *
	 *  XRes is an X server extension that returns information about an X process, such as the identifier (The window title),
	 *  the amount of memory the window is using for pixmaps, etc.
	 */
	void setXResData(long long pid, const QStringList& data);
	
	/** If we are localhost, then we can offer additional help such as looking up information about a user, offering superuser
	 *  'kill' etc. */
	void setIsLocalhost(bool isLocalhost);

	/** The iconname is the name of the process or an aliases for it (like 'daemon' etc)
	 *  @return A QPixmap, that may or may not be null.
	 */
	QPixmap getIcon(const QString& iconname) const;
	
	/** Return a string with the pid of the process and the name of the process.  E.g.  13343: ksyguard
	 */
	QString getStringForProcess(Process *process) const;
	inline Process *getProcess(long long pid) { return mPidToProcess[pid]; }
        
	/** Returns whether this user can log in or not.
	 *  @see mUidCanLogin
	 */
	bool canUserLogin(long long uid) const;


public slots:
	void setShowTotals(int totals);
	

private:
	/** This returns a QModelIndex for the given process.  It has to look up the parent for this pid, find the offset this 
	 *  pid is from the parent, and return that.  It's not that slow, but does involve a couple of hash table lookups.
	 */
	QModelIndex getQModelIndex ( Process *process, int column) const;

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

	/** For new data that comes in, this list matches up with that, and gives the type of each heading using a letter. */
	QByteArray mColType;
	
	/** For a given process id, it returns a Process structure.
	 *  When a process is made, it will be stored here.  If the process is removed for here, it must be deleted to avoid leaking.
	 *  @see class Process
	 */
	QHash<long long, Process *> mPidToProcess;

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

	/** Each process keeps track of the system and user cpu usage of itself and all its children.
	 *  So when a process is changed/added/removed, those totals have to be updated for all its parents.
	 *  This function does that.  It is called when a process is changed/added/removed
	 */
	void updateProcessTotals(Process *process, double sysChange, double userChange, int numChildrenChange);
		
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

	/** A mapping of a user id to whether this user can log in.  We have to guess based on the shell. All are set to true to non localhost.
	 *  It is set to:
	 *    0 if the user cannot login
	 *    1 is the user can login
	 *  The reason for using an int and not a bool is so that we can do  mUidCanLogin.value(uid,-1)  and thus we get a tristate for whether
	 *  they are logged in, not logged in, or not known yet.
	 *  */
	mutable QHash<long long, int> mUidCanLogin; 
	/** A set of all the pids we know about.  Stored in a set so we can easily compare them against new data sets*/
	QSet<long long> mPids;

	enum { HeadingUser, HeadingName, HeadingXIdentifier, HeadingXMemory, HeadingCPUUsage, HeadingRSSMemory, HeadingMemory, HeadingCommand, HeadingOther };
	
	/** This are used when there is new data.  There are cleared as soon as the data is merged into the current model */
	QSet<long long> new_pids;
	QHash<long long, QStringList> newData;
	QHash<long long, long long> newPidToPpidMapping;

	/** A translated list of headings (column titles) in the order we want to display them. Used in headerData() */
	QStringList mHeadings;
	/** A list that matches up with headings and gives the type of each, using the enum HeadingUser, etc. Used in data() */
	QList<int> mHeadingsToType;

	/** A mapping of running,stopped,etc  to a friendly description like 'Stopped, either by a job control signal or because it is being traced.'*/
	QHash<QString, QString> mStatusDescription; 

	bool mShowChildTotals; ///If set to true, a parent will return the CPU usage of all its children recursively

	typedef enum { DataColumnLogin, DataColumnGid, DataColumnPid, DataColumnPPid, DataColumnUid, DataColumnName, DataColumnTracerPid, DataColumnUserUsage, DataColumnSystemUsage, DataColumnNice, DataColumnVmSize, DataColumnVmRss, DataColumnCommand, DataColumnStatus, DataColumnOtherLong, DataColumnOtherPrettyLong, DataColumnOtherPrettyFloat, DataColumnError } DataColumnType;


	int mPidColumn;  ///Column the PID is in, so we can extract it quickly and easily from incoming data
	int mPPidColumn; ///Column the Parent's PID is in, so we can extract it quickly and easily from incoming data
	int mCPUHeading; ///Heading in the table that we show in which the memory is shown in - so that we can update it efficiently
	
	int mXResNumColumns;
	int mXResPidColumn;
	int mXResIdentifierColumn;
	int mXResPxmMemColumn;
	int mXResNumPxmColumn;
	int mXResMemOtherColumn;
	
};

#endif

