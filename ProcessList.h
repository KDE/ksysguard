/*
    KTop, the KDE Task Manager
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net

	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!
*/

// $Id$

#ifndef _ProcessList_h_
#define _ProcessList_h_

#include <qwidget.h>
#include <qlistview.h>

#include <kiconloader.h>

#include "OSProcessList.h"
#include "ProcessMenu.h"

#define NONE -1

/**
 * To support bi-directional sorting, and sorting of text, intergers etc. we
 * need a specialized version of QListViewItem. The only specialization is
 * the key function.
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

	// possible values for the refresh rate. 
	enum
	{
		REFRESH_MANUAL = 0,
		REFRESH_SLOW,
		REFRESH_MEDIUM,
		REFRESH_FAST
	};

	/// The constructor.
	ProcessList(QWidget* parent = 0, const char* name = 0);

	/// The destructor.
	~ProcessList();

	void saveSettings(void);

	void loadSettings(void);

	/**
	 * This function can be used to control the auto update feature of the
	 * widget. If auto update mode is enabled the display is refreshed
	 * according to the set refresh rate.
	 */
	int setAutoUpdateMode(bool mode = TRUE);

	/**
	 * To support bi-directional sorting we need to re-implement setSorting
	 * to respect the direction and the different contense (text, number, etc).
	 */
	virtual void setSorting(int column, bool increasing = TRUE);

	/**
	 * This function clears the current selection and sends out a signal.
	 */
	void clearSelection(void)
	{
		if (currentItem())
			setSelected(currentItem(), false);
		emit(processSelected(-1));
	}

public slots:
	/**
	 * The udpate function can be used to update the displayed process list.
	 * A current list of processes is requested from the OS.
	 */
	void update(void);

	void killProcess(void)
	{
		processMenu->killProcess(selectedPid());
		update();
	}

	/**
	 * This slot allows the refresh rate to be set by other widgets. Possible
	 * values are REFRESH_MANUAL, REFRESH_SLOW, REFRESH_MEDIUM and
	 * REFRESH_FAST.
	 */
	void setRefreshRate(int);

	void setTreeView(bool tv)
	{
		treeViewEnabled = tv;
		update();
	}

	/**
	 * This slot allows the filter mode to be set by other widgets. Possible
	 * values are FILTER_ALL, FILTER_SYSTEM, FILTER_USER and FILTER_OWN. This
	 * filter mechanism will be much more sophisticated in the future.
	 */
	void setFilterMode(int fm)
	{
		filterMode = fm;
		update();
	}

signals:
	// This signal is emitted whenever the refresh rate has been changed.
	void refreshRateChanged(int);

	// This signal is emitted whenever the filter mode has been changed.
	void filterModeChanged(int);

	void treeViewChanged(bool);

	// This signal is emitted whenever a new process has been selected.
	void processSelected(int);

protected:
	virtual void viewportMousePressEvent(QMouseEvent* e);

private:
	// items of table header RMB popup menu
	enum
	{
		HEADER_REMOVE = 0,
		HEADER_ADD,
		HEADER_HELP
	};
	// timer multipliers for different refresh rates
    enum
	{
		UPDATE_SLOW_VALUE = 20,
		UPDATE_MEDIUM_VALUE = 7,
		UPDATE_FAST_VALUE = 1
	};

	/**
	 * This function returns the process ID of the currently selected process.
	 * If there isn't any -1 is returned.
	 */
	int selectedPid(void) const;

	void initTabCol(void);

	// Get a current list of processes from the operating system.
	void load();

	/*
	 * This function determines whether a process matches the current filter
	 * mode or not. If it machtes the criteria it returns true, false
	 * otherwise.
	 */
	bool matchesFilter(OSProcess* p) const;

	/*
	 * This function constructs the list of processes for list mode. It's a
	 * straightforward appending operation to the QListView widget.
	 */
	ProcessLVI* buildList(int selectedProcess);

	/*
	 * This fuction constructs the tree of processes for tree mode. It filters
	 * out leaf-sub-trees that contain no processes that match the filter
	 * criteria.
	 */
	ProcessLVI* buildTree(int selectedProcess);

	/*
	 * This function deletes the leaf-sub-trees that do not match the filter
	 * criteria.
	 */
	void deleteLeaves(void);

	/* This function returns true if the process is a leaf process with
	 * respect to the other processes in the process list. It does not
	 * have to be a leaf process in the overall list of processes.
	 */
	bool isLeafProcess(int pid);

	/*
	 * This function is used to recursively construct the tree by removing
	 * processes from the process list an inserting them into the tree.	
	 */
	void extendTree(OSProcessList* pl, ProcessLVI* parent, int ppid,
					ProcessLVI** newSelection, int selectedProcess);

	/*
	 * This function adds a process to the list/tree.
	 */
	void addProcess(OSProcess* p, ProcessLVI* pli);

	/**
	 * This function is automatically triggered by timer events. It refreshes
	 * the displayed process list.
	 */
    virtual void timerEvent(QTimerEvent*)
	{
		update();
	}

	/**
	 * Since some columns of our process table might be invisible the columns
	 * of the QListView and the data structure do not match. We have to map
	 * the visible columns to the table columns (V2T).
	 */
	int mapV2T(int vcol);

	/**
	 * This function maps a table columns index to a visible columns index.
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

	/**
	 * This functions stops the timer that triggers automatic refreshed of the
	 * process list.
	 */
	void timerOff()
	{
		if (timerId != NONE)
		{
			killTimer(timerId);
			timerId = NONE;
		} 
	}

	/**
	 * This function starts the timer that triggers the automatic refreshes
	 * of the process list. It reads the interval from the member object
	 * timerInterval. To change the interval the timer must be stoped first
	 * with timerOff() and than started again with timeOn().
	 */
	void timerOn()
	{
		if (timerId == NONE && refreshRate != REFRESH_MANUAL)
			timerId = startTimer(timerInterval);
	}

private:
	int filterMode;
	int sortColumn;
	bool increasing;
	int refreshRate;
	int currColumn;
	int timerInterval;
	int timerId;
	bool treeViewEnabled;

	OSProcessList pl;
    KIconLoader* icons;
	ProcessMenu* processMenu;
	QPopupMenu* headerPM;
};

#endif
