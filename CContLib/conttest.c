/*
    Simple test program for the C container library.
   
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "ccont.h"

#define NUMBERS 15000
#define new     malloc
#define delete  free

/*
============================ private part ============================
*/

static int compare(int* i1, int* i2);
static void get_timer(struct timeval*);
static long lap_time(void);
static void start_timer(void);
static void test(CONTAINER_TYPE type);

static struct timeval timer;

static int 
compare(int* i1, int* i2)
{
	return (*i1 < *i2 ? -1 : *i1 > *i2 ? 1 : 0);
}

void
get_timer(struct timeval* tv)
{
	struct timezone tz;

	gettimeofday(tv, &tz);
}

static long 
lap_time(void)
{
	struct timeval tv2;

	get_timer(&tv2);
/*	printf("Timer1: %d %d\nTimer2: %d %d\n",
		   timer.tv_sec, timer.tv_usec,
		   tv2.tv_sec, tv2.tv_usec); */
	return ((tv2.tv_sec - timer.tv_sec) * 1000 +
			(tv2.tv_usec - timer.tv_usec) / 1000);
}

static void 
start_timer(void)
{
	get_timer(&timer);
}

static void 
test_insert(CONTAINER_TYPE type)
{
	CONTAINER cont;
	int* buf;
	INDEX i;

	buf = new(sizeof(int));

	srand(1234);
	cont = new_ctnr(type);
	start_timer();
	for(i = (INDEX) 0; i < (INDEX) 1000; i++)
		insert_ctnr(cont, buf, rand() % ((int) i + 1));
	printf(" %6.3f", lap_time() / 1000.0);
	destr_ctnr(cont, (DESTR_FUNC) NIL);

	delete(buf);
}

static void 
test_sort(CONTAINER_TYPE type)
{
	CONTAINER nlist;
	int* buf;
	int i;

	srand(1234);
	nlist = new_ctnr(type);
	if (check_ctnr(nlist, (CHECK_FUNC) NIL) < 0)
		printf("\nContainer corrupted!");
	for (i = 0; i < NUMBERS; i++)
	{
		buf = new(sizeof(int));
		*buf = rand();
		if (insert_ctnr(nlist, buf, (INDEX) i) < 0)
		{
			printf("\nContainer full at %d!", i);
			delete(buf);
			destr_ctnr(nlist, (DESTR_FUNC) delete);
			return;
		}
	}

	if (check_ctnr(nlist, (CHECK_FUNC) NIL) < 0)
		printf("\nContainer corrupted!");
	start_timer();
	qsort_ctnr(nlist, (COMPARE_FUNC) compare, NULL);
	printf(" %6.3f", lap_time() / 1000.0);

	if (check_ctnr(nlist, (CHECK_FUNC) NIL) < 0)
		printf("\nContainer corrupted!");
	for (i = 0; i < NUMBERS - 1; i++)
		if (*((int*) get_ctnr(nlist, (INDEX) i)) >
		    *((int*) get_ctnr(nlist, (INDEX) i + 1)))
			printf("\nSort error at %d (%d, %d)", i,
			       *((int*) get_ctnr(nlist, (INDEX) i)),
		           *((int*) get_ctnr(nlist, (INDEX) i + 1)));
	destr_ctnr(nlist, delete);
	printf("\n");
}

static void 
test(CONTAINER_TYPE type)
{
	test_insert(type);
	test_sort(type);
}

/*
============================ public part =============================
*/

int 
main()
{
	printf("     Insert   Sort\n");
	printf("SLL:");
	test(CT_SLL);

	printf("DLL:");
	test(CT_DLL);

	printf("PGA:");
	test(CT_PGA);

	return (0);
}


