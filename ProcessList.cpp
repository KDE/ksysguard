/*
    KTop, the KDE Taskmanager
   
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
#include <assert.h>

#include <qmessagebox.h>

#include <kapp.h>
#include <klocale.h>

#include "OSProcessList.h"
#include "ProcessList.moc"

#define NONE -1

typedef struct
{
	const char* header;
	char* trHeader;
	char* placeholder;
	bool visible;
	bool supported;
	KTabListBox::ColumnType type;
	OSProcessList::SORTKEY sortMethod;
} TABCOLUMN;

/*
 * The following array defined the setup of the tab dialog. It contains
 * the column header, a placeholder to determine the column width, the
 * visible flag and the type of the column.
 *
 * In later the columns can be turned on and off through an options dialog.
 * The same mechanism can be used if a platform does not support a certain
 * type of information.
 *
 * This table, the construction of the KTabListBox lines in
 * ProcessList::initTabCol and ProcessList::load() MUST be keept in sync!
 * Also, the order and existance of the first two columns (icon and
 * pid) is mandatory! This i18n() mechanism is way too inflexible!
 */
static TABCOLUMN TabCol[] =
{
	{ "", 0, "++++", true, true, KTabListBox::MixedColumn,
	  OSProcessList::SORTBY_NAME },
	{ "PID", 0, "++++++", true, true, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_PID },
	{ "Name", 0, "kfontmanager++", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_NAME},
	{ "User ID", 0, "rootuseroot", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_UID },
	{ "CPU", 0, "100.00%+", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_CPU },
	{ "Time", 0, "100:00++", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_TIME },
	{ "Nice", 0, "-20+", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_PRIORITY },
	{ "Status", 0, "Status+++", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_STATUS },
	{ "Memory", 0, "VmSize++", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_VMSIZE },
	{ "Resident", 0, "VmSize++", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_VMRSS },
	{ "Shared", 0, "VmSize++", true, false, KTabListBox::TextColumn,
	  OSProcessList::SORTBY_VMLIB }
};

static const MaxCols = sizeof(TabCol) / sizeof(TABCOLUMN);

inline int max(int a, int b)
{
	return ((a) < (b) ? (b) : (a));
}

ProcessList::ProcessList(QWidget *parent = 0, const char* name = 0)
	: KTabListBox(parent, name)
{
	setSeparator(';');

	// no timer started yet
	timer_id = NONE;

	enableKey();

	/*
	 * The first selected process is ktop itself because we are sure it
	 * exists and we can easyly get the pid.
	 */
	lastSelectionPid = getpid();

	/*
	 * The default filter mode is 'own processes'. This can be overridden by
	 * the config file.
	 */
	filtermode = FILTER_OWN;

	/*
	 * The default update rate is 'fast'. This can be overridden by the
	 * config file.
	 */
	update_rate = UPDATE_FAST;

	// load the icons we display with the processes
	icons = new KtopIconList;
	CHECK_PTR(icons);

	// make sure we can retrieve process lists from the OS
	if (!pl.ok())
	{
		QMessageBox::critical(this, "ktop", pl.getErrMessage(), 0, 0);
		assert(0);
	}

	initTabCol();

	// Clicking on the header changes the sort order.
	connect(this, SIGNAL(headerClicked(int)),
			SLOT(userClickOnHeader(int)));

	// Clicking in the table can change the process selection.
	connect(this, SIGNAL(highlighted(int,int)),
			SLOT(procHighlighted(int, int)));
}

ProcessList::~ProcessList()
{
	// remove icon list from memory
	delete icons;

	// switch off timer
	if (timer_id != NONE)
		killTimer(timer_id);
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
ProcessList::setSortColumn(int c)
{
	/*
	 * We need to make sure that the specified column is visible. If it's not
	 * we use the first column (PID).
	 */
	if ((c >= 0) && (c < MaxCols) && TabCol[c].visible)
		sortColumn = c;
	else
		sortColumn = 1;
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
	// save the index of the first row item
	int top_Item = topItem();

	// disable the auto-update and save current mode
	int lastmode = setAutoUpdateMode(FALSE);
	setAutoUpdate(FALSE);

	// retrieve current process list from OS and update tab dialog
	load();

	try2restoreSelection();

	// restore the top visible item if possible
	setTopItem(top_Item);

	setAutoUpdate(TRUE);
	setAutoUpdateMode(lastmode);

    if(isVisible())
		repaint();
}

void 
ProcessList::load()
{
	pl.clear();

	pl.setSortCriteria(TabCol[sortColumn].sortMethod);

	// request current list of processes
	if (!pl.update())
	{
		QMessageBox::critical(this, "ktop", pl.getErrMessage(), 0, 0);
		assert(0);
	}

	clear();

	// clear the tab dialog's dictionary (stores the icons by name)
	dict().clear();  

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

			// insert icon into tab dialog's dictionary
			dict().insert((const char*)p->getName(), pix);

			/*
			 * Construct the string for the KTabListBox lines. These lines
			 * and the TabCol array MUST be kept in sync!
			 */
			QString line = "";
			QString s;

			// icon
			line += QString("{") + p->getName() + "};";

			// pid
			line += s.setNum(p->getPid()) + ";";

			TABCOLUMN* tc = &TabCol[2];
			// process name
			if (tc->visible && tc->supported)
				line += p->getName() + QString(";");
			tc++;

			// user name
			if (tc->visible && tc->supported)
				line += p->getUserName() + QString(";");
			tc++;

			// CPU load
			if (tc->visible && tc->supported)
				line += s.sprintf("%.2f%%;",
								  p->getUserLoad() + p->getSysLoad());
			tc++;

			// total processing time
			if (tc->visible && tc->supported)
			{
				int totalTime = p->getUserTime() + p->getSysTime();
				line += s.sprintf("%d:%02d;",
								  (totalTime / 100) / 60,
								  (totalTime / 100) % 60);
			}
			tc++;

			// process nice level
			if (tc->visible && tc->supported)
				line += s.sprintf("%d;", p->getNiceLevel());
			tc++;

			// process status
			if (tc->visible && tc->supported)
				line += p->getStatusTxt() + QString(";");
			tc++;

			// VM size (total memory in kBytes)
			if (tc->visible && tc->supported)
				line += s.setNum(p->getVm_size() / 1024) + ";";
			tc++;

			// VM rss
			if (tc->visible && tc->supported)
				line += s.setNum(p->getVm_rss() / 1024) + ";";
			tc++;

			// VM lib
			if (tc->visible && tc->supported)
				line += s.setNum(p->getVm_lib() / 1024) + ";";
			tc++;

			appendItem(line);
		}
		pl.removeFirst();
    }
}

void 
ProcessList::userClickOnHeader(int colIndex)
{
	setSortColumn(colIndex);
	update();
}

void 
ProcessList::try2restoreSelection()
{
	int err = 0;

	// Find out if the selected process in still in process list.
	if (lastSelectionPid != NONE)
		err = kill(lastSelectionPid, 0);

	// If not select ktop again.
	if (err || (lastSelectionPid == NONE))
		lastSelectionPid = getpid();
 	restoreSelection();
}

void 
ProcessList::restoreSelection()
{
	QString txt;
	int cnt = count();
	int pid;
	bool res = FALSE;

	// Find the last selected process and select it again.
	for (int i = 0; i < cnt; i++)
	{
		// the line starts with the pid
		txt = text(i, 1);
		res = FALSE;
		pid = txt.toInt(&res);
		if (res && (pid == lastSelectionPid))
		{
			setCurrentItem(i);
			return;
		}
	}
}

void 
ProcessList::procHighlighted(int indx, int)
{ 
	lastSelectionPid = NONE;
	sscanf(text(indx, 1), "%d", &lastSelectionPid);
} 

int 
ProcessList::cellHeight(int row)
{
	const QPixmap *pix = icons->procIcon(text(row, 2));

	if (pix)
		return (pix->height());

	return (18);	// Why not 42? How can we make this more sensible?
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
	TABCOLUMN* tc = &TabCol[1];

	tc->trHeader = new char[strlen(i18n("PID")) + 1];
	strcpy(tc->trHeader, i18n("PID"));
	tc++;

	SETTABCOL(i18n("Name"), pl.hasName());
	SETTABCOL(i18n("User ID"), pl.hasUid());
	SETTABCOL(i18n("CPU"), pl.hasUserLoad && pl.hasSysLoad());
	SETTABCOL(i18n("Time"), pl.hasUserTime() && pl.hasSysTime());
	SETTABCOL(i18n("Nice"), pl.hasNiceLevel());
	SETTABCOL(i18n("Status"), pl.hasStatus());
	SETTABCOL(i18n("Memory"), pl.hasVmSize());
	SETTABCOL(i18n("Resident"), pl.hasVmRss());
	SETTABCOL(i18n("Shared"), pl.hasVmLib());

	// determine the number of visible columns
	int columns = 0;
	int cnt;
	for (cnt = 0; cnt < MaxCols; cnt++)
		if (TabCol[cnt].visible && TabCol[cnt].supported)
			columns++;
	setNumCols(columns);

	/*
	 * Set the column witdh for all columns in the process list table.
	 * A dummy string that is somewhat longer than the real text is used
	 * to determine the width with the current font metrics.
	 */
	QFontMetrics fm = fontMetrics();
	int col = 0;
	for (cnt = 0; cnt < MaxCols; cnt++)
		if (TabCol[cnt].visible && TabCol[cnt].supported)
		{
			setColumn(col++, TabCol[cnt].trHeader,
					  max(fm.width(TabCol[cnt].placeholder),
						  fm.width(TabCol[cnt].trHeader) + 7),
					  TabCol[cnt].type);
		}
}
