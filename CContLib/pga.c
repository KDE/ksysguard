/*
    C Container Library
   
	Copyright (c) 1992-1999 Chris Schlaeger <cs@kde.org>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

	$Id$
*/

#include <stdlib.h>
#include <string.h>

#include "ccont.h"

/*
============================ private part ============================
*/

#define new(a)    malloc(a)
#define delete(a) free(a)

#define PAGESIZE ((INDEX) 64)	/* should be carefully chosen */

typedef struct Page
{
	void*  objects[PAGESIZE];	/* Pointer to objects */
	struct Page* pred;			/* Pointer to previos page */
	struct Page* succ;			/* Pointer to next page */
} PAGE;

typedef struct
{
	INDEX pos;					/* Position of object in the cache */
	INDEX pos1;					/* Position of 1st object on that page */
	PAGE* page;					/* Page where the object is on */
} CACHEMARK;

#define CACHES 4

typedef struct
{
	PAGE*    first;				/* Pointer to first page */
	PAGE*    last;				/* Pointer to last page */
	INDEX    count;				/* Total number of objects stored */
	CACHEMARK cachetab[CACHES];	/* Cachetable */
	int      lastcache;
} PGA;

static int   check_pga(void* pga, CHECK_FUNC check);
static void  destr_pga(void* pga, DESTR_FUNC destr_obj);
static void* get_pga(void* pga, INDEX pos);
static PAGE* find_page(PGA* pga, INDEX* pos);
static int   insert_pga(void* pga, void* object, INDEX pos);
static INDEX level_pga(void* pga);
static void* new_pga(void);
static void* remove_pga(void* pga, INDEX pos);
static int   swop_pga(void* pga, INDEX pos1, INDEX pos2);

static int 
check_pga(void* pga, CHECK_FUNC check)
{
	PAGE* page;
	PGA*  hndl = (PGA*) pga;
	INDEX i, pos;

	if ((hndl->first == NIL && hndl->last != NIL) ||
	     (hndl->first != NIL && hndl->last == NIL))
		return (-1);
	for (page = hndl->first, pos = hndl->count; page != NIL; page = page->succ)
	{
		if ((page == hndl->first && page->pred != NIL) ||
		     (page == hndl->last && page->succ != NIL) ||
		     (page->pred == NIL && page != hndl->first) ||
		     (page->succ == NIL && page != hndl->last) ||
		     (page->pred != NIL && page->pred->succ != page) ||
		     (page->succ != NIL && page->succ->pred != page))
			return (-1);
		for (i = 0; i < PAGESIZE && i < pos; i++)
			if (check != NIL)
				if ((*check)(page->objects[i]) < 0)
					return (-1);
		pos -= PAGESIZE;
	}
	return (0);
}

static void 
destr_pga(void* pga, DESTR_FUNC destr_obj)
{
	PAGE* page;
	PGA*  hndl = (PGA*) pga;
	INDEX i, pos;

	pos = hndl->count;
	while ((page = hndl->first) != NIL)
	{
		for (i = (INDEX) 0; i < PAGESIZE && i < pos; i++)
			if (page->objects[i] != NIL && destr_obj != NIL)
				(*destr_obj)(page->objects[i]);
		hndl->first = page->succ;
		delete(page);
		pos -= PAGESIZE;
	}
	delete(hndl);
}

static void* 
get_pga(void* pga, INDEX pos)
{
	PAGE* page;

	page = find_page((PGA*) pga, &pos);
	return (page->objects[pos]);
}

static PAGE* 
find_page(PGA* hndl, INDEX* pos)
{
	PAGE*     page;
	CACHEMARK* cache;
	int       i;

	if (*pos < PAGESIZE)
		return (hndl->first);			/* it's on the first page */

	if (*pos >= (hndl->count / PAGESIZE) * PAGESIZE)
	{
		*pos %= PAGESIZE;				/* it's on the last page */
		return (hndl->last);
	}

	for (i = 0; i < CACHES; i++)
	{
		cache = &hndl->cachetab[(hndl->lastcache + CACHES - i) % CACHES];
		if (cache->page != NIL)
		{
			if (cache->pos == *pos)
			{
				*pos -= cache->pos1;
				return (cache->page);
			}
		}
	}

	for (page = hndl->first; *pos >= PAGESIZE; 
	     *pos -= PAGESIZE, page = page->succ)
		;
	return (page);
}

static int 
insert_pga(void* pga, void* object, INDEX pos)
{
	PAGE* page;
	PAGE* help;
	PGA*  hndl = (PGA*) pga;

	if (hndl->count % PAGESIZE == 0)	/* All pages full => new's needed */
	{
		page = new(sizeof(PAGE));
		page->succ = NIL;
		if (hndl->first == NIL)
			(hndl->first = hndl->last = page)->pred = NIL;
		else
		{
			(page->pred = hndl->last)->succ = page;
			hndl->last = page;
		}
	}

	page = find_page(hndl, &pos);

	for (help = hndl->last; help != page; help = help->pred)
	{
		memmove(&help->objects[1], &help->objects[0],
		        (PAGESIZE - 1) * sizeof(void*));
		help->objects[0] = help->pred->objects[PAGESIZE - 1];
	}
	if (pos < PAGESIZE - 1)
		memmove(&page->objects[pos + 1], &page->objects[pos],
		        (PAGESIZE - pos - 1) * sizeof(void*));

	page->objects[pos] = object;
	hndl->count++;
	return (0);
}

static INDEX 
level_pga(void* pga)
{
	return (((PGA*) pga)->count);
}

static void* 
new_pga(void)
{
	PGA* hndl;
	int  i;

	hndl = new(sizeof(PGA));
	hndl->first = hndl->last = NIL;
	hndl->count = (INDEX) 0;
	for (i = 0; i < CACHES; i++)
		hndl->cachetab[i].page = NIL;
	hndl->lastcache = 0;
	return (hndl);
}

static void* 
remove_pga(void* pga, INDEX pos)
{
	PAGE* page;
	void* object;
	PGA* hndl = (PGA*) pga;

	page = find_page(hndl, &pos);
	object = page->objects[pos];

	if (pos < PAGESIZE - 1)
		memmove(&page->objects[pos], &page->objects[pos + (INDEX) 1],
		        (PAGESIZE - pos - 1) * sizeof(void*));
	while (page != hndl->last)
	{
		page->objects[PAGESIZE - (INDEX) 1] = page->succ->objects[(INDEX) 0];
		memmove(&page->succ->objects[(INDEX) 0],
		        &page->succ->objects[(INDEX) 1],
		        (PAGESIZE - (INDEX) 1) * sizeof(void*));
		page = page->succ;
	}

	if (--hndl->count % PAGESIZE == 0)
	{
		if ((hndl->last = hndl->last->pred) == NIL)
		{
			delete(hndl->first);
			hndl->first = NIL;
		}
		else
		{
			delete(hndl->last->succ);
			hndl->last->succ = NIL;
		}
	}

	return (object);
}

static int 
swop_pga(void* pga, INDEX pos1, INDEX pos2)
{
	PAGE* page1;
	PAGE* page2;
	void* object;

	page1 = find_page((PGA*) pga, &pos1);
	page2 = find_page((PGA*) pga, &pos2);

	object = page1->objects[pos1];
	page1->objects[pos1] = page2->objects[pos2];
	page2->objects[pos2] = object;

	return (0);
}

/*
============================= public part ============================
*/

CONT_TOOLS Pga_tools =
{
	new_pga,
	destr_pga,
	check_pga,
	insert_pga,
	get_pga,
	remove_pga,
	level_pga,
	swop_pga
} ;
