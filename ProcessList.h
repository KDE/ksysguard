/*
    KTop, the KDE Task Manager
   
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
#include <qlistview.h>

#include "IconList.h"
#include "OSProcessList.h"

#define NONE -1

class ProcessLVI : public QListViewItem
{
public:
	ProcessLVI(QListView* lvi) : QListViewItem(lvi) { }

	virtual const char* key(int column, bool) const;
} ;

/**
 * This class implements a widget that displays processes in a table. The
 * KTabListBox is used for the handling of the table. The widget has four
 * buttons that control the update rate, the process filter, manual refresh
 * and killing of processes. The display is updated automatically when
 * auto mode is enabled.
 */
class ProcessList : public QListView
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

	/// The constructor.
	ProcessList(QWidget* parent , const char* name);

	/// The destructor.
	~ProcessList();

	/**
	 * The udpate function can be used to update the displayed process list.
	 * A current list of processes is requested from the OS.
	 */
	void update(void);

	/**
	 * This function can be used to control the auto update feature of the
	 * widget. If auto update mode is enabled the display is refreshed
	 * according to the set refresh rate.
	 */
	int setAutoUpdateMode(bool mode = TRUE);

	/**
	 * This function returns the current refresh rate. It's not the rate
	 * itself but the index of the combo button item used to control the rate.
	 */
	int getUpdateRate()
	{
		return (update_rate);
	}

	/// This function sets the refresh rate.
	void setUpdateRate(int);

	/**
	 * This function returns the current filter mode. The value is the index
	 * of the combo button item used to set the filter mode.
	 */
	int getFilterMode()
	{
		return (filtermode);
	}

	/// This function sets the filter mode.
	void setFilterMode(int m)
	{
		filtermode = m;
	}

	virtual void setSorting(int column, bool increasing = TRUE);

	/**
	 * Return the index of the column that we sort for.
	 */
	int getSortColumn() const
	{
		return (sortColumn);
	}

	/**
	 * Return wheather we sort in increasing direction or not.
	 */
	int getIncreasing() const
	{
		return (increasing);
	}

	/**
	 * This functions specifies the column that is used to sort the process
	 * list.
	 */
	void setSortColumn(int c, bool inc = FALSE);

	int selectedPid(void) const;

signals:
	void popupMenu(int, int);

public slots:
	void hideColumn(int col)
	{
		printf("Request to hide columnd %d\n", col);
	}

protected:
	virtual void mousePressEvent(QMouseEvent* e);

private slots:
	void handleRMBPopup(int item);

private:
    enum
	{
		UPDATE_SLOW_VALUE = 20,
		UPDATE_MEDIUM_VALUE = 7,
		UPDATE_FAST_VALUE = 1
	};

	void initTabCol(void);

    virtual void timerEvent(QTimerEvent*)
	{
		update();
	}

	void load();

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

	int filtermode;
	int sortColumn;
	bool increasing;
	int update_rate;
	int timer_interval;
	int timer_id;

	OSProcessList pl;
    KtopIconList* icons;
};

#endif
