/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpuimanager.c
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

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libgimpbase/gimpbase.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "widgets-types.h"

#include "core/gimp.h"
#include "core/gimpmarshal.h"

#include "gimpactiongroup.h"
#include "gimphelp.h"
#include "gimphelp-ids.h"
#include "gimpuimanager.h"


enum
{
  PROP_0,
  PROP_NAME,
  PROP_GIMP
};

enum
{
  UPDATE,
  LAST_SIGNAL
};


static void   gimp_ui_manager_init           (GimpUIManager         *manager);
static void   gimp_ui_manager_class_init     (GimpUIManagerClass    *klass);

static GObject * gimp_ui_manager_constructor (GType                  type,
                                              guint                  n_params,
                                              GObjectConstructParam *params);
static void   gimp_ui_manager_dispose        (GObject               *object);
static void   gimp_ui_manager_finalize       (GObject               *object);
static void   gimp_ui_manager_set_property   (GObject               *object,
                                              guint                  prop_id,
                                              const GValue          *value,
                                              GParamSpec            *pspec);
static void   gimp_ui_manager_get_property   (GObject               *object,
                                              guint                  prop_id,
                                              GValue                *value,
                                              GParamSpec            *pspec);
static void   gimp_ui_manager_connect_proxy  (GtkUIManager          *manager,
                                              GtkAction             *action,
                                              GtkWidget             *proxy);
static void   gimp_ui_manager_real_update    (GimpUIManager         *manager,
                                              gpointer               update_data);

/*  help support  */

static void     gimp_ui_manager_item_realize   (GtkWidget     *widget,
                                                GimpUIManager *manager);
static gboolean gimp_ui_manager_item_key_press (GtkWidget     *widget,
                                                GdkEventKey   *kevent,
                                                GimpUIManager *manager);


static guint manager_signals[LAST_SIGNAL] = { 0 };

static GtkUIManagerClass *parent_class = NULL;


GType
gimp_ui_manager_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo type_info =
      {
        sizeof (GimpUIManagerClass),
	NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gimp_ui_manager_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GimpUIManager),
        0, /* n_preallocs */
        (GInstanceInitFunc) gimp_ui_manager_init,
      };

      type = g_type_register_static (GTK_TYPE_UI_MANAGER,
                                     "GimpUIManager",
				     &type_info, 0);
    }

  return type;
}

static void
gimp_ui_manager_class_init (GimpUIManagerClass *klass)
{
  GObjectClass      *object_class  = G_OBJECT_CLASS (klass);
  GtkUIManagerClass *manager_class = GTK_UI_MANAGER_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->constructor    = gimp_ui_manager_constructor;
  object_class->dispose        = gimp_ui_manager_dispose;
  object_class->finalize       = gimp_ui_manager_finalize;
  object_class->set_property   = gimp_ui_manager_set_property;
  object_class->get_property   = gimp_ui_manager_get_property;

#ifdef __GNUC__
#warning FIXME: remove version check as soon as we depend on GTK+ 2.4.2
#endif
  if (! gtk_check_version (2, 4, 2))
    manager_class->connect_proxy = gimp_ui_manager_connect_proxy;

  klass->update                = gimp_ui_manager_real_update;

  manager_signals[UPDATE] =
    g_signal_new ("update",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GimpUIManagerClass, update),
		  NULL, NULL,
		  gimp_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  g_object_class_install_property (object_class, PROP_NAME,
                                   g_param_spec_string ("name",
                                                        NULL, NULL,
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_GIMP,
                                   g_param_spec_object ("gimp",
                                                        NULL, NULL,
                                                        GIMP_TYPE_GIMP,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  klass->managers = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           g_free, NULL);
}

static void
gimp_ui_manager_init (GimpUIManager *manager)
{
  manager->name = NULL;
  manager->gimp = NULL;

#ifdef __GNUC__
#warning FIXME: remove this hack as soon as we depend on GTK+ 2.4.2
#endif
  if (gtk_check_version (2, 4, 2))
    g_signal_connect (manager, "connect-proxy",
                      G_CALLBACK (gimp_ui_manager_connect_proxy),
                      NULL);
}

static GObject *
gimp_ui_manager_constructor (GType                  type,
                             guint                  n_params,
                             GObjectConstructParam *params)
{
  GObject       *object;
  GimpUIManager *manager;

  object = G_OBJECT_CLASS (parent_class)->constructor (type, n_params, params);

  manager = GIMP_UI_MANAGER (object);

  if (manager->name)
    {
      GimpUIManagerClass *manager_class;
      GList              *list;

      manager_class = GIMP_UI_MANAGER_GET_CLASS (object);

      list = g_hash_table_lookup (manager_class->managers, manager->name);

      list = g_list_append (list, manager);

      g_hash_table_replace (manager_class->managers,
                            g_strdup (manager->name), list);
    }

  return object;
}

static void
gimp_ui_manager_dispose (GObject *object)
{
  GimpUIManager *manager = GIMP_UI_MANAGER (object);

  if (manager->name)
    {
      GimpUIManagerClass *manager_class;
      GList              *list;

      manager_class = GIMP_UI_MANAGER_GET_CLASS (object);

      list = g_hash_table_lookup (manager_class->managers, manager->name);

      if (list)
        {
          list = g_list_remove (list, manager);

          if (list)
            g_hash_table_replace (manager_class->managers,
                                  g_strdup (manager->name), list);
          else
            g_hash_table_remove (manager_class->managers, manager->name);
        }
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gimp_ui_manager_finalize (GObject *object)
{
  GimpUIManager *manager = GIMP_UI_MANAGER (object);
  GList         *list;

  for (list = manager->registered_uis; list; list = g_list_next (list))
    {
      GimpUIManagerUIEntry *entry = list->data;

      g_free (entry->ui_path);
      g_free (entry->basename);

      if (entry->widget)
        g_object_unref (entry->widget);

      g_free (entry);
    }

  g_list_free (manager->registered_uis);
  manager->registered_uis = NULL;

  if (manager->name)
    {
      g_free (manager->name);
      manager->name = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_ui_manager_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GimpUIManager *manager = GIMP_UI_MANAGER (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_free (manager->name);
      manager->name = g_value_dup_string (value);
      break;
    case PROP_GIMP:
      manager->gimp = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gimp_ui_manager_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GimpUIManager *manager = GIMP_UI_MANAGER (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, manager->name);
      break;
    case PROP_GIMP:
      g_value_set_object (value, manager->gimp);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gimp_ui_manager_connect_proxy (GtkUIManager *manager,
                               GtkAction    *action,
                               GtkWidget    *proxy)
{
  g_object_set_qdata (G_OBJECT (proxy), GIMP_HELP_ID,
                      g_object_get_qdata (G_OBJECT (action),
                                          GIMP_HELP_ID));

  if (GTK_IS_MENU_ITEM (proxy))
    g_signal_connect_after (proxy, "realize",
                            G_CALLBACK (gimp_ui_manager_item_realize),
                            manager);
}

static void
gimp_ui_manager_real_update (GimpUIManager *manager,
                             gpointer       update_data)
{
  GList *list;

  for (list = gtk_ui_manager_get_action_groups (GTK_UI_MANAGER (manager));
       list;
       list = g_list_next (list))
    {
      gimp_action_group_update (list->data, update_data);
    }
}

/**
 * gimp_ui_manager_new:
 * @gimp: the @Gimp instance this ui manager belongs to
 *
 * Creates a new #GimpUIManager object.
 *
 * Returns: the new #GimpUIManager
 */
GimpUIManager *
gimp_ui_manager_new (Gimp        *gimp,
                     const gchar *name)
{
  GimpUIManager *manager;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), NULL);

  manager = g_object_new (GIMP_TYPE_UI_MANAGER,
                          "name", name,
                          "gimp", gimp,
                          NULL);

  return manager;
}

GList *
gimp_ui_managers_from_name (const gchar *name)
{
  GimpUIManagerClass *manager_class;
  GList              *list;

  g_return_val_if_fail (name != NULL, NULL);

  manager_class = g_type_class_ref (GIMP_TYPE_UI_MANAGER);

  list = g_hash_table_lookup (manager_class->managers, name);

  g_type_class_unref (manager_class);

  return list;
}

void
gimp_ui_manager_update (GimpUIManager *manager,
                        gpointer       update_data)
{
  g_return_if_fail (GIMP_IS_UI_MANAGER (manager));

  g_signal_emit (manager, manager_signals[UPDATE], 0, update_data);
}

GimpActionGroup *
gimp_ui_manager_get_action_group (GimpUIManager   *manager,
                                  const gchar     *name)
{
  GList *list;

  g_return_val_if_fail (GIMP_IS_UI_MANAGER (manager), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  for (list = gtk_ui_manager_get_action_groups (GTK_UI_MANAGER (manager));
           list;
           list = g_list_next (list))
    {
      GimpActionGroup *group = list->data;

      if (! strcmp (gtk_action_group_get_name (GTK_ACTION_GROUP (group)), name))
        return group;
    }

  return NULL;
}

void
gimp_ui_manager_ui_register (GimpUIManager          *manager,
                             const gchar            *ui_path,
                             const gchar            *basename,
                             GimpUIManagerSetupFunc  setup_func)
{
  GimpUIManagerUIEntry *entry;

  g_return_if_fail (GIMP_IS_UI_MANAGER (manager));
  g_return_if_fail (ui_path != NULL);
  g_return_if_fail (basename != NULL);

  entry = g_new0 (GimpUIManagerUIEntry, 1);

  entry->ui_path    = g_strdup (ui_path);
  entry->basename   = g_strdup (basename);
  entry->setup_func = setup_func;
  entry->merge_id   = 0;
  entry->widget     = NULL;

  manager->registered_uis = g_list_prepend (manager->registered_uis, entry);
}

GtkWidget *
gimp_ui_manager_ui_get (GimpUIManager *manager,
                        const gchar   *ui_path)
{
  GList *list;

  g_return_val_if_fail (GIMP_IS_UI_MANAGER (manager), NULL);
  g_return_val_if_fail (ui_path != NULL, NULL);

  for (list = manager->registered_uis; list; list = g_list_next (list))
    {
      GimpUIManagerUIEntry *entry = list->data;

      if (! strcmp (entry->ui_path, ui_path))
        {
          if (! entry->merge_id)
            {
              gchar  *filename;
              GError *error = NULL;

              filename = g_build_filename (gimp_data_directory (), "menus",
                                           entry->basename, NULL);

              g_print ("loading menu: %s for %s\n", filename,
                       entry->ui_path);

              entry->merge_id =
                gtk_ui_manager_add_ui_from_file (GTK_UI_MANAGER (manager),
                                                 filename, &error);

              g_free (filename);

              if (! entry->merge_id)
                {
                  g_message (error->message);
                  g_clear_error (&error);

                  return NULL;
                }
            }

          if (! entry->widget)
            {
              gtk_ui_manager_ensure_update (GTK_UI_MANAGER (manager));

              entry->widget =
                gtk_ui_manager_get_widget (GTK_UI_MANAGER (manager),
                                           entry->ui_path);

              if (entry->widget)
                {
                  g_object_ref (entry->widget);

                  if (entry->setup_func)
                    entry->setup_func (manager, entry->ui_path);
                }
            }

          return entry->widget;
        }
    }

  g_warning ("%s: no entry registered for \"%s\"",
             G_STRFUNC, ui_path);

  return NULL;
}

static void
gimp_ui_manager_menu_position (GtkMenu  *menu,
                               gint     *x,
                               gint     *y,
                               gpointer  data)
{
  GdkScreen      *screen;
  GtkRequisition  requisition;
  GdkRectangle    rect;
  gint            monitor;
  gint            pointer_x;
  gint            pointer_y;

  g_return_if_fail (GTK_IS_MENU (menu));
  g_return_if_fail (x != NULL);
  g_return_if_fail (y != NULL);
  g_return_if_fail (GTK_IS_WIDGET (data));

  gdk_display_get_pointer (gtk_widget_get_display (GTK_WIDGET (data)),
                           &screen, &pointer_x, &pointer_y, NULL);

  monitor = gdk_screen_get_monitor_at_point (screen, pointer_x, pointer_y);
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);

  gtk_menu_set_screen (menu, screen);

  gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

  if (gtk_widget_get_direction (GTK_WIDGET (menu)) == GTK_TEXT_DIR_RTL)
    {
      *x = pointer_x - 2 - requisition.width;

      if (*x < rect.x)
        *x = pointer_x + 2;
    }
  else
    {
      *x = pointer_x + 2;

      if (*x + requisition.width > rect.x + rect.width)
        *x = pointer_x - 2 - requisition.width;
    }

  *y = pointer_y + 2;

  if (*y + requisition.height > rect.y + rect.height)
    *y = pointer_y - 2 - requisition.height;

  if (*x < rect.x) *x = rect.x;
  if (*y < rect.y) *y = rect.y;
}

typedef struct
{
  guint x;
  guint y;
} MenuPos;

static void
gimp_ui_manager_menu_pos (GtkMenu  *menu,
                          gint     *x,
                          gint     *y,
                          gboolean *push_in,
                          gpointer  data)
{
  MenuPos *menu_pos = data;

  *x = menu_pos->x;
  *y = menu_pos->y;
}

static void
gimp_ui_manager_delete_popup_data (GtkObject     *object,
                                   GimpUIManager *manager)
{
  g_signal_handlers_disconnect_by_func (object,
                                        gimp_ui_manager_delete_popup_data,
                                        manager);
  g_object_set_data (G_OBJECT (manager), "popup-data", NULL);
}

void
gimp_ui_manager_ui_popup (GimpUIManager        *manager,
                          const gchar          *ui_path,
                          gpointer              popup_data,
                          GtkWidget            *parent,
                          GimpMenuPositionFunc  position_func,
                          gpointer              position_data,
                          GtkDestroyNotify      popdown_func)
{
  GtkWidget *widget;
  GdkEvent  *current_event;
  gint       x, y;
  guint      button;
  guint32    activate_time;
  MenuPos   *menu_pos;

  g_return_if_fail (GIMP_IS_UI_MANAGER (manager));
  g_return_if_fail (ui_path != NULL);
  g_return_if_fail (parent == NULL || GTK_IS_WIDGET (parent));

  widget = gimp_ui_manager_ui_get (manager, ui_path);

  g_return_if_fail (GTK_IS_MENU (widget));

  if (! position_func)
    {
      position_func = gimp_ui_manager_menu_position;
      position_data = parent;
    }

  (* position_func) (GTK_MENU (widget), &x, &y, position_data);

  current_event = gtk_get_current_event ();

  if (current_event && current_event->type == GDK_BUTTON_PRESS)
    {
      GdkEventButton *bevent = (GdkEventButton *) current_event;

      button        = bevent->button;
      activate_time = bevent->time;
    }
  else
    {
      button        = 0;
      activate_time = 0;
    }

  menu_pos = g_object_get_data (G_OBJECT (widget), "menu-pos");

  if (! menu_pos)
    {
      menu_pos = g_new0 (MenuPos, 1);
      g_object_set_data_full (G_OBJECT (widget), "menu-pos", menu_pos, g_free);
    }

  menu_pos->x = x;
  menu_pos->y = y;

  if (popup_data != NULL)
    {
      g_object_set_data_full (G_OBJECT (manager), "popup-data",
                              popup_data, popdown_func);
      g_signal_connect (widget, "selection-done",
                        G_CALLBACK (gimp_ui_manager_delete_popup_data),
                        manager);
    }

  gtk_menu_popup (GTK_MENU (widget),
                  NULL, NULL,
                  gimp_ui_manager_menu_pos, menu_pos,
                  button, activate_time);
}


/*  private functions  */

static void
gimp_ui_manager_item_realize (GtkWidget     *widget,
                              GimpUIManager *manager)
{
  GtkWidget *submenu;

  g_signal_handlers_disconnect_by_func (widget,
                                        gimp_ui_manager_item_realize,
                                        manager);

  if (GTK_IS_MENU_SHELL (widget->parent))
    {
      static GQuark quark_key_press_connected = 0;

      if (! quark_key_press_connected)
        quark_key_press_connected =
          g_quark_from_static_string ("gimp-menu-item-key-press-connected");

      if (! GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (widget->parent),
                                                 quark_key_press_connected)))
        {
          g_signal_connect (widget->parent, "key_press_event",
                            G_CALLBACK (gimp_ui_manager_item_key_press),
                            manager);

          g_object_set_qdata (G_OBJECT (widget->parent),
                              quark_key_press_connected,
                              GINT_TO_POINTER (TRUE));
        }
    }

  submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));

  if (submenu)
    g_object_set_qdata (G_OBJECT (submenu), GIMP_HELP_ID,
                        g_object_get_qdata (G_OBJECT (widget),
                                            GIMP_HELP_ID));
}

static gboolean
gimp_ui_manager_item_key_press (GtkWidget     *widget,
                                GdkEventKey   *kevent,
                                GimpUIManager *manager)
{
  gchar *help_id = NULL;

  while (! help_id)
    {
      GtkWidget *menu_item;

      menu_item = GTK_MENU_SHELL (widget)->active_menu_item;

      /*  first, get the help page from the item...
       */
      if (menu_item)
        {
          help_id = g_object_get_qdata (G_OBJECT (menu_item), GIMP_HELP_ID);

          if (help_id && ! strlen (help_id))
            help_id = NULL;
        }

      /*  ...then try the parent menu...
       */
      if (! help_id)
        {
          help_id = g_object_get_qdata (G_OBJECT (widget), GIMP_HELP_ID);

          if (help_id && ! strlen (help_id))
            help_id = NULL;
        }

      /*  ...finally try the menu's parent (if any)
       */
      if (! help_id)
        {
          menu_item = NULL;

          if (GTK_IS_MENU (widget))
            menu_item = gtk_menu_get_attach_widget (GTK_MENU (widget));

          if (! menu_item)
            break;

          widget = menu_item->parent;

          if (! widget)
            break;
        }
    }

  /*  For any valid accelerator key except F1, continue with the
   *  standard GtkMenuShell callback and assign a new shortcut, but
   *  don't assign a shortcut to the help menu entries ...
   */
  if (kevent->keyval != GDK_F1)
    {
      if (help_id                                   &&
          gtk_accelerator_valid (kevent->keyval, 0) &&
          (strcmp (help_id, GIMP_HELP_HELP)         == 0 ||
           strcmp (help_id, GIMP_HELP_HELP_CONTEXT) == 0))
        {
          return TRUE;
        }

      return FALSE;
    }

  /*  ...finally, if F1 was pressed over any menu, show its help page...  */

  if (help_id)
    {
      gchar *help_domain = NULL;
      gchar *help_string = NULL;
      gchar *domain_separator;

      help_id = g_strdup (help_id);

      domain_separator = strchr (help_id, '?');

      if (domain_separator)
        {
          *domain_separator = '\0';

          help_domain = g_strdup (help_id);
          help_string = g_strdup (domain_separator + 1);
        }
      else
        {
          help_string = g_strdup (help_id);
        }

      gimp_help (manager->gimp, help_domain, help_string);

      g_free (help_domain);
      g_free (help_string);
      g_free (help_id);
    }

  return TRUE;
}
