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

#include "ccont.h"

/*
============================ private part ============================
*/

#define new(a)    malloc(a)
#define delete(a) free(a)

typedef struct Trailer
{
	struct Trailer* pred;
	struct Trailer* succ;
	void*           object;
} TRAILER;

typedef struct
{
	TRAILER* trailer;
	INDEX    pos;
} CACHEMARK;

#define CACHES 8

typedef struct
{
	TRAILER* first;				/* Pointer to first trailer */
	TRAILER* last;				/* Pointer to last trailer */
	INDEX    count;				/* Number of objects in list */
	CACHEMARK cachetab[CACHES];	/* Cachetable */
	int      lastcache;
} DCL;

static int   check_dcl(void* dcl, CHECK_FUNC check);
static void  destr_dcl(void* dcl, DESTR_FUNC destr_obj);
static TRAILER* find_trailer(DCL* hndl, INDEX pos);
static void* get_dcl(void* dcl, INDEX pos);
static int   insert_dcl(void* dcl, void* object, INDEX pos);
static INDEX level_dcl(void* dcl);
static void* new_dcl(void);
static void* remove_dcl(void* dcl, INDEX pos);
static int   swop_dcl(void* dcl, INDEX pos1, INDEX pos2);

static int 
check_dcl(void* dcl, CHECK_FUNC check)
{
	TRAILER* help;
	DCL*     hndl = (DCL*) dcl;
	int      i;
	INDEX    j;

	if (hndl == NIL ||
	    (hndl->first == NIL && hndl->last != NIL) ||
	    (hndl->first != NIL && hndl->last == NIL) ||
	    (hndl->first == NIL && hndl->count != (INDEX) 0) ||
	    (hndl->first != NIL && hndl->count == (INDEX) 0))
		return (-1);
	if (hndl->first != NIL)
	{
		if (hndl->first->pred != NIL || hndl->last->succ != NIL)
			return (-1);
		for (help = hndl->first, j = (INDEX) 1;
		     help->succ != NIL; help = help->succ, j++)
		{
			if (help->succ->pred != help)
				return (-1);
			if (check != NIL)
				if ((*check)(help->object) < 0)
					return (-1);
		}
		if (help != hndl->last || j != hndl->count)
			return (-1);
	}
	if (hndl->lastcache < 0 || hndl->lastcache >= CACHES)
		return (-1);
	for (i = 0; i < CACHES; i++)
		if (hndl->cachetab[i].trailer != NIL)
		{
			if (hndl->cachetab[i].pos < 0 ||
				hndl->cachetab[i].pos > hndl->count)
			{
				return (-1);
			}
			for (j = 0, help = hndl->first;
			     j < hndl->cachetab[i].pos && help != NIL;
			     j++, help = help->succ)
				;
			if (hndl->cachetab[i].pos != j ||
				help != hndl->cachetab[i].trailer)
			{
				return (-1);
			}
		}
	return (0);
}

static void 
destr_dcl(void* dcl, DESTR_FUNC destr_obj)
{
	TRAILER* help;
	DCL*     hndl = (DCL*) dcl;

	while ((help = hndl->first) != NIL)
	{
		if (help->object != NIL && destr_obj != NIL)
			(*destr_obj)(help->object);
		hndl->first = help->succ;
		delete(help);
	}
	delete(hndl);
}

#define ABS(a) ((a) < 0 ? -(a) : (a))

static TRAILER* 
find_trailer(DCL* hndl, INDEX pos)
{
	TRAILER* help;
	CACHEMARK cache;
	INDEX offset;
	int i;

	if (pos < hndl->count / 2)
	{
		if ((offset = pos) == (INDEX) 0)
			return (hndl->first);
		help = hndl->first;
	}
	else if (pos >= hndl->count)
		return (NIL);
	else
	{
		if ((offset = pos - hndl->count + (INDEX) 1) == (INDEX) 0)
			return (hndl->last);
		help = hndl->last;
	}
	for (i = 0; i < CACHES; i++)
	{
		cache = hndl->cachetab[(hndl->lastcache + CACHES - i) % CACHES];
		if (cache.trailer != NIL)
		{
			if (cache.pos == pos)
				return (cache.trailer);
			else
				if (ABS(cache.pos - pos) < ABS(offset))
				{
					offset = pos - cache.pos;
					help = cache.trailer;
				}
		}
	}
	while (offset != 0 && help != NIL)
		if (offset < 0)
		{
			help = help->pred;
			offset++;
		}
		else
		{
			help = help->succ;
			offset--;
		}
	hndl->lastcache = (hndl->lastcache + 1) % CACHES;
	hndl->cachetab[hndl->lastcache].trailer = help;
	hndl->cachetab[hndl->lastcache].pos = pos;
	return (help);
}

static void* 
get_dcl(void* dcl, INDEX pos)
{
	TRAILER* help;

	return ((help = find_trailer((DCL*) dcl, pos)) != NIL ?
			help->object : NIL);
}

static int 
insert_dcl(void* dcl, void* object, INDEX pos)
{
	TRAILER* help;
	DCL*     hndl = (DCL*) dcl;
	int      i;

	if ((help = new(sizeof(TRAILER))) == NIL)
		return (-1);
	help->object = object;
	if ((help->succ = find_trailer(hndl, pos)) == NIL)
	{
		if ((help->pred = hndl->last) != NIL)
			help->pred->succ = help;
		else
			hndl->first = help;
		hndl->last = help;
	}
	else
	{
		if ((help->pred = help->succ->pred) == NIL)
			hndl->first = help;
		else
			help->pred->succ = help;
		help->succ->pred = help;
	}
	hndl->count++;

	for (i = 0; i < CACHES; i++)
		if (hndl->cachetab[i].pos >= pos)
			hndl->cachetab[i].pos++;
	hndl->lastcache = (hndl->lastcache + 1) % CACHES;
	hndl->cachetab[hndl->lastcache].trailer = help;
	hndl->cachetab[hndl->lastcache].pos = pos;

	return (0);
}

static INDEX 
level_dcl(void* dcl)
{
	return (((DCL*) dcl)->count);
}

static void* 
new_dcl(void)
{
	DCL* hndl;
	int  i;

	hndl = new(sizeof(DCL));
	hndl->first = hndl->last = NIL;
	hndl->count = (INDEX) 0;
	for (i = 0; i < CACHES; i++)
		hndl->cachetab[i].trailer = NIL;
	hndl->lastcache = 0;
	return (hndl);
}

static void* 
remove_dcl(void* dcl, INDEX pos)
{
	TRAILER* help;
	void*    object;
	DCL* hndl = (DCL*) dcl;
	int i;

	if ((help = find_trailer(hndl, pos)) == NIL)
		return (NIL);

	if (help->pred != NIL)
		help->pred->succ = help->succ;
	else
		hndl->first = help->succ;

	if (help->succ != NIL)
		help->succ->pred = help->pred;
	else
		hndl->last = help->pred;
	object = help->object;
	delete(help);
	hndl->count--;

	for (i = 0; i < CACHES; i++)
		if (hndl->cachetab[i].pos > pos)
			hndl->cachetab[i].pos--;
		else
			if (hndl->cachetab[i].pos == pos)
				hndl->cachetab[i].trailer = NIL;

	return (object);
}

static int 
swop_dcl(void* dcl, INDEX pos1, INDEX pos2)
{
	TRAILER* trl1;
	TRAILER* trl2;
	void*    object;

	if ((trl1 = find_trailer((DCL*) dcl, pos1)) == NIL)
		return (-1);
	if ((trl2 = find_trailer((DCL*) dcl, pos2)) == NIL)
		return (-1);
	object = trl1->object;
	trl1->object = trl2->object;
	trl2->object = object;
	return (0);
}

/*
============================= public part ============================
*/

CONT_TOOLS Dcl_tools =
{
	new_dcl,
	destr_dcl,
	check_dcl,
	insert_dcl,
	get_dcl,
	remove_dcl,
	level_dcl,
	swop_dcl
} ;
