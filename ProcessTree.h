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

#include "ptree.h"
#include "IconList.h"

#ifndef _ProcessTree_h_
#define _ProcessTree_h_

class KtopProcTree : public ProcTree
{
    Q_OBJECT;

public  :
     enum sortID { SORTBY_PID=0, 
                   SORTBY_NAME , 
                   SORTBY_UID     
          	 };

     KtopProcTree(QWidget *parent=0, const char *name=0, WFlags f= 0);
    ~KtopProcTree();

     void update             ( );
     int  sortMethod         ( );
     void setSortMethod      ( int );
     int  selectionPid       ( );
     void changeRoot         ( );

signals:
    void popupMenu(QPoint);

private :
	void           sort                ( );
	void           readProcDir         ( );
	void           reorder             ( ProcTree* );
    static  ProcTreeItem*  parentItem          ( ProcTreeItem* , int ); 
    static  ProcTreeItem*  sortByName          ( ProcTreeItem* );
    static  ProcTreeItem*  sortByPid           ( ProcTreeItem* , bool reverse=FALSE );
    static  ProcTreeItem*  sortByUid           ( ProcTreeItem* );
            void           try2restoreSelection( ); 
            ProcTreeItem*  restoreSelection    ( ProcTreeItem* item ); 

    KtopIconList *icons;
    bool          updating,
                  mouseRightButton;
    int           lastSelectionPid,
                  sort_method;

private slots:
    void procHighlighted(int);
    void mouseEvent(QMouseEvent*);
};

#endif
