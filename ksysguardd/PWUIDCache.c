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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "ccont.h"

#include "PWUIDCache.h"

/* Cached values become invalid after 5 minutes */
#define TIMEOUT 300

typedef struct {
  uid_t uid;
  char* uName;
  time_t tStamp;
} CachedPWUID;

static CONTAINER UIDCache = 0;
static time_t lastCleanup = 0;

void PWUIDCache_cleanup( void* c );

static int uidCmp( void* p1, void* p2 )
{
  return ( ((CachedPWUID*)p1)->uid - ((CachedPWUID*)p2)->uid );
}

void PWUIDCache_cleanup( void* c )
{
  if ( c ) {
    if ( ((CachedPWUID*)c)->uName )
      free ( ((CachedPWUID*)c)->uName );
    free ( c );
	}
}
			
void initPWUIDCache()
{
  UIDCache = new_ctnr();
}

void exitPWUIDCache()
{
  destr_ctnr( UIDCache, PWUIDCache_cleanup );
}

const char* getCachedPWUID( uid_t uid )
{
  CachedPWUID key;
  CachedPWUID* entry = 0;
  long idx;
  time_t stamp;

  stamp = time( 0 );
  if ( stamp - lastCleanup > TIMEOUT ) {
    /* Cleanup cache entries every TIMEOUT seconds so that we
     * don't pile tons of unused entries, and to make sure that
     * our entries are not outdated. */
    for ( entry = first_ctnr( UIDCache ); entry; entry = next_ctnr( UIDCache ) ) {
      /* If a cache entry has not been updated for TIMEOUT
       * seconds the entry is removed. */
      if ( stamp - entry->tStamp > TIMEOUT )
        PWUIDCache_cleanup( remove_ctnr( UIDCache ) );
    }

    lastCleanup = stamp;
  }

  key.uid = uid;
  if ( ( idx = search_ctnr( UIDCache, uidCmp, &key ) ) < 0 ) {
    struct passwd* pwent;

    /* User id is not yet known */
    entry = (CachedPWUID*)malloc( sizeof( CachedPWUID ) );
    entry->tStamp = stamp;
    entry->uid = uid;

    pwent = getpwuid( uid );
    if ( pwent )
      entry->uName = strdup( pwent->pw_name );
    else
      entry->uName = strdup( "?" );

    push_ctnr( UIDCache, entry );
    bsort_ctnr( UIDCache, uidCmp );
	} else {
    /* User is is already known */
    entry = get_ctnr( UIDCache, idx );
  }

  return entry->uName;
}
