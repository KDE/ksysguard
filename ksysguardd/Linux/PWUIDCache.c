/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

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
#include <time.h>
#include <pwd.h>
#include <sys/types.h>

#include "ccont.h"
#include "PWUIDCache.h"

#define TIMEOUT 300 /* Cached values become invalid after 5 minutes */

typedef struct
{
	uid_t uid;
	char* uName;
	time_t tStamp;
} CachedPWUID;

static CONTAINER UIDCache = 0;
static time_t lastCleanup = 0;

static int 
uidCmp(void* p1, void* p2)
{
	return (((CachedPWUID*) p1)->uid - ((CachedPWUID*) p2)->uid);
}

void
destrCachedPWUID(void* c)
{
	if (c)
	{
		if (((CachedPWUID*) c)->uName)
			free (((CachedPWUID*) c)->uName);
		free (c);
	}
}
			
void
initPWUIDCache()
{
	UIDCache = new_ctnr(CT_DLL);
}

void
exitPWUIDCache()
{
	destr_ctnr(UIDCache, destrCachedPWUID);
}

const char*
getCachedPWUID(uid_t uid)
{
	CachedPWUID key;
	CachedPWUID* entry;
	long index;
	time_t stamp;

	stamp = time(0);
	if (stamp - lastCleanup > TIMEOUT)
	{
		/* Cleanup cache entries every TIMEOUT seconds so that we
		 * don't pile tons of unused entries, and to make sure that
		 * our entries are not outdated. */
		long i;
		for (i = 0; i < level_ctnr(UIDCache); ++i)
		{
			/* If a cache entry has not been updated for TIMEOUT
			 * seconds the entry is removed. */
			entry = get_ctnr(UIDCache, i);
			if (stamp - entry->tStamp > TIMEOUT)
			{
				remove_ctnr(UIDCache, i--);
				destrCachedPWUID(entry);
			}
		}
		lastCleanup = stamp;
	}

	key.uid = uid;
	if ((index = search_ctnr(UIDCache, uidCmp, &key)) < 0)
	{
		struct passwd* pwent;

		/* User id is not yet known */
		entry = malloc(sizeof(CachedPWUID));
		entry->tStamp = stamp;
		entry->uid = uid;

		pwent = getpwuid(uid);
		if (pwent)
		{
			entry->uName = malloc(strlen(pwent->pw_name) + 1);
			strcpy(entry->uName, pwent->pw_name);
		}
		else
		{
			entry->uName = malloc(strlen("?") + 1);
			strcpy(entry->uName, "?");
		}
		push_ctnr(UIDCache, entry);
		bsort_ctnr(UIDCache, uidCmp, 0);
	}
	else
	{
		/* User is is already known */
		entry = get_ctnr(UIDCache, index);
	}

	return (entry->uName);
}
