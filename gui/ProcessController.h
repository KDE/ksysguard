/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _ProcessController_h_
#define _ProcessController_h_

#include <qwidget.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kapp.h>

#include "SensorDisplay.h"
#include "ProcessList.h"

extern KApplication* Kapp;

/**
 * This widget implements a process list page. Besides the process
 * list which is implemented as a ProcessList, it contains two
 * comboxes and two buttons.  The combo boxes are used to set the
 * update rate and the process filter.  The buttons are used to force
 * an immediate update and to kill a process.
 */
class ProcessController : public SensorDisplay
{
	Q_OBJECT

public:
	// possible values for the refresh rate. 
	enum
	{
		REFRESH_MANUAL = 0,
		REFRESH_SLOW,
		REFRESH_MEDIUM,
		REFRESH_FAST
	};
	// timer multipliers for different refresh rates
    enum
	{
		UPDATE_SLOW_VALUE = 20,
		UPDATE_MEDIUM_VALUE = 7,
		UPDATE_FAST_VALUE = 1
	};

	ProcessController(QWidget* parent = 0, const char* name = 0);
	virtual ~ProcessController()
	{
		// switch off timer
		timerOff();

		delete treeViewCB;
		delete bKill;
		delete bRefresh;
		delete box;
		delete cbFilter;
		delete pList;
		delete gm;
	}

	void resizeEvent(QResizeEvent*);

	void clearSelection(void)
	{
		pList->clearSelection();
	}

	void saveSettings(void)
	{
		pList->saveSettings();
	}

	void refreshList(void)
	{
		sensorAgent->sendRequest("ps?", (SensorClient*) this, 2);
		debug("Update request sent");
	}

	virtual bool addSensor(SensorAgent*, const QString&, const QString&);

	/**
	 * This function allows the refresh rate to be set by other
	 * widgets. Possible values are REFRESH_MANUAL, REFRESH_SLOW,
	 * REFRESH_MEDIUM and REFRESH_FAST.
	 */
	virtual void setRefreshRate(int r);

	virtual int setAutoUpdateMode(bool mode);

	virtual void answerReceived(int id, const QString& answer);

	/**
	 * This functions stops the timer that triggers automatic
	 * refreshed of the process list.
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
	 * This function starts the timer that triggers the automatic
	 * refreshes of the process list. It reads the interval from the
	 * member object timerInterval. To change the interval the timer
	 * must be stoped first with timerOff() and than started again
	 * with timeOn().
	 */
	void timerOn()
	{
		if (timerId == NONE && refreshRate != REFRESH_MANUAL)
			timerId = startTimer(timerInterval);
	}

	virtual void timerEvent(QTimerEvent*)
	{
		sensorAgent->sendRequest("ps", (SensorClient*) this, 2);
	}

public slots:
	void filterModeChanged(int filter)
	{
		cbFilter->setCurrentItem(filter);
	}

	void setTreeView(bool tv)
	{
		pList->setTreeView(tv);
		sensorAgent->sendRequest("ps", this, 2);
	}

signals:
	void setFilterMode(int);

private:
	QVBoxLayout* gm;

	/// The frame around the other widgets.
    QGroupBox* box;

	/// The process list.
    ProcessList* pList;

	QHBoxLayout* gm1;

	QCheckBox* treeViewCB;

	/// This combo boxes control the process filter.
	QComboBox* cbFilter;
	
	/// These buttons force an immedeate refresh or kill a process.
	QPushButton* bRefresh;
	QPushButton* bKill;

	SensorAgent* sensorAgent;

	/**
	 * This variable stores the index of the currently selected item of
	 * the cbRefresh combo box.
	 */
	int refreshRate;

	int timerInterval;
	int timerId;
} ;

#endif
