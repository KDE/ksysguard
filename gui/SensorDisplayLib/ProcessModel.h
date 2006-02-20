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
#include <QPixmap>
#include <QObject>
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QHash>
#include <QSet>

/** These are where the data is stored in the new data.  This should only be used in a few places just to parse the new data */
#define PROCESS_NAME 0
#define PROCESS_PID 1
#define PROCESS_PPID 2
#define PROCESS_UID 3
#define PROCESS_DATASTART 4

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
	void setHeader(const QStringList &header) {
		beginInsertColumns(QModelIndex(), 0, header.count()-1);
		mHeader = header;
		endInsertColumns();
	}
	void setColType(const QList<char> &coltype) {mColType = coltype;}
	/** This is called from outside every few seconds when we have a new answer.
	 *  It checks the new data against what we have currently, and tries to be efficent in merging in the new data.
	 */
	void setData(const QList<QStringList> &data);
	/** By telling the model whether the sensor it is recieving the data from is for the localhost,
	 *  then we can provide more information to the user.  For example, we can show the actual username
	 *  rather than just the userid.
	 */
	void setIsLocalhost(bool isLocalhost);

	/** The iconname is the name of the process or an aliases for it (like 'daemon' etc)
	 *  @return A QPixmap, that may or may not be null.
	 */
	QPixmap getIcon(const QString& iconname) const;

	class Process {
	  public:
		typedef enum { Daemon, Kernel, Init, Other, Invalid } ProcessType;
		Process() { uid = 0; pid = 0; parent_pid = 0; processType=Invalid;}
		Process(const QString &_name, long long _pid, long long _ppid, long long _uid, ProcessType _processType )  {
			name = _name; pid=_pid; parent_pid=_ppid; uid = _uid; processType=_processType;}
		bool isValid() {return processType != Process::Invalid;}
		
		long long pid;    //The systems ID for this process
		long long parent_pid;  //The systems ID for the parent of this process.  0 for init.
		long long uid;
		ProcessType processType;
		QString name;  //The name (e.g. "ksysguard", "konversation", "init")
		QList<long long> children_pids;
		QList<QVariant> data;  //The column data, excluding the name, pid, ppid and uid
	};

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
	QHash<long long, Process> mPidToProcess;

	/** Return a qt markup tooltip string for a local user.  It will have their full name etc.
	 *  This will be slow the first time, as it practically indirectly reads the whole of /etc/passwd
	 *  But the second time will be as fast as hash lookup as we cache the result
	 */
	inline QString getTooltipForUser(long long localhost_uid) const;

	/** Return a username (as a QString) for a local user if it can, otherwise just their uid (as a long long).
	 *  This will be slow the first time, as it practically indirectly reads the whole of /etc/passwd
	 *  But the second time will be as fast as hash lookup as we cache the result
	 */
	inline QVariant getUsernameForUser(long long localhost_uid) const;

		
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

	/** A caching hash for (QString) username for a user. If the username can't be found, it returns the uid as a long long.
	 *  @see getUsernameForUser */
	mutable QHash<long long, QVariant> mUserUsername;

	/** A set of all the pids we know about.  Stored in a set so we can easily compare them against new data sets*/
	QSet<long long> mPids;

	/** This are used when there is new data.  There are cleared as soon as the data is merged into the current model */
	QSet<long long> new_pids;
	QHash<long long, QStringList> newData;
	QHash<long long, long long> newPidToPpidMapping;
		
};

#endif

