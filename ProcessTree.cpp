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

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>

#include "ProcessTree.moc"

#define ktr           klocale->translate
#define PROC_BASE     "/proc"
#define KDE_ICN_DIR   "/share/icons/mini"
#define KTOP_ICN_DIR  "/share/apps/ktop/pics"
#define INIT_PID      1
#define NONE         -1

KtopProcTree::KtopProcTree(QWidget *parent=0, const char *name=0, WFlags f= 0)
             :ProcTree(parent,name,f)
{ 
  initMetaObject();

  lastSelectionPid = getpid();
  mouseRightButton = FALSE;
  updating         = TRUE;

  icons = new KtopIconList;
  CHECK_PTR(icons);

  connect(this,SIGNAL(highlighted(int)),SLOT(procHighlighted(int)));
  connect(this,SIGNAL(clicked(QMouseEvent*)),SLOT(mouseEvent(QMouseEvent*)));

  installEventFilter(this);
}

KtopProcTree::~KtopProcTree()
{
  delete icons;

  #ifdef DEBUG_MODE
     printf("KTop debug : KtopProcTree::~KtopProcTree : delete icons done !\n");
  #endif
}

int 
KtopProcTree::selectionPid()
{
  return lastSelectionPid;
}

int 
KtopProcTree::sortMethod()
{
  return sort_method;
}

void 
KtopProcTree::setSortMethod(int m)
{
  sort_method = m;
}

void 
KtopProcTree::mouseEvent(QMouseEvent* e)
{
#ifdef DEBUG_MODE
	printf("KTop debug : KtopProcTree::mouseEvent : button=%d.\n",e->button());
#endif

	mouseRightButton = ( e->button() == RightButton ) ? TRUE : FALSE;
}

void 
KtopProcTree::procHighlighted(int indx)
{ 
  if ( updating ) return;

  #ifdef DEBUG_MODE
    printf("KTop debug : KtopProcTree::procHighlighted : item=%d.\n",indx);
  #endif
  
  if ( indx == NONE ) {
       lastSelectionPid = NONE;
       return;
  }

  ProcTreeItem *item = itemAt(indx);
  if ( !item ) return;
  lastSelectionPid = item->getProcId();

  #ifdef DEBUG_MODE
    printf("KTop debug : KtopProcTree::procHighlighted : lastSelectionPid=%d.\n"
           ,lastSelectionPid);
  #endif  

  if ( mouseRightButton ) 
       emit popupMenu(QCursor::pos());
  mouseRightButton = FALSE;
} 

void 
KtopProcTree::try2restoreSelection()
{
 int err = 0;
 
 if ( lastSelectionPid != NONE ) {
      err = kill(lastSelectionPid,0);
      #ifdef DEBUG_MODE
             printf("KTop debug : KtopProcTree::try2restoreSelection : kill:%s\n"
                    ,strerror(errno));    
      #endif
 }
 if ( err || (lastSelectionPid == NONE) )
     lastSelectionPid = getpid();

 restoreSelection(itemAt(0));

}

ProcTreeItem* 
KtopProcTree::restoreSelection(ProcTreeItem* item)
{
 ProcTreeItem* anItem;

 if ( !item || (lastSelectionPid==NONE) ) {
      return NULL;
 }
 if ( item->getProcId() == lastSelectionPid ) {
      int indx  = itemVisibleIndex(item);
      setCurrentItem(indx);
      #ifdef DEBUG_MODE
             printf("KTop debug : KtopProcTree::restoreSelection : cur pid : %d\n"
                    ,lastSelectionPid);    
      #endif
      return item;
 }
 if ( (anItem = restoreSelection(item->getSibling())) )
    return anItem;
 return restoreSelection(item->getChild());
}


void 
KtopProcTree::update(void)
{ 

  #ifdef DEBUG_MODE
         printf("KTop debug : KtopProcTree::update\n");    
  #endif
  int top_Item = topCell();
  updating = TRUE;
  	setUpdatesEnabled(FALSE);  
    	  setExpandLevel(0); 
    	  clear();
    	  readProcDir();
    	  sort();
    	  setExpandLevel(50);
          setTopCell(top_Item);
          try2restoreSelection();
  	setUpdatesEnabled(TRUE);
  	if ( isVisible() ) repaint(TRUE);
  updating = FALSE;
}

void 
KtopProcTree::sort( void )
{
  ProcTreeItem *mainItem = itemAt(0);
  if ( ! mainItem ) return;

  switch ( sort_method ) {
   case SORTBY_PID:
	mainItem->setSortPidText(); 
        mainItem->setChild(sortByPid(mainItem->getChild(),false));
	break; 
   case SORTBY_NAME:
        mainItem->setSortNameText(); 
        mainItem->setChild(sortByName(mainItem->getChild()));
        break; 
   case SORTBY_UID:
        mainItem->setSortUidText(); 
        mainItem->setChild(sortByUid(mainItem->getChild()));
        break;
  }
}

ProcTreeItem* 
KtopProcTree::sortByName( ProcTreeItem* ref ) 
{
 ProcTreeItem *newTop,*cur,*aTempItem,*prev=NULL;
 if ( ! ref ) return ref;

 ref->setSortNameText();
 if ( ref->hasChild() )
   ref->setChild(sortByName(ref->getChild()));
 if ( ! ref->hasSibling() ) 
   return ref;
 newTop = sortByName(ref->getSibling());
 ref->setSibling(newTop);  
 if ( ! newTop ) return newTop;  
 cur = newTop;
 int counter = 0; 
 while ( cur )   
   {     
     aTempItem = cur->getSibling();
     if ( strcmp(ref->getProcName(),cur->getProcName()) > 0 ) {
	 cur->setSibling(ref); 
	 ref->setSibling(aTempItem);
	 if ( prev ) prev->setSibling(cur);
	 prev = cur;
	 counter++;   
     } 
     cur = aTempItem;
   }
 if ( counter ) return newTop; else return ref; 
}

ProcTreeItem* 
KtopProcTree::sortByPid(ProcTreeItem* ref, bool reverse) 
{
 ProcTreeItem *newTop,*cur,*aTempItem,*prev=NULL;
 if ( ! ref ) return ref;
 
 ref->setSortPidText();

 if ( ref->hasChild() )
   ref->setChild(sortByPid(ref->getChild(),reverse) );
 if ( ! ref->hasSibling() ) 
   return ref;
 newTop = sortByPid(ref->getSibling(),reverse);
 ref->setSibling(newTop);  
 if ( ! newTop ) return newTop;  
 cur = newTop;
 int   counter = 0; 
 while ( cur )   
   { 
     aTempItem = cur->getSibling();
     
     if ( ! reverse ) {
     	if ( ref->getProcId() > cur->getProcId() ) {
	 	cur->setSibling(ref); 
	 	ref->setSibling(aTempItem);
	 	if ( prev ) prev->setSibling(cur);
	 	prev = cur;
	 	counter++;   
     	}
      }
      else {
     	if ( ref->getProcId() < cur->getProcId() ) {
	 	cur->setSibling(ref); 
	 	ref->setSibling(aTempItem);
	 	if ( prev ) prev->setSibling(cur);
	 	prev = cur;
	 	counter++;   
     	}
      }
 
     cur = aTempItem;
   }
 if ( counter ) return newTop; else return ref; 
}

ProcTreeItem* 
KtopProcTree::sortByUid(ProcTreeItem* ref ) 
{
 ProcTreeItem *newTop,*cur,*aTempItem,*prev=NULL;
 if ( ! ref ) return ref;
 
 ref->setSortUidText();
 if ( ref->hasChild() )
   ref->setChild(sortByUid(ref->getChild()) );
 if ( ! ref->hasSibling() ) 
   return ref;
 newTop = sortByUid(ref->getSibling());
 ref->setSibling(newTop);  
 if ( ! newTop ) return newTop;  
 cur = newTop;
 int   counter = 0; 
 while ( cur )   
   { 
     aTempItem = cur->getSibling();
     
     if ( ref->getProcUid() > cur->getProcUid() ) {
	 	cur->setSibling(ref); 
	 	ref->setSibling(aTempItem);
	 	if ( prev ) prev->setSibling(cur);
	 	prev = cur;
	 	counter++;   
     }
     cur = aTempItem;
   }
 if ( counter ) return newTop; else return ref; 
}

ProcTreeItem* 
KtopProcTree::parentItem(ProcTreeItem* item, int ppid )
{
 ProcTreeItem* anItem;

 if ( ! item ) return NULL ; 
 if ( item->getProcId() == ppid ) 
      return item;
 if ( (anItem = parentItem(item->getSibling(),ppid)) )
    return anItem;
 return parentItem(item->getChild(),ppid);
 
}

void 
KtopProcTree::reorder( ProcTree* alist )
{
  ProcTreeItem   *cur = alist->itemAt(0),*token,*next;
  if ( !cur ) return;

  while ( cur ) 
    { 
      ProcTreeItem *parent = parentItem(itemAt(0),cur->getParentId()); 
          next  = cur->getSibling();
      int indx  = alist->itemVisibleIndex(cur);
          token = alist->takeItem(indx);
      if ( parent ) 
	{ 
          ProcTreeItem *child = new ProcTreeItem(cur->getProcInfo()
                                         ,icons->procIcon(cur->getProcName()));
          CHECK_PTR(child);
	  parent->appendChild(child);
	}
        else { // parent not already in tree => move item to bottom
           alist->insertItem(token,alist->count()-1,false);
        }
        cur = next;
    }
}

void 
KtopProcTree::changeRoot()
{  
  int newRootIndx = currentItem();
  if ( newRootIndx == -1 ) return;
  
  ProcTreeItem *newRoot = takeItem(newRootIndx);
  if ( !newRoot ) return;

  setUpdatesEnabled(FALSE);  
    clear();
    insertItem(newRoot);
    setCurrentItem(0);
  setUpdatesEnabled(TRUE);
  repaint(TRUE); 
}

/*
 * Most of this code (this routine) is :
 * Copyright 1993-1998 Werner Almesberger (pstree author). All rights reserved.
 */
void 
KtopProcTree::readProcDir(  )
{
    DIR           *dir;
    struct dirent *de;
    struct passwd *pwent;
    FILE          *file;
    struct stat    st;
    char           path[PATH_MAX+1];
    int            dummy;
    ProcInfo       pi;

    ProcTree      *alist = new ProcTree();  
    CHECK_PTR(alist);

    if (!(dir = opendir(PROC_BASE))) 
      {
        delete alist;
	perror(PROC_BASE);
	exit(1);
      }

    while ((de = readdir(dir)))
         
         if(isdigit(de->d_name[0])) { 
      
            sscanf(de->d_name,"%d",&(pi.pid));

	    sprintf(path,"%s/%d/stat",PROC_BASE,pi.pid);

	    if ( ( file = fopen(path,"r") ) ) {

		if (fstat(fileno(file),&st) < 0) {
                    delete alist; 
		    perror(path);
		    exit(1);    // BETTER ERROR HANDLING NEEDED !!!!
		}

                pi.uid = st.st_uid;
		if (fscanf(file,"%d (%[^)]) %c %d"
			       ,&dummy
			       ,pi.name
			       ,(char*)&dummy
			       ,&(pi.ppid)) == 4) 
		  { 
                    
                    // get user name
                    pwent = getpwuid(pi.uid);
                    if( pwent )
			strcpy(pi.uname,pwent->pw_name);
                    else 
                        strcpy(pi.uname,"????");

		    strcpy(pi.arg,"Not implemented");
                    
                    // create new item
                    ProcTreeItem *child = new ProcTreeItem(pi,icons->procIcon(pi.name));
                    CHECK_PTR(child);
                     
                    //insert new item in tree
		    if ( pi.pid == INIT_PID )
		         insertItem(child);
		    else if ( pi.ppid == INIT_PID ) 
			 addChildItem(child,0); 
		    else { 
		      ProcTreeItem *item=parentItem(itemAt(0),pi.ppid);   
		      if ( item )  
			  item->appendChild(child); 
		      else  
			  alist->insertItem(child); 
		    }	
	          }
		(void)fclose(file);
	    }
	}
    (void)closedir(dir);

    reorder(alist);
    delete alist;
}
