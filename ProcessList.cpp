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
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <signal.h>

#include <kapp.h>
#include <klocale.h>

#include "ProcessList.moc"

#define DEBUG_MODE

#define ktr           klocale->translate
#define PROC_BASE     "/proc"
#define KDE_ICN_DIR   "/share/icons/mini"
#define KTOP_ICN_DIR  "/share/apps/ktop/pics"
#define INIT_PID      1
#define NONE         -1

#define NUM_COL 10

// I have to find a better method...
static const char *col_headers[] = 
{
     " "      ,
     "procID" ,
     "Name"   ,
     "userID" ,
     "CPU"    ,
     "Time"   ,
     "Status" ,
     "VmSize" ,
     "VmRss"  ,
     "VmLib"  ,
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
#ifdef DEBUG_MODE
	printf("KTop debug: KtopProcList()\n");
#endif

  initMetaObject();
  setSeparator(';');

  ps_list  = NULL;
  timer_id = NONE;
  lastSelectionPid = getpid();
  filtermode = FILTER_OWN;
  update_rate = UPDATE_FAST;

  icons = new KtopIconList;
  CHECK_PTR(icons);

  QFontMetrics fm = fontMetrics();
  for ( int cnt=0 ; col_headers[cnt] ; cnt++ ) {
        setColumn(cnt,col_headers[cnt]
                     ,fm.width(dummies[cnt])
                     ,col_types[cnt]);
  }

  connect(this,SIGNAL(headerClicked(int)),SLOT(userClickOnHeader(int)));
  connect(this,SIGNAL(highlighted(int,int)),SLOT(procHighlighted(int,int)));

  installEventFilter(this);

}

KtopProcList::~KtopProcList()
{
  delete icons;

  #ifdef DEBUG_MODE
     printf("KTop debug : KtopProcList::~KtopProcList : delete icons done !\n");
  #endif

  if ( timer_id != NONE ) killTimer(timer_id);

  while( ps_list ) {
    psPtr tmp = ps_list;
    ps_list = ps_list->next;
    delete tmp;
  }

  #ifdef DEBUG_MODE
     printf("KTop debug : KtopProcList::~KtopProcList : delete ps_list done !\n");
  #endif

}

void 
KtopProcList::setUpdateRate(int r)
{
#ifdef DEBUG_MODE
    printf("KTop debug : KtopProcList::setUpdateRate\n");
#endif

	update_rate = r;
	switch (update_rate)
	{
	case UPDATE_SLOW:
		timer_interval = UPDATE_SLOW_VALUE*1000;
		break;
	case UPDATE_MEDIUM:
		timer_interval = UPDATE_MEDIUM_VALUE*1000;
		break;
	case UPDATE_FAST:
		timer_interval = UPDATE_FAST_VALUE*1000;
		break;
	default:
		timer_interval = UPDATE_FAST_VALUE*1000;
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
  int oldmode = ( timer_id != NONE ) ? TRUE : FALSE; 
  if ( mode ) timerOn(); else timerOff();
  return oldmode;
}

void 
KtopProcList::timerOff(void)
{
    if ( timer_id != NONE ) {
         killTimer(timer_id);
         timer_id = NONE;
    } 
}

void 
KtopProcList::timerOn()
{
    timer_id = startTimer(timer_interval);
}

void 
KtopProcList::update(void)
{
#ifdef DEBUG_MODE
	printf("Ktop debug: KtopProcList::update()\n");
#endif

    int top_Item = topItem();
    int lastmode = setAutoUpdateMode(FALSE);
    setAutoUpdate(FALSE);
      load();
      try2restoreSelection();
      setTopItem(top_Item);
    setAutoUpdate(TRUE);
    setAutoUpdateMode(lastmode);
    if( isVisible() ) repaint();
#ifdef DEBUG_MODE
	else
		printf("Ktop debug: KtopProcList::update(): page invisible\n");
#endif
}

void 
KtopProcList::load()
{
	DIR *dir;
	char line[256];
	struct dirent *entry;
	struct passwd *pwent;  

	psList_clearProcVisit();
	dir = opendir(PROC_BASE);
	while((entry = readdir(dir))) 
	{
		if( isdigit(entry->d_name[0]) ) 
			psList_getProcStatus(entry->d_name);
	}
	closedir(dir);
	psList_removeProcUnvisited();

	psList_sort();

	clear();
	dict().clear();  

	psPtr tmp;
	const QPixmap *pix;
	char  usrName[32];
    int   i;
	for(tmp=ps_list, i=1; tmp; tmp=tmp->next, i++)
	{
		switch (filtermode)
		{
		case FILTER_ALL:
			break;
		case FILTER_SYSTEM:
			if (tmp->uid >= 100)
				continue;
			break;
		case FILTER_USER:
			if (tmp->uid < 100)
				continue;
			break;
		case FILTER_OWN:
		default:
			if (tmp->uid != getuid())
				continue;
			break;
		}
        pwent = getpwuid(tmp->uid);
        if ( pwent ) 
			strncpy(usrName,pwent->pw_name,31);
        else 
			strcpy(usrName,"????");
        pix = icons->procIcon((const char*)tmp->name);
        dict().insert((const char*)tmp->name,pix);
        sprintf(line, "{%s};%d;%s;%s;%5.2f%%;%d:%02d;%s;%d;%d;%d", 
				tmp->name,
	            tmp->pid, 
				tmp->name, 
				usrName,
				1.0*(tmp->time-tmp->otime)/(tmp->abstime-tmp->oabstime)*100, 
				(tmp->time/100)/60,(tmp->time/100)%60, 
				tmp->statusTxt,
				tmp->vm_size, 
				tmp->vm_rss, 
				tmp->vm_lib
		    );         
        appendItem(line);
    }
}

void 
KtopProcList::psList_sort() 
{

    int   swap;
    psPtr start, 
          item, 
          tmp;

    for ( start=ps_list ; start ; start=start->next ) { 

        for ( item=ps_list ; item && item->next ; item=item->next ) {

	    switch ( sort_method ) {
	        case SORTBY_PID:
		    swap = item->pid > item->next->pid;
		    break;
	        case SORTBY_UID:
		    swap = item->uid > item->next->uid;
		    break;
	        case SORTBY_NAME:
                    swap = strcmp(item->name, item->next->name) > 0;
		    break;
	        case SORTBY_TIME:
		    swap = item->time < item->next->time;
		    break;
	        case SORTBY_STATUS:
		    swap = item->status > item->next->status;
		    break;
	        case SORTBY_VMSIZE:
		    swap = item->vm_size < item->next->vm_size;
		    break;
	        case SORTBY_VMRSS:
		    swap = item->vm_rss < item->next->vm_rss;
		    break;
	        case SORTBY_VMLIB:
		    swap = item->vm_lib < item->next->vm_lib;
		    break;
	        case SORTBY_CPU:
	        default        :
		    swap = (item->time-item->otime) < (item->next->time-item->next->otime);
	    }

	    if ( swap ) {
	        tmp = item->next;
	        if ( item->prev ) 
                     item->prev->next = tmp;
		else 
                     ps_list = tmp;
	        if( tmp->next ) 
                     tmp->next->prev = item;
		tmp->prev  = item->prev;
		item->prev = tmp;
		item->next = tmp->next;
		tmp->next  = item;
		if( (start=item) ) start=tmp;
		item=tmp;
	    }
	}
    } 
}

void 
KtopProcList::psList_clearProcVisit() 
{
    psPtr tmp;
    for( tmp=ps_list ; tmp ; tmp->visited=0 , tmp=tmp->next );
}

psPtr 
KtopProcList::psList_getProcItem(char* aName)
{
    psPtr tmp;
    int   pid;

    sscanf(aName,"%d",&pid);
    for ( tmp=ps_list ; tmp && (tmp->pid!=pid) ; tmp=tmp->next );

    if( ! tmp ) {
        // create an new elem & insert it 
        // at the top of the linked list
        tmp = new psStruct;
        if ( tmp ) {
           memset(tmp,0,sizeof(psStruct));
           tmp->pid=pid;
           tmp->next=ps_list;
           if( ps_list )
               ps_list->prev=tmp;
           ps_list=tmp;
        }
    }
    return tmp;
}

void 
KtopProcList::psList_removeProcUnvisited() 
{
    psPtr item, tmp;

    for ( item=ps_list ; item ; ) {
        if( ! item->visited ) {
            tmp = item;
            if ( item->prev )
                 item->prev->next = item->next;
	    else
	         ps_list = item->next;
            if ( item->next )
	         item->next->prev = item->prev;
            item = item->next; 
            delete tmp;
        }
	item = item->next;
    }
}

int 
KtopProcList::psList_getProcStatus(char *pid)
{
    char   buffer[1024], temp[128];
    FILE   *fd;
    int    u1, u2, u3, u4, time1, time2;
    psPtr  ps;

    ps = psList_getProcItem(pid);
    if ( ! ps ) return 0;
    
    sprintf(buffer, "/proc/%s/status", pid);
    if((fd = fopen(buffer, "r")) == 0)
        return 0;

    fscanf(fd, "%s %s", buffer, ps->name);
    fscanf(fd, "%s %c %s", buffer, &ps->status, temp);
    switch ( ps->status ) {
       case 'R':
           strcpy(ps->statusTxt,"Run");
           break;
       case 'S':
           strcpy(ps->statusTxt,"Sleep");
           break;
       case 'D': 
           strcpy(ps->statusTxt,"Disk");
           break;
       case 'Z': 
           strcpy(ps->statusTxt,"Zombie");
           break;
       case 'T': 
           strcpy(ps->statusTxt,"Stop");
           break;
       case 'W': 
           strcpy(ps->statusTxt,"Swap");
           break;
       default:
           strcpy(ps->statusTxt,"????");
           break;
    }
    fscanf(fd, "%s %d", buffer, &ps->pid);
    fscanf(fd, "%s %d", buffer, &ps->ppid);
    fscanf(fd, "%s %d %d %d %d", buffer, &u1, &u2, &u3, &u4);
    ps->uid = u1;
    fscanf(fd, "%s %d %d %d %d", buffer, &u1, &u2, &u3, &u4);
    ps->gid = u1;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_size);
    if(strcmp(buffer, "VmSize:"))
        ps->vm_size=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_lock);
    if(strcmp(buffer, "VmLck:"))
        ps->vm_lock=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_rss);
    if(strcmp(buffer, "VmRSS:"))
        ps->vm_rss=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_data);
    if(strcmp(buffer, "VmData:"))
        ps->vm_data=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_stack);
    if(strcmp(buffer, "VmStk:"))
        ps->vm_stack=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_exe);
    if(strcmp(buffer, "VmExe:"))
        ps->vm_exe=0;
    fscanf(fd, "%s %d %*s\n", buffer, &ps->vm_lib);
    if(strcmp(buffer, "VmLib:"))
        ps->vm_lib=0;
    fclose(fd);

    sprintf(buffer, "/proc/%s/stat", pid);
    if((fd = fopen(buffer, "r")) == 0)
        return 0;
    
    fscanf(fd, "%*s %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d", &time1, &time2);

    struct timeval tv;
    gettimeofday(&tv,0);
    ps->oabstime=ps->abstime;
    ps->abstime=tv.tv_sec*100+tv.tv_usec/10000;

    // if no time between two cycles - don't show any usable cpu percentage
    if(ps->oabstime==ps->abstime)
        ps->oabstime=ps->abstime-100000;
    ps->otime=ps->time;
    ps->time=time1+time2;

    fclose(fd);
    ps->visited=1;

    return 1;
}

void 
KtopProcList::userClickOnHeader(int indxCol)
{
#ifdef DEBUG_MODE
    printf("KTop debug : KtopProcList::userClickOnHeader : col = %d.\n",
		   indxCol);
#endif

  if ( indxCol ) {
       switch ( indxCol-1 ) {
          case SORTBY_PID: 
          case SORTBY_NAME: 
          case SORTBY_UID: 
          case SORTBY_CPU: 
          case SORTBY_TIME:
          case SORTBY_STATUS:
          case SORTBY_VMSIZE:
          case SORTBY_VMRSS:
          case SORTBY_VMLIB:
               setSortMethod(indxCol-1);
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
 
 if ( lastSelectionPid != NONE )
      err = kill(lastSelectionPid,0);

 if ( err || (lastSelectionPid == NONE) )
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

	for (int i = 0; i < cnt; i++)
	{
		txt = text(i, 1);
		res = FALSE;
		pid = txt.toInt(&res);
		if (res && (pid == lastSelectionPid))
		{
			setCurrentItem(i);
#ifdef DEBUG_MODE
			printf("KTop debug : KtopKtopProcList::restoreSelection :"
				   "cur pid : %d\n" ,pid);    
#endif
			return;
		}
	}
}

void 
KtopProcList::procHighlighted(int indx,int)
{ 
#ifdef DEBUG_MODE
    printf("KTop debug : KtopProcList::procHighlighted : item %d selected.\n",
		   indx);
#endif
  
	lastSelectionPid = NONE;
	sscanf(text(indx,1),"%d",&lastSelectionPid);

#ifdef DEBUG_MODE
    printf("KTop debug : KtopProcList::procHighlighted : lastSelectionPid"
		   "= %d.\n"
           ,lastSelectionPid);
#endif 
} 

int 
KtopProcList::cellHeight(int row)
{
#ifdef DEBUG_MODE
	printf("KTop debug : KtopProcList::cellHeight : called for row %d.\n",
		   row);
#endif

	const QPixmap *pix = icons->procIcon(text(row, 2));
	if (pix)
		return (pix->height());

	return (18);
}
