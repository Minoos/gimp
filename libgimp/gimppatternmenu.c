/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimppatternmenu.c
 * Copyright (C) 1998 Andy Thomas 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <string.h>

#ifdef __GNUC__
#warning GTK_DISABLE_DEPRECATED
#endif
#undef GTK_DISABLE_DEPRECATED

#include "gimp.h"
#include "gimpui.h"

/* Idea is to have a function to call that returns a widget that 
 * completely controls the selection of a pattern.
 * you get a widget returned that you can use in a table say.
 * In:- Initial pattern name. Null means use current selection.
 *      pointer to func to call when pattern changes (GimpRunPatternCallback).
 * Returned:- Pointer to a widget that you can use in UI.
 * 
 * Widget simply made up of a preview widget (20x20) containing the pattern
 * and a button that can be clicked on to change the pattern.
 */


#define PSEL_DATA_KEY       "__psel_data"
#define CELL_SIZE           20
#define PREVIEW_EVENT_MASK  GDK_EXPOSURE_MASK       | \
                            GDK_BUTTON_PRESS_MASK   | \
			    GDK_BUTTON_RELEASE_MASK | \
                            GDK_BUTTON1_MOTION_MASK 

struct __patterns_sel 
{
  gchar                  *dname;
  GimpRunPatternCallback  cback;
  GtkWidget              *pattern_preview;
  GtkWidget              *device_patpopup; 
  GtkWidget              *device_patpreview;
  GtkWidget              *button;
  GtkWidget              *top_hbox;
  gchar                  *pattern_name;      /* Local copy */
  gint                    width;
  gint                    height;
  gint                    bytes;
  gchar                  *mask_data;         /* local copy */
  gchar                  *pattern_popup_pnt; /* Pointer use to control the popup */
  gpointer                data;
};

typedef struct __patterns_sel PSelect;

static void
pattern_popup_open (gint     x,
		    gint     y,
		    PSelect *psel)
{
  gint   x_org;
  gint   y_org;
  gint   scr_w;
  gint   scr_h;
  gchar *src;
  gchar *buf;

  /* make sure the popup exists and is not visible */
  if (psel->device_patpopup == NULL)
    {
      GtkWidget *frame;

      psel->device_patpopup = gtk_window_new (GTK_WINDOW_POPUP);
      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
      gtk_container_add (GTK_CONTAINER (psel->device_patpopup), frame);
      gtk_widget_show (frame);
      psel->device_patpreview = gtk_preview_new (GTK_PREVIEW_COLOR);
      gtk_container_add (GTK_CONTAINER (frame), psel->device_patpreview);
      gtk_widget_show (psel->device_patpreview);
    }
  else
    {
      gtk_widget_hide (psel->device_patpopup);
    }

  /* decide where to put the popup */
  gdk_window_get_origin (psel->pattern_preview->window, &x_org, &y_org);
  scr_w = gdk_screen_width ();
  scr_h = gdk_screen_height ();
  x = x_org + x - (psel->width >> 1);
  y = y_org + y - (psel->height >> 1);
  x = (x < 0) ? 0 : x;
  y = (y < 0) ? 0 : y;
  x = (x + psel->width > scr_w) ? scr_w - psel->width : x;
  y = (y + psel->height > scr_h) ? scr_h - psel->height : y;
  gtk_preview_size (GTK_PREVIEW (psel->device_patpreview), 
		    psel->width, psel->height);

  gtk_window_move (GTK_WINDOW (psel->device_patpopup), x, y);
  gtk_widget_show (psel->device_patpopup);
  
  /*  Draw the pattern  */
  buf = g_new (gchar, psel->width * 3);
  src = psel->mask_data;
  for (y = 0; y < psel->height; y++)
    {
      if (psel->bytes == 1)
	for (x = 0; x < psel->width; x++)
	  {
	    buf[x*3+0] = src[x];
	    buf[x*3+1] = src[x];
	    buf[x*3+2] = src[x];
	  }
      else
	for (x = 0; x < psel->width; x++)
	  {
	    buf[x*3+0] = src[x*3+0];
	    buf[x*3+1] = src[x*3+1];
	    buf[x*3+2] = src[x*3+2];
	  }
      gtk_preview_draw_row (GTK_PREVIEW (psel->device_patpreview), 
			    (guchar *)buf, 0, y, psel->width);
      src += psel->width * psel->bytes;
    }
  g_free (buf);

  gtk_widget_queue_draw (psel->device_patpreview);
}

static void
pattern_popup_close (PSelect *psel)
{
  if (psel->device_patpopup != NULL)
    gtk_widget_hide (psel->device_patpopup);
}

static gint
pattern_preview_events (GtkWidget *widget,
			GdkEvent  *event,
			gpointer   data)
{
  GdkEventButton *bevent;
  PSelect        *psel = (PSelect*)data;

  if (psel->mask_data)
    {
      switch (event->type)
	{
	case GDK_EXPOSE:
	  break;
	  
	case GDK_BUTTON_PRESS:
	  bevent = (GdkEventButton *) event;
	  
	  if (bevent->button == 1)
	    {
	      gtk_grab_add (widget);
	      pattern_popup_open (bevent->x, bevent->y, psel);
	    }
	  break;
	  
	case GDK_BUTTON_RELEASE:
	  bevent = (GdkEventButton *) event;
	  
	  if (bevent->button == 1)
	    {
	      gtk_grab_remove (widget);
	      pattern_popup_close (psel);
	    }
	  break;
	case GDK_DELETE:
	  break;
	  
	default:
	  break;
	}
    }

  return FALSE;
}

static void
pattern_pre_update (GtkWidget *pattern_preview,
		    gint       width,
		    gint       height,
		    gint       bytes,
		    gchar     *mask_data)
{
  gint   x;
  gint   y;
  gchar *src;
  gchar *buf;

  /*  Draw the pattern  */
  buf = g_new (gchar, width * 3);
  src = mask_data;
  for (y = 0; y < CELL_SIZE && y < height; y++)
    {
      if (bytes == 1)
	for (x = 0; x < width && x < CELL_SIZE; x++)
	  {
	    buf[x*3+0] = src[x];
	    buf[x*3+1] = src[x];
	    buf[x*3+2] = src[x];
	  }
      else
	for (x = 0; x < width && x < CELL_SIZE; x++)
	  {
	    buf[x*3+0] = src[x*3+0];
	    buf[x*3+1] = src[x*3+1];
	    buf[x*3+2] = src[x*3+2];
	  }
      gtk_preview_draw_row (GTK_PREVIEW (pattern_preview), 
			    (guchar *)buf, 0, y, (width < CELL_SIZE) ? width : CELL_SIZE);
      src += width * bytes;
    }
  g_free (buf);

  gtk_widget_queue_draw (pattern_preview);
}

static void
pattern_select_invoker (gchar    *name,
			gint      width,
			gint      height,
			gint      bytes,
			gchar    *mask_data,
			gint      closing,
			gpointer  data)
{
  gint     mask_d_sz;
  PSelect *psel = (PSelect*)data;

  if (psel->mask_data != NULL)
    g_free(psel->mask_data);

  psel->width  = width;
  psel->height = height;
  psel->bytes  = bytes;
  mask_d_sz    = width * height * bytes;
  psel->mask_data = g_malloc (mask_d_sz);
  g_memmove (psel->mask_data, mask_data, mask_d_sz); 

  pattern_pre_update (psel->pattern_preview, 
		      psel->width, psel->height, psel->bytes, psel->mask_data);

  if (psel->cback != NULL)
     (psel->cback)(name, width, height, bytes, mask_data, closing, psel->data);

  if(closing)
    {
      gtk_widget_set_sensitive (psel->button, TRUE);
      psel->pattern_popup_pnt = NULL;
    }
}


static void
patterns_select_callback (GtkWidget *widget,
			  gpointer   data)
{
  PSelect *psel = (PSelect*)data;

  gtk_widget_set_sensitive (psel->button, FALSE);
  psel->pattern_popup_pnt = 
    gimp_interactive_selection_pattern ((psel->dname) ? psel->dname : 
					                "Pattern Plugin Selection",
					psel->pattern_name,
					pattern_select_invoker,psel);
}

/**
 * gimp_pattern_select_widget:
 * @dname: Title of the dialog to use.  NULL means to use the default title.
 * @ipattern: Initial pattern name. NULL means to use current selection. 
 * @cback: a function to call when the selected pattern changes.
 * @data: a pointer to arbitary data to be used in the call to @cback.
 *
 * Creates a new #GtkWidget that completely controls the selection of a 
 * pattern.  This widget is suitable for placement in a table in a
 * plug-in dialog.
 *
 * Returns:A #GtkWidget that you can use in your UI.
 */
GtkWidget * 
gimp_pattern_select_widget (gchar                  *dname,
			    gchar                  *ipattern, 
			    GimpRunPatternCallback  cback,
			    gpointer                data)
{
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *pattern;
  GtkWidget *button;
  gint       width;
  gint       height;
  gint       bytes;
  gint       mask_data_size;
  guint8    *mask_data;
  gchar     *pattern_name;
  PSelect   *psel;
  
  psel = g_new (PSelect, 1);

  hbox = gtk_hbox_new (FALSE, 3);
  gtk_widget_show(hbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_widget_show (frame);

  pattern = gtk_preview_new (GTK_PREVIEW_COLOR);
  gtk_preview_size (GTK_PREVIEW (pattern), CELL_SIZE, CELL_SIZE); 
  gtk_widget_show (pattern);
  gtk_container_add (GTK_CONTAINER (frame), pattern); 

  gtk_widget_set_events (pattern, PREVIEW_EVENT_MASK);

  g_signal_connect (pattern, "event",
                    G_CALLBACK (pattern_preview_events),
                    psel);
  
  psel->cback             = cback;
  psel->data              = data;
  psel->mask_data         = NULL;
  psel->device_patpopup   = psel->device_patpreview = NULL;
  psel->pattern_preview   = pattern;
  psel->pattern_name      = ipattern;
  psel->dname             = dname;
  psel->pattern_popup_pnt = NULL;

  /* Do initial pattern setup */
  pattern_name = 
    gimp_patterns_get_pattern_data (ipattern, 
				    &width, 
				    &height, 
				    &bytes, 
				    &mask_data_size, 
				    &mask_data);

  if(pattern_name)
    {
      pattern_pre_update (psel->pattern_preview, 
			  width, height, bytes, mask_data);
      psel->mask_data    = mask_data;
      psel->pattern_name = pattern_name;
      psel->width        = width;
      psel->height       = height;
      psel->bytes        = bytes;
    }

  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("... ");
  gtk_container_add (GTK_CONTAINER (hbox), button); 
  gtk_widget_show (button);

  psel->button = button;
  g_signal_connect (button, "clicked",
                    G_CALLBACK (patterns_select_callback),
                    psel);

  g_object_set_data (G_OBJECT (hbox), PSEL_DATA_KEY, psel);

  return hbox;
}

/**
 * gimp_pattern_select_widget_close_popup:
 * @widget: A pattern select widget.
 *
 * Closes the popup window associated with @widget.
 */
void
gimp_pattern_select_widget_close_popup (GtkWidget *widget)
{
  PSelect  *psel;

  psel = (PSelect *) g_object_get_data (G_OBJECT (widget), PSEL_DATA_KEY);

  if (psel && psel->pattern_popup_pnt)
    {
      gimp_patterns_close_popup (psel->pattern_popup_pnt);
      psel->pattern_popup_pnt = NULL;
    }
}

/**
 * gimp_pattern_select_widget_set_popup:
 * @widget: A pattern select widget.
 * @pname: Pattern name to set. NULL means no change. 
 *
 * Sets the current pattern for the pattern
 * select widget.  Calls the callback function if one was
 * supplied in the call to gimp_pattern_select_widget().
 */
void
gimp_pattern_select_widget_set_popup (GtkWidget *widget,
				      gchar     *pname)
{
  gint      width;
  gint      height;
  gint      bytes;
  gint      mask_data_size;
  guint8   *mask_data;
  gchar    *pattern_name;
  PSelect  *psel;
  
  psel = (PSelect*) g_object_get_data (G_OBJECT (widget), PSEL_DATA_KEY);

  if (psel)
    {
      pattern_name = 
	gimp_patterns_get_pattern_data (pname,
					&width, 
					&height, 
					&bytes, 
					&mask_data_size,
					&mask_data);
  
      pattern_select_invoker (pname, width, height, bytes, mask_data, 0, psel);
      
      if (psel->pattern_popup_pnt)
	gimp_patterns_set_popup (psel->pattern_popup_pnt, pname);
    }
}

