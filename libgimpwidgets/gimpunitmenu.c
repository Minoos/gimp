/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1999 Peter Mattis and Spencer Kimball
 *
 * gimpunitmenu.c
 * Copyright (C) 1999 Michael Natterer <mitch@gimp.org>
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

#include <gtk/gtk.h>

#include "libgimpbase/gimpbase.h"

#include "gimpwidgetstypes.h"

#include "gimpdialog.h"
#include "gimphelpui.h"
#include "gimpunitmenu.h"
#include "gimpwidgets.h"

#include "libgimp/libgimp-intl.h"


enum
{
  UNIT_CHANGED,
  LAST_SIGNAL
};

enum
{
  UNIT_COLUMN,
  FACTOR_COLUMN,
  DATA_COLUMN,
  NUM_COLUMNS
};


static void          gimp_unit_menu_class_init   (GimpUnitMenuClass *klass);
static void          gimp_unit_menu_init         (GimpUnitMenu      *menu);

static void          gimp_unit_menu_finalize     (GObject           *object);

static const gchar * gimp_unit_menu_build_string (const gchar       *format,
						  GimpUnit           unit);
static void          gimp_unit_menu_callback     (GtkWidget         *widget,
						  gpointer           data);


static guint gimp_unit_menu_signals[LAST_SIGNAL] = { 0 };

static GtkOptionMenuClass *parent_class = NULL;


GType
gimp_unit_menu_get_type (void)
{
  static GType menu_type = 0;

  if (! menu_type)
    {
      static const GTypeInfo menu_info =
      {
        sizeof (GimpUnitMenuClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_unit_menu_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (GimpUnitMenu),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_unit_menu_init,
      };

      menu_type = g_type_register_static (GTK_TYPE_OPTION_MENU,
                                         "GimpUnitMenu",
                                         &menu_info, 0);
    }

  return menu_type;
}

static void
gimp_unit_menu_class_init (GimpUnitMenuClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  /**
   * GimpUnitMenu::unit-changed:
   *
   * This signal is emitted whenever the user selects a #GimpUnit from
   * the #GimpUnitMenu.
   **/
  gimp_unit_menu_signals[UNIT_CHANGED] =
    g_signal_new ("unit_changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GimpUnitMenuClass, unit_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  object_class->finalize = gimp_unit_menu_finalize;

  klass->unit_changed    = NULL;
}

static void
gimp_unit_menu_init (GimpUnitMenu *menu)
{
  menu->format       = NULL;
  menu->unit         = GIMP_UNIT_PIXEL;
  menu->show_pixels  = FALSE;
  menu->show_percent = FALSE;
  menu->selection    = NULL;
  menu->tv           = NULL;
}

static void
gimp_unit_menu_finalize (GObject *object)
{
  GimpUnitMenu *menu;

  g_return_if_fail (GIMP_IS_UNIT_MENU (object));

  menu = GIMP_UNIT_MENU (object);

  if (menu->format)
    {
      g_free (menu->format);
      menu->format = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * gimp_unit_menu_new:
 * @format:       A printf-like format string which is used to create the unit
 *                strings.
 * @unit:         The initially selected unit.
 * @show_pixels:  %TRUE if the unit menu should contain an item for
 *                GIMP_UNIT_PIXEL.
 * @show_percent: %TRUE in the unit menu should contain an item for
 *                GIMP_UNIT_PERCENT.
 * @show_custom:  %TRUE if the unit menu should contain a "More..." item for
 *                opening the user-defined-unit selection dialog.
 *
 * Creates a new #GimpUnitMenu widget.
 *
 * The @format string supports the following percent expansions:
 *
 * <informaltable pgwide="1" frame="none" role="enum">
 *   <tgroup cols="2"><colspec colwidth="1*"/><colspec colwidth="8*"/>
 *     <tbody>
 *       <row>
 *         <entry>% f</entry>
 *         <entry>Factor (how many units make up an inch)</entry>
 *        </row>
 *       <row>
 *         <entry>% y</entry>
 *         <entry>Symbol (e.g. "''" for GIMP_UNIT_INCH)</entry>
 *       </row>
 *       <row>
 *         <entry>% a</entry>
 *         <entry>Abbreviation</entry>
 *       </row>
 *       <row>
 *         <entry>% s</entry>
 *         <entry>Singular</entry>
 *       </row>
 *       <row>
 *         <entry>% p</entry>
 *         <entry>Plural</entry>
 *       </row>
 *       <row>
 *         <entry>%%</entry>
 *         <entry>Literal percent</entry>
 *       </row>
 *     </tbody>
 *   </tgroup>
 * </informaltable>
 *
 * Returns: A pointer to the new #GimpUnitMenu widget.
 **/
GtkWidget *
gimp_unit_menu_new (const gchar *format,
		    GimpUnit     unit,
		    gboolean     show_pixels,
		    gboolean     show_percent,
		    gboolean     show_custom)
{
  GimpUnitMenu *unit_menu;
  GtkWidget    *menu;
  GtkWidget    *menuitem;
  GimpUnit      u;

  g_return_val_if_fail (((unit >= GIMP_UNIT_PIXEL) &&
			 (unit < gimp_unit_get_number_of_units ())) ||
			(unit == GIMP_UNIT_PERCENT), NULL);

  if ((unit >= gimp_unit_get_number_of_built_in_units ()) &&
      (unit != GIMP_UNIT_PERCENT))
    show_custom = TRUE;

  unit_menu = g_object_new (GIMP_TYPE_UNIT_MENU, NULL);

  unit_menu->format       = g_strdup (format);
  unit_menu->show_pixels  = show_pixels;
  unit_menu->show_percent = show_percent;

  menu = gtk_menu_new ();
  for (u = show_pixels ? GIMP_UNIT_PIXEL : GIMP_UNIT_INCH;
       u < gimp_unit_get_number_of_built_in_units ();
       u++)
    {
      /*  special cases "pixels" and "percent"  */
      if (u == GIMP_UNIT_INCH)
	{
	  if (show_percent)
	    {
	      menuitem =
		gtk_menu_item_new_with_label
		(gimp_unit_menu_build_string (format, GIMP_UNIT_PERCENT));
	      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	      g_object_set_data (G_OBJECT (menuitem), "gimp_unit_menu",
                                 GINT_TO_POINTER (GIMP_UNIT_PERCENT));
	      gtk_widget_show (menuitem);

	      g_signal_connect (menuitem, "activate",
                                G_CALLBACK (gimp_unit_menu_callback),
                                unit_menu);
	    }

	  if (show_pixels || show_percent)
	    {
	      menuitem = gtk_menu_item_new ();
	      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	      gtk_widget_set_sensitive (menuitem, FALSE);
	      gtk_widget_show (menuitem);
	    }
	}

      menuitem =
	gtk_menu_item_new_with_label (gimp_unit_menu_build_string (format, u));
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      g_object_set_data (G_OBJECT (menuitem), "gimp_unit_menu",
                         GINT_TO_POINTER (u));
      gtk_widget_show (menuitem);

      g_signal_connect (menuitem, "activate",
                        G_CALLBACK (gimp_unit_menu_callback),
                        unit_menu);
    }

  if ((unit >= gimp_unit_get_number_of_built_in_units ()) &&
      (unit != GIMP_UNIT_PERCENT))
    {
      menuitem = gtk_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      gtk_widget_set_sensitive (menuitem, FALSE);
      gtk_widget_show (menuitem);

      menuitem =
	gtk_menu_item_new_with_label (gimp_unit_menu_build_string (format,
                                                                   unit));
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      g_object_set_data (G_OBJECT (menuitem), "gimp_unit_menu",
                         GINT_TO_POINTER (unit));
      gtk_widget_show (menuitem);

      g_signal_connect (menuitem, "activate",
                        G_CALLBACK (gimp_unit_menu_callback),
                        unit_menu);
    }

  if (show_custom)
    {
      menuitem = gtk_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      gtk_widget_set_sensitive (menuitem, FALSE);
      gtk_widget_show (menuitem);

      menuitem =
	gtk_menu_item_new_with_label (_("More..."));
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      g_object_set_data (G_OBJECT (menuitem), "gimp_unit_menu",
                         GINT_TO_POINTER (GIMP_UNIT_PERCENT + 1));
      gtk_widget_show (menuitem);

      g_signal_connect (menuitem, "activate",
                        G_CALLBACK (gimp_unit_menu_callback),
                        unit_menu);
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (unit_menu), menu);

  unit_menu->unit = unit;
  gtk_option_menu_set_history (GTK_OPTION_MENU (unit_menu),
			       (unit == GIMP_UNIT_PIXEL) ? 0 :
			       ((unit == GIMP_UNIT_PERCENT) ?
				(show_pixels ? 1 : 0) :
				(((show_pixels || show_percent) ? 2 : 0) +
				 ((show_pixels && show_percent) ? 1 : 0) +
				 ((unit < GIMP_UNIT_END) ?
				  (unit - 1) : GIMP_UNIT_END))));

  return GTK_WIDGET (unit_menu);
}

/**
 * gimp_unit_menu_set_unit:
 * @menu:  The unit menu you want to set the unit for.
 * @unit: The new unit.
 *
 * Sets a new #GimpUnit for the specified #GimpUnitMenu.
 **/
void
gimp_unit_menu_set_unit (GimpUnitMenu *menu,
			 GimpUnit      unit)
{
  GtkWidget *menuitem = NULL;
  GList     *items;
  gint       user_unit;

  g_return_if_fail (GIMP_IS_UNIT_MENU (menu));
  g_return_if_fail (((unit >= GIMP_UNIT_PIXEL) &&
		     ((unit > GIMP_UNIT_PIXEL) || menu->show_pixels) &&
		     (unit < gimp_unit_get_number_of_units ())) ||
		    ((unit == GIMP_UNIT_PERCENT) && menu->show_percent));

  if (unit == menu->unit)
    return;

  items = GTK_MENU_SHELL (GTK_OPTION_MENU (menu)->menu)->children;
  user_unit = (GIMP_UNIT_END +
	       (((menu->show_pixels || menu->show_percent) ? 2 : 0) +
		((menu->show_pixels && menu->show_percent) ? 1 : 0)));

  if ((unit >= GIMP_UNIT_END) && (unit != GIMP_UNIT_PERCENT))
    {
      if ((g_list_length (items) - 3) >= user_unit)
	{
	  gtk_widget_destroy (GTK_WIDGET (g_list_nth_data (items,
							   user_unit - 1)));
	  gtk_widget_destroy (GTK_WIDGET (g_list_nth_data (items,
							   user_unit - 1)));
	}

      menuitem = gtk_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (GTK_OPTION_MENU (menu)->menu),
			     menuitem);
      gtk_widget_set_sensitive (menuitem, FALSE);
      gtk_menu_reorder_child (GTK_MENU (GTK_OPTION_MENU (menu)->menu),
			      menuitem, user_unit - 1);
      gtk_widget_show (menuitem);

      menuitem =
	gtk_menu_item_new_with_label (gimp_unit_menu_build_string (menu->format,
								   unit));
      gtk_menu_shell_append (GTK_MENU_SHELL (GTK_OPTION_MENU (menu)->menu),
			     menuitem);
      g_object_set_data (G_OBJECT (menuitem), "gimp_unit_menu",
                         GINT_TO_POINTER (unit));
      gtk_menu_reorder_child (GTK_MENU (GTK_OPTION_MENU (menu)->menu),
			      menuitem, user_unit);
      gtk_widget_show (menuitem);

      g_signal_connect (menuitem, "activate",
                        G_CALLBACK (gimp_unit_menu_callback),
                        menu);
    }

  menu->unit = unit;
  gtk_option_menu_set_history (GTK_OPTION_MENU (menu),
			       (unit == GIMP_UNIT_PIXEL) ? 0 :
			       ((unit == GIMP_UNIT_PERCENT) ?
				(menu->show_pixels ? 1 : 0) :
				(((menu->show_pixels ||
				   menu->show_percent) ? 2 : 0) +
				 ((menu->show_pixels &&
				   menu->show_percent) ? 1 : 0) +
				 ((unit < GIMP_UNIT_END) ?
				  (unit - 1) : GIMP_UNIT_END))));

  g_signal_emit (menu, gimp_unit_menu_signals[UNIT_CHANGED], 0);
}

/**
 * gimp_unit_menu_get_unit:
 * @menu: The unit menu you want to know the unit of.
 *
 * Returns the #GimpUnit the user has selected from the #GimpUnitMenu.
 *
 * Returns: The unit the user has selected.
 **/
GimpUnit
gimp_unit_menu_get_unit (GimpUnitMenu *menu)
{
  g_return_val_if_fail (GIMP_IS_UNIT_MENU (menu), GIMP_UNIT_INCH);

  return menu->unit;
}


/**
 * gimp_unit_menu_set_pixel_digits:
 * @menu: a #GimpUnitMenu
 * @digits: the number of digits to display for a pixel size
 *
 * A GimpUnitMenu can be setup to control the number of digits shown
 * by attached spinbuttons. Please refer to the documentation of
 * gimp_unit_menu_update() to see how this is done.
 *
 * This function allows to specify the number of digits shown for a
 * size in pixels. Usually this is 0 (only full pixels). If you want
 * to allow the user to specify sub-pixel sizes using the attached
 * spinbuttons, specify the number of digits after the decimal point
 * here. You should do this after attaching your spinbuttons.
 **/
void
gimp_unit_menu_set_pixel_digits (GimpUnitMenu *menu,
                                 gint          digits)
{
  GimpUnit unit;

  g_return_if_fail (GIMP_IS_UNIT_MENU (menu));

  menu->pixel_digits = digits;

  gimp_unit_menu_update (GTK_WIDGET (menu), &unit);
}

/**
 * gimp_unit_menu_get_pixel_digits:
 * @menu: a #GimpUnitMenu
 *
 * Retrieve the number of digits for a pixel size as set by
 * gimp_unit_set_pixel_digits().
 *
 * Return value: the configured number of digits for a pixel size
 **/
gint
gimp_unit_menu_get_pixel_digits (GimpUnitMenu *menu)
{
  g_return_val_if_fail (GIMP_IS_UNIT_MENU (menu), 0);

  return menu->pixel_digits;
}


/*  most of the next two functions is stolen from app/gdisplay.c  */
static gint
print (gchar       *buf,
       gint         len,
       gint         start,
       const gchar *fmt,
       ...)
{
  va_list args;
  gint printed;

  va_start (args, fmt);

  printed = g_vsnprintf (buf + start, len - start, fmt, args);
  if (printed < 0)
    printed = len - start;

  va_end (args);

  return printed;
}

static const gchar *
gimp_unit_menu_build_string (const gchar *format,
			     GimpUnit     unit)
{
  static gchar buffer[64];
  gint i = 0;

  while (i < (sizeof (buffer) - 1) && *format)
    {
      switch (*format)
	{
	case '%':
	  format++;
	  switch (*format)
	    {
	    case 0:
	      g_warning ("unit-menu-format string ended within %%-sequence");
	      break;

	    case '%':
	      buffer[i++] = '%';
	      break;

	    case 'f': /* factor (how many units make up an inch) */
	      i += print (buffer, sizeof (buffer), i, "%f",
			  gimp_unit_get_factor (unit));
	      break;

	    case 'y': /* symbol ("''" for inch) */
	      i += print (buffer, sizeof (buffer), i, "%s",
			  gimp_unit_get_symbol (unit));
	      break;

	    case 'a': /* abbreviation */
	      i += print (buffer, sizeof (buffer), i, "%s",
			  gimp_unit_get_abbreviation (unit));
	      break;

	    case 's': /* singular */
	      i += print (buffer, sizeof (buffer), i, "%s",
			  gimp_unit_get_singular (unit));
	      break;

	    case 'p': /* plural */
	      i += print (buffer, sizeof (buffer), i, "%s",
			  gimp_unit_get_plural (unit));
	      break;

	    default:
	      g_warning ("gimp_unit_menu_build_string(): "
			 "unit-menu-format contains unknown format sequence "
			 "'%%%c'", *format);
	      break;
	    }
	  break;

	default:
	  buffer[i++] = *format;
	  break;
	}

      format++;
    }

  buffer[MIN (i, sizeof (buffer) - 1)] = 0;

  return buffer;
}

/*  private callback of gimp_unit_menu_create_selection ()  */
static void
gimp_unit_menu_selection_response (GtkWidget    *widget,
                                   gint          response_id,
                                   GimpUnitMenu *menu)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      GtkTreeSelection *sel;
      GtkTreeModel     *model;
      GtkTreeIter       iter;

      sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (menu->tv));
      if (menu->selection && gtk_tree_selection_get_selected (sel, &model,
                                                              &iter))
        {
          GValue   val = { 0, };
          GimpUnit unit;

          gtk_tree_model_get_value (model, &iter, 2, &val);
          unit = (GimpUnit) g_value_get_int (&val);
          g_value_unset (&val);

          gimp_unit_menu_set_unit (menu, unit);
        }
    }

  gtk_widget_destroy (menu->selection);
}

static void
gimp_unit_menu_selection_row_activated_callback (GtkTreeView       *tv,
						 GtkTreePath       *path,
						 GtkTreeViewColumn *column,
						 GimpUnitMenu      *menu)
{
  gtk_dialog_response (GTK_DIALOG (menu->selection), GTK_RESPONSE_OK);
}

/*  private function of gimp_unit_menu_callback ()  */
static void
gimp_unit_menu_create_selection (GimpUnitMenu *menu)
{
  GtkWidget        *vbox;
  GtkWidget        *scrolled_win;
  GtkListStore     *list;
  GtkTreeSelection *sel;
  GtkTreeIter       iter;
  GtkTreePath      *path;
  GimpUnit          unit;
  gint              num_units;

  menu->selection = gimp_dialog_new (_("Unit Selection"), "gimp-unit-selection",
                                     GTK_WIDGET (menu),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     gimp_standard_help_func, "gimp-unit-dialog",

                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                     NULL);

  g_object_add_weak_pointer (G_OBJECT (menu->selection),
                             (gpointer) &menu->selection);

  g_signal_connect (menu->selection, "response",
                    G_CALLBACK (gimp_unit_menu_selection_response),
                    menu);

  g_signal_connect_object (menu, "unmap",
                           G_CALLBACK (gtk_widget_destroy),
                           menu->selection, G_CONNECT_SWAPPED);

  /*  the main vbox  */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (menu->selection)->vbox), vbox);
  gtk_widget_show (vbox);

  /*  the selection list  */
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_win),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_ALWAYS);
  gtk_container_add (GTK_CONTAINER (vbox), scrolled_win);
  gtk_widget_show (scrolled_win);

  list = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING,
			     G_TYPE_INT);
  menu->tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list));
  g_object_unref (list);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (menu->tv),
					       -1, _("Unit"),
					       gtk_cell_renderer_text_new (),
					       "text", UNIT_COLUMN, NULL);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (menu->tv),
					       -1, _("Factor"),
					       gtk_cell_renderer_text_new (),
					       "text", FACTOR_COLUMN, NULL);

  /*  the unit lines  */
  num_units = gimp_unit_get_number_of_units ();
  for (unit = GIMP_UNIT_END; unit < num_units; unit++)
    {
      gtk_list_store_append (list, &iter);
      gtk_list_store_set (list, &iter,
			  UNIT_COLUMN,
			  gimp_unit_menu_build_string (menu->format, unit),
			  -1);
      gtk_list_store_set (list, &iter,
			  FACTOR_COLUMN,
			  gimp_unit_menu_build_string ("(%f)", unit),
			  -1);
      gtk_list_store_set (list, &iter, DATA_COLUMN, unit, -1);
    }

  gtk_widget_set_size_request (menu->tv, -1, 150);

  gtk_container_add (GTK_CONTAINER (scrolled_win), menu->tv);

  g_signal_connect (menu->tv, "row_activated",
                    G_CALLBACK (gimp_unit_menu_selection_row_activated_callback),
                    menu);

  gtk_widget_show (menu->tv);

  g_signal_connect (menu->tv, "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &menu->tv);

  gtk_widget_show (vbox);
  gtk_widget_show (menu->selection);

  if (menu->unit >= GIMP_UNIT_END)
    {
      path = gtk_tree_path_new ();
      gtk_tree_path_append_index (path, menu->unit - GIMP_UNIT_END);

      sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (menu->tv));
      gtk_tree_selection_select_path (sel, path);

      gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (menu->tv), path, NULL,
				    FALSE, 0.0, 0.0);
    }
}

static void
gimp_unit_menu_callback (GtkWidget *widget,
			 gpointer   data)
{
  GimpUnitMenu *menu;
  GimpUnit      new_unit;

  menu = data;
  new_unit = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget),
                                                  "gimp_unit_menu"));

  if (menu->unit == new_unit)
    return;

  /*  was "More..." selected?  */
  if (new_unit == (GIMP_UNIT_PERCENT + 1))
    {
      gtk_option_menu_set_history (GTK_OPTION_MENU (menu),
				   (menu->unit == GIMP_UNIT_PIXEL) ? 0 :
				   ((menu->unit == GIMP_UNIT_PERCENT) ?
				    (menu->show_pixels ? 1 : 0) :
				    ((menu->show_pixels ||
				      menu->show_percent ? 2 : 0) +
				     (menu->show_pixels &&
				      menu->show_percent ? 1 : 0) +
				     ((menu->unit < GIMP_UNIT_END) ?
				      menu->unit - 1 : GIMP_UNIT_END))));
      if (! menu->selection)
	gimp_unit_menu_create_selection (menu);
      return;
    }
  else if (menu->selection)
    {
      gtk_widget_destroy (menu->selection);
    }

  gimp_unit_menu_set_unit (menu, new_unit);
}
