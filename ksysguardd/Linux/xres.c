/*
 *  XResTop - A 'top' like tool for monitoring X Client server resource
 *            usage.
 *
 *  Copyright 2003 Matthew Allum
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XRes.h>

#define DEBUG 1

#ifdef __GNUC__
#ifdef DEBUG
#define DBG(txt, args... ) fprintf(stderr, txt , ##args )
#else
#define DBG(txt, args... ) /* nothing */
#endif
#endif


enum {
  ATOM_PIXMAP = 0,
  ATOM_WINDOW,
  ATOM_GC,
  ATOM_FONT,
  ATOM_GLYPHSET,
  ATOM_PICTURE,
  ATOM_COLORMAP_ENTRY,
  ATOM_PASSIVE_GRAB,
  ATOM_CURSOR,
  ATOM_NET_CLIENT_LIST,
  ATOM_NET_WM_PID,
  ATOM_NET_WM_NAME,
  ATOM_UTF8_STRING,
  ATOM_COUNT
};

static char *AtomNames[] =
  {
    "PIXMAP",
    "WINDOW",
    "GC",
    "FONT",
    "GLYPHSET",
    "PICTURE",
    "COLORMAP ENTRY",
    "PASSIVE GRAB",
    "CURSOR",
    "_NET_CLIENT_LIST",
    "_NET_WM_PID",
    "_NET_WM_NAME",
    "UTF8_STRING"
  };


#define MAX_SIZE_IDENTIFIER 256
typedef struct XResTopClient
{
  XID            resource_base, resource_mask;
  pid_t          pid;
  unsigned char identifier[MAX_SIZE_IDENTIFIER+1];
  unsigned long  pixmap_bytes;
  unsigned long  other_bytes;

  int            n_pixmaps;
  int            n_windows; 
  int            n_gcs;
  int            n_pictures;
  int            n_glyphsets; 
  int            n_fonts;
  int            n_colormaps;
  int            n_passive_grabs;
  int            n_cursors;
  int            n_other;

} XResTopClient;

typedef struct XResTopApp 
{
  Display    *dpy;
  char       *dpy_name;
  int         screen;
  Window      win_root, win_dummy;
  Atom        atoms[ATOM_COUNT];
  int         n_clients;
  int         n_xerrors;

} XResTopApp;


XResTopApp *app = NULL;

/* X Error trapping */

static int trapped_error_code = 0;
static int (*old_error_handler) (Display *d, XErrorEvent *e);

static int
error_handler(Display     *display,
	      XErrorEvent *error)
{
   trapped_error_code = error->error_code;
   return 0;
}

static void
trap_errors(void)
{
   trapped_error_code = 0;
   old_error_handler = XSetErrorHandler(error_handler);
}

static int
untrap_errors(void)
{
   XSetErrorHandler(old_error_handler);
   return trapped_error_code;
}


/* Misc util funcs */

pid_t
window_get_pid(Window win)
{
  Atom  type;
  unsigned long  bytes_after, n_items;
  long *data = NULL;
  pid_t result = -1;
  int   format;

  if (XGetWindowProperty (app->dpy, win, 
			  app->atoms[ATOM_NET_WM_PID],
			  0, 2L,
			  False, XA_CARDINAL,
			  &type, &format, &n_items,
			  &bytes_after, (unsigned char **)&data) == Success
      && n_items && data != NULL)
    {
      result = *data;
    }

  if (data) XFree(data);

  return result;
}

unsigned char*
window_get_utf8_name(Window win)
{
  Atom type;
  int format;
  unsigned long  bytes_after, n_items;
  unsigned char *str = NULL;
  int result;

  result =  XGetWindowProperty (app->dpy, win, app->atoms[ATOM_NET_WM_NAME],
				0, 1024L,
				False, app->atoms[ATOM_UTF8_STRING],
				&type, &format, &n_items,
				&bytes_after, (unsigned char **)&str);

  if (result != Success || str == NULL)
    {
      if (str) XFree (str);
      return NULL;
    }

  if (type != app->atoms[ATOM_UTF8_STRING] || format != 8 || n_items == 0)
    {
      XFree (str);
      return NULL;
    }

  /* XXX should probably utf8_validate this  */

  return str;
}



void
xrestop_client_free(XResTopClient *client)
{
  free(client);
}

static Bool
check_win_for_info(XResTopClient *client, Window win)
{
  XTextProperty  text_prop;
  XID            match_xid ;
  char *identifier;

  /* 
   *  Figure out if a window belongs in a XResClients resource range,
   *  and if it does try and get a name for it.
   *
   *  XXX Should also check for CLASS and TRANSIENT props so we 
   *      get the name for top level window. 
   */

  match_xid = (client->resource_base & ~client->resource_mask);

  if ( (win & ~client->resource_mask) == match_xid )
    {
      trap_errors(); 
      identifier = window_get_utf8_name(win);
      if (identifier == NULL)
	{
	  if (XGetWMName(app->dpy, win, &text_prop))
	    {
	      strncpy(client->identifier, (char *) text_prop.value, MAX_SIZE_IDENTIFIER);
	      XFree((char *) text_prop.value);
	    }
	  else
	    {
	      XFetchName(app->dpy, win, &client->identifier);
	      if(identifier) {
	        strncpy(client->identifier, identifier, MAX_SIZE_IDENTIFIER);
		XFree(identifier);
		identifier = NULL;
	      }
	    }
	}
      else {
        strncpy(client->identifier, identifier, MAX_SIZE_IDENTIFIER);
        XFree(identifier);
	identifier = NULL;
      }

		      
      if (untrap_errors())
	{
	  app->n_xerrors++;
	  return False;
	}
    }

  if (client->identifier[0] != NULL)
    return True;

  return False;
}

static XID
recurse_win_tree(XResTopClient *client, Window win_top)
{
  Window       *children, dummy;
  unsigned int  nchildren;
  int           i;
  XID           w = 0;
  Status        qtres;
  
  if (check_win_for_info(client, win_top))
    return win_top;
  
  trap_errors();

  qtres = XQueryTree(app->dpy, win_top, &dummy, &dummy, &children, &nchildren);

  if (untrap_errors())
    {
      app->n_xerrors++;
      return 0;
    }

  if (!qtres) return 0;

  for (i=0; i<nchildren; i++) 
    {
      w = recurse_win_tree(client, children[i]);
      if(w != 0)
	  break;
    }

  if (children) XFree ((char *)children);

  return w;
}

void
xrestop_client_get_stats(XResTopClient *client)
{
  int               j = 0;
  XResType         *types = NULL;
  int               n_types;

  trap_errors();
  
  XResQueryClientResources (app->dpy, client->resource_base, &n_types, &types);
  
  XResQueryClientPixmapBytes (app->dpy, client->resource_base, 
			      &client->pixmap_bytes);
  
  if (untrap_errors())
    {
      app->n_xerrors++;
      goto cleanup;
    }
  
  for (j=0; j < n_types; j++)
    {
      int this_type = types[j].resource_type;
      
      if (this_type == app->atoms[ATOM_PIXMAP])
	client->n_pixmaps += types[j].count;
      else if (this_type == app->atoms[ATOM_WINDOW])
	client->n_windows += types[j].count;
      else if (this_type == app->atoms[ATOM_GC])
	client->n_gcs += types[j].count;
      else if (this_type == app->atoms[ATOM_FONT])
	client->n_fonts += types[j].count;
      else if (this_type == app->atoms[ATOM_GLYPHSET])
	client->n_glyphsets += types[j].count;
      else if (this_type == app->atoms[ATOM_PICTURE])
	client->n_pictures  += types[j].count;
      else if (this_type == app->atoms[ATOM_COLORMAP_ENTRY])
	client->n_colormaps += types[j].count;
      else if (this_type == app->atoms[ATOM_PASSIVE_GRAB])
	client->n_passive_grabs += types[j].count;
      else if (this_type == app->atoms[ATOM_CURSOR])
	client->n_cursors += types[j].count;
      else client->n_other += types[j].count;
    }

  /* All approx currently - same as gnome system monitor */
   client->other_bytes += client->n_windows * 24;
   client->other_bytes += client->n_gcs * 24;
   client->other_bytes += client->n_pictures * 24;
   client->other_bytes += client->n_glyphsets * 24;
   client->other_bytes += client->n_fonts * 1024;
   client->other_bytes += client->n_colormaps * 24;
   client->other_bytes += client->n_passive_grabs * 24;
   client->other_bytes += client->n_cursors * 24;
   client->other_bytes += client->n_other * 24;
  
 cleanup:
   if (types) XFree(types);

   return;
}

void
printXres(FILE *CurrentClient)
{
  int         i;
  XResClient *clients;

  trap_errors();

  XResQueryClients(app->dpy, &app->n_clients, &clients); 

  if (untrap_errors())
    {
      app->n_xerrors++;
      goto cleanup;
    }

  Window window = None;
  pid_t pid;
  XResTopClient client;
  for(i = 0; i < app->n_clients; i++) 
  {
    if ( (clients[i].resource_base & ~clients[i].resource_mask)
        != (app->win_dummy & ~clients[i].resource_mask) )  /*Ignore ourselves*/
    {
      client.resource_base = clients[i].resource_base;
      client.resource_mask = clients[i].resource_mask;
      client.pixmap_bytes = 0;
      client.n_pixmaps = 0;
      
      client.identifier[0] = 0;
      window = recurse_win_tree(&client, app->win_root);
      if (window && client.identifier[0] != 0) {
        pid = window_get_pid(window);
        if(pid != -1) {
          xrestop_client_get_stats(&client); 
	  /*"xPid\tXIdentifier\tXPxmMem\tXNumPxm\n"*/
	  fprintf(CurrentClient, "%d\t%s\t%ld\t%d\n", pid, client.identifier, client.pixmap_bytes, client.n_pixmaps);
        }
      }
    }
  }

 cleanup:

  if (clients) XFree(clients);
}

#if 0
void
xrestop_populate_client_data()
{
  int         i;
  XResClient *clients;

  for (i=0; i < app->n_clients; i++)
    xrestop_client_free(app->clients[i]);

  trap_errors();

  XResQueryClients(app->dpy, &app->n_clients, &clients); 

  if (untrap_errors())
    {
      app->n_xerrors++;
      goto cleanup;
    }

  for(i = 0; i < app->n_clients; i++) 
    {
      app->clients[i] = xrestop_client_new(app);

      app->clients[i]->resource_base = clients[i].resource_base;
      app->clients[i]->resource_mask = clients[i].resource_mask;

      xrestop_client_get_info(app, app->clients[i]); 
      if(app->clients[i]->pid != -1) {
        xrestop_client_get_stats(app, app->clients[i]); 
      }
    }

 cleanup:

  if (clients) XFree(clients);
}

#endif 

int setup_xres()
{
  int      event, error, major, minor;

  app = malloc(sizeof(XResTopApp));
  memset(app, 0, sizeof(XResTopApp));


  if ((app->dpy = XOpenDisplay(app->dpy_name)) == NULL)
  {
    fprintf(stderr, "Unable to open display!\n");
    return 0;
  }

  app->screen = DefaultScreen(app->dpy);
  app->win_root = RootWindow(app->dpy, app->screen); 
    
  XInternAtoms (app->dpy, AtomNames, ATOM_COUNT,False, app->atoms);

  if(!XResQueryExtension(app->dpy, &event, &error)) {
    fprintf(stderr, "XResQueryExtension failed. Display Missing XRes extension ?\n");
    return 0;
  }

  if(!XResQueryVersion(app->dpy, &major, &minor)) {
    fprintf(stderr, "XResQueryVersion failed, cannot continue.\n");
    return 0;
  }

  app->n_clients = 0;

  /* Create our own never mapped window so we can figure out this connection */
  app->win_dummy = XCreateSimpleWindow(app->dpy, app->win_root, 
				       0, 0, 16, 16, 0, None, None); 
  return 1;
}

