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

#ifndef _ProcListPage_h_
#define _ProcListPage_h_

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include "ProcessList.h"

class ProcListPage : public QWidget
{
	Q_OBJECT;

public:
	ProcListPage(QWidget *parent = 0, const char *name = 0);
	~ProcListPage()
	{
		delete pList_bKill;
		delete pList_bRefresh;
		delete pList_box;
		delete pList_cbRefresh;
		delete pList_cbFilter;
		delete pList;
	}

	void resizeEvent(QResizeEvent*);
	int selectionPid(void)
	{
		return (pList->selectionPid());
	}

	int setAutoUpdateMode(bool mode)
	{
		return (pList->setAutoUpdateMode(mode));
	}
	
	void saveSettings(void);

public slots:
	void pList_update()
	{
		pList->update();
	}
	void pList_cbRefreshActivated (int);
	void pList_cbProcessFilter(int);
	void pList_popupMenu(int, int);
	void pList_killTask();

private:
	QPushButton* pList_bKill;
	QPushButton* pList_bRefresh;

    QGroupBox* pList_box;
	QComboBox* pList_cbRefresh;
	QComboBox* pList_cbFilter;
    KtopProcList* pList;

	int pList_sortby;
	int pList_refreshRate;
	char cfgkey_pListUpdate[12];
	char cfgkey_pListFilter[12];
	char cfgkey_pListSort[12];
} ;

#endif
