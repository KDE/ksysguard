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

#ifndef _ProcessList_h_
#define _ProcessList_h_

#include <sys/time.h>
#include <sys/resource.h>       
#include <sys/types.h>
#include <sys/stat.h>

#include <qwidget.h>

#include <ktablistbox.h>

#include "IconList.h"

typedef struct pS {
    char      name[101],
		status,
		statusTxt[10],
		visited;
    pid_t     pid, ppid;
    uid_t     uid;
    gid_t     gid;
    unsigned  vm_size, 
		vm_lock, 
		vm_rss, 
		vm_data, 
		vm_stack, 
		vm_exe, 
		vm_lib;
    int       otime, 
		time, 
		oabstime, 
		abstime;
    struct pS *next, 
		*prev;  
} psStruct, *psPtr;

/*=============================================================================
 class  : KtopProcList
 =============================================================================*/
class KtopProcList : public KTabListBox
{
    Q_OBJECT;

public:
	enum rateID
	{
		UPDATE_SLOW = 0, 
		UPDATE_MEDIUM, 
		UPDATE_FAST   
	};
	enum
	{
		FILTER_ALL = 0,
		FILTER_SYSTEM,
		FILTER_USER,
		FILTER_OWN
	};
 	enum sortID
	{
		SORTBY_PID = 0, 
		SORTBY_NAME, 
		SORTBY_UID, 
		SORTBY_CPU,
		SORTBY_TIME,
		SORTBY_STATUS,
		SORTBY_VMSIZE,
		SORTBY_VMRSS,
		SORTBY_VMLIB
	};	

	KtopProcList(QWidget* parent , const char* name);
	~KtopProcList();

	void update(void);

	int  setAutoUpdateMode(bool mode = TRUE);

	int updateRate()
	{
		return (update_rate);
	}

	void setUpdateRate(int);

	int sortMethod(void)
	{
		return (sort_method);
	}

	int filterMode()
	{
		return (filtermode);
	}

	void setFilterMode(int m)
	{
		filtermode = m;
	}

	void setSortMethod(int m)
	{
		sort_method = m;
	}

	int selectionPid(void)
	{
		return lastSelectionPid;
	}

protected:
	virtual int cellHeight(int);  

private:
    enum
	{
		UPDATE_SLOW_VALUE = 20,
		UPDATE_MEDIUM_VALUE = 7,
		UPDATE_FAST_VALUE = 1
	};

    virtual void timerEvent(QTimerEvent*)
	{
		update();
	}

	void load();
	void try2restoreSelection(); 
	void restoreSelection();
	void timerOff();
	void timerOn();
	int psList_getProcStatus(char*);
	void psList_clearProcVisit();
	void psList_removeProcUnvisited();
	void psList_sort();
	psPtr psList_getProcItem(char*);
    
    psPtr ps_list;
    int lastSelectionPid;
	int filtermode;
	int sort_method;
	int update_rate;
	int timer_interval;
	int timer_id;

    KtopIconList *icons;

	void repaint()
		{
			KTabListBox::repaint();
			printf("ProcessList::repaint()\n");
		}
 private slots:
	void procHighlighted(int, int);
	void userClickOnHeader(int);
};

#endif

