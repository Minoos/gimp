/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include "config/gimpcoreconfig.h"

#include "core/gimp.h"
#include "core/gimplist.h"
#include "core/gimpimage.h"
#include "core/gimpimage-undo.h"
#include "core/gimpundostack.h"

#include "gimpcontainertreeview.h"
#include "gimpcontainerview.h"
#include "gimphelp-ids.h"
#include "gimpmenufactory.h"
#include "gimpundoeditor.h"

#include "gimp-intl.h"


enum
{
  PROP_0,
  PROP_PREVIEW_SIZE
};


static void      gimp_undo_editor_class_init   (GimpUndoEditorClass   *klass);
static void      gimp_undo_editor_init         (GimpUndoEditor        *undo_editor);
static GObject * gimp_undo_editor_constructor    (GType              type,
                                                  guint              n_params,
                                                  GObjectConstructParam *params);
static void      gimp_undo_editor_set_property   (GObject           *object,
                                                  guint              property_id,
                                                  const GValue      *value,
                                                  GParamSpec        *pspec);

static void      gimp_undo_editor_set_image      (GimpImageEditor   *editor,
                                                  GimpImage         *gimage);

static void      gimp_undo_editor_fill           (GimpUndoEditor    *editor);
static void      gimp_undo_editor_clear          (GimpUndoEditor    *editor);

static void      gimp_undo_editor_undo_event     (GimpImage         *gimage,
                                                  GimpUndoEvent      event,
                                                  GimpUndo          *undo,
                                                  GimpUndoEditor    *editor);

static void      gimp_undo_editor_select_item    (GimpContainerView *view,
                                                  GimpUndo          *undo,
                                                  gpointer           insert_data,
                                                  GimpUndoEditor    *editor);


static GimpImageEditorClass *parent_class = NULL;


GType
gimp_undo_editor_get_type (void)
{
  static GType editor_type = 0;

  if (! editor_type)
    {
      static const GTypeInfo editor_info =
      {
        sizeof (GimpUndoEditorClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gimp_undo_editor_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (GimpUndoEditor),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) gimp_undo_editor_init,
      };

      editor_type = g_type_register_static (GIMP_TYPE_IMAGE_EDITOR,
                                            "GimpUndoEditor",
                                            &editor_info, 0);
    }

  return editor_type;
}

static void
gimp_undo_editor_class_init (GimpUndoEditorClass *klass)
{
  GObjectClass         *object_class       = G_OBJECT_CLASS (klass);
  GimpImageEditorClass *image_editor_class = GIMP_IMAGE_EDITOR_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->constructor     = gimp_undo_editor_constructor;
  object_class->set_property    = gimp_undo_editor_set_property;

  image_editor_class->set_image = gimp_undo_editor_set_image;

  g_object_class_install_property (object_class,
                                   PROP_PREVIEW_SIZE,
                                   g_param_spec_enum ("preview-size",
                                                      NULL, NULL,
                                                      GIMP_TYPE_PREVIEW_SIZE,
                                                      GIMP_PREVIEW_SIZE_LARGE,
                                                      G_PARAM_WRITABLE |
                                                      G_PARAM_CONSTRUCT_ONLY));
}

static void
gimp_undo_editor_init (GimpUndoEditor *undo_editor)
{
}

static GObject *
gimp_undo_editor_constructor (GType                  type,
                              guint                  n_params,
                              GObjectConstructParam *params)
{
  GimpUndoEditor *undo_editor;
  GObject        *object;

  object = G_OBJECT_CLASS (parent_class)->constructor (type, n_params, params);

  undo_editor = GIMP_UNDO_EDITOR (object);

  undo_editor->view = gimp_container_tree_view_new (NULL, NULL,
                                                    undo_editor->preview_size,
                                                    1);

  gtk_container_add (GTK_CONTAINER (undo_editor), undo_editor->view);
  gtk_widget_show (undo_editor->view);

  g_signal_connect (undo_editor->view, "select_item",
                    G_CALLBACK (gimp_undo_editor_select_item),
                    undo_editor);

  undo_editor->undo_button =
    gimp_editor_add_action_button (GIMP_EDITOR (undo_editor), "edit",
                                   "edit-undo", NULL);

  undo_editor->redo_button =
    gimp_editor_add_action_button (GIMP_EDITOR (undo_editor), "edit",
                                   "edit-redo", NULL);

  undo_editor->clear_button =
    gimp_editor_add_action_button (GIMP_EDITOR (undo_editor), "edit",
                                   "edit-undo-clear", NULL);

  return object;
}

static void
gimp_undo_editor_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GimpUndoEditor *undo_editor = GIMP_UNDO_EDITOR (object);

  switch (property_id)
    {
    case PROP_PREVIEW_SIZE:
      undo_editor->preview_size = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_undo_editor_set_image (GimpImageEditor *image_editor,
                            GimpImage       *gimage)
{
  GimpUndoEditor *editor = GIMP_UNDO_EDITOR (image_editor);

  if (image_editor->gimage)
    {
      gimp_undo_editor_clear (editor);

      g_signal_handlers_disconnect_by_func (image_editor->gimage,
					    gimp_undo_editor_undo_event,
					    editor);
    }

  GIMP_IMAGE_EDITOR_CLASS (parent_class)->set_image (image_editor, gimage);

  if (image_editor->gimage)
    {
      if (gimp_image_undo_is_enabled (image_editor->gimage))
        gimp_undo_editor_fill (editor);

      g_signal_connect (image_editor->gimage, "undo_event",
			G_CALLBACK (gimp_undo_editor_undo_event),
			editor);
    }
}


/*  public functions  */

GtkWidget *
gimp_undo_editor_new (GimpCoreConfig  *config,
                      GimpMenuFactory *menu_factory)
{
  g_return_val_if_fail (GIMP_IS_CORE_CONFIG (config), NULL);
  g_return_val_if_fail (GIMP_IS_MENU_FACTORY (menu_factory), NULL);

  return g_object_new (GIMP_TYPE_UNDO_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<UndoEditor>",
                       "preview-size",    config->undo_preview_size,
                       NULL);
}


/*  private functions  */

static void
gimp_undo_editor_fill (GimpUndoEditor *editor)
{
  GimpImage *gimage;
  GimpUndo  *top_undo_item;
  GList     *list;

  gimage = GIMP_IMAGE_EDITOR (editor)->gimage;

  /*  create a container as model for the undo history list  */
  editor->container = gimp_list_new (GIMP_TYPE_UNDO, FALSE);
  editor->base_item = gimp_undo_new (gimage, GIMP_UNDO_GROUP_NONE,
                                     _("[ Base Image ]"),
                                     NULL, 0, FALSE, NULL, NULL);

  /*  the list prepends its items, so first add the redo items...  */
  for (list = GIMP_LIST (gimage->redo_stack->undos)->list;
       list;
       list = g_list_next (list))
    {
      gimp_container_add (editor->container, GIMP_OBJECT (list->data));
    }

  /*  ...reverse the list so the redo items are in ascending order...  */
  gimp_list_reverse (GIMP_LIST (editor->container));

  /*  ...then add the undo items in descending order...  */
  for (list = GIMP_LIST (gimage->undo_stack->undos)->list;
       list;
       list = g_list_next (list))
    {
      /*  Don't add the topmost item if it is an open undo group,
       *  it will be added upon closing of the group.
       */
      if (list->prev || ! GIMP_IS_UNDO_STACK (list->data) ||
          gimage->pushing_undo_group == GIMP_UNDO_GROUP_NONE)
        {
          gimp_container_add (editor->container, GIMP_OBJECT (list->data));
        }
    }

  /*  ...finally, the first item is the special "base_item" which stands
   *  for the image with no more undos available to pop
   */
  gimp_container_add (editor->container, GIMP_OBJECT (editor->base_item));

  /*  display the container  */
  gimp_container_view_set_container (GIMP_CONTAINER_VIEW (editor->view),
                                     editor->container);

  top_undo_item = gimp_undo_stack_peek (gimage->undo_stack);

  g_signal_handlers_block_by_func (editor->view,
                                   gimp_undo_editor_select_item,
                                   editor);

  /*  select the current state of the image  */
  if (top_undo_item)
    {
      gimp_container_view_select_item (GIMP_CONTAINER_VIEW (editor->view),
                                       GIMP_VIEWABLE (top_undo_item));
      gimp_undo_create_preview (top_undo_item, FALSE);
    }
  else
    {
      gimp_container_view_select_item (GIMP_CONTAINER_VIEW (editor->view),
                                       GIMP_VIEWABLE (editor->base_item));
      gimp_undo_create_preview (editor->base_item, TRUE);
    }

  g_signal_handlers_unblock_by_func (editor->view,
                                     gimp_undo_editor_select_item,
                                     editor);
}

static void
gimp_undo_editor_clear (GimpUndoEditor *editor)
{
  if (editor->container)
    {
      gimp_container_view_set_container (GIMP_CONTAINER_VIEW (editor->view),
                                         NULL);
      g_object_unref (editor->container);
      editor->container = NULL;
    }

  if (editor->base_item)
    {
      g_object_unref (editor->base_item);
      editor->base_item = NULL;
    }
}

static void
gimp_undo_editor_undo_event (GimpImage      *gimage,
                             GimpUndoEvent   event,
                             GimpUndo       *undo,
                             GimpUndoEditor *editor)
{
  GimpUndo *top_undo_item;

  top_undo_item = gimp_undo_stack_peek (gimage->undo_stack);

  switch (event)
    {
    case GIMP_UNDO_EVENT_UNDO_PUSHED:
      g_signal_handlers_block_by_func (editor->view,
                                       gimp_undo_editor_select_item,
                                       editor);

      gimp_container_insert (editor->container, GIMP_OBJECT (undo), -1);
      gimp_container_view_select_item (GIMP_CONTAINER_VIEW (editor->view),
                                       GIMP_VIEWABLE (undo));
      gimp_undo_create_preview (undo, FALSE);

      g_signal_handlers_unblock_by_func (editor->view,
                                         gimp_undo_editor_select_item,
                                         editor);
      break;

    case GIMP_UNDO_EVENT_UNDO_EXPIRED:
    case GIMP_UNDO_EVENT_REDO_EXPIRED:
      gimp_container_remove (editor->container, GIMP_OBJECT (undo));
      break;

    case GIMP_UNDO_EVENT_UNDO:
    case GIMP_UNDO_EVENT_REDO:
      g_signal_handlers_block_by_func (editor->view,
                                       gimp_undo_editor_select_item,
                                       editor);

      if (top_undo_item)
        {
          gimp_container_view_select_item (GIMP_CONTAINER_VIEW (editor->view),
                                           GIMP_VIEWABLE (top_undo_item));
          gimp_undo_create_preview (top_undo_item, FALSE);
        }
      else
        {
          gimp_container_view_select_item (GIMP_CONTAINER_VIEW (editor->view),
                                           GIMP_VIEWABLE (editor->base_item));
          gimp_undo_create_preview (editor->base_item, TRUE);
        }

      g_signal_handlers_unblock_by_func (editor->view,
                                         gimp_undo_editor_select_item,
                                         editor);
      break;

    case GIMP_UNDO_EVENT_UNDO_FREE:
      if (gimp_image_undo_is_enabled (gimage))
        gimp_undo_editor_clear (editor);
      break;

    case GIMP_UNDO_EVENT_UNDO_FREEZE:
      gimp_undo_editor_clear (editor);
      break;

    case GIMP_UNDO_EVENT_UNDO_THAW:
      gimp_undo_editor_fill (editor);
      break;
    }
}

static void
gimp_undo_editor_select_item (GimpContainerView *view,
                              GimpUndo          *undo,
                              gpointer           insert_data,
                              GimpUndoEditor    *editor)
{
  GimpImage *gimage;
  GimpUndo  *top_undo_item;

  if (! undo)
    return;

  gimage = GIMP_IMAGE_EDITOR (editor)->gimage;

  top_undo_item = gimp_undo_stack_peek (gimage->undo_stack);

  if (undo == editor->base_item)
    {
      /*  the base_item was selected, pop all available undo items
       */
      while (top_undo_item != NULL)
        {
          gimp_image_undo (gimage);

          top_undo_item = gimp_undo_stack_peek (gimage->undo_stack);
        }
    }
  else if (gimp_container_have (gimage->undo_stack->undos, GIMP_OBJECT (undo)))
    {
      /*  the selected item is on the undo stack, pop undos until it
       *  is on top of the undo stack
       */
      while (top_undo_item != undo)
        {
          gimp_image_undo (gimage);

          top_undo_item = gimp_undo_stack_peek (gimage->undo_stack);
        }
    }
  else if (gimp_container_have (gimage->redo_stack->undos, GIMP_OBJECT (undo)))
    {
      /*  the selected item is on the redo stack, pop redos until it
       *  is on top of the undo stack
       */
      while (top_undo_item != undo)
        {
          gimp_image_redo (gimage);

          top_undo_item = gimp_undo_stack_peek (gimage->undo_stack);
        }
    }

  gimp_image_flush (gimage);
}
