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

#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <signal.h>

#include <kapp.h>
#include <klocale.h>

#include "OSProcessList.h"
#include "ProcessList.moc"

// #define DEBUG_MODE

#define PROC_BASE     "/proc"
#define INIT_PID      1
#define NONE -1

#define NUM_COL 10

inline int max(int a, int b)
{
	return ((a) < (b) ? (b) : (a));
}

static const char *col_headers[] = 
{
	"  ",
	"procID",
	"Name",
	"userID",
	"CPU",
	"Time",
	"Status",
	"VmSize",
	"VmRss",
	"VmLib",
	0
};

static const char *dummies[] = 
{
     "++++"              ,
     "procID++"          ,
     "kfontmanager++"    ,
     "rootuseroot"       ,
     "100.00%+"          ,
     "100:00++"          ,
     "Status+++"         ,
     "VmSize++"          ,
     "VmSize++"          ,
     "VmSize++"          ,
     0
};

static KTabListBox::ColumnType col_types[] = 
{
     KTabListBox::MixedColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn,
     KTabListBox::TextColumn
};

KtopProcList::KtopProcList(QWidget *parent = 0, const char* name = 0)
	: KTabListBox(parent, name, NUM_COL)
{
	setSeparator(';');

	// no process list generated yet
	ps_list  = NULL;
	// no timer started yet
	timer_id = NONE;

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

	/*
	 * Set the column witdh for all columns in the process list table.
	 * A dummy string that is somewhat longer than the real text is used
	 * to determine the width with the current font metrics.
	 */
	QFontMetrics fm = fontMetrics();
	for (int cnt = 0; col_headers[cnt]; cnt++)
		setColumn(cnt, i18n(col_headers[cnt]),
				  max(fm.width(dummies[cnt]),
					  fm.width(i18n(col_headers[cnt]))),
				  col_types[cnt]);

	// Clicking on the header changes the sort order.
	connect(this, SIGNAL(headerClicked(int)),
			SLOT(userClickOnHeader(int)));

	// Clicking in the table can change the process selection.
	connect(this, SIGNAL(highlighted(int,int)),
			SLOT(procHighlighted(int, int)));
}

KtopProcList::~KtopProcList()
{
	// remove icon list from memory
	delete icons;

	// switch off timer
	if (timer_id != NONE)
		killTimer(timer_id);

	// delete process list
	while (ps_list)
	{
		psPtr tmp = ps_list;
		ps_list = ps_list->next;
		delete tmp;
	}
}

void 
KtopProcList::setUpdateRate(int r)
{
	update_rate = r;
	switch (update_rate)
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

	if (timer_id != NONE)
	{
		timerOff();
		timerOn();
	}
}

int 
KtopProcList::setAutoUpdateMode(bool mode)
{
	int oldmode = (timer_id != NONE) ? TRUE : FALSE; 

	if (mode)
		timerOn();
	else
		timerOff();

	return (oldmode);
}

void 
KtopProcList::update(void)
{
	int top_Item = topItem();
	int lastmode = setAutoUpdateMode(FALSE);
	setAutoUpdate(FALSE);
	load();
	try2restoreSelection();
	setTopItem(top_Item);
	setAutoUpdate(TRUE);
	setAutoUpdateMode(lastmode);

    if(isVisible())
		repaint();
}

void 
KtopProcList::load()
{
	OSProcessList pl;

	pl.setSortCriteria(sort_method);
	pl.update();

	clear();
	dict().clear();  

	const QPixmap *pix;
	char  usrName[32];
	struct passwd *pwent;  
	char line[256];
	while (!pl.isEmpty())
	{
		OSProcess* p = pl.first();

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
			pwent = getpwuid(p->getUid());
			if (pwent) 
				strncpy(usrName, pwent->pw_name, 31);
			else 
				strcpy(usrName, "????");
			pix = icons->procIcon((const char*)p->getName());
			dict().insert((const char*)p->getName(), pix);
			sprintf(line, "{%s};%d;%s;%s;%5.2f%%;%d:%02d;%s;%d;%d;%d", 
					p->getName(),
					p->getPid(),
					p->getName(),
					usrName,
					1.0 * (p->getTime() - p->getOtime()) /
					(p->getAbstime() - p->getOabstime()) * 100, 
					(p->getTime() / 100) / 60,
					(p->getTime() / 100) % 60, 
					p->getStatusTxt(),
					p->getVm_size(), 
					p->getVm_rss(), 
					p->getVm_lib());         
			appendItem(line);
		}
		pl.removeFirst();
    }
}

void 
KtopProcList::userClickOnHeader(int indxCol)
{
	if (indxCol)
	{
		switch (indxCol - 1)
		{
		case OSProcessList::SORTBY_PID: 
		case OSProcessList::SORTBY_NAME: 
		case OSProcessList::SORTBY_UID: 
		case OSProcessList::SORTBY_CPU: 
		case OSProcessList::SORTBY_TIME:
		case OSProcessList::SORTBY_STATUS:
		case OSProcessList::SORTBY_VMSIZE:
		case OSProcessList::SORTBY_VMRSS:
		case OSProcessList::SORTBY_VMLIB:
			setSortMethod((OSProcessList::SORTKEY) (indxCol - 1));
			break;
		default: 
			return;
			break;
		}
		update();
	}
}

void 
KtopProcList::try2restoreSelection()
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
KtopProcList::restoreSelection()
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
KtopProcList::procHighlighted(int indx, int)
{ 
	lastSelectionPid = NONE;
	sscanf(text(indx, 1), "%d", &lastSelectionPid);
} 

int 
KtopProcList::cellHeight(int row)
{
	const QPixmap *pix = icons->procIcon(text(row, 2));

	if (pix)
		return (pix->height());

	return (18);
}
