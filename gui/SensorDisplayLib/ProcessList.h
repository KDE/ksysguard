/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me
	first. Thanks!

 */

#ifndef _ProcessList_h_
#define _ProcessList_h_

#include <q3dict.h>
#include <qdom.h>
#include <q3listview.h>
#include <qstringlist.h>
#include <q3valuelist.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PopupMenu>
#include <Q3PtrList>

#include <kiconloader.h>
#include <klistview.h>

#include "SensorClient.h"

typedef const char* (*KeyFunc)(const char*);

/**
 * To support bi-directional sorting, and sorting of text, integers etc. we
 * need a specialized version of QListViewItem.
 */
class ProcessLVI : public Q3ListViewItem
{
public:
	ProcessLVI(Q3ListView* lv) : Q3ListViewItem(lv) { }
	ProcessLVI(Q3ListViewItem* lvi) : Q3ListViewItem(lvi) { }

	virtual int compare( Q3ListViewItem *item, int column, bool ) const;
};

class Q3PopupMenu;

/**
 * This class implementes a table filled with information about the running
 * processes. The table is derived from QListView.
 */
class ProcessList : public KListView
{
    Q_OBJECT

public:
	// possible values for the filter mode
	enum
	{
		FILTER_ALL = 0,
		FILTER_SYSTEM,
		FILTER_USER,
		FILTER_OWN
	};

    enum ColumnType { Text, Int, Float, Time };

	/// The constructor.
	ProcessList(QWidget* parent = 0);

	/// The destructor.
	~ProcessList();

	void removeColumns();

	void addColumn(const QString& header, const QString& type);

	void setSortColumn(uint col, bool inc)
	{
		sortColumn = col;
		increasing = inc;
		setSorting(col, inc);
	}

	uint getSortColumn() const
	{
		return sortColumn;
	}

	bool getIncreasing() const
	{
		return increasing;
	}

	const Q3ValueList<int>& getSelectedPIds();
	const QStringList& getSelectedAsStrings();

	/**
	 * The udpate function can be used to update the displayed process
	 * list.  A current list of processes is requested from the OS. In
	 * case the list contains invalid or corrupted info, FALSE is
	 * returned.
	 */
	bool update(const QString& list);

	int columnType( int col ) const;

	void setSensorOk(bool ok);

	void setKillSupported(bool supported)
	{
		killSupported = supported;
	}

	bool load(QDomElement& el);
	bool save(QDomDocument& doc, QDomElement& display);

public slots:
	void setTreeView(bool tv);

	/**
	 * This slot allows the filter mode to be set by other
	 * widgets. Possible values are FILTER_ALL, FILTER_SYSTEM,
	 * FILTER_USER and FILTER_OWN. This filter mechanism will be much
	 * more sophisticated in the future.
	 */
	void setFilterMode(int fm)
	{
		filterMode = fm;
		setModified(true);
	}

	void sortingChanged(int col);

	void handleRMBPressed(Q3ListViewItem* lvi, const QPoint& p, int col);

	void sizeChanged(int, int, int)
	{
		setModified(true);
	}

	void indexChanged(int, int, int)
	{
		setModified(true);
	}

	virtual void setModified(bool mfd)
	{
		if (mfd != modified)
		{
			modified = mfd;
			emit listModified(modified);
		}
	}

signals:
	// This signal is emitted when process pid should receive signal sig.
	void killProcess(int pid, int sig);

	// This signal is emitted when process pid should be reniced.
	void reniceProcess(int pid, int niceValue);

	void listModified(bool);

private:
	// items of table header RMB popup menu
	enum
	{
		HEADER_REMOVE = 0,
		HEADER_ADD,
		HEADER_HELP
	};

	/**
	 * This function updates the lists of selected PID und the closed
	 * sub trees.
	 */
	void updateMetaInfo(void);

	/**
	 * This function determines whether a process matches the current
	 * filter mode or not. If it machtes the criteria it returns true,
	 * false otherwise.
	 */
	bool matchesFilter(KSGRD::SensorPSLine* p) const;

	/**
	 * This function constructs the list of processes for list
	 * mode. It's a straightforward appending operation to the
	 * QListView widget.
	 */
	void buildList();

	/**
	 * This function constructs the tree of processes for tree mode. It
	 * filters out leaf-sub-trees that contain no processes that match
	 * the filter criteria.
	 */
	void buildTree();

	/**
	 * This function deletes the leaf-sub-trees that do not match the
	 * filter criteria.
	 */
	void deleteLeaves(void);

	/**
	 * This function returns true if the process is a leaf process with
	 * respect to the other processes in the process list. It does not
	 * have to be a leaf process in the overall list of processes.
	 */
	bool isLeafProcess(int pid);

	/**
	 * This function is used to recursively construct the tree by
	 * removing processes from the process list an inserting them into
	 * the tree.
	 */
	void extendTree(Q3PtrList<KSGRD::SensorPSLine>* pl, ProcessLVI* parent, int ppid);

	/**
	 * This function adds a process to the list/tree.
	 */
	void addProcess(KSGRD::SensorPSLine* p, ProcessLVI* pli);

private:
	void selectAllItems(bool select);
	void selectAllChilds(int pid, bool select);

	bool modified;
	int filterMode;
	int sortColumn;
	bool increasing;
	int refreshRate;
	int currColumn;
	bool killSupported;
	bool treeViewEnabled;
	bool openAll;

	/* The following lists are primarily used to store table specs between
	 * load() and the actual table creation in addColumn(). */
	Q3ValueList<int> savedWidth;
	Q3ValueList<int> currentWidth;
	Q3ValueList<int> index;

	Q3PtrList<KSGRD::SensorPSLine> pl;

	QStringList mColumnTypes;
	Q3Dict<QString> columnDict;

	Q3ValueList<int> selectedPIds;
	Q3ValueList<int> closedSubTrees;
	QStringList selectedAsStrings;

	static Q3Dict<QString> aliases;

	Q3Dict<QPixmap> iconCache;

	KIconLoader* icons;
	Q3PopupMenu* processPM;
	Q3PopupMenu* headerPM;
};

#endif
