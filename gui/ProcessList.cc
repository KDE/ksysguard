/*
    KTop, the KDE Task Manager
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net

	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <config.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <qheader.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpaintdevice.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstddirs.h>

#include "ProcessController.h"
#include "SensorManager.h"
#include "ProcessList.moc"

#define NONE -1
#define INIT_PID 1

static const char* intKey(const char* text);
static const char* timeKey(const char* text);
static const char* floatKey(const char* text);

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
	static char key[32];
	sprintf(key, "%016d", val);

	return (key);
}

static const char*
timeKey(const char* text)
{
	int h, m;
	sscanf(text, "%d:%d", &h, &m);
	int t = h * 60 + m;
	static char key[32];
	sprintf(key, "%010d", t);

	return (key);
}

static const char*
floatKey(const char* text)
{
	double percent;
	sscanf(text, "%lf", &percent);

	static char key[32];
	sprintf(key, "%010.2f", percent);

	return (key);
}

QString
ProcessLVI::key(int column, bool) const
{
	QValueList<KeyFunc> kf = ((ProcessList*) listView())->getSortFunc();
	KeyFunc func = *(kf.at(column));
	if (func)
		return (func(text(column)));

	return (text(column));
}

ProcessList::ProcessList(QWidget *parent, const char* name)
	: QListView(parent, name)
{
	/* The filter mode is controlled by a combo box of the parent. If
	 * the mode is changed we get a signal. */
	connect(parent, SIGNAL(setFilterMode(int)),
			this, SLOT(setFilterMode(int)));

	/* When a new process is selected to receive a signal from the
	 * base class and repeat the signal to the menu. The menu keeps
	 * track of the currently selected process. */
	connect(this, SIGNAL(selectionChanged(QListViewItem *)),
			SLOT(selectionChangedSlot(QListViewItem*)));

	/* As long as the scrollbar sliders are pressed and hold the process
	 * list is frozen. */
	connect(verticalScrollBar(), SIGNAL(sliderPressed(void)),
			parent, SLOT(timerOff()));
	connect(verticalScrollBar(), SIGNAL(sliderReleased(void)),
			parent, SLOT(timerOn()));
	connect(horizontalScrollBar(), SIGNAL(sliderPressed(void)),
			parent, SLOT(timerOff()));
	connect(horizontalScrollBar(), SIGNAL(sliderReleased(void)),
			parent, SLOT(timerOn()));

	treeViewEnabled = false;

	filterMode = FILTER_ALL;

	sortColumn = 1;
	increasing = FALSE;

	// Elements in the process list may only live in this list.
	pl.setAutoDelete(TRUE);

	// load the icons we display with the processes
	icons = new KIconLoader();
	CHECK_PTR(icons);

	setItemMargin(1);
	setAllColumnsShowFocus(TRUE);
	setTreeStepSize(17);
	setSorting(sortColumn, increasing);
	setSelectionMode(Multi);

	// Create RMB popup to modify process attributes
	processMenu = new ProcessMenu();
	connect(this, SIGNAL(processSelected(int)),
			processMenu, SLOT(processSelected(int)));

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

	delete(processMenu);
	delete(headerPM);
}

void
ProcessList::loadSettings(void)
{
#if 0
	treeViewEnabled = Kapp->config()->readNumEntry("TreeView",
												   treeViewEnabled);

	emit(treeViewChanged(treeViewEnabled));

	/* The default filter mode is 'own processes'. This can be overridden by
	 * the config file. */
	filterMode = Kapp->config()->readNumEntry("FilterMode", filterMode);
	emit(filterModeChanged(filterMode));

	// The default sorting is for the PID in decreasing order.
	sortColumn = Kapp->config()->readNumEntry("SortColumn", 1);
	increasing = Kapp->config()->readNumEntry("SortIncreasing", FALSE);
	setSorting(sortColumn, increasing);
#endif
}

void
ProcessList::saveSettings(void)
{
#if 0
	Kapp->config()->writeEntry("TreeView", treeViewEnabled);
	Kapp->config()->writeEntry("FilterMode", filterMode);
	Kapp->config()->writeEntry("SortColumn", sortColumn);
	Kapp->config()->writeEntry("SortIncreasing", increasing);
#endif
}

void 
ProcessList::update(const QString& list)
{
	pl.clear();

	// Convert ps answer in a list of tokenized lines
	SensorTokenizer procs(list, '\n');
	for (unsigned int i = 0; i < procs.numberOfTokens(); i++)
		pl.append(new SensorPSLine(procs[i]));

	int vpos = verticalScrollBar()->value();
	int hpos = horizontalScrollBar()->value();

	updateSelectedPIds();

	clear();
#if 0
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
#endif

	if (treeViewEnabled)
		buildTree();
	else
		buildList();

	verticalScrollBar()->setValue(vpos);
	horizontalScrollBar()->setValue(hpos);
}

bool
ProcessList::matchesFilter(SensorPSLine* p) const
{
	// This mechanism is likely to change in the future!

	switch (filterMode)
	{
	case FILTER_ALL:
		return (true);

	case FILTER_SYSTEM:
		return (p->getUId() < 100 ? true : false);

	case FILTER_USER:
		return (p->getUId() >= 100 ? true : false);

	case FILTER_OWN:
	default:
		return (p->getUId() == (long) getuid() ? true : false);
	}
}

void
ProcessList::buildList()
{
	/* Get the first process in the list, check whether it matches the
	 * filter and append it to QListView widget if so. */
	while (!pl.isEmpty())
	{
		SensorPSLine* p = pl.first();

		if (matchesFilter(p))
		{
			ProcessLVI* pli = new ProcessLVI(this);

			addProcess(p, pli);

			if (selectedPIds.findIndex(p->getPid()) != -1)
				pli->setSelected(true);
		}
		pl.removeFirst();
    }
}

void
ProcessList::buildTree()
{
	// remove all leaves that do not match the filter
	deleteLeaves();

	SensorPSLine* ps = pl.first();

	while (ps)
	{
		if (ps->getPid() == INIT_PID)
		{
			// insert root item into the tree widget
			ProcessLVI* pli = new ProcessLVI(this);
			addProcess(ps, pli);

			// remove the process from the process list, ps is now invalid
			int pid = ps->getPid();
			pl.remove();

			if (selectedPIds.findIndex(pid) != -1)
				pli->setSelected(true);

			// insert all child processes of current process
			extendTree(&pl, pli, pid);
			break;
		}
		else
			ps = pl.next();
	}
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
		if (pl.at(i)->getPPid() == pid)
			return (false);

	return (true);
}

void
ProcessList::extendTree(QList<SensorPSLine>* pl, ProcessLVI* parent, int ppid)
{
	SensorPSLine* ps;

	// start at top list
	ps = pl->first();

	while (ps)
	{
		// look for a child process of the current parent
		if (ps->getPPid() == ppid)
		{
			ProcessLVI* pli = new ProcessLVI(parent);
			
			addProcess(ps, pli);

			if (selectedPIds.findIndex(ps->getPid()) != -1)
				pli->setSelected(true);

			// set parent to 'open'
			setOpen(parent, TRUE);

			// remove the process from the process list, ps is now invalid
			int pid = ps->getPid();
			pl->remove();

			// now look for the childs of the inserted process
			extendTree(pl, pli, pid);

			/* Since buildTree can remove processes from the list we
			 * can't find a "current" process. So we start searching
			 * at the top again. It's no endless loops since this
			 * branch is only entered when there are children of the
			 * current parent in the list. When we have removed them
			 * all the while loop will exit. */
			ps = pl->first();
		}
		else
			ps = pl->next();
	}
}

void
ProcessList::addProcess(SensorPSLine* p, ProcessLVI* pli)
{
	/* Get icon from icon list that might be appropriate for a process
	 * with this name. */
	QPixmap pix = icons->loadIcon(p->getName(), KIcon::Desktop,
								  KIcon::SizeSmall);
	if (pix.isNull())
	{
		pix = QPixmap(BarIcon(p->getName()));
		if (pix.isNull())
			pix = icons->loadIcon("default", KIcon::Desktop,
								  KIcon::SizeSmall);
	}

	/* We copy the icon into a 24x16 pixmap to add a 4 pixel margin on
	 * the left and right side. In tree view mode we use the original
	 * icon. */
	QPixmap icon(24, 16, pix.depth());
	if (!treeViewEnabled)
	{
		icon.fill();
		bitBlt(&icon, 4, 0, &pix, 0, 0, pix.width(), pix.height());
		QBitmap mask(24, 16, TRUE);
		bitBlt(&mask, 4, 0, pix.mask(), 0, 0, pix.width(), pix.height());
		icon.setMask(mask);
	}

	// icon + process name
	pli->setPixmap(0, treeViewEnabled ? pix : icon);
	pli->setText(0, p->getName());

	QString s;

	// insert remaining field into table
	for (unsigned int col = 1; col < p->numberOfTokens(); col++)
		pli->setText(col, (*p)[col]);
}

void
ProcessList::updateSelectedPIds(void)
{
	selectedPIds.clear();

    QListViewItemIterator it(this);

	// iterate through all items of the listview
	for ( ; it.current(); ++it )
	{
		if (it.current()->isSelected())
			selectedPIds.append(it.current()->text(1).toInt());
    }	
}

void
ProcessList::removeColumns(void)
{
	for (int i = columns() - 1; i >= 0; --i)
		removeColumn(i);
}

void
ProcessList::addColumn(const QString& header, const QString& type)
{
	uint col = sortFunc.count();
	QListView::addColumn(header);
	if (type == "s")
	{
		setColumnAlignment(col, AlignLeft);
		sortFunc.append(0);
	}
	else if (type == "d")
	{
		setColumnAlignment(col, AlignRight);
		sortFunc.append(&intKey);
	}
	else if (type == "t")
	{
		setColumnAlignment(col, AlignRight);
		sortFunc.append(&timeKey);
	}
	else	// should be type "f"
	{
		setColumnAlignment(col, AlignRight);
		sortFunc.append(floatKey);
	}
}

int 
ProcessList::mapV2T(int vcol)
{
#if 0
	int tcol;
	int i = 0;
	for (tcol = 0; !TabCol[tcol].visible || (i < vcol); tcol++)
		if (TabCol[tcol].visible)
			i++;
	return (tcol);
#endif
	return vcol;
}

int
ProcessList::mapT2V(int tcol)
{
#if 0
	int vcol = 0;
	for (int i = 0; i < tcol; i++)
		if (TabCol[i].visible)
			vcol++;

	return (vcol);
#endif
	return (tcol);
}

void
ProcessList::handleRMBPopup(int item)
{
	switch (item)
	{
	case HEADER_REMOVE:
		/* The icon, name and PID columns cannot be removed, so
		 * currColumn must be greater than 2. */
		if (currColumn > 2)
		{
			setColumnWidthMode(currColumn, Manual);
			setColumnWidth(currColumn, 0);
//			header()->setCellSize(currColumn, 0);
//			update();
		}
		break;
	case HEADER_ADD:
		break;
	case HEADER_HELP:
		break;
	}
}

#if 0
void 
ProcessList::viewportMousePressEvent(QMouseEvent* e)
{
	/* I haven't found a better way to catch RMB clicks on the header
	 * than this hacking of the mousePressEvent function. RMB clicks
	 * are dealt with, all other events are passed through to the base
	 * class implementation. */
	if (e->button() == RightButton)
	{
		/* As long as QListView does not support removing or hiding of
		 * columns I will probably not implement this feature. I hope
		 * the Trolls will do this with the next Qt release! */
		if (e->pos().y() <= 0)
		{
			/*
			 * e->pos().y() <= 0 means header.
			 */
			currColumn = header()->cellAt(e->pos().x());
			headerPM->popup(QCursor::pos());
		}
		else
		{
			/* The RMB was pressed over a process in the list. This
			 * process gets selected and a context menu pops up. The
			 * context menu is provided by the TaskMan class. A signal
			 * is emmited to notifiy the TaskMan object. */
			ProcessLVI* lvi = (ProcessLVI*) itemAt(e->pos());
			setSelected(lvi, TRUE);
			/* I tried e->pos() instead of QCursor::pos() but then the
			 * menu appears centered above the cursor which is
			 * annoying. */
			processMenu->popup(QCursor::pos());
		}
	}
	else if (e->button() == LeftButton)
#if 0
	{
	}
	else
#endif
		QListView::mousePressEvent(e);
}
#endif
