/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net

	Copyright (c) 1999 Chris Schlaeger <cs@axys.de>
    
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

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include "IconList.h"

#define KDE_ICN_DIR   "/share/icons/mini"
#define KTOP_ICN_DIR  "/share/apps/ktop/pics"

// exec.xpm = default Icon for processes tree.
// drawn  by Mark Donohoe for the K Desktop Environment 
static const char* execXpm[]={
"16 16 7 1",
"# c #000000",
"d c #008080",
"a c #beffff",
"c c #00c0c0",
"b c #585858",
"e c #00c0c0",
". c None",
"................",
".....######.....",
"..###abcaba###..",
".#cabcbaababaa#.",
".#cccaaccaacaa#.",
".###accaacca###.",
"#ccccca##aaccaa#",
"#dd#ecc##cca#cc#",
"#d#dccccccacc#c#",
"##adcccccccccd##",
".#ad#c#cc#d#cd#.",
".#a#ad#cc#dd#d#.",
".###ad#dd#dd###.",
"...#ad#cc#dd#...",
"...####cc####...",
"......####......"};

/*=============================================================================
 Class : KtopIconListElem (methods)
 =============================================================================*/
/*-----------------------------------------------------------------------------
  Routine : KtopIconListElem::KtopIconListElem (constructor)
 -----------------------------------------------------------------------------*/
KtopIconListElem::KtopIconListElem(const char* fName,const char* iName)
{
  QPixmap  new_xpm;

  pm = new QPixmap(fName);
  if ( pm && pm->isNull() ) {
       delete pm;
       pm = NULL;
  }
  strcpy(icnName,iName);
}

/*-----------------------------------------------------------------------------
  Routine : KtopIconListElem::~KtopIconListElem (destructor)
 -----------------------------------------------------------------------------*/
KtopIconListElem::~KtopIconListElem()
{
  if ( pm ) delete pm;
}

/*-----------------------------------------------------------------------------
  Routine : KtopIconListElem::pixmap
 -----------------------------------------------------------------------------*/
const QPixmap* KtopIconListElem::pixmap()
{
  return pm;
}

/*-----------------------------------------------------------------------------
  Routine : KtopIconListElem::name()
 -----------------------------------------------------------------------------*/
const char* KtopIconListElem::name()
{
  return icnName;
}

//*****************************************************************************
//*****************************************************************************
/*=============================================================================
 Class : KtopIconList (methods)
 =============================================================================*/

/*-----------------------------------------------------------------------------
 definition of the KtopIconList's static member "icnList".
 -----------------------------------------------------------------------------*/
QList<KtopIconListElem> *KtopIconList::icnList     = NULL;
int                      KtopIconList::instCounter = 0;
const QPixmap*           KtopIconList::defaultIcon = NULL;

/*-----------------------------------------------------------------------------
  Routine : KtopIconList::KtopIconList (constructor)
 -----------------------------------------------------------------------------*/
KtopIconList::KtopIconList()
{
  if ( ! KtopIconList::instCounter ) {
       #ifdef DEBUG_MODE
         printf("KTop debug : KtopIconList : No instance. Loading icons...\n");
       #endif
       KtopIconList::load();
       defaultIcon = new QPixmap(execXpm);
       CHECK_PTR(defaultIcon);
  }
  KtopIconList::instCounter++;
  #ifdef DEBUG_MODE
     printf("KTop debug : KtopIconList: Instance n°%d created.\n"
            ,KtopIconList::instCounter);
  #endif
}

/*-----------------------------------------------------------------------------
  Routine : KtopIconList::~KtopIconList (destructor)
 -----------------------------------------------------------------------------*/
KtopIconList::~KtopIconList()
{ 
  #ifdef DEBUG_MODE
     printf("KTop debug : KtopIconList : Deleting instance n°%d\n"
            ,KtopIconList::instCounter);
  #endif
  KtopIconList::instCounter--;

  if ( ! KtopIconList::instCounter ) {
    #ifdef DEBUG_MODE
      printf("KTop debug : KtopIconList : No more instance. Deleting icons...\n");
    #endif
    delete KtopIconList::defaultIcon;
    KtopIconList::icnList->setAutoDelete(TRUE);
    KtopIconList::icnList->clear();
  }
}

/*-----------------------------------------------------------------------------
  Routine : KtopIconList::procIcon
 -----------------------------------------------------------------------------*/
const QPixmap* KtopIconList::procIcon( const char* pname )
{
 KtopIconListElem* cur  = KtopIconList::icnList->first();
 KtopIconListElem* last = KtopIconList::icnList->getLast();
 bool              goOn = TRUE;
 char              iName[128];

 sprintf(iName,"%s.xpm",pname);
 do {
    if ( cur == last ) goOn = FALSE;
    if (! cur ) goto end;
    if ( !strcmp(cur->name(),iName) ) {
         const QPixmap *pix = cur->pixmap();
         if ( pix ) return pix;
         else return defaultIcon;
    }
    cur = KtopIconList::icnList->next(); 
 } while ( goOn ) ;

 end: 
   return defaultIcon;
}

/*-----------------------------------------------------------------------------
  Routine : KtopIconList::load
 -----------------------------------------------------------------------------*/
void KtopIconList::load()
{
    DIR           *dir;
    struct dirent *de;
    char           path[PATH_MAX+1];
    char           icnFile[PATH_MAX+1];
    char           prefix[32];

    icnList = new QList<KtopIconListElem>;
    CHECK_PTR(icnList);

    #ifdef DEBUG_MODE
       printf("KTop debug : KtopIconList::load : looking for icons...\n");
    #endif

    char *kde_dir = getenv("KDEDIR");

    if ( kde_dir ) {
         sprintf(path,"%s%s",kde_dir,KDE_ICN_DIR);
         dir = opendir(path);
    } 
    else {
          sprintf(prefix,"/opt/kde");
          sprintf(path,"%s%s",prefix,KDE_ICN_DIR);
          dir = opendir(path);
          if ( ! dir ) {
               sprintf(prefix,"/usr/local");
               sprintf(path,"%s%s",prefix,KDE_ICN_DIR);
               dir = opendir(path);  
          }
    }
  
    if ( ! dir ) return;  // default icon will be used
    
    while ( ( de = readdir(dir) ) )
     {
	if( strstr(de->d_name,".xpm") ) { 
            sprintf(icnFile,"%s/%s",path,de->d_name);
            KtopIconListElem *newElem = new KtopIconListElem(icnFile,de->d_name);
            CHECK_PTR(newElem);
            icnList->append(newElem);  
	}       
     }

    (void)closedir(dir);
  
    if ( kde_dir ) {
         sprintf(path,"%s%s",kde_dir,KTOP_ICN_DIR);
    } else {
          sprintf(path,"%s%s",prefix,KTOP_ICN_DIR);
          #ifdef DEBUG_MODE
            printf("KTop debug : KtopIconList::load : trying %s...\n",path);
          #endif
    }

    dir = opendir(path);     
    if ( ! dir ) return; // default icon will be used
  
    while ( ( de = readdir(dir) ) )
     {
	if( strstr(de->d_name,".xpm") ) 
         { 
            sprintf(icnFile,"%s/%s",path,de->d_name);
            KtopIconListElem *newElem = new KtopIconListElem(icnFile,de->d_name);
            CHECK_PTR(newElem);
            icnList->append(newElem);  
	}       
     }

    (void)closedir(dir);
}

