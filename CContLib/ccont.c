/*
    Lightweight C Container Library
   
	Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

	Original code was written by Chris Schlaeger <cs@kde.org>
    
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "ccont.h"

#include <stdio.h>
#include <stdlib.h>

struct container_info {
	INDEX count;
	CONTAINER currentNode;
};

typedef struct container_info T_CONTAINER_INFO;
typedef struct container_info* CONTAINER_INFO;

#define rpterr(x) fprintf(stderr, "%s\n", x)

CONTAINER new_ctnr(void)
{
	CONTAINER_INFO info;
	CONTAINER rootNode;

	rootNode = (CONTAINER)malloc(sizeof(T_CONTAINER));

	info = (CONTAINER_INFO)malloc(sizeof(T_CONTAINER_INFO));
	info->count = 0;
	info->currentNode = rootNode;

	rootNode->next = rootNode;
	rootNode->prev = rootNode;
	rootNode->data = info;

	return rootNode;
}

void zero_destr_ctnr(CONTAINER rootNode, DESTR_FUNC destr_func)
{
	INDEX counter;

	if (rootNode == NIL || destr_func == NIL) {
		rpterr("destr_ctnr: NIL argument");
		return;
	}

	for (counter = level_ctnr(rootNode); counter > -1; --counter)
		destr_func(pop_ctnr(rootNode));

	if (rootNode->data)
		free(rootNode->data);

	free(rootNode);
	rootNode = 0;

	return;
}

INDEX level_ctnr(CONTAINER rootNode)
{
	if (rootNode == NIL) {
		rpterr("level_ctnr: NIL argument");
		return -1;
	}

	return ((CONTAINER_INFO)rootNode->data)->count;
}

void insert_ctnr(CONTAINER rootNode, void* object, INDEX pos)
{
	CONTAINER it;
	INDEX counter = 0;
	
	if (rootNode == NIL || object == NIL) {
		rpterr("insert_ctnr: NIL argument");
		return;
	}

	for (it = rootNode->next; it != rootNode; it = it->next) {
		if (counter == pos) {
			CONTAINER newNode = (CONTAINER)malloc(sizeof(T_CONTAINER));

			newNode->prev = it;
			newNode->next = it->next;
			it->next->prev = newNode;
			it->next = newNode;

			newNode->data = object;
			((CONTAINER_INFO)rootNode->data)->count++;
			return;
		}

		counter++;
	}
}

void push_ctnr(CONTAINER rootNode, void* object)
{
	CONTAINER newNode;

	if (rootNode == NIL || object == NIL) {
		rpterr("push_ctnr: NIL argument");
		return;
	}

	newNode = (CONTAINER)malloc(sizeof(T_CONTAINER));
	newNode->next = rootNode;
	newNode->prev = rootNode->prev;
	rootNode->prev->next = newNode;
	rootNode->prev = newNode;

	newNode->data = object;
	((CONTAINER_INFO)rootNode->data)->count++;
}

void* remove_at_ctnr(CONTAINER rootNode, INDEX pos)
{
	CONTAINER it;
	INDEX counter = 0;
	void* retval;
	
	if (rootNode == NIL) {
		rpterr("remove_ctnr: NIL argument");
		return NIL;
	}

	for (it = rootNode->next; it != rootNode; it = it->next) {
		if (counter == pos) {
			retval = it->data;

			it->prev->next = it->next;
			it->next->prev = it->prev;

			free(it);

			((CONTAINER_INFO)rootNode->data)->count--;
			return retval;
		}

		counter++;
	}

	return NIL;
}

void* pop_ctnr(CONTAINER rootNode)
{
	CONTAINER ptr;
	void* retval;

	if (rootNode == NIL) {
		rpterr("pop_ctnr: NIL argument");
		return NIL;
	}

	if (rootNode->next == rootNode)
		return NIL;

	ptr = rootNode->next;	
	retval = ptr->data;

	rootNode->next = ptr->next;
	ptr->next->prev = rootNode;

	((CONTAINER_INFO)rootNode->data)->count--;

	free(ptr);

	return retval;
}

void* get_ctnr(CONTAINER rootNode, INDEX pos)
{
	CONTAINER it;
	INDEX counter = 0;

	if (rootNode == NIL) {
		rpterr("get_ctnr: NIL argument");
		return NIL;
	}

	for (it = rootNode->next; it != rootNode; it = it->next) {
		if (counter == pos)
			return it->data;

		counter++;
	}

	return NIL;
}

INDEX search_ctnr(CONTAINER rootNode, COMPARE_FUNC compare_func, void* pattern)
{
	CONTAINER it;
	INDEX counter = 0;

	if (rootNode == NIL || compare_func == NIL || pattern == NIL) {
		rpterr("search_ctnr: NIL argument");
		return -1;
	}
	
	for (it = rootNode->next; it != rootNode; it = it->next) {
		if (compare_func(pattern, it->data) == 0)
			return counter;

		counter++;
	}

	return -1;
}

void swap_ctnr(CONTAINER rootNode, INDEX pos1, INDEX pos2)
{
	CONTAINER it, node1, node2;
	INDEX counter = 0;
	int found = 0;
	void* tmpData;

	if (rootNode == NIL) {
		rpterr("swap_ctnr: NIL argument");
		return;
	}

	if (pos1 == pos2)
		return;

	for (it = rootNode->next; it != rootNode; it = it->next) {
		if (counter == pos1) {
			node1 = it;
			found++;
			if (found == 2)
				break;
		}
		if (counter == pos2) {
			node2 = it;
			found++;
			if (found == 2)
				break;
		}

		counter++;
	}	

	if (found == 2) {
		tmpData = node1->data;
		node1->data = node2->data;
		node2->data = tmpData;
	}
}

void bsort_ctnr(CONTAINER rootNode, COMPARE_FUNC compare_func)
{
	/* Use mergesort adapted from http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.c */
	
	CONTAINER p, q, e, tail, oldhead;
	INDEX insize, nmerges, psize, qsize, i;

	if (rootNode == NIL || compare_func == NIL) {
		rpterr("bsort_ctnr: NIL argument");
		return;
	}

	if (level_ctnr(rootNode) == 0) {
		/* Nothing to sort */
		return;
	}

	insize = 1;

	while (1) {
		p = rootNode->next;
		oldhead = rootNode;	  /* only used for circular linkage */
		rootNode->next = NULL;
		tail = NULL;

		nmerges = 0;  /* count number of merges we do in this pass */

		while (p) {
			nmerges++;  /* there exists a merge to be done */
			/* step `insize' places along from p */
			q = p;
			psize = 0;
			for (i = 0; i < insize; i++) {
				psize++;
				q = (q->next == oldhead ? NULL : q->next);
				if (!q) break;
			}

			/* if q hasn't fallen off end, we have two lists to merge */
			qsize = insize;

			/* now we have two lists; merge them */
			while (psize > 0 || (qsize > 0 && q)) {
				/* decide whether next element of merge comes from p or q */
				if (psize == 0) {
					/* p is empty; e must come from q. */
					e = q; q = q->next; qsize--;
					if (q == oldhead) q = NULL;
				} else if (qsize == 0 || !q) {
					/* q is empty; e must come from p. */
					e = p; p = p->next; psize--;
					if (p == oldhead) p = NULL;
				} else if (compare_func(p,q) <= 0) {
					/* First element of p is lower (or same);
					 * e must come from p. */
					e = p; p = p->next; psize--;
					if (p == oldhead) p = NULL;
				} else {
					/* First element of q is lower; e must come from q. */
					e = q; q = q->next; qsize--;
					if (q == oldhead) q = NULL;
				}

				/* add the next element to the merged list */
				if (tail) {
					tail->next = e;
					e->prev = tail;
				} else {
					rootNode->next = e;
					e->prev = rootNode;
				}

				tail = e;
			}

			/* now p has stepped `insize' places along, and q has too */
			p = q;
		}
		
		tail->next = rootNode;
		rootNode->prev = tail;

		/* If we have done only one merge, we're finished. */
		if (nmerges <= 1) {  /* allow for nmerges==0, the empty list case */
			break;
		}

		/* Otherwise repeat, merging lists twice the size */
		insize *= 2;
	}
}

void* first_ctnr(CONTAINER rootNode)
{
	if (rootNode == NIL) {
		rpterr("first_ctnr: NIL argument");
		return NIL;
	}
	
	if (rootNode->next == rootNode)
		return NIL;

	((CONTAINER_INFO)rootNode->data)->currentNode = rootNode->next;

	return rootNode->next->data;
}

void* next_ctnr(CONTAINER rootNode)
{
	CONTAINER_INFO info;

	if (rootNode == NIL) {
		rpterr("next_ctnr: NIL argument");
		return NIL;
	}

	info = (CONTAINER_INFO)rootNode->data;

	if (info->currentNode->next == rootNode)
		return NIL;

	info->currentNode = info->currentNode->next;

	return info->currentNode->data;
}

void* remove_ctnr(CONTAINER rootNode)
{
	CONTAINER currentNode, tmp;
	CONTAINER_INFO info;
	void* retval;
	
	if (rootNode == NIL) {
		rpterr("remove_curr_ctnr: NIL argument");
		return NIL;
	}

	info = (CONTAINER_INFO)rootNode->data;
	currentNode = info->currentNode;

	if (currentNode == rootNode) { /* should never happen */
		rpterr("remove_curr_ctnr: delete root node");
		return NIL;
	}

	retval = currentNode->data;
	tmp = currentNode->prev;

	currentNode->prev->next = currentNode->next;
	currentNode->next->prev = currentNode->prev;

	free(currentNode);

	info->count--;
	info->currentNode = tmp;

	return retval;
}
