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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
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
#include <qdict.h>

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
	virtual ~ProcessController() { }

	void resizeEvent(QResizeEvent*);

	void clearSelection(void)
	{
		pList->clearSelection();
	}

	bool createFromDOM(QDomElement& el);

	bool addToDOM(QDomDocument& doc, QDomElement& display, bool save = true);

	bool hasBeenModified() const
	{
		return (modified || pList->hasBeenModified());
	}

	void refreshList(void)
	{
		updateList();
	}

	virtual void timerEvent(QTimerEvent*)
	{
		updateList();
	}

	virtual bool addSensor(const QString&, const QString&, const QString&);

	virtual void answerReceived(int id, const QString& answer);

	virtual void sensorError(int, bool err);

	void settings() { }

	virtual bool hasSettingsDialog() const
	{
		return (TRUE);
	}

public slots:
	void filterModeChanged(int filter)
	{
		pList->setFilterMode(filter);
		updateList();
		setModified(true);
	}

	void setTreeView(bool tv)
	{
		pList->setTreeView(tv);
		updateList();
		setModified(true);
	}

	void togglePause(bool p)
	{
		if (p)
			timerOff();
		else
			timerOn();
		setModified(true);
	}

	void killProcess();
	void killProcess(int pid, int sig);

	void updateList();

signals:
	void setFilterMode(int);

private:
	QVBoxLayout* gm;

	bool killSupported;

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

	/// Dictionary for header translations.
	QDict<QString> dict;
} ;

#endif
