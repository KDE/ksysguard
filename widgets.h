/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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

/*=============================================================================
  HEADERs
 =============================================================================*/
#include "ptree.h"
#include <ktablistbox.h>

/*=============================================================================
  TYPEDEFs
 =============================================================================*/
typedef struct proc_status {
    char  name[101];
    char  status;
    char  statusTxt[10];
    pid_t pid, ppid;
    uid_t uid;
    gid_t gid;
    unsigned vm_size, vm_lock, vm_rss, vm_data, vm_stack, vm_exe;
#ifdef __FreeBSD__
    u_char  priority;
#else
    unsigned vm_lib;
#endif
    int otime, time, oabstime, abstime;
    struct proc_status* next;
    struct proc_status* prev;
    char visited;
}psStruct,*psPtr;

/*=============================================================================
  CLASSes
 =============================================================================*/
//-----------------------------------------------------------------------------
// class  : IconListElem
//-----------------------------------------------------------------------------
class IconListElem
{
public:
      IconListElem(const char* fName,const char* iName);
     ~IconListElem();

      const QPixmap* getPixmap();
      const char*    getName();

private:
    QPixmap *pm;
    char     icnName[128];
};

//-----------------------------------------------------------------------------
// class  : ProcList
//-----------------------------------------------------------------------------
class ProcList : public KTabListBox
{
    Q_OBJECT
public:
      ProcList(QWidget* parent , const char* name, int nCol);
     ~ProcList();
protected:
      virtual int  cellHeight      ( int );
signals:
      void clicked(QMouseEvent*);    
};

//-----------------------------------------------------------------------------
// class TaskMan
//-----------------------------------------------------------------------------
class TaskMan : public QTabDialog
{
    Q_OBJECT

public:

     enum { MENU_ID_SIGINT=500, 
            MENU_ID_SIGQUIT   ,
            MENU_ID_SIGTERM   ,
            MENU_ID_SIGKILL   ,
            MENU_ID_SIGUSR1   ,
            MENU_ID_SIGUSR2  
          };
     enum { PAGE_PLIST=0 , 
            PAGE_PTREE   , 
            PAGE_PERF   
          };
     enum { MENU_ID_PROP=100 ,
            MENU_ID_KILL 
          };
     enum { UPDATE_SLOW=0 , 
            UPDATE_MEDIUM , 
            UPDATE_FAST     
          }; 
     enum { UPDATE_SLOW_VALUE=20  , 
            UPDATE_MEDIUM_VALUE=7 , 
            UPDATE_FAST_VALUE=1  
          }; 
     enum { SORTBY_PID=0   , 
            SORTBY_NAME    , 
            SORTBY_UID     , 
            SORTBY_CPU     ,
            SORTBY_TIME    ,
            SORTBY_STATUS  ,
            SORTBY_VMSIZE  ,
            SORTBY_VMRSS   ,
#ifdef __FreeBSD__
            SORTBY_PRIOR   ,
#else
            SORTBY_VMLIB   ,
#endif
          };

     TaskMan(QWidget *parent = 0, const char *name = 0, int sfolder=0);
    ~TaskMan();
     
            int  setUpdateInterval    ( int );
            int  getUpdateInterval    ( );
            void invokeSettings       ( );
            void raiseStartUpPage     ( );
            void saveSettings         ( );

     static void           TaskMan_initIconList  ( ); 
     static void           TaskMan_clearIconList ( ); 
     static const QPixmap* TaskMan_getProcIcon   ( const char* );     

private:

    QPushButton  *pList_bKill, 
                 *pList_bRefresh, 
                 *pTree_bRefresh,
                 *pTree_bRoot,
	         *pTree_bKill; 
    QWidget      *pages[3],
     		 *mem_cur,
		 *cpu_cur,
	         *cpu_icon;
    QGroupBox    *pList_box,
	         *pTree_box,
		 *cpubox, 
		 *membox, 
		 *cpubox1, 
		 *membox1;
    QComboBox    *pList_cbRefresh,
                 *pTree_cbSort;
    ProcList     *pList;
    ProcTree     *pTree;
    QPopupMenu   *pSig;
    CpuMon       *cpumon;
    MemMon       *memmon;

    AppSettings  *settings;

    int          pList_sortby,
                 pList_refreshRate,
                 pTree_sortby;

    int          timer_interval, 
                 tid, 
                 startup_page;

    char         cfgkey_startUpPage[12],
                 cfgkey_pListUpdate[12],
                 cfgkey_pListSort[12],
                 cfgkey_pTreeSort[12];
                 
    bool         pTree_updating,
                 restoreStartupPage,
                 mouseRightButDown;

    int          pList_lastSelectionPid,
                 pTree_lastSelectionPid;

    psPtr        ps;
    psPtr        ps_list;

#ifdef __FreeBSD__
    QGroupBox    *mem_detail_box;
    QLabel       *mem_details[10];
    QLabel       *l_mem_details[10];
#endif

    static QList<IconListElem> *icons;

    virtual void           resizeEvent     ( QResizeEvent* );
    virtual void           timerEvent      ( QTimerEvent*  );
 
    virtual void           pList_load                ( );
#ifndef __FreeBSD__
    virtual int    	   pList_getProcStatus       ( char * );
#endif
    virtual void   	   pList_clearProcVisit      ( );
    virtual void           pList_removeProcUnvisited ( );
    virtual void           pList_sort                ( );
    virtual psPtr	   pList_getProcItem         ( char* aName );
            void           pList_restoreSelection    ( int lvc = 0 );
            const QPixmap* pList_getProcIcon         ( const char* name );

    virtual void           pTree_sort                ( );
    virtual void           pTree_readProcDir         ( );
            void           pTree_reorder             ( ProcTree* );
    static  ProcTreeItem*  pTree_getParentItem       ( ProcTreeItem* , int ); 
    static  ProcTreeItem*  pTree_sortByName          ( ProcTreeItem* );
    static  ProcTreeItem*  pTree_sortByPid           ( ProcTreeItem* , bool );
    static  ProcTreeItem*  pTree_sortByUid           ( ProcTreeItem* );
            ProcTreeItem*  pTree_restoreSelection    ( ProcTreeItem* item ); 
            const QPixmap* pTree_getProcIcon         ( const char* name );

public slots:

    void pSigHandler              ( int );
    void tabBarSelected           ( int );

    void pList_update             ( );
    void pList_procHighlighted    ( int , int );
    void pList_cbRefreshActivated ( int );
    void pList_headerClicked      ( int );
    void pList_popupMenu          ( int , int );
    void pList_killTask           ( );

    void pTree_procHighlighted    ( int );
    void pTree_cbSortActivated    ( int );
    void pTree_clicked            ( QMouseEvent* );
    void pTree_killTask           ( );
    void pTree_update             ( );
    void pTree_changeRoot         ( ); 
    void pTree_sortUpdate         ( );
};

