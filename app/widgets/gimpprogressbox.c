/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpprogressbox.c
 * Copyright (C) 2004 Michael Natterer <mitch@gimp.org>
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

#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "widgets-types.h"

#include "core/gimpprogress.h"

#include "gimpprogressbox.h"

#include "gimp-intl.h"


static void     gimp_progress_box_progress_iface_init (GimpProgressInterface *progress_iface);

static GimpProgress *
                gimp_progress_box_progress_start     (GimpProgress *progress,
                                                      const gchar  *message,
                                                      gboolean      cancelable);
static void     gimp_progress_box_progress_end       (GimpProgress *progress);
static gboolean gimp_progress_box_progress_is_active (GimpProgress *progress);
static void     gimp_progress_box_progress_set_text  (GimpProgress *progress,
                                                      const gchar  *message);
static void     gimp_progress_box_progress_set_value (GimpProgress *progress,
                                                      gdouble       percentage);
static gdouble  gimp_progress_box_progress_get_value (GimpProgress *progress);
static void     gimp_progress_box_progress_pulse     (GimpProgress *progress);



GType
gimp_progress_box_get_type (void)
{
  static GType box_type = 0;

  if (! box_type)
    {
      static const GTypeInfo box_info =
      {
        sizeof (GimpProgressBoxClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        NULL,           /* class_init     */
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (GimpProgressBox),
        0,              /* n_preallocs    */
        NULL            /* instance_init  */
      };

      static const GInterfaceInfo progress_iface_info =
      {
        (GInterfaceInitFunc) gimp_progress_box_progress_iface_init,
        NULL,           /* iface_finalize */
        NULL            /* iface_data     */
      };

      box_type = g_type_register_static (GTK_TYPE_VBOX,
                                         "GimpProgressBox",
                                         &box_info, 0);

      g_type_add_interface_static (box_type, GIMP_TYPE_PROGRESS,
                                   &progress_iface_info);
    }

  return box_type;
}

static void
gimp_progress_box_progress_iface_init (GimpProgressInterface *progress_iface)
{
  progress_iface->start     = gimp_progress_box_progress_start;
  progress_iface->end       = gimp_progress_box_progress_end;
  progress_iface->is_active = gimp_progress_box_progress_is_active;
  progress_iface->set_text  = gimp_progress_box_progress_set_text;
  progress_iface->set_value = gimp_progress_box_progress_set_value;
  progress_iface->get_value = gimp_progress_box_progress_get_value;
  progress_iface->pulse     = gimp_progress_box_progress_pulse;
}

static GimpProgress *
gimp_progress_box_progress_start (GimpProgress *progress,
                                  const gchar  *message,
                                  gboolean      cancelable)
{
  GimpProgressBox *box = GIMP_PROGRESS_BOX (progress);

  if (! box->active)
    {
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_label_set_text (GTK_LABEL (box->label), message);
      gtk_progress_bar_set_fraction (bar, 0.0);

      box->active     = TRUE;
      box->cancelable = cancelable;

      return progress;
    }

  return NULL;
}

static void
gimp_progress_box_progress_end (GimpProgress *progress)
{
  GimpProgressBox *box = GIMP_PROGRESS_BOX (progress);

  if (box->active)
    {
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_label_set_text (GTK_LABEL (box->label), "");
      gtk_progress_bar_set_fraction (bar, 0.0);

      box->active     = FALSE;
      box->cancelable = FALSE;
    }
}

static gboolean
gimp_progress_box_progress_is_active (GimpProgress *progress)
{
  GimpProgressBox *box = GIMP_PROGRESS_BOX (progress);

  return box->active;
}

static void
gimp_progress_box_progress_set_text (GimpProgress *progress,
                                     const gchar  *message)
{
  GimpProgressBox *box = GIMP_PROGRESS_BOX (progress);

  if (box->active)
    {
      gtk_label_set_text (GTK_LABEL (box->label), message);
    }
}

static void
gimp_progress_box_progress_set_value (GimpProgress *progress,
                                      gdouble       percentage)
{
  GimpProgressBox *box = GIMP_PROGRESS_BOX (progress);

  if (box->active)
    {
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_progress_bar_set_fraction (bar, percentage);

      if (GTK_WIDGET_DRAWABLE (box->progress))
        gdk_window_process_updates (box->progress->window, TRUE);
    }
}

static gdouble
gimp_progress_box_progress_get_value (GimpProgress *progress)
{
  GimpProgressBox *box = GIMP_PROGRESS_BOX (progress);

  if (box->active)
    {
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      return gtk_progress_bar_get_fraction (bar);
    }

  return 0.0;
}

static void
gimp_progress_box_progress_pulse (GimpProgress *progress)
{
  GimpProgressBox *box = GIMP_PROGRESS_BOX (progress);

  if (box->active)
    {
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_progress_bar_pulse (bar);

      if (GTK_WIDGET_DRAWABLE (box->progress))
        gdk_window_process_updates (box->progress->window, TRUE);
    }
}

GtkWidget *
gimp_progress_box_new (void)
{
  GimpProgressBox *box;

  box = g_object_new (GIMP_TYPE_PROGRESS_BOX,
                      "spacing", 12,
                      NULL);

  box->label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (box->label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (box), box->label, FALSE, FALSE, 0);
  gtk_widget_show (box->label);

  box->progress = gtk_progress_bar_new ();
  gtk_widget_set_size_request (box->progress, 250, 20);
  gtk_box_pack_start (GTK_BOX (box), box->progress, FALSE, FALSE, 0);
  gtk_widget_show (box->progress);

  return GTK_WIDGET (box);
}
