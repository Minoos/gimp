/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * colorsel_gtk module (C) 1998 Austin Donnelly <austin@greenend.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include <stdio.h>
#include <gtk/gtk.h>
#include <libgimp/color_selector.h>
#include <libgimp/gimpintl.h>
#include <libgimp/gimpmodule.h>
#include "modregister.h"

/* prototypes */
static GtkWidget * colorsel_gtk_new (int, int, int,
				     GimpColorSelector_Callback, void *,
				     void **);
static void        colorsel_gtk_free (void *);
static void        colorsel_gtk_setcolor (void *, int, int, int, int);
static void        colorsel_gtk_update (GtkWidget *, gpointer);


/* local methods */
static GimpColorSelectorMethods methods = 
{
  colorsel_gtk_new,
  colorsel_gtk_free,
  colorsel_gtk_setcolor
};

static GimpModuleInfo info = {
    NULL,
    N_("GTK colour selector as a pluggable colour selector"),
    "Austin Donnelly <austin@gimp.org>",
    "v0.02",
    "(c) 1999, released under the GPL",
    "17 Jan 1999"
};


/* globaly exported init function */
G_MODULE_EXPORT GimpModuleStatus
module_init (GimpModuleInfo **inforet)
{
  GimpColorSelectorID id;

#ifndef __EMX__
  id = gimp_color_selector_register ("GTK", "gtk.html", &methods);
#else
  id = mod_color_selector_register  ("GTK", "gtk.html", &methods);
#endif   
  if (id)
  {
    info.shutdown_data = id;
    *inforet = &info;
    return GIMP_MODULE_OK;
  }
  else
  {
    return GIMP_MODULE_UNLOAD;
  }
}


G_MODULE_EXPORT void
module_unload (void *shutdown_data,
	       void (*completed_cb)(void *),
	       void *completed_data)
{
#ifndef __EMX__
  gimp_color_selector_unregister (shutdown_data, completed_cb, completed_data);
#else
  mod_color_selector_unregister (shutdown_data, completed_cb, completed_data);
#endif
}



/**************************************************************/
/* GTK colour selector methods */

typedef struct {
  GtkWidget                      *selector;
  GimpColorSelector_Callback      callback;
  void                           *client_data;
} ColorselGtk;


static GtkWidget *
colorsel_gtk_new (int r, int g, int b,
		  GimpColorSelector_Callback callback,  void *client_data,
		  /* RETURNS: */
		  void **selector_data)
{
  GtkWidget   *hbox;
  GtkWidget   *vbox;
  ColorselGtk *p;

  p = g_malloc (sizeof (ColorselGtk));

  p->selector = gtk_color_selection_new ();
  p->callback = callback;
  p->client_data = client_data;

  colorsel_gtk_setcolor (p, r, g, b, FALSE);

  gtk_signal_connect (GTK_OBJECT (p->selector), "color_changed",
		      (GtkSignalFunc) colorsel_gtk_update, p);

  vbox = gtk_vbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), p->selector, FALSE, FALSE, 0);
  gtk_widget_show (p->selector);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  (*selector_data) = p;
  return hbox;
}


static void
colorsel_gtk_free (void *data)
{
  ColorselGtk *p = data;  

  /* don't need to gtk_widget_destroy() the selector, since that's
   * done for us. */

  g_free (p);
}


static void
colorsel_gtk_setcolor (void *data,
		       int r, int g, int b, int set_current)
{
  ColorselGtk *p = data;
  double color[3];

  color[0] = ((gdouble) r) / 255.999;
  color[1] = ((gdouble) g) / 255.999;
  color[2] = ((gdouble) b) / 255.999;

  gtk_color_selection_set_color (GTK_COLOR_SELECTION (p->selector), color);
}



static void
colorsel_gtk_update (GtkWidget *widget, gpointer data)
{
  ColorselGtk *p = data;
  int r;
  int g;
  int b;
  double color[3];

  gtk_color_selection_get_color (GTK_COLOR_SELECTION (p->selector), color);

  r = (int) (color[0] * 255.999);
  g = (int) (color[1] * 255.999);
  b = (int) (color[2] * 255.999);

  p->callback (p->client_data, r, g, b);
}

/* End of colorsel_gtk.c */
