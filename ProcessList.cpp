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

#include <ctype.h>
#include <string.h>
#include <signal.h>

#include <qmessagebox.h>
#include <qheader.h>

#include <kapp.h>
#include <klocale.h>

#include "OSProcessList.h"
#include "ProcessList.moc"

#define NONE -1

typedef const char* (*KeyFunc)(const char*);

typedef struct
{
	const char* header;
	char* trHeader;
	bool visible;
	bool supported;
	bool sortable;
	int alignment;
	KeyFunc key;
} TABCOLUMN;

static const char* intKey(const char* text);
static const char* timeKey(const char* text);
static const char* percentKey(const char* text);

/*
 * The following array defined the setup of the tab dialog. Not all platforms
 * may support all columns which is indicated by the supported flag. Also
 * not all columns may be visible at all times.
 *
 * This table, the construction of the KTabListBox lines in
 * ProcessList::initTabCol and ProcessList::load() MUST be keept in sync!
 * Also, the order and existance of the first three columns (icon, name and
 * pid) is mandatory! This i18n() mechanism is way too inflexible!
 */
static TABCOLUMN TabCol[] =
{
	{ "",            0, true, true,  false, 1, 0 },
	{ "Name",        0, true, true,  true,  1, 0 },
	{ "PID",         0, true, true,  true,  2, intKey },
	{ "User",        0, true, false, true,  1, 0 },
	{ "CPU",         0, true, false, true,  2, percentKey },
	{ "Time",        0, true, false, true,  2, timeKey },
	{ "Nice",        0, true, false, true,  2, intKey },
	{ "Status",      0, true, false, true,  1, 0 },
	{ "Memory",      0, true, false, true,  2, intKey },
	{ "Resident",    0, true, false, true,  2, intKey },
	{ "Shared",      0, true, false, true,  2, intKey },
	{ "Commandline", 0, true, false, true,  1, 0 }
};

static const int MaxCols = sizeof(TabCol) / sizeof(TABCOLUMN);

inline int max(int a, int b)
{
	return ((a) < (b) ? (b) : (a));
}

/*
 * The *key functions are used to sort the list. Since QListView can only sort
 * strings we have to massage the original contense so that the string sort
 * will produce the expected result.
 */
static const char*
intKey(const char* text)
{
	int val;
	sscanf(text, "%d", &val);
	static char key[16];
	sprintf(key, "%010d", val);

	return (key);
}

static const char*
timeKey(const char* text)
{
	int h, m;
	sscanf(text, "%d:%d", &h, &m);
	int t = h * 60 + m;
	static char key[16];
	sprintf(key, "%07d", t);

	return (key);
}

static const char*
percentKey(const char* text)
{
	double percent;
	sscanf(text, "%lf%%", &percent);

	static char key[16];
	sprintf(key, "%03.2f", percent);

	return (key);
}

const char*
ProcessLVI::key(int column, bool dir) const
{
	if (TabCol[column].key)
		return ((*TabCol[column].key)(text(column)));
	else
		return (text(column));
}

ProcessList::ProcessList(QWidget *parent = 0, const char* name = 0)
	: QListView(parent, name)
{
	// no timer started yet
	timer_id = NONE;

	/*
	 * The default filter mode is 'own processes'. This can be overridden by
	 * the config file.
	 */
	filtermode = FILTER_OWN;

	/*
	 * The default update rate is 'medium'. This can be overridden by the
	 * config file.
	 */
	update_rate = UPDATE_MEDIUM;

	// The default sorting is for the PID in decreasing order.
	sortColumn = 1;
	increasing = FALSE;

	// load the icons we display with the processes
	icons = new KtopIconList;
	CHECK_PTR(icons);

	// make sure we can retrieve process lists from the OS
	if (!pl.ok())
	{
		QMessageBox::critical(this, "Task Manager", pl.getErrMessage(), 0, 0);
		abort();
	}

	initTabCol();

	// Create popup menu for RMB clicks on table header
	headerPM = new QPopupMenu();
	connect(headerPM, SIGNAL(activated(int)),
			this, SLOT(handleRMBPopup(int)));
	
	headerPM->insertItem(i18n("Remove Column"), HEADER_REMOVE);
	headerPM->insertItem(i18n("Add Column"), HEADER_ADD);
	headerPM->insertItem(i18n("Help on Column"), HEADER_HELP);
}

ProcessList::~ProcessList()
{
	// remove icon list from memory
	delete icons;

	// switch off timer
	if (timer_id != NONE)
		killTimer(timer_id);

	delete(headerPM);
}

void 
ProcessList::setUpdateRate(int r)
{
	switch (update_rate = r)
	{
	case UPDATE_SLOW:
		timer_interval = UPDATE_SLOW_VALUE * 1000;
		break;
	case UPDATE_MEDIUM:
		timer_interval = UPDATE_MEDIUM_VALUE * 1000;
		break;
	case UPDATE_FAST:
	default:
		timer_interval = UPDATE_FAST_VALUE * 1000;
		break;
	}

	// only re-start the timer if auto mode is enabled
	if (timer_id != NONE)
	{
		timerOff();
		timerOn();
	}
}

void
ProcessList::setSorting(int column, bool inc)
{
	/*
	 * If the new column is equal to the current column we flip the sorting
	 * direction. Otherwise we just change the column we sort for. Since some
	 * columns may be invisible we have to map the view column to the table
	 * column.
	 */
	int tcol = mapV2T(column);

	if (sortColumn == tcol)
		increasing = !inc;
	else
		sortColumn = tcol;

	QListView::setSorting(column, increasing);
}

void
ProcessList::setSortColumn(int c, bool inc)
{
	/*
	 * We need to make sure that the specified column is visible. If it's not
	 * we use the first column (PID).
	 */
	if ((c >= 0) && (c < MaxCols) && TabCol[c].visible)
		sortColumn = c;
	else
		sortColumn = 1;

	increasing = inc;
}

int
ProcessList::setAutoUpdateMode(bool mode)
{
	/*
	 * If auto mode is enabled the display is updated regurlarly triggered
	 * by a timer event.
	 */

	// save current setting of the timer
	int oldmode = (timer_id != NONE) ? TRUE : FALSE; 

	// set new setting
	if (mode)
		timerOn();
	else
		timerOff();

	// return the previous setting
	return (oldmode);
}

void 
ProcessList::update(void)
{
	// disable the auto-update and save current mode
	int lastmode = setAutoUpdateMode(FALSE);

	// retrieve current process list from OS and update tab dialog
	load();

	setAutoUpdateMode(lastmode);

    if(isVisible())
		repaint();
}

void 
ProcessList::load()
{
	pl.clear();

	// request current list of processes
	if (!pl.update())
	{
		QMessageBox::critical(this, "Task Manager", pl.getErrMessage(), 0, 0);
		abort();
	}

	int selectedProcess = selectedPid();

	clear();
	ProcessLVI* newSelection = 0;

	while (!pl.isEmpty())
	{
		OSProcess* p = pl.first();

		// filter out processes we are not interested in
		bool ignore;
		switch (filtermode)
		{
		case FILTER_ALL:
			ignore = false;
			break;
		case FILTER_SYSTEM:
			ignore = p->getUid() >= 100 ? true : false;
			break;
		case FILTER_USER:
			ignore = p->getUid() < 100 ? true : false;
			break;
		case FILTER_OWN:
		default:
			ignore = p->getUid() != getuid() ? true : false;
			break;
		}
		if (!ignore)
		{
			/*
			 * Get icon from icon list might be appropriate for a process
			 * with this name.
			 */
			const QPixmap* pix = icons->procIcon((const char*)p->getName());

			ProcessLVI* pli = new ProcessLVI(this);

			int col = 0;
			// icon
			pli->setPixmap(col++, *pix);

			// process name
			pli->setText(col++, p->getName());

			QString s;

			// pid
			pli->setText(col++, s.setNum(p->getPid()).data());

			TABCOLUMN* tc = &TabCol[2];

			// user name
			if (tc->visible && tc->supported)
				pli->setText(col++, p->getUserName().data());
			tc++;

			// CPU load
			if (tc->visible && tc->supported)
				pli->setText(col++, s.sprintf("%.2f%%", 
									p->getUserLoad() + p->getSysLoad()));
			tc++;

			// total processing time
			if (tc->visible && tc->supported)
			{
				int totalTime = p->getUserTime() + p->getSysTime();
				pli->setText(col++, s.sprintf("%d:%02d",
											  (totalTime / 100) / 60,
											  (totalTime / 100) % 60));
			}
			tc++;

			// process nice level
			if (tc->visible && tc->supported)
				pli->setText(col++, s.sprintf("%d", p->getNiceLevel()));
			tc++;

			// process status
			if (tc->visible && tc->supported)
				pli->setText(col++, p->getStatusTxt());
			tc++;

			// VM size (total memory in kBytes)
			if (tc->visible && tc->supported)
				pli->setText(col++, s.setNum(p->getVm_size() / 1024));
			tc++;

			// VM RSS (Resident memory in kBytes)
			if (tc->visible && tc->supported)
				pli->setText(col++, s.setNum(p->getVm_rss() / 1024));
			tc++;

			// VM LIB (Shared memory in kBytes)
			if (tc->visible && tc->supported)
				pli->setText(col++, s.setNum(p->getVm_lib() / 1024));
			tc++;

			// Commandline
			if (tc->visible && tc->supported)
				pli->setText(col++, p->getCmdLine());
			tc++;

			if (p->getPid() == selectedProcess)
				newSelection = pli;
		}
		pl.removeFirst();
    }
	
	if (newSelection)
	{
		setSelected(newSelection, TRUE);
		ensureItemVisible(newSelection);
	}
}

int
ProcessList::selectedPid(void) const
{
	ProcessLVI* current = (ProcessLVI*) currentItem();

	if (!current)
		return (NONE);

	// get PID from 3rd column of the selected row
	QString pidStr = current->text(2);

	return (pidStr.toInt());
}

#define SETTABCOL(text, has) \
	tc->trHeader = new char[strlen(text) + 1]; \
	strcpy(tc->trHeader, text); \
	if (has) \
		tc->supported = true; \
	tc++

void
ProcessList::initTabCol(void)
{
	TABCOLUMN* tc = &TabCol[0];

	tc->trHeader = new char[strlen("") + 1];
	strcpy(tc->trHeader, "");
	tc++;

	tc->trHeader = new char[strlen(i18n("Name")) + 1];
	strcpy(tc->trHeader, i18n("Name"));
	tc++;

	tc->trHeader = new char[strlen(i18n("PID")) + 1];
	strcpy(tc->trHeader, i18n("PID"));
	tc++;

	SETTABCOL(i18n("User ID"), pl.hasUid());
	SETTABCOL(i18n("CPU"), pl.hasUserLoad() && pl.hasSysLoad());
	SETTABCOL(i18n("Time"), pl.hasUserTime() && pl.hasSysTime());
	SETTABCOL(i18n("Nice"), pl.hasNiceLevel());
	SETTABCOL(i18n("Status"), pl.hasStatus());
	SETTABCOL(i18n("Memory"), pl.hasVmSize());
	SETTABCOL(i18n("Resident"), pl.hasVmRss());
	SETTABCOL(i18n("Shared"), pl.hasVmLib());
	SETTABCOL(i18n("Command line"), pl.hasCmdLine());

	// determine the number of visible columns
	int cnt;

	/*
	 * Set the column witdh for all columns in the process list table.
	 * A dummy string that is somewhat longer than the real text is used
	 * to determine the width with the current font metrics.
	 */
	int col;
	QFontMetrics fm = fontMetrics();
	for (cnt = col = 0; cnt < MaxCols; cnt++)
		if (TabCol[cnt].visible && TabCol[cnt].supported)
		{
			addColumn(TabCol[cnt].trHeader);
			setColumnAlignment(col++, TabCol[cnt].alignment);
		}
	setItemMargin(1);
	setAllColumnsShowFocus(TRUE);
}

int 
ProcessList::mapV2T(int vcol)
{
	int tcol;
	int i = 0;
	for (tcol = 0; !TabCol[tcol].visible || (i < vcol); tcol++)
		if (TabCol[tcol].visible)
			i++;

	return (tcol);
}

int
ProcessList::mapT2V(int tcol)
{
	int vcol = 0;
	for (int i = 0; i < tcol; i++)
		if (TabCol[i].visible)
			vcol++;

	return (vcol);
}

void
ProcessList::handleRMBPopup(int item)
{
	switch (item)
	{
	case HEADER_REMOVE:
		/*
		 * The icon, name and PID columns cannot be removed, so currColumn
		 * must be greater than 2.
		 */
		if (currColumn > 2)
		{
			setColumnWidthMode(currColumn, Manual);
			setColumnWidth(currColumn, 0);
			update();
		}
		break;
	case HEADER_ADD:
		break;
	case HEADER_HELP:
		break;
	}
}

void 
ProcessList::mousePressEvent(QMouseEvent* e)
{
	/*
	 * I haven't found a better way to catch RMB clicks on the header than
	 * this hacking of the mousePressEvent function. RMB clicks are dealt
	 * with, all other events are passed through to the base class
	 * implementation.
	 */
	if (e->button() == RightButton)
	{
#if 0
		/*
		 * As long as QListView does not support removing or hiding of columns
		 * I will probably not implement this feature. I hope the Trolls will
		 * do this with the next Qt release!
		 */
		if (e->pos().y() <= 0)
		{
			/*
			 * e->pos().y() <= 0 means header.
			 */
			currColumn = header()->cellAt(e->pos().x());
			headerPM->popup(QCursor::pos());
		}
		else
#endif
		{
			/*
			 * The RMB was pressed over a process in the list. This process
			 * gets selected and a context menu pops up. The context menu is
			 * provided by the TaskMan class. A signal is emmited to notifiy
			 * the TaskMan object.
			 */
			ProcessLVI* lvi = (ProcessLVI*) itemAt(e->pos());
			setSelected(lvi, TRUE);
			emit(popupMenu(0, 0));
		}
	}
	else
		QListView::mousePressEvent(e);
}
