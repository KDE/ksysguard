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
	ProcessController(QWidget* parent = 0, const char* name = 0);
	virtual ~ProcessController()
	{
		delete xbTreeView;
		delete xbPause;
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
	}

	virtual bool addSensor(SensorAgent*, const QString&, const QString&);

	virtual void answerReceived(int id, const QString& answer);

	virtual void timerEvent(QTimerEvent*)
	{
		sensorAgent->sendRequest("ps", (SensorClient*) this, 2);
	}

public slots:
	void filterModeChanged(int filter)
	{
		cbFilter->setCurrentItem(filter);
		updateList();
	}

	void setTreeView(bool tv)
	{
		pList->setTreeView(tv);
		updateList();
	}

	void togglePause(bool p)
	{
		if (p)
			timerOff();
		else
			timerOn();
	}

	void updateList()
	{
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

	/// Checkbox to switch between tree and list view
	QCheckBox* xbTreeView;

	/// Checkbox to pause the automatic update of the process list
	QCheckBox* xbPause;

	/// This combo boxes control the process filter.
	QComboBox* cbFilter;
	
	/// These buttons force an immedeate refresh or kill a process.
	QPushButton* bRefresh;
	QPushButton* bKill;

	SensorAgent* sensorAgent;
} ;

#endif
