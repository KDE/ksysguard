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

#ifndef _ProcessTree_h_
#define _ProcessTree_h_

#include <qlist.h>

#include <ktreelist.h>

#include "IconList.h"
#include "OSProcessList.h"

class KtopProcTree : public KTreeList
{
	Q_OBJECT

public:
	KtopProcTree(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~KtopProcTree()
	{
		delete icons;
	}

	void update();

	void setSortMethod(OSProcessList::SORTKEY m)
	{
		sortKey = m;
	}

	int selectedProcess(void);

	void setRootProcess(void);

signals:
	void popupMenu(QPoint);

protected:
	virtual void mouseReleaseEvent (QMouseEvent* e)
	{
		if ((currentItem() >= 0) && (e->button() == RightButton))
			emit popupMenu(e->pos());
	}

private:
	void loadProcesses(void);
	void buildTree(int parentIdx, int ppid, OSProcessList* pl, int& cntr);

	KtopIconList* icons;
	OSProcessList::SORTKEY sortKey;

	/**
	 * This list stores the PIDs of the items in the tree widget. The index
	 * in the KTreeList widget and the position in the list correspond. I
	 * think this double book-keeping is cleaner than reverse-engineering the
	 * pid from the item text.
	 */
	QList<int> pids;

	int rootProcess;
};

#endif
