/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
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

*/

// $Id$

#ifndef TASKMAN_H
#define TASKMAN_H

#include <qtabdialog.h>

#include <ktablistbox.h>

#include "cpu.h"
#include "memory.h"
#include "settings.h"
#include "ptree.h"
#include "IconList.h"
#include "ProcessList.h"
#include "ProcessTree.h"
#include "ProcListPage.h"

//#define ADD_SWAPMON

class TaskMan : public QTabDialog
{
	Q_OBJECT;

public:
	enum 
	{
		MENU_ID_SIGINT = 500,
		MENU_ID_SIGQUIT,
		MENU_ID_SIGTERM,
		MENU_ID_SIGKILL,
		MENU_ID_SIGUSR1,
		MENU_ID_SIGUSR2,
	    MENU_ID_RENICE 
	};
	enum 
	{
		PAGE_PLIST = 0, 
		PAGE_PTREE, 
		PAGE_PERF
	};
	enum 
	{
		MENU_ID_PROP = 100,
		MENU_ID_KILL
	};

	TaskMan(QWidget *parent = 0, const char *name = 0, int sfolder=0);
    ~TaskMan()
	{
		delete procListPage;
	}
     
	int setUpdateInterval(int);
	int getUpdateInterval();
	void invokeSettings();
	void raiseStartUpPage();
	void saveSettings();

	static void TaskMan_initIconList(); 
	static void TaskMan_clearIconList();
	static const QPixmap* TaskMan_getProcIcon(const char*);

private:

	virtual void resizeEvent(QResizeEvent*);

	ProcListPage* procListPage;
	QPushButton* pTree_bRefresh;
	QPushButton* pTree_bRoot;
	QPushButton* pTree_bKill; 
    QWidget* pages[3];
	QWidget* mem_cur;
	QWidget* cpu_cur;
	QGroupBox* cpubox;
	QGroupBox* membox;
	QGroupBox* cpubox1;
	QGroupBox* membox1;
	
	QComboBox* pTree_cbSort;
	QGroupBox* pTree_box;
	KtopProcTree* pTree;
	QPopupMenu* pSig;
	CpuMon* cpumon;
	MemMon* memmon;
	AppSettings* settings;

	int pTree_sortby;
    int timer_interval;
	int tid;
	int startup_page;
	int filtermode;
	char cfgkey_startUpPage[12];
	char cfgkey_pTreeSort[12];
	bool restoreStartupPage;

#ifdef ADD_SWAPMON
	QGroupBox* swapbox1;
	QWidget* swap_cur;	
	SwapMon* swapmon;
	QGroupBox* swapbox;
#endif

public slots:

	void pSigHandler(int);
	void tabBarSelected(int);

	void popupMenu(int, int)
	{
		pSig->popup(QCursor::pos());
	}

	void pTree_update();
	void pTree_cbSortActivated(int);
	void pTree_popupMenu(QPoint);
	void pTree_killTask();
	void pTree_changeRoot();
};

class SetNice : public QDialog
{
	Q_OBJECT

public:
	SetNice(QWidget *parent, const char *name , int currentPPrio);

private:
	int value;

private slots:
	void setPriorityValue(int);

public slots:
	void ok();
	void cancel();
};

#endif
