/*
    Container Library
   
	Copyright (c) 1992-1999 Chris Schlaeger <cs@kde.org>
    
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

	$Id$
*/

#include <stdlib.h>

#include "ccont.h"

#define new(a)    malloc(a)
#define delete(a) free(a)

/*
============================ private part ============================
*/

typedef struct Trailer
{
	struct Trailer* next;
	void*           object;
} TRAILER;

typedef struct
{
	TRAILER* trailer;
	INDEX    pos;
} CACHEMARK;

#define CACHES 32

typedef struct
{
	TRAILER* first;				/* Pointer to first trailer */
	INDEX    count;				/* Number of objects in list */
	CACHEMARK cachetab[CACHES];	/* Cachetable */
	int      lastcache;
} SCL;

static int   check_scl(void* scl, CHECK_FUNC check);
static void  destr_scl(void* scl, DESTR_FUNC destr_obj);
static TRAILER* find_trailer(SCL* hndl, INDEX pos);
static void* get_scl(void* scl, INDEX pos);
static int   insert_scl(void* scl, void* object, INDEX pos);
static INDEX level_scl(void* scl);
static void* new_scl(void);
static void* remove_scl(void* scl, INDEX pos);
static int   swop_scl(void* scl, INDEX pos1, INDEX pos2);

static int 
check_scl(void* scl, CHECK_FUNC check)
{
	TRAILER* help;
	SCL*     hndl = (SCL*) scl;
	int      i;
	INDEX    j;

	if (hndl == NIL)
		return (-1);
	for (help = hndl->first, j = (INDEX) 0;
	     help != NIL; help = help->next, j++)
		if (check != NIL)
			if ((*check)(help->object) < 0)
				return (-1);
	if (j != hndl->count)
		return (-1);
	for (i = 0; i < CACHES; i++)
		if (hndl->cachetab[j].trailer != NIL)
		{
			for (j = 0, help = hndl->first;
			     j < hndl->cachetab[i].pos && help != NIL;
			     j++, help = help->next)
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
destr_scl(void* scl, DESTR_FUNC destr_obj)
{
	TRAILER* help;
	SCL*     hndl = (SCL*) scl;

	while ((help = hndl->first) != NIL)
	{
		if (help->object != NIL && destr_obj != NIL)
			(*destr_obj)(help->object);
		hndl->first = help->next;
		delete(help);
	}
	delete(hndl);
}

#define ABS(a) ((a) < 0 ? -(a) : (a))

static TRAILER* 
find_trailer(SCL* hndl, INDEX pos)
{
	TRAILER* help;
	CACHEMARK cache;
	INDEX offset;
	int i;

	if (pos == (INDEX) 0)
		return (hndl->first);
	else if ((offset = pos) == (INDEX) -1)
		return (NIL);
	help = hndl->first;

	for (i = 0; i < CACHES; i++)
	{
		cache = hndl->cachetab[(hndl->lastcache + CACHES - i) % CACHES];
		if (cache.trailer != NIL)
		{
			if (cache.pos == pos)
				return (cache.trailer);
			else
				if (cache.pos < pos && pos - cache.pos < offset)
				{
					offset = pos - cache.pos;
					help = cache.trailer;
				}
		}
	}

	while (offset > 0 && help != NIL)
	{
		help = help->next;
		offset--;
	}
	hndl->lastcache = (hndl->lastcache + 1) % CACHES;
	hndl->cachetab[hndl->lastcache].trailer = help;
	hndl->cachetab[hndl->lastcache].pos = pos;
	return (help);
}

static void* 
get_scl(void* scl, INDEX pos)
{
	TRAILER* help;

	return ((help = find_trailer((SCL*) scl, pos)) != NIL ?
			help->object : NIL);
}

static int 
insert_scl(void* scl, void* object, INDEX pos)
{
	TRAILER* help;
	TRAILER* pred;
	SCL*     hndl = (SCL*) scl;
	int      i;

	if (pos < (INDEX) 0 || pos > hndl->count)
		return (-1);
	if ((help = new(sizeof(TRAILER))) == NIL)
		return (-1);
	help->object = object;
	if ((pred = find_trailer(hndl, pos - 1)) == NIL)
	{
		help->next = hndl->first;
		hndl->first = help;
	}
	else
	{
		help->next = pred->next;
		pred->next = help;
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
level_scl(void* scl)
{
	return (((SCL*) scl)->count);
}

static void* 
new_scl(void)
{
	SCL* hndl;
	int  i;

	hndl = new(sizeof(SCL));
	hndl->first = NIL;
	hndl->count = (INDEX) 0;
	for (i = 0; i < CACHES; i++)
		hndl->cachetab[i].trailer = NIL;
	hndl->lastcache = 0;
	return (hndl);
}

static void* 
remove_scl(void* scl, INDEX pos)
{
	TRAILER* help;
	TRAILER* pred;
	void*    object;
	SCL* hndl = (SCL*) scl;
	int i;

	if (pos < (INDEX) 0 || pos >= hndl->count)
		return (NIL);
	if ((pred = find_trailer(hndl, pos - 1)) == NIL)
	{
		object = hndl->first->object;
		hndl->first = (help = hndl->first)->next;
	}
	else
	{
		pred->next = (help = pred->next)->next;
		object = help->object;
	}

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
swop_scl(void* scl, INDEX pos1, INDEX pos2)
{
	TRAILER* trl1;
	TRAILER* trl2;
	void*    object;

	if ((trl1 = find_trailer((SCL*) scl, pos1)) == NIL)
		return (-1);
	if ((trl2 = find_trailer((SCL*) scl, pos2)) == NIL)
		return (-1);
	object = trl1->object;
	trl1->object = trl2->object;
	trl2->object = object;
	return (0);
}

/*
============================= public part ============================
*/

CONT_TOOLS Scl_tools =
{
	new_scl,
	destr_scl,
	check_scl,
	insert_scl,
	get_scl,
	remove_scl,
	level_scl,
	swop_scl
} ;
