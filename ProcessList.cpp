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

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!
*/

// $Id$

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include <qheader.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpaintdevice.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include "MainMenu.h"
#include "OSProcessList.h"
#include "ProcessList.moc"

#define NONE -1
#define INIT_PID 1

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

extern KApplication* Kapp;

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
	sprintf(key, "%06.2f", percent);

	return (key);
}

QString
ProcessLVI::key(int column, bool) const
{
	if (TabCol[column].key)
		return ((*TabCol[column].key)(text(column)));
	else
		return (text(column));
}

ProcessList::ProcessList(QWidget *parent, const char* name)
	: QListView(parent, name)
{
	/*
	 * The refresh rate can be changed from the main menu. If this happens
	 * a signal is send. The menu has the current refresh rate checked. So we
	 * need to send a signal to the menu if the rate was changed.
	 */
	connect(this, SIGNAL(refreshRateChanged(int)),
			MainMenuBar, SLOT(checkRefreshRate(int)));
	connect(MainMenuBar, SIGNAL(setRefreshRate(int)),
			this, SLOT(setRefreshRate(int)));

	/*
	 * The filter mode is controlled by a combo box of the parent. If the
	 * mode is changed we get a signal. To notify the combo box of mode
	 * changes we send out a signal.
	 */
	connect(this, SIGNAL(filterModeChanged(int)),
			parent, SLOT(filterModeChanged(int)));
	connect(parent, SIGNAL(setFilterMode(int)),
			this, SLOT(setFilterMode(int)));

	/*
	 * When a new process is selected to receive a signal from the base
	 * class and repeat the signal to the menu. The menu keeps track of the
	 * currently selected process.
	 */
	connect(this, SIGNAL(selectionChanged(QListViewItem *)),
			SLOT(selectionChangedSlot(QListViewItem*)));
	connect(this, SIGNAL(processSelected(int)),
			MainMenuBar, SLOT(processSelected(int)));

	/*
	 * If the process list has changed we need to be informed by a signal
	 * about it to update the displayed process list.
	 */
	connect(MainMenuBar, SIGNAL(requestUpdate(void)),
			this, SLOT(update(void)));

	/* As long as the scrollbar sliders are pressed and hold the process
	 * list is frozen. */
	connect(verticalScrollBar(), SIGNAL(sliderPressed(void)),
			this, SLOT(timerOff()));
	connect(verticalScrollBar(), SIGNAL(sliderReleased(void)),
			this, SLOT(timerOn()));
	connect(horizontalScrollBar(), SIGNAL(sliderPressed(void)),
			this, SLOT(timerOff()));
	connect(horizontalScrollBar(), SIGNAL(sliderReleased(void)),
			this, SLOT(timerOn()));

	// no timer started yet
	timerId = NONE;

	treeViewEnabled = false;

	filterMode = FILTER_OWN;

	refreshRate = REFRESH_MEDIUM;

	sortColumn = 1;
	increasing = FALSE;

	// load the icons we display with the processes
	icons = new KIconLoader();
	CHECK_PTR(icons);

	// make sure we can retrieve process lists from the OS
	if (!pl.ok())
	{
		KMessageBox::error(this, pl.getErrMessage());
		abort();
	}

	initTabCol();

	// Create RMB popup to modify process attributes
	processMenu = new ProcessMenu();
	connect(this, SIGNAL(processSelected(int)),
			processMenu, SLOT(processSelected(int)));
	connect(processMenu, SIGNAL(requestUpdate(void)),
			this, SLOT(update(void)));

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
	timerOff();

	delete(processMenu);
	delete(headerPM);
}

void
ProcessList::loadSettings(void)
{
	treeViewEnabled = Kapp->getConfig()->readNumEntry("TreeView",
													  treeViewEnabled);
	emit(treeViewChanged(treeViewEnabled));

	/*
	 * The default filter mode is 'own processes'. This can be overridden by
	 * the config file.
	 */
	filterMode = Kapp->getConfig()->readNumEntry("FilterMode", filterMode);
	emit(filterModeChanged(filterMode));

	/*
	 * The default update rate is 'medium'. This can be overridden by the
	 * config file.
	 */
	setRefreshRate(Kapp->getConfig()->readNumEntry("RefreshRate",
												   refreshRate));

	// The default sorting is for the PID in decreasing order.
	sortColumn = Kapp->getConfig()->readNumEntry("SortColumn", 1);
	increasing = Kapp->getConfig()->readNumEntry("SortIncreasing", FALSE);
	QListView::setSorting(sortColumn, increasing);
}

void
ProcessList::saveSettings(void)
{
	Kapp->getConfig()->writeEntry("TreeView", treeViewEnabled);
	Kapp->getConfig()->writeEntry("FilterMode", filterMode);
	Kapp->getConfig()->writeEntry("RefreshRate", refreshRate);
	Kapp->getConfig()->writeEntry("SortColumn", sortColumn);
	Kapp->getConfig()->writeEntry("SortIncreasing", increasing);
}

void 
ProcessList::setRefreshRate(int r)
{
	assert(r >= REFRESH_MANUAL && r <= REFRESH_FAST);

	timerOff();
	switch (refreshRate = r)
	{
	case REFRESH_MANUAL:
		break;

	case REFRESH_SLOW:
		timerInterval = 20000;
		break;

	case REFRESH_MEDIUM:
		timerInterval = 7000;
		break;

	case REFRESH_FAST:
	default:
		timerInterval = 1000;
		break;
	}

	// only re-start the timer if auto mode is enabled
	if (refreshRate != REFRESH_MANUAL)
		timerOn();

	emit(refreshRateChanged(refreshRate));
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
	{
		sortColumn = tcol;
		increasing = inc;
	}

	QListView::setSorting(sortColumn, increasing);
}

int
ProcessList::setAutoUpdateMode(bool mode)
{
	/*
	 * If auto mode is enabled the display is updated regurlarly triggered
	 * by a timer event.
	 */

	// save current setting of the timer
	int oldmode = (timerId != NONE) ? TRUE : FALSE; 

	// set new setting
	if (mode && (refreshRate != REFRESH_MANUAL))
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
}

void 
ProcessList::load()
{
	int vpos = verticalScrollBar()->value();
	int hpos = horizontalScrollBar()->value();

	pl.clear();

	/* This piece of code tries to work around the QListView
	 * auto-shrink bug. The column width is always reset to the size
	 * required for the header. If any column entry needs more space
	 * QListView will resize the column again. Unfortunately this
	 * causes heavy flickering! */
	QFontMetrics fm = fontMetrics();
	int col = 0;
	for (int i = 0; i < MaxCols; i++)
		if (TabCol[i].visible && TabCol[i].supported)
			setColumnWidth(col++, fm.width(TabCol[i].trHeader) + 10);

	// request current list of processes
	if (!pl.update())
	{
		KMessageBox::error(this, pl.getErrMessage());
		abort();
	}

	if (treeViewEnabled)
		deleteLeaves();

	int selectedProcess = selectedPid();

	clear();

	ProcessLVI* newSelection = treeViewEnabled ?
		buildTree(selectedProcess) :
		buildList(selectedProcess);

#if 0
	if (newSelection)
	{
		setSelected(newSelection, TRUE);
		ensureItemVisible(newSelection);
	}
#endif

	/* This is necessary because the selected process may has
	 * disappeared without ktop's interaction. Since there are widgets
	 * that always need to know the currently selected process we send
	 * out a processSelected signal. */
	emit(processSelected(selectedPid()));

	verticalScrollBar()->setValue(vpos);
	horizontalScrollBar()->setValue(hpos);
}

bool
ProcessList::matchesFilter(OSProcess* p) const
{
	// This mechanism is likely to change in the future!

	switch (filterMode)
	{
	case FILTER_ALL:
		return (true);

	case FILTER_SYSTEM:
		return (p->getUid() < 100 ? true : false);

	case FILTER_USER:
		return (p->getUid() >= 100 ? true : false);

	case FILTER_OWN:
	default:
		return (p->getUid() == getuid() ? true : false);
	}
}

ProcessLVI*
ProcessList::buildList(int selectedProcess)
{
	ProcessLVI* newSelection = 0;

	/*
	 * Get the first process in the list, check whether it matches the filter
	 * and append it to QListView widget if so.
	 */
	while (!pl.isEmpty())
	{
		OSProcess* p = pl.first();

		if (matchesFilter(p))
		{
			ProcessLVI* pli = new ProcessLVI(this);

			addProcess(p, pli);

			if (p->getPid() == selectedProcess)
				newSelection = pli;
		}
		pl.removeFirst();
    }

	return (newSelection);
}

ProcessLVI*
ProcessList::buildTree(int selectedProcess)
{
	ProcessLVI* newSelection = 0;

	if (treeViewEnabled)
	{
		OSProcess* ps = pl.first();

		while (ps)
		{
			if (ps->getPid() == INIT_PID)
			{
				// insert root item into the tree widget
				ProcessLVI* pli = new ProcessLVI(this);
				addProcess(ps, pli);

				if (ps->getPid() == selectedProcess)
					newSelection = pli;

				extendTree(&pl, pli, ps->getPid(),
						  &newSelection, selectedProcess);
				break;
			}
			else
				ps = pl.next();
		}
	}

	return (newSelection);
}

void
ProcessList::deleteLeaves(void)
{
	for ( ; ; )
	{
		unsigned int i;
		for (i = 0; i < pl.count() &&
		            (!isLeafProcess(pl.at(i)->getPid()) ||
					 matchesFilter(pl.at(i))); i++)
			;
		if (i == pl.count())
			return;

		pl.remove(i);
	}
}

bool
ProcessList::isLeafProcess(int pid)
{
	
	for (unsigned int i = 0; i < pl.count(); i++)
		if (pl.at(i)->getPpid() == pid)
			return (false);

	return (true);
}

void
ProcessList::extendTree(OSProcessList* pl, ProcessLVI* parent, int ppid,
						ProcessLVI** newSelection, int selectedProcess)
{
	OSProcess* ps;

	// start at top list
	ps = pl->first();

	while (ps)
	{
		// look for a child process of the current parent
		if (ps->getPpid() == ppid)
		{
			ProcessLVI* pli = new ProcessLVI(parent);
			
			addProcess(ps, pli);

			// if process was the previous selected one save pointer to LVI
			if (ps->getPid() == selectedProcess)
				*newSelection = pli;

			// set parent to 'open'
			setOpen(parent, TRUE);

			// remove the process from the process list
			pl->remove();

			// now look for the childs of the inserted process
			extendTree(pl, pli, ps->getPid(), newSelection, selectedProcess);

			/*
			 * Since buildTree can remove processes from the list we can't
			 * find a "current" process. So we start searching at the top
			 * again. It's no endless loops since this branch is only entered
			 * when there are childs of the current parents in the list. When
			 * we have removed them all the while loop will exit.
			 */
			ps = pl->first();
		}
		else
			ps = pl->next();
	}
	
}

void
ProcessList::addProcess(OSProcess* p, ProcessLVI* pli)
{
	/*
	 * Get icon from icon list that might be appropriate for a process
	 * with this name.
	 */
	QPixmap pix = icons->loadApplicationMiniIcon(QString(p->getName())
												 + ".png", 16, 16);
	if (pix.isNull())
	{
		QString s = locate("toolbar", QString(p->getName()) + ".png");
		debug(QString("using %1...").arg(s));
		pix = QPixmap(s);
		if (pix.isNull())
			pix = icons->loadApplicationMiniIcon("default.png", 16, 16);
	}

	/*
	 * We copy the icon into a 24x16 pixmap to add a 4 pixel margin on the
	 * left and right side. In tree view mode we use the original icon.
	 */
	QPixmap icon(24, 16, pix.depth());
	if (!treeViewEnabled)
	{
		icon.fill();
		bitBlt(&icon, 4, 0, &pix, 0, 0, pix.width(), pix.height());
		QBitmap mask(24, 16, TRUE);
		bitBlt(&mask, 4, 0, pix.mask(), 0, 0, pix.width(), pix.height());
		icon.setMask(mask);
	}

	int col = 0;
	// icon + process name
	pli->setPixmap(col, treeViewEnabled ? pix : icon);
	pli->setText(col++, p->getName());

	QString s;

	// pid
	pli->setText(col++, s.setNum(p->getPid()));

	TABCOLUMN* tc = &TabCol[2];

	// user name
	if (tc->visible && tc->supported)
		pli->setText(col++, p->getUserName());
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
		pli->setText(col++, s.setNum(p->getNiceLevel()));
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
}

int
ProcessList::selectedPid(void) const
{
	ProcessLVI* current = (ProcessLVI*) currentItem();

	if (!current || !isSelected(current))
		return (NONE);

	// get PID from 2nd column of the selected row
	QString pidStr = current->text(1);

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
	setTreeStepSize(17);
	QListView::setSorting(sortColumn, increasing);
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
//			header()->setCellSize(currColumn, 0);
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
ProcessList::viewportMousePressEvent(QMouseEvent* e)
{
	printf("mousePressEvent\n");
	/*
	 * I haven't found a better way to catch RMB clicks on the header than
	 * this hacking of the mousePressEvent function. RMB clicks are dealt
	 * with, all other events are passed through to the base class
	 * implementation.
	 */
	if (e->button() == RightButton)
	{
		printf("RMB pressed\n");
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
			/*
			 * I tried e->pos() instead of QCursor::pos() but then the menu
			 * appears centered above the cursor which is annoying.
			 */
			processMenu->popup(QCursor::pos());
		}
	}
	else if (e->button() == LeftButton)
	{
		printf("LMB pressed\n");
	}
	else
		QListView::mousePressEvent(e);
}





