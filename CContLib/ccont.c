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

#include <stdio.h>
#include <stdlib.h>

#include "ccont.h"

extern CONT_TOOLS Dcl_tools;
extern CONT_TOOLS Scl_tools;
extern CONT_TOOLS Pga_tools;

/*
============================ private part ============================
*/

#define new(a)  malloc(a)
#define delete free
static int Debuglevel = 1;
#define BAD_CALL -1

static void rprt_err(int level, const char* text);

static CONT_TOOLS* Tools[] =
{
	&Dcl_tools,
	&Scl_tools,
	&Pga_tools,
} ;

static void 
rprt_err(int level, const char* text)
{
	if (level >= Debuglevel)
		fprintf(stderr, "\n%s", text);
}

/*
============================= public part ============================
*/

void 
bsort_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, REPORT_FUNC report)
{
	INDEX right, i, level, last;

	if (ctnr == NIL || compare == NIL)
	{
		rprt_err(BAD_CALL, "bsort_ctnr: bad parameter");
		return;
	}
	last = level = (*ctnr->tools->level)(ctnr->handle);
	do
	{
		right = last;
		last = 0;
		if (report != NIL)
			(*report)(level - right, level);
		for (i = 1; i < right; i++)
			if ((*compare)((*ctnr->tools->get)(ctnr->handle, i - 1),
			               (*ctnr->tools->get)(ctnr->handle, i)) > 0)
				(*ctnr->tools->swop)(ctnr->handle, i - 1, last = i);
	} while (last > 0);
	if (report != NIL)
		(*report)(level, level);
}

int 
check_ctnr(CONTAINER ctnr, CHECK_FUNC check)
{
	if (ctnr == NIL)
	{
		rprt_err(BAD_CALL, "check_ctnr: bad_parameter");
		return (-1);
	}
	return ((*ctnr->tools->check)(ctnr->handle, check));
}

void 
destr_ctnr(CONTAINER ctnr, DESTR_FUNC destr_obj)
{
	if (ctnr == NIL)
	{
		rprt_err(BAD_CALL, "destr_ctnr: bad parameter");
		return;
	}
	(*ctnr->tools->destruct)(ctnr->handle, destr_obj);
	delete(ctnr);
}

void* 
get_ctnr(CONTAINER ctnr, INDEX pos)
{
	if (ctnr == NIL || pos < 0 ||
	    pos >= (*ctnr->tools->level)(ctnr->handle))
	{
		rprt_err(BAD_CALL, "get_ctnr: bad parameter");
		return (NIL);
	}
	return ((*ctnr->tools->get)(ctnr->handle, pos));
}

int 
insert_ctnr(CONTAINER ctnr, void* object, INDEX pos)
{
	if (ctnr == NIL || pos > ((INDEX) 1 << (sizeof(INDEX) * 8 - 2)) ||
	    pos < (INDEX) 0 || pos > (*ctnr->tools->level)(ctnr->handle))
	{
		rprt_err(BAD_CALL, "insert_ctnr: bad parameter");
		return (-1);
	}
	return ((*ctnr->tools->insert)(ctnr->handle, object, pos));
}

INDEX 
level_ctnr(CONTAINER ctnr)
{
	if (ctnr == NIL)
	{
		rprt_err(BAD_CALL, "level_ctnr: bad parameter");
		return ((INDEX) 0);
	}
	return ((*ctnr->tools->level)(ctnr->handle));
}

CONTAINER 
new_ctnr(CONTAINER_TYPE type)
{
	CONTAINER ctnr;

	ctnr = new(sizeof(CONT));
	ctnr->tools = Tools[type];
	ctnr->handle = (*ctnr->tools->new)();
	return (ctnr);
}

void* 
plop_ctnr(CONTAINER ctnr)
{
	INDEX level;

	if (ctnr == NIL)
	{
		rprt_err(BAD_CALL, "plop_ctnr: bad parameter");
		return (NIL);
	}
	if ((level = (*ctnr->tools->level)(ctnr->handle)) == 0)
	{
		rprt_err(BAD_CALL, "plop_ctnr: container is empty");
		return (NIL);
	}
	return ((*ctnr->tools->remove)(ctnr->handle, level - (INDEX) 1));
}

void* 
pop_ctnr(CONTAINER ctnr)
{
	if (ctnr == NIL)
	{
		rprt_err(BAD_CALL, "pop_ctnr: bad parameter");
		return (NIL);
	}
	if ((*ctnr->tools->level)(ctnr->handle) == 0)
	{
		rprt_err(BAD_CALL, "pop_ctnr: container is empty");
		return (NIL);
	}
	return ((*ctnr->tools->remove)(ctnr->handle, (INDEX) 0));
}

int 
push_ctnr(CONTAINER ctnr, void* object)
{
	if (ctnr == NIL)
	{
		rprt_err(BAD_CALL, "push_ctnr: bad parameter");
		return (-1);
	}
	return ((*ctnr->tools->insert)(ctnr->handle, object, (INDEX) 0));
}

void 
qsort_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, REPORT_FUNC report)
{
	typedef struct
	{
		INDEX left;
		INDEX right;
	} CUT;

	void*     pivot;
	INDEX     level, sorted, i, j, left, right;
	CONTAINER stack;
	CUT*      cut;

	if (ctnr == NIL || compare == NIL)
	{
		rprt_err(BAD_CALL, "qsort_ctnr: bad parameter");
		return;
	}
	if ((level = (*ctnr->tools->level)(ctnr->handle)) < 2)
		return;
	stack = new_ctnr(CT_SLL);
	cut = new(sizeof(CUT));
	cut->left = (INDEX) 0;
	cut->right = level - 1;
	push_ctnr(stack, cut);
	sorted = (INDEX) 0;
	do
	{
		if (report != NIL)
			(*report)(sorted, level);
		cut = (CUT*) pop_ctnr(stack);
		left = cut->left;
		right = cut->right;
		delete(cut);
		pivot = (*ctnr->tools->get)(ctnr->handle, (left + right) / 2);
		j = right;
		i = left;
		do
		{
			while (compare((*ctnr->tools->get)(ctnr->handle, i), pivot) < 0)
				i++;
			while (compare((*ctnr->tools->get)(ctnr->handle, j), pivot) > 0)
				j--;
			if (i <= j)
				(*ctnr->tools->swop)(ctnr->handle, i++, j--);
		} while (i <= j);
		sorted += (i <= right ? i : right) - (j >= left ? j : left);
		if (left < j)
		{
			cut = new(sizeof(CUT));
			cut->left = left;
			cut->right = j;
			push_ctnr(stack, cut);
		}
		if (i < right)
		{
			cut = new(sizeof(CUT));
			cut->left = i;
			cut->right = right;
			push_ctnr(stack, cut);
		}
	} while (level_ctnr(stack) > (INDEX) 0);
	destr_ctnr(stack, delete);
	if (report != NIL)
		(*report)(level, level);
}

void* 
remove_ctnr(CONTAINER ctnr, INDEX pos)
{
	if (ctnr == NIL || pos < 0 ||
	    pos >= (*ctnr->tools->level)(ctnr->handle))
	{
		rprt_err(BAD_CALL, "remove_ctnr: bad parameter");
		return (NIL);
	}
	return ((*ctnr->tools->remove)(ctnr->handle, pos));
}

INDEX 
search_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, void* pattern)
{
	INDEX i;

	if (ctnr == NIL || compare == NIL || pattern == NIL)
	{
		rprt_err(BAD_CALL, "search_ctnr: bad parameter");
		return (-1);
	}
	i = (*ctnr->tools->level)(ctnr->handle);
	while (i-- > 0)
	{
		if ((*compare)(pattern, (*ctnr->tools->get)(ctnr->handle, i)) == 0)
			return (i);
	}
	return (-1);	/* nothing found */
}
