/*
    KSysGuard, the KDE System Guard

    Copyright (C) 1997 Bernd Johannes Wuebben
                       <wuebben@math.cornell.edu>

    Copyright (C) 1998 Nicolas Leclercq <nicknet@planete.net>

    Copyright (c) 1999, 2000, 2001, 2002 Chris Schlaeger <cs@kde.org>

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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <assert.h>
#include <config.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <qbitmap.h>
#include <qheader.h>
#include <qimage.h>
#include <qpopupmenu.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "ProcessController.h"
#include "ProcessList.moc"
#include "ReniceDlg.h"
#include "SignalIDs.h"

#define NONE -1
#define INIT_PID 1

extern const char* intKey(const char* text);
extern const char* timeKey(const char* text);
extern const char* floatKey(const char* text);

QDict<QString> ProcessList::aliases;

QString
ProcessLVI::key(int column, bool) const
{
	QValueList<KeyFunc> kf = ((ProcessList*) listView())->getSortFunc();
	KeyFunc func = *(kf.at(column));
	if (func)
		return (func(text(column).latin1()));

	return (text(column));
}

ProcessList::ProcessList(QWidget *parent, const char* name)
	: QListView(parent, name)
{
	iconCache.setAutoDelete(true);

	columnDict.setAutoDelete(true);
	columnDict.insert("running",
					  new QString(i18n("process status", "running")));
	columnDict.insert("sleeping",
					  new QString(i18n("process status", "sleeping")));
	columnDict.insert("disk sleep",
					  new QString(i18n("process status", "disk sleep")));
	columnDict.insert("zombie", new QString(i18n("process status", "zombie")));
	columnDict.insert("stopped",
					  new QString(i18n("process status", "stopped")));
	columnDict.insert("paging", new QString(i18n("process status", "paging")));
	columnDict.insert("idle", new QString(i18n("process status", "idle")));

	if (aliases.isEmpty())
	{
		aliases.insert("init", new QString("penguin"));
		/* kernel stuff */
		aliases.insert("bdflush", new QString("kernel"));
		aliases.insert("dhcpcd", new QString("kernel"));
		aliases.insert("kapm-idled", new QString("kernel"));
		aliases.insert("keventd", new QString("kernel"));
		aliases.insert("khubd", new QString("kernel"));
		aliases.insert("klogd", new QString("kernel"));
		aliases.insert("kreclaimd", new QString("kernel"));
		aliases.insert("kreiserfsd", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU0", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU1", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU2", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU3", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU4", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU5", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU6", new QString("kernel"));
		aliases.insert("ksoftirqd_CPU7", new QString("kernel"));
		aliases.insert("kswapd", new QString("kernel"));
		aliases.insert("kupdated", new QString("kernel"));
		aliases.insert("mdrecoveryd", new QString("kernel"));
		aliases.insert("scsi_eh_0", new QString("kernel"));
		aliases.insert("scsi_eh_1", new QString("kernel"));
		aliases.insert("scsi_eh_2", new QString("kernel"));
		aliases.insert("scsi_eh_3", new QString("kernel"));
		aliases.insert("scsi_eh_4", new QString("kernel"));
		aliases.insert("scsi_eh_5", new QString("kernel"));
		aliases.insert("scsi_eh_6", new QString("kernel"));
		aliases.insert("scsi_eh_7", new QString("kernel"));
		/* daemon and other service providers */
		aliases.insert("artsd", new QString("daemon"));
		aliases.insert("atd", new QString("daemon"));
		aliases.insert("automount", new QString("daemon"));
		aliases.insert("cardmgr", new QString("daemon"));
		aliases.insert("cron", new QString("daemon"));
		aliases.insert("cupsd", new QString("daemon"));
		aliases.insert("in.identd", new QString("daemon"));
		aliases.insert("lpd", new QString("daemon"));
		aliases.insert("mingetty", new QString("daemon"));
		aliases.insert("nscd", new QString("daemon"));
		aliases.insert("portmap", new QString("daemon"));
		aliases.insert("rpc.statd", new QString("daemon"));
		aliases.insert("rpciod", new QString("daemon"));
		aliases.insert("sendmail", new QString("daemon"));
		aliases.insert("sshd", new QString("daemon"));
		aliases.insert("syslogd", new QString("daemon"));
		aliases.insert("usbmgr", new QString("daemon"));
		aliases.insert("wwwoffled", new QString("daemon"));
		aliases.insert("xntpd", new QString("daemon"));
		aliases.insert("ypbind", new QString("daemon"));
		/* kde applications */
		aliases.insert("appletproxy", new QString("kdeapp"));
		aliases.insert("dcopserver", new QString("kdeapp"));
		aliases.insert("kcookiejar", new QString("kdeapp"));
		aliases.insert("kde", new QString("kdeapp"));
		aliases.insert("kded", new QString("kdeapp"));
		aliases.insert("kdeinit", new QString("kdeapp"));
		aliases.insert("kdesktop", new QString("kdeapp"));
		aliases.insert("kdesud", new QString("kdeapp"));
		aliases.insert("kdm", new QString("kdeapp"));
		aliases.insert("khotkeys", new QString("kdeapp"));
		aliases.insert("kio_file", new QString("kdeapp"));
		aliases.insert("kio_uiserver", new QString("kdeapp"));
		aliases.insert("klauncher", new QString("kdeapp"));
		aliases.insert("ksmserver", new QString("kdeapp"));
		aliases.insert("kwrapper", new QString("kdeapp"));
		aliases.insert("kwrited", new QString("kdeapp"));
		aliases.insert("kxmlrpcd", new QString("kdeapp"));
		aliases.insert("startkde", new QString("kdeapp"));
		/* other processes */
		aliases.insert("bash", new QString("shell"));
		aliases.insert("cat", new QString("tools"));
		aliases.insert("egrep", new QString("tools"));
		aliases.insert("emacs", new QString("wordprocessing"));
		aliases.insert("fgrep", new QString("tools"));
		aliases.insert("find", new QString("tools"));
		aliases.insert("grep", new QString("tools"));
		aliases.insert("ksh", new QString("shell"));
		aliases.insert("screen", new QString("openterm"));
		aliases.insert("sh", new QString("shell"));
		aliases.insert("sort", new QString("tools"));
		aliases.insert("ssh", new QString("shell"));
		aliases.insert("su", new QString("tools"));
		aliases.insert("tcsh", new QString("shell"));
		aliases.insert("tee", new QString("tools"));
		aliases.insert("vi", new QString("wordprocessing"));
	}

	/* The filter mode is controlled by a combo box of the parent. If
	 * the mode is changed we get a signal. */
	connect(parent, SIGNAL(setFilterMode(int)),
			this, SLOT(setFilterMode(int)));

	/* We need to catch this signal to show various popup menues. */
	connect(this,
			SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
			this,
			SLOT(handleRMBPressed(QListViewItem*, const QPoint&, int)));

	/* Since Qt does not tell us the sorting details we have to do our
	 * own bookkeping, so we can save and restore the sorting
	 * settings. */
	connect(header(), SIGNAL(clicked(int)), this, SLOT(sortingChanged(int)));

	treeViewEnabled = false;
	openAll = true;

	filterMode = FILTER_ALL;

	sortColumn = 1;
	increasing = false;

	// Elements in the process list may only live in this list.
	pl.setAutoDelete(true);

	// load the icons we display with the processes
	icons = new KIconLoader();
	Q_CHECK_PTR(icons);

	setItemMargin(2);
	setAllColumnsShowFocus(true);
	setTreeStepSize(17);
	setSorting(sortColumn, increasing);
	setSelectionMode(Multi);

	// Create popup menu for RMB clicks on table header
	headerPM = new QPopupMenu();
	headerPM->insertItem(i18n("Remove Column"), HEADER_REMOVE);
	headerPM->insertItem(i18n("Add Column"), HEADER_ADD);
	headerPM->insertItem(i18n("Help on Column"), HEADER_HELP);

	connect(header(), SIGNAL(sizeChange(int, int, int)),
			this, SLOT(sizeChanged(int, int, int)));
	connect(header(), SIGNAL(indexChange(int, int, int)),
			this, SLOT(indexChanged(int, int, int)));

	killSupported = false;
	setModified(false);
}

ProcessList::~ProcessList()
{
	// remove icon list from memory
	delete icons;

	delete(headerPM);
}

const QValueList<int>&
ProcessList::getSelectedPIds()
{
	selectedPIds.clear();
	// iterate through all items of the listview and find selected processes
	QListViewItemIterator it(this);
	for ( ; it.current(); ++it )
		if (it.current()->isSelected())
			selectedPIds.append(it.current()->text(1).toInt());

	return (selectedPIds);
}

bool
ProcessList::update(const QString& list)
{
	/* Disable painting to avoid flickering effects,
	 * especially when in tree view mode.
	 * Ditto for the scrollbar. */
	setUpdatesEnabled(false);
	viewport()->setUpdatesEnabled(false);

	pl.clear();

	// Convert ps answer in a list of tokenized lines
	KSGRD::SensorTokenizer procs(list, '\n');
	for (unsigned int i = 0; i < procs.numberOfTokens(); i++)
	{
		KSGRD::SensorPSLine* line = new KSGRD::SensorPSLine(procs[i]);
		if (line->numberOfTokens() != (uint) columns())
		{
#if 0
			// This is needed for debugging only.
			kdDebug() << list << endl;
			QString l;
			for (uint j = 0; j < line->numberOfTokens(); j++)
				l += (*line)[j] + "|";
			kdDebug() << "Incomplete ps line:" << l << endl;
#endif
			return (false);
		}
		else
			pl.append(line);
	}

	int currItemPos = itemRect(currentItem()).y();
	int vpos = verticalScrollBar()->value();
	int hpos = horizontalScrollBar()->value();

	updateMetaInfo();

	clear();

	if (treeViewEnabled)
		buildTree();
	else
		buildList();

	setCurrentItem(itemAt(QPoint(1, currItemPos)));
	verticalScrollBar()->setValue(vpos);
	horizontalScrollBar()->setValue(hpos);

	// Re-enable painting, and force an update.
	viewport()->setUpdatesEnabled(true);
	setUpdatesEnabled(true);

	triggerUpdate();

	return (true);
}

void
ProcessList::setTreeView(bool tv)
{
	if (treeViewEnabled = tv)
	{
		savedWidth[0] = columnWidth(0);
		openAll = true;
	}
	else
	{
		/* In tree view the first column is wider than in list view mode.
		 * So we shrink it to 1 pixel. The next update will resize it again
		 * appropriately. */
		setColumnWidth(0, savedWidth[0]);
	}
	/* In tree view mode borders are added to the icons. So we have to clear
	 * the cache when we change the tree view mode. */
	iconCache.clear();
}

bool
ProcessList::load(QDomElement& el)
{
	QDomNodeList dnList = el.elementsByTagName("column");
	for (uint i = 0; i < dnList.count(); ++i)
	{
		QDomElement lel = dnList.item(i).toElement();
		if (savedWidth.count() <= i)
			savedWidth.append(lel.attribute("savedWidth").toInt());
		else
			savedWidth[i] = lel.attribute("savedWidth").toInt();
		if (currentWidth.count() <= i)
			currentWidth.append(lel.attribute("currentWidth").toInt());
		else
			currentWidth[i] = lel.attribute("currentWidth").toInt();
		if (index.count() <= i)
			index.append(lel.attribute("index").toInt());
		else
			index[i] = lel.attribute("index").toInt();
	}

	setModified(false);

	return (true);
}

bool
ProcessList::save(QDomDocument& doc, QDomElement& display)
{
	for (int i = 0; i < columns(); ++i)
	{
		QDomElement col = doc.createElement("column");
		display.appendChild(col);
		col.setAttribute("currentWidth", columnWidth(i));
		col.setAttribute("savedWidth", savedWidth[i]);
		col.setAttribute("index", header()->mapToIndex(i));
	}

	setModified(false);

	return (true);
}

void
ProcessList::sortingChanged(int col)
{
	if (col == sortColumn)
		increasing = !increasing;
	else
	{
		sortColumn = col;
		increasing = true;
	}
	setSorting(sortColumn, increasing);
	setModified(true);
}

bool
ProcessList::matchesFilter(KSGRD::SensorPSLine* p) const
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
		KSGRD::SensorPSLine* p = pl.first();

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

	KSGRD::SensorPSLine* ps = pl.first();

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
ProcessList::extendTree(QPtrList<KSGRD::SensorPSLine>* pl, ProcessLVI* parent, int ppid)
{
	KSGRD::SensorPSLine* ps;

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

			if (ps->getPPid() != INIT_PID &&
				 closedSubTrees.findIndex(ps->getPPid()) != -1)
				setOpen(parent, false);
			else
				setOpen(parent, true);

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
ProcessList::addProcess(KSGRD::SensorPSLine* p, ProcessLVI* pli)
{
	QString name = p->getName();
	if (aliases[name])
		name = *aliases[name];

	/* Get icon from icon list that might be appropriate for a process
	 * with this name. */
	QPixmap pix;
	if (!iconCache[name])
	{
		pix = icons->loadIcon(name, KIcon::Small,
							  KIcon::SizeSmall, KIcon::DefaultState,
							  0L, true);
		if (pix.isNull() || !pix.mask())
			pix = icons->loadIcon("unknownapp", KIcon::User,
								  KIcon::SizeSmall);

		if (pix.width() != 16 || pix.height() != 16)
		{
			/* I guess this isn't needed too often. The KIconLoader should
			 * scale the pixmaps already appropriately. Since I got a bug
			 * report claiming that it doesn't work with GNOME apps I've
			 * added this safeguard. */
			QImage img;
			img = pix;
			img.smoothScale(16, 16);
			pix = img;
		}
		/* We copy the icon into a 24x16 pixmap to add a 4 pixel margin on
		 * the left and right side. In tree view mode we use the original
		 * icon. */
		QPixmap icon(24, 16, pix.depth());
		if (!treeViewEnabled)
		{
			icon.fill();
			bitBlt(&icon, 4, 0, &pix, 0, 0, pix.width(), pix.height());
			QBitmap mask(24, 16, true);
			bitBlt(&mask, 4, 0, pix.mask(), 0, 0, pix.width(), pix.height());
			icon.setMask(mask);
			pix = icon;
		}
		iconCache.insert(name, new QPixmap(pix));
	}
	else
		pix = *(iconCache[name]);

	// icon + process name
	pli->setPixmap(0, pix);
	pli->setText(0, p->getName());

	// insert remaining field into table
	for (unsigned int col = 1; col < p->numberOfTokens(); col++)
	{
		if (columnTypes[col] == "S" && columnDict[(*p)[col]])
			pli->setText(col, *columnDict[(*p)[col]]);
		else
			pli->setText(col, (*p)[col]);
	}
}

void
ProcessList::updateMetaInfo(void)
{
	selectedPIds.clear();
	closedSubTrees.clear();

	QListViewItemIterator it(this);

	// iterate through all items of the listview
	for ( ; it.current(); ++it )
	{
		if (it.current()->isSelected())
			selectedPIds.append(it.current()->text(1).toInt());
		if (treeViewEnabled && !it.current()->isOpen())
			closedSubTrees.append(it.current()->text(1).toInt());
	}

	/* In list view mode all list items are set to closed by QListView.
	 * If the tree view is now selected, all item will be closed. This is
	 * annoying. So we use the openAll flag to force all trees to open when
	 * the treeViewEnbled flag was set to true. */
	if (openAll)
	{
		if (treeViewEnabled)
			closedSubTrees.clear();
		openAll = false;
	}
}

void
ProcessList::removeColumns(void)
{
	for (int i = columns() - 1; i >= 0; --i)
		removeColumn(i);
	sortFunc.clear();
}

void
ProcessList::addColumn(const QString& label, const QString& type)
{
	uint col = sortFunc.count();
	QListView::addColumn(label);
	if (type == "s" || type == "S")
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
	else if (type == "f")
	{
		setColumnAlignment(col, AlignRight);
		sortFunc.append(floatKey);
	}
	else
	{
		kdDebug() << "Unknown type " << type << " of column " << label
				  << " in ProcessList!" << endl;
		return;
	}

	columnTypes.append(type);

	/* Just use some sensible default values as initial setting. */
	QFontMetrics fm = fontMetrics();
	setColumnWidth(col, fm.width(label) + 10);

	if (currentWidth.count() - 1 == col)
	{
		/* Table has been loaded from file. We can restore the settings
		 * when the last column has been added. */
		for (uint i = 0; i < col; ++i)
		{
			/* In case the language has been changed the column width
			 * might need to be increased. */
			if (currentWidth[i] == 0)
			{
				if (fm.width(header()->label(i)) + 10 > savedWidth[i])
					savedWidth[i] = fm.width(header()->label(i)) + 10;
				setColumnWidth(i, 0);
			}
			else
			{
				if (fm.width(header()->label(i)) + 10 > currentWidth[i])
					setColumnWidth(i, fm.width(header()->label(i)) + 10);
				else
					setColumnWidth(i, currentWidth[i]);
			}
			setColumnWidthMode(i, currentWidth[i] == 0 ?
							   QListView::Manual : QListView::Maximum);
			header()->moveSection(i, index[i]);
		}
		setSorting(sortColumn, increasing);
	}
}

void
ProcessList::handleRMBPressed(QListViewItem* lvi, const QPoint& p, int col)
{
	if (!lvi)
		return;

	/* Qt (un)selectes LVIs on RMB clicks. Since we don't want this, we have
	 * to invert it again. */
	lvi->setSelected(!lvi->isSelected());

	/* lvi is only valid until the next time we hit the main event
	 * loop. So we need to save the information we need after calling
	 * processPM->exec(). */
	int currentPId = lvi->text(1).toInt();

	int currentNiceValue = 0;
	for (int i = 0; i < columns(); ++i)
		if (QString::compare(header()->label(i), i18n("Nice")) == 0)
			currentNiceValue = lvi->text(i).toInt();

	processPM = new QPopupMenu();
	processPM->insertItem(i18n("Hide column"), 5);
	QPopupMenu* hiddenPM = new QPopupMenu(processPM);
	for (int i = 0; i < columns(); ++i)
		if (columnWidth(i) == 0)
			hiddenPM->insertItem(header()->label(i), i + 100);
	processPM->insertItem(i18n("Show column"), hiddenPM);

	processPM->insertSeparator();

	processPM->insertItem(i18n("Select all processes"), 1);
	processPM->insertItem(i18n("Unselect all processes"), 2);

	QPopupMenu* signalPM = new QPopupMenu(processPM);
	if (killSupported && lvi->isSelected())
	{
		processPM->insertSeparator();
		processPM->insertItem(i18n("Select all child processes"), 3);
		processPM->insertItem(i18n("Unselect all child processes"), 4);

		signalPM->insertItem(i18n("SIGABRT"), MENU_ID_SIGABRT);
		signalPM->insertItem(i18n("SIGALRM"), MENU_ID_SIGALRM);
		signalPM->insertItem(i18n("SIGCHLD"), MENU_ID_SIGCHLD);
		signalPM->insertItem(i18n("SIGCONT"), MENU_ID_SIGCONT);
		signalPM->insertItem(i18n("SIGFPE"), MENU_ID_SIGFPE);
		signalPM->insertItem(i18n("SIGHUP"), MENU_ID_SIGHUP);
		signalPM->insertItem(i18n("SIGILL"), MENU_ID_SIGILL);
		signalPM->insertItem(i18n("SIGINT"), MENU_ID_SIGINT);
		signalPM->insertItem(i18n("SIGKILL"), MENU_ID_SIGKILL);
		signalPM->insertItem(i18n("SIGPIPE"), MENU_ID_SIGPIPE);
		signalPM->insertItem(i18n("SIGQUIT"), MENU_ID_SIGQUIT);
		signalPM->insertItem(i18n("SIGSEGV"), MENU_ID_SIGSEGV);
		signalPM->insertItem(i18n("SIGSTOP"), MENU_ID_SIGSTOP);
		signalPM->insertItem(i18n("SIGTERM"), MENU_ID_SIGTERM);
		signalPM->insertItem(i18n("SIGTSTP"), MENU_ID_SIGTSTP);
		signalPM->insertItem(i18n("SIGTTIN"), MENU_ID_SIGTTIN);
		signalPM->insertItem(i18n("SIGTTOU"), MENU_ID_SIGTTOU);
		signalPM->insertItem(i18n("SIGUSR1"), MENU_ID_SIGUSR1);
		signalPM->insertItem(i18n("SIGUSR2"), MENU_ID_SIGUSR2);

		processPM->insertSeparator();
		processPM->insertItem(i18n("Send Signal"), signalPM);
	}

	/* differ between killSupported and reniceSupported in a future
	 * version. */
	if (killSupported && lvi->isSelected())
	{
		processPM->insertSeparator();
		processPM->insertItem(i18n("Renice process"), 300);
	}

	int id;
	switch (id = processPM->exec(p))
	{
	case -1:
		break;
	case 1:
	case 2:
		selectAll(id & 1);
		break;
	case 3:
	case 4:
		selectAllChilds(currentPId, id & 1);
		break;
	case 5:
		setColumnWidthMode(col, QListView::Manual);
		savedWidth[col] = columnWidth(col);
		setColumnWidth(col, 0);
		setModified(true);
		break;
	case 300:
		{
		ReniceDlg *reniceDlg = new ReniceDlg(this, "reniceDlg", currentNiceValue, currentPId);
		Q_CHECK_PTR(reniceDlg);

		int reniceVal;
		if ((reniceVal = reniceDlg->exec()) != 40)
			emit reniceProcess(currentPId, reniceVal);

		delete reniceDlg;
		}
		break;
	default:
		/* IDs < 100 are used for signals. */
		if (id < 100)
		{
			/* we go through list to get all task also
			   when update interval is paused */
			selectedPIds.clear();
			QListViewItemIterator it(this);

			// iterate through all items of the listview
			for ( ; it.current(); ++it )
			{
				if (it.current()->isSelected()) {
					selectedPIds.append(it.current()->text(1).toInt());
				}
			}

			QString msg = i18n("Do you really want to send signal %1 to the selected process?", 
					"Do you really want to send signal %1 to the %n selected processes?",
					selectedPIds.count())
				.arg(signalPM->text(id));
			int answ;
			switch(answ = KMessageBox::questionYesNo(this, msg))
			{
			case KMessageBox::Yes:
			{
				QValueList<int>::Iterator it;
				for (it = selectedPIds.begin(); it != selectedPIds.end(); ++it)
					emit (killProcess(*it, id));
				break;
			}
			default:
				break;
			}
		}
		else
		{
			/* IDs >= 100 are used for hidden columns. */
			int col = id - 100;
			setColumnWidthMode(col, QListView::Maximum);
			setColumnWidth(col, savedWidth[col]);
			setModified(true);
		}
	}
	delete processPM;
}

void
ProcessList::selectAll(bool select)
{
	selectedPIds.clear();

	QListViewItemIterator it(this);

	// iterate through all items of the listview
	for ( ; it.current(); ++it )
	{
		it.current()->setSelected(select);
		repaintItem(it.current());
		if (select)
			selectedPIds.append(it.current()->text(1).toInt());
	}
}

void
ProcessList::selectAllChilds(int pid, bool select)
{
	QListViewItemIterator it(this);

	// iterate through all items of the listview
	for ( ; it.current(); ++it )
	{
		// Check if PPID matches the pid (current is a child of pid)
		if (it.current()->text(2).toInt() == pid)
		{
			int currPId = it.current()->text(1).toInt();
			it.current()->setSelected(select);
			repaintItem(it.current());
			if (select)
				selectedPIds.append(currPId);
			else
				selectedPIds.remove(currPId);
			selectAllChilds(currPId, select);
		}
	}
}
