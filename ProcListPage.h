/*
    KTop, a taskmanager and cpu load monitor
   
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

#ifndef _ProcListPage_h_
#define _ProcListPage_h_

#include <qwidget.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kapp.h>

#include "ProcessList.h"

extern KApplication* Kapp;

/**
 * This widget implements a process list page. Besides the process list which
 * is implemented as a ProcessList, it contains two comboxes and two buttons.
 * The combo boxes are used to set the update rate and the process filter.
 * The buttons are used to force an immediate update and to kill a process.
 */
class ProcListPage : public QWidget
{
	Q_OBJECT

public:
	ProcListPage(QWidget* parent = 0, const char* name = 0);
	virtual ~ProcListPage()
	{
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

	int setAutoUpdateMode(bool mode)
	{
		return (pList->setAutoUpdateMode(mode));
	}
	
	void saveSettings(void)
	{
		pList->saveSettings();
	}

public slots:
	void filterModeChanged(int filter)
	{
		cbFilter->setCurrentItem(filter);
	}

	void treeViewChanged(bool tv)
	{
		treeViewCB->setChecked(tv);
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

	/**
	 * This variable stores the index of the currently selected item of
	 * the cbRefresh combo box.
	 */
	int refreshRate;
} ;

#endif
