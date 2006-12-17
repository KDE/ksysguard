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

#ifndef _ProcessController_h_
#define _ProcessController_h_

#include <QWidget>
#include <QAbstractItemModel>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QList>
#include <QProcess>


#include <kapplication.h>

#include <SensorDisplay.h>

#include "ui_ProcessWidgetUI.h"
#include "ProcessModel.h"

#include "ProcessFilter.h"
class QAction;

extern KApplication* Kapp;

/**
 * This widget implements a process list page. Besides the process
 * list which is implemented as a ProcessList, it contains two
 * comboxes and two buttons.  The combo boxes are used to set the
 * update rate and the process filter.  The buttons are used to force
 * an immediate update and to kill a process.
 */
class ProcessController : public KSGRD::SensorDisplay
{
	Q_OBJECT

public:
	ProcessController(QWidget* parent, const QString& title, SharedSettings *workSheetSettings);
	virtual ~ProcessController() { }

	/* Functions for SensorDisplay*/
	
	void resizeEvent(QResizeEvent*);

	bool restoreSettings(QDomElement& element);

	bool saveSettings(QDomDocument& doc, QDomElement& element);

	virtual void timerTick()
	{
		updateList();
	}

	virtual bool addSensor(const QString&, const QString&, const QString&, const QString&);

	virtual void answerReceived(int id, const QList<QByteArray>& answer);

	virtual void sensorError(int, bool err);

	void configureSettings() { }

	virtual bool hasSettingsDialog() const
	{
		return false;
	}

public slots:

	void expandAllChildren(const QModelIndex &parent);
	void currentRowChanged(const QModelIndex &current);
		
	void killProcess();
	void killProcess(int pid, int sig);

	void reniceProcess();
	void reniceProcess(int pid, int niceValue);

	void updateList();
private slots:
	void expandInit();
	void showContextMenu(const QPoint &point);
	void showProcessContextMenu(const QPoint &point);
	void showOrHideColumn(QAction *);
	void killFailed();
	void reniceFailed();
	void setSimpleMode(int index);
signals:
	void setFilterMode(int);

private:
	/** Tells us if the user can see the xres columns.  If they can't, then this lets us optimise our code paths
	 *  @return false only if the user can see the xres columns, otherwise true.  Note: Also returns true if we have no information about xres */
	bool areXResColumnsHidden() const;
	enum { Ps_Info_Command = 1, Ps_Command, Kill_Command, Kill_Supported_Command, Renice_Command, XRes_Info_Command, XRes_Command, XRes_Supported_Command, MemFree_Command, MemUsed_Command };
	/** Does the ksysguardd daemon support killing a process? */
	bool mKillSupported;
	/** Is the XRes extension supported where the ksysguardd daemon is? (And does the daemon support it) */
	bool mXResSupported;
	/** To compress function calls, we set these bools then set single shot timers.  Then in the function, we unset these*/
	bool mWillUpdateList;
	
	/** The column context menu when you right click on a column.*/
	QMenu *mColumnContextMenu;
	/** The context menu when you right click on a process */
	QMenu *mProcessContextMenu;
	
	/** We do not want to send out a "ps" command before we have the results from "ps?" so this is set to false until we get a result from ps?
	 */
	bool mReadyForPs;
	
	/** The amount of real physical memory being used in total, in kilobytes */
	long mMemUsed;
	/** The amount of free physical memory, in kilobytes */
	long mMemFree;
	/** The total amount of physical memory in the machine, in kilobytes */
	long mMemTotal;

	/** When we restore the settings, we remember which column to sort by here.
	 *  Then when columns are actually added, we can set the column to sort by.
	 */
	int mInitialSortCol;
	/** Same as mInitialSortCol but for the direction to sort
	 */
        bool mInitialSortInc;
	/** The 'xres' call is very expensive - takes around 1 second for me.
	 *  So rather than calling it every time, instead we just call it occasionally.
	 *  When this countdown reaches 0 then we call xres again.
	 */
	int mXResCountdown;
	
	/** Whether we are in simple mode.  Must be kept in sync with the cmbFilter and mModel.mSimple
	 */
	bool mSimple;

	QProcess *mKillProcess;
	QProcess *mReniceProcess;

	/** The process model.  This contains all the data on all the processes running on the system */
	ProcessModel mModel;
	/** The process filter.  The mModel is connected to this, and this filter model connects to the view.  This lets us
	 *  sort the view and filter (by using the combo box or the search line)
	 */
	ProcessFilter mFilterModel;
	/** The graphical user interface for this process list widget, auto-generated by Qt Designer */
	Ui::ProcessWidget mUi;
	/** The logical index in the header of the xres headings inclusive
	 */
	int mXResHeadingStart;
	/** The logical index in the header of the xres headings inclusive
	 */
	int mXResHeadingEnd;
};

#endif
