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

#ifndef _ProcessList_h_
#define _ProcessList_h_

#include <qwidget.h>

#include <ktablistbox.h>

#include "IconList.h"
#include "OSProcessList.h"

#define NONE -1

class KtopProcList : public KTabListBox
{
    Q_OBJECT

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

	int getFilterMode()
	{
		return (filtermode);
	}

	void setFilterMode(int m)
	{
		filtermode = m;
	}

	int getSortMethod()
	{
		return (sort_method);
	}

	void setSortMethod(int m)
	{
		sort_method = (OSProcessList::SORTKEY) m;
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

	void timerOff()
	{
		if (timer_id != NONE)
		{
			killTimer(timer_id);
			timer_id = NONE;
		} 
	}

	void timerOn()
	{
		timer_id = startTimer(timer_interval);
	}

    int lastSelectionPid;
	int filtermode;
	OSProcessList::SORTKEY sort_method;
	int update_rate;
	int timer_interval;
	int timer_id;

    KtopIconList *icons;

 private slots:
	void procHighlighted(int, int);
	void userClickOnHeader(int);
};

#endif

