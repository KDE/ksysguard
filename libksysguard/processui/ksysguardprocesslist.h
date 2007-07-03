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

#ifndef _KSysGuardProcessList_h_
#define _KSysGuardProcessList_h_

#include <QWidget>
#include <kapplication.h>
#include "ProcessModel.h"
#include "ProcessFilter.h"

class QShowEvent;
class QHideEvent;
class QAction;
class QMenu;
class QLineEdit;
class QTreeView;
namespace Ui {
  class ProcessWidget;
}

extern KApplication* Kapp;

/**
 * This widget implements a process list page. Besides the process
 * list which is implemented as a ProcessList, it contains two
 * comboxes and two buttons.  The combo boxes are used to set the
 * update rate and the process filter.  The buttons are used to force
 * an immediate update and to kill a process.
 */
class KDE_EXPORT KSysGuardProcessList : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( bool showTotalsInTree READ showTotals WRITE setShowTotals );
	Q_PROPERTY( ProcessFilter::State state READ state WRITE setState );
	Q_PROPERTY( int updateIntervalMSecs READ updateIntervalMSecs WRITE setUpdateIntervalMSecs );
	Q_ENUMS( ProcessFilter::State );

public:
	KSysGuardProcessList(QWidget* parent);
	virtual ~KSysGuardProcessList() { }

	QLineEdit *filterLineEdit() const;
	QTreeView *treeView() const;
	
	/** Returns which processes we are currently filtering for and the way in which we show them.
	 *  @see setState()
	 */
	ProcessFilter::State state() const;
	
	/** Returns the number of milliseconds that have to elapse before updating the list of processes */
	int updateIntervalMSecs() const;

	/** Whether the widget will show child totals for CPU and Memory etc usage */
	bool showTotals() const;
public slots:
	/** Inform the view that the user has changed the current row */
	void currentRowChanged(const QModelIndex &current);
	
	/** Send a kill signal to all the processes that the user has selected.  Pops up a dialog box to confirm with the user */
	void killSelectedProcesses();
	
	/** Send a signal to a list of given processes.
	 *   @p pids A list of PIDs that should be sent the signal 
	 *   @p sig  The signal to send. 
	 */
	void killProcesses(const QList< long long> &pids, int sig);

	/** Renice all the processes that the user has selected.  Pops up a dialog box to ask for the nice value and confirm */
	void reniceSelectedProcesses();
	
	/** Renice the processes given to the given niceValue. */ 
	void reniceProcesses(const QList<long long> &pids, int niceValue);

	/** Fetch new process information and redraw the display */
	void updateList();
	
	/** Set which processes we are currently filtering for and the way in which we show them. */
	void setState(ProcessFilter::State state);
	
 	/** Set the number of milliseconds that have to elapse before updating the list of processes */
	void setUpdateIntervalMSecs(int intervalMSecs);

	/** Set whether to show child totals for CPU and Memory etc usage */
	void setShowTotals(bool showTotals);

private slots:

	/** Expand all the children, recursively, of the node given.  Pass an empty QModelIndex to expand all the top level children */
	void expandAllChildren(const QModelIndex &parent);

	/** Expand init to show its children, but not the sub children processes. */
	void expandInit();
	
	/** Display a context menu for the column headings allowing the user to show or hide columns. */
	void showColumnContextMenu(const QPoint &point);
	
	/** Display a context menu for the selected processes allowing the user to kill etc the process */
	void showProcessContextMenu(const QPoint &point);
	
	/** Show or hide the column indicated by the action */
	void showOrHideColumn(QAction *action);
	
	/** Handle the situation where killing a process has failed - usually due to insufficent rights */
	void killFailed();
	
	/** Handle the situation where renicing a process has failed - usually due to insufficent rights */
	void reniceFailed();

	/** Set state from combo box int value */
	void setStateInt(int state);
private:
	/** The column context menu when you right click on a column.*/
	QMenu *mColumnContextMenu;
	
	/** The context menu when you right click on a process */
	QMenu *mProcessContextMenu;
	
	/** The process model.  This contains all the data on all the processes running on the system */
	ProcessModel mModel;
	
	/** The process filter.  The mModel is connected to this, and this filter model connects to the view.  This lets us
	 *  sort the view and filter (by using the combo box or the search line)
	 */
	ProcessFilter mFilterModel;
	
	/** The graphical user interface for this process list widget, auto-generated by Qt Designer */
	Ui::ProcessWidget *mUi;

	/** The time to wait, in milliseconds, between updating the process list */
	int mUpdateIntervalMSecs;
	
	/** A timer to call updateList() every mUpdateIntervalMSecs */
	QTimer *mUpdateTimer;

	/** A timer to rapidly pulse a process being killed */
	QTimer *mPulseTimer;

	/** Inherit QWidget::showEvent(QShowEvent *) to enable the timer, for updates, when visible */
	virtual void showEvent(QShowEvent*);
	
	/** Inherit QWidget::hideEvent(QShowEvent *) to disable the timer, for updates, when not visible */
	virtual void hideEvent(QHideEvent*);
};

#endif
