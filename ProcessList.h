/*
    KTop, the KDE Task Manager
   
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

	virtual QString key(int column, bool) const;
} ;

class QPopupMenu;

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
	// items of "Filter" combo box
	enum
	{
		FILTER_ALL = 0,
		FILTER_SYSTEM,
		FILTER_USER,
		FILTER_OWN
	};

	/// The constructor.
	ProcessList(QWidget* parent = 0, const char* name = 0);

	/// The destructor.
	~ProcessList();

	void saveSettings(void);

	/**
	 * This function can be used to control the auto update feature of the
	 * widget. If auto update mode is enabled the display is refreshed
	 * according to the set refresh rate.
	 */
	int setAutoUpdateMode(bool mode = TRUE);

	virtual void setSorting(int column, bool increasing = TRUE);

	int selectedPid(void) const;

public slots:
	/**
	 * The udpate function can be used to update the displayed process list.
	 * A current list of processes is requested from the OS.
	 */
	void update(void);

	void setRefreshRate(int);
	void setFilterMode(int fm)
	{
		filterMode = fm;
		update();
	}

signals:
	void popupMenu(int, int);
	void refreshRateChanged(int);
	void filterModeChanged(int);

protected:
	virtual void mousePressEvent(QMouseEvent* e);

private slots:
	void handleRMBPopup(int item);

private:
	// items of table header RMB popup menu
	enum
	{
		HEADER_REMOVE = 0,
		HEADER_ADD,
		HEADER_HELP
	};
	// timer multipliers for different refresh rates
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
		if (timerId != NONE)
		{
			killTimer(timerId);
			timerId = NONE;
		} 
	}

	void timerOn()
	{
		timerId = startTimer(timerInterval);
	}

	/*
	 * Since some columns of our process table might be invisible the columns
	 * of the QListView and the data structure do not match. We have to map
	 * the visible columns to the table columns (V2T).
	 */
	int mapV2T(int vcol);

	/*
	 * This function maps a table columns index to a visible columns index.
	 */
	int mapT2V(int tcol);

	int filterMode;
	int sortColumn;
	bool increasing;
	int refreshRate;
	int currColumn;
	int timerInterval;
	int timerId;

	OSProcessList pl;
    KtopIconList* icons;
	QPopupMenu* headerPM;
};

#endif
