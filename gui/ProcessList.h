/*
    KTop, the KDE Task Manager and System Monitor
   
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me
	first. Thanks!

	$Id$
 */

#ifndef _ProcessList_h_
#define _ProcessList_h_

#include <qwidget.h>
#include <qlistview.h>
#include <qvaluelist.h>
#include <kiconloader.h>

#include "ProcessMenu.h"
#include "SensorClient.h"

typedef const char* (*KeyFunc)(const char*);

/**
 * To support bi-directional sorting, and sorting of text, integers etc. we
 * need a specialized version of QListViewItem.
 */
class ProcessLVI : public QListViewItem
{
public:
	ProcessLVI(QListView* lv) : QListViewItem(lv) { }
	ProcessLVI(QListViewItem* lvi) : QListViewItem(lvi) { }

	virtual QString key(int column, bool) const;
} ;

class QPopupMenu;

/**
 * This class implementes a table filled with information about the running
 * processes. The table is derived from QListView.
 */
class ProcessList : public QListView
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

	/// The constructor.
	ProcessList(QWidget* parent = 0, const char* name = 0);

	/// The destructor.
	~ProcessList();

	void removeColumns(void);

	void addColumn(const QString& header, const QString& type);

	/**
	 * This function clears the current selection and sends out a signal.
	 */
	void clearSelection()
	{
		if (currentItem())
			setSelected(currentItem(), false);
		emit(processSelected(-1));
	}

	const QValueList<int>& getSelectedPIds()
	{
		return (selectedPIds);
	}

	/**
	 * The udpate function can be used to update the displayed process
	 * list.  A current list of processes is requested from the OS. In
	 * case the list contains invalid or corrupted info, FALSE is
	 * returned.
	 */
	bool update(const QString& list);

	const QValueList<KeyFunc>& getSortFunc()
	{
		return (sortFunc);
	}

public slots:
	void setTreeView(bool tv)
	{
		if (treeViewEnabled = tv)
			openAll = TRUE;
	}

	/**
	 * This slot allows the filter mode to be set by other
	 * widgets. Possible values are FILTER_ALL, FILTER_SYSTEM,
	 * FILTER_USER and FILTER_OWN. This filter mechanism will be much
	 * more sophisticated in the future.
	 */
	void setFilterMode(int fm)
	{
		filterMode = fm;
	}

signals:
	// This signal is emitted whenever a new process has been selected.
	void processSelected(int);

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
	bool matchesFilter(SensorPSLine* p) const;

	/**
	 * This function constructs the list of processes for list
	 * mode. It's a straightforward appending operation to the
	 * QListView widget.
	 */
	void buildList();

	/**
	 * This fuction constructs the tree of processes for tree mode. It
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
	void extendTree(QList<SensorPSLine>* pl, ProcessLVI* parent, int ppid);

	/**
	 * This function adds a process to the list/tree.
	 */
	void addProcess(SensorPSLine* p, ProcessLVI* pli);

	/**
	 * Since some columns of our process table might be invisible the
	 * columns of the QListView and the data structure do not
	 * match. We have to map the visible columns to the table columns
	 * (V2T).
	 */
	int mapV2T(int vcol);

	/**
	 * This function maps a table columns index to a visible columns
	 * index.
	 */
	int mapT2V(int tcol);

private slots:
	void handleRMBPopup(int item);
	void selectionChangedSlot(QListViewItem* lvi)
	{
		if (lvi)
		{
			QString pidStr = lvi->text(1);
			emit(processSelected(pidStr.toInt()));
		}
	}

private:
	int filterMode;
	int sortColumn;
	bool increasing;
	int refreshRate;
	int currColumn;
	bool treeViewEnabled;
	bool openAll;

	QList<SensorPSLine> pl;

	QValueList<KeyFunc> sortFunc;

	QValueList<int> selectedPIds;
	QValueList<int> closedSubTrees;

    KIconLoader* icons;
	ProcessMenu* processMenu;
	QPopupMenu* headerPM;
};

#endif
