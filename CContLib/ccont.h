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

#if !defined _CCONT_H_
#define _CCONT_H_

typedef long INDEX;

typedef void (*DESTR_FUNC)(void*);
typedef int  (*COMPARE_FUNC)(void*, void*);
typedef void (*REPORT_FUNC)(INDEX, INDEX);
typedef int  (*CHECK_FUNC)(void*);

typedef enum
{
	CT_DLL = 0,
	CT_SLL = 1,
	CT_PGA = 2,
} CONTAINER_TYPE;

typedef struct
{
	void* (*new)(void);
	void  (*destruct)(void*, DESTR_FUNC);
	int   (*check)(void*, CHECK_FUNC);
	int   (*insert)(void*, void*, INDEX);
	void* (*get)(void*, INDEX);
	void* (*remove)(void*, INDEX);
	INDEX (*level)(void*);
	int   (*swop)(void*, INDEX, INDEX);
} CONT_TOOLS;

typedef struct
{
	CONT_TOOLS* tools;
	void*       handle;
} CONT;

typedef CONT* CONTAINER;

void      bsort_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, REPORT_FUNC report);
int       check_ctnr(CONTAINER ctnr, CHECK_FUNC check);
void      destr_ctnr(CONTAINER ctnr, DESTR_FUNC destr_obj);
void*     get_ctnr(CONTAINER ctnr, INDEX pos);
int       insert_ctnr(CONTAINER ctnr, void* object, INDEX pos);
INDEX     level_ctnr(CONTAINER ctnr);
CONTAINER new_ctnr(CONTAINER_TYPE type);
void*     plop_ctnr(CONTAINER ctnr);
void*     pop_ctnr(CONTAINER ctnr);
int       push_ctnr(CONTAINER ctnr, void* object);
void      qsort_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, REPORT_FUNC report);
void*     remove_ctnr(CONTAINER ctnr, INDEX pos);
INDEX     search_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, void* pattern);

#define NIL     ((void*) 0L)

#endif
