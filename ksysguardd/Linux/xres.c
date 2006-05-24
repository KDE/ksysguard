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

/*This is defined by configure checks. If not found, we'll just #ifdef out the whole file */

#include "config-ksysguardd.h" /*For HAVE_XRES*/
#ifdef HAVE_XRES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XRes.h>


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

static const char *AtomNames[] =
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

void printXres(FILE *CurrentClient);
int setup_xres();

XResTopApp *app = NULL;
FILE *sCurrentClient = NULL;

/* X Error trapping */

static int trapped_error_code = 0;
static int (*old_error_handler) (Display *d, XErrorEvent *e);

static int error_handler(Display     *display,
	      XErrorEvent *error)
{
   (void)display;
   trapped_error_code = error->error_code;
   return 0;
}

static void trap_errors(void)
{
   trapped_error_code = 0;
   old_error_handler = XSetErrorHandler(error_handler);
}

static int untrap_errors(void)
{
   XSetErrorHandler(old_error_handler);
   return trapped_error_code;
}


/* Misc util funcs */

static pid_t window_get_pid(Window win)
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

static unsigned char* window_get_utf8_name(Window win)
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

static Bool check_win_for_info(XResTopClient *client, Window win)
{
  XTextProperty  text_prop;
  unsigned char *identifier;
 
  {
      trap_errors(); 
      identifier = window_get_utf8_name(win);
      if (identifier == NULL)
	{
	  if (XGetWMName(app->dpy, win, &text_prop))
	    {
	      strncpy((char*)client->identifier, (char *) text_prop.value, MAX_SIZE_IDENTIFIER);
	      XFree((char *) text_prop.value);
	    }
	  else
	    {
	      XFetchName(app->dpy, win, (char **)(&client->identifier));
#if 0 /* impossible to reach */
	      if(identifier) {
	        strncpy((char*)client->identifier, (char*)identifier, MAX_SIZE_IDENTIFIER);
		XFree(identifier);
		identifier = NULL;
	      }
#endif
	    }
	}
      else {
        strncpy((char*)client->identifier, (char*)identifier, MAX_SIZE_IDENTIFIER);
        XFree(identifier);
	identifier = NULL;
      }

		      
      if (untrap_errors())
	{
	  app->n_xerrors++;
	  return False;
	}
    }

  if (client->identifier[0] != 0)
    return True;

  return False;
}
static void xrestop_client_get_stats(XResTopClient *client)
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
      unsigned int this_type = types[j].resource_type;
      
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


static Bool recurse_win_tree(XResTopClient *client, Window win_top)
{
  Window       *children, dummy;
  unsigned int  nchildren;
  unsigned int  i;
  pid_t pid = -1;
  Status        qtres;
  client->resource_base = win_top;
  client->resource_mask = 0;
  client->identifier[0] = 0;
  client->pixmap_bytes = 0;
  client->other_bytes = 0;
  client->n_pixmaps = 0;
  client->n_windows = 0; 
  client->n_gcs = 0;
  client->n_pictures = 0;
  client->n_glyphsets = 0; 
  client->n_fonts = 0;
  client->n_colormaps = 0;
  client->n_passive_grabs = 0;
  client->n_cursors = 0;
  client->n_other = 0;

  if (check_win_for_info(client, win_top)) {
    pid = window_get_pid(win_top);

    if(pid != -1) {
      /*If we have seen this pid before don't bother recalculating.  It won't catch all cases, but will catch most for minimal effort */
      if(pid == client->pid)
	      return True;
      client->pid = pid;
      xrestop_client_get_stats(client);
      /*"xPid\tXIdentifier\tXPxmMem\tXNumPxm\tXMemOther\n"*/
      fprintf(sCurrentClient, "%d\t%s\t%ld\t%d\t%ld\n", pid, client->identifier, client->pixmap_bytes, client->n_pixmaps, client->other_bytes);
    }
  }
  
  trap_errors();

  qtres = XQueryTree(app->dpy, win_top, &dummy, &dummy, &children, &nchildren);

  if (untrap_errors())
    {
      app->n_xerrors++;
      return False;
    }

  if (!qtres) return False;

  for (i=0; i<nchildren; i++) 
  {
    if(!recurse_win_tree(client, children[i]))
      return False;	      /*some error*/
  }

  if (children) XFree ((char *)children);

  return True;
}

void printXres(FILE *CurrentClient)
{
  trap_errors();
  sCurrentClient = CurrentClient;

  XResTopClient client;
  client.pid = -1;
  recurse_win_tree(&client, app->win_root);
  return;
}

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

#endif /*For ifndef HAVE_XRES */
