/*
    KTop, a taskmanager and cpu load monitor
   
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
#include <stdio.h>
#include <signal.h>

#include <kapp.h>

#include "OSProcessList.h"
#include "ProcessTree.moc"

#define INIT_PID 1
#define NONE -1

KtopProcTree::KtopProcTree(QWidget *parent, const char *name, WFlags f)
	: KTreeList(parent, name, f)
{
	setShowItemText(true);
	setTreeDrawing(true);
	setBottomScrollBar(true);
	setAutoUpdate(true);

	pids.setAutoDelete(true);

	icons = new KtopIconList;
	CHECK_PTR(icons);

	rootProcess = INIT_PID;
}

int
KtopProcTree::selectedProcess(void)
{
	if (currentItem() < 0)
		return (-1);

	// return the process ID of the selected process
	return (*(pids.at((unsigned int) currentItem())));
}

void
KtopProcTree::setRootProcess(void)
{
	int pid = selectedProcess();

	rootProcess = pid > 0 ? pid : INIT_PID;
	update();
}

void 
KtopProcTree::update(void)
{
	setAutoUpdate(false);
	loadProcesses();
	setAutoUpdate(true);

	if (isVisible())
	{
		repaint(TRUE);
	}
}

void 
KtopProcTree::loadProcesses()
{
	OSProcessList pl;

	pl.setSortCriteria(sortKey);

	// request current list of processes
	pl.update();

	// remove all items from the tree widget
	clear();

	// clear PID list
	pids.clear();

	OSProcess* ps = pl.first();

	int cntr = 0;
	// find the process with the PID 'rootProcess'
	while (ps)
	{
		if (ps->getPid() == rootProcess)
		{
			// insert root item into the tree widget
			insertItem(ps->getName(), icons->procIcon(ps->getName()));

			// insert PID into PID list at position cntr
			pids.append(new int(ps->getPid()));

			cntr++;
			// insert child processes for this process
			buildTree(cntr - 1, ps->getPid(), &pl, cntr);
			break;
		}
		else
			ps = pl.next();
	}
}

void
KtopProcTree::buildTree(int parentIdx, int ppid, OSProcessList* pl, int& cntr)
{
	OSProcess* ps;

	// start at top list
	ps = pl->first();

	while (ps)
	{
		// look for a child process of the current parent
		if (ps->getPpid() == ppid)
		{
			QString text;
			text.sprintf("%s  ( %d / %s )", ps->getName(),
						 ps->getPid(), ps->getUserName().data());

			// add child process to the widget
			addChildItem(text, icons->procIcon(ps->getName()),
						 parentIdx);

			// remove the process from the process list
			pl->remove();

			// insert PID into PID list at position cntr
			pids.append(new int(ps->getPid()));

			// increase the item counter
			cntr++;

			// now look for the childs of the inserted process
			buildTree(cntr - 1, ps->getPid(), pl, cntr);

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
