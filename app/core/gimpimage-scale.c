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

#include <glib-object.h>

#include "core-types.h"

#include "base/tile-manager.h"

#include "gimp.h"
#include "gimpimage.h"
#include "gimpimage-guides.h"
#include "gimpimage-scale.h"
#include "gimpimage-undo.h"
#include "gimpimage-undo-push.h"
#include "gimplayer.h"
#include "gimplayer-floating-sel.h"
#include "gimplist.h"

#include "config/gimpguiconfig.h"

#include "gimp-intl.h"


void
gimp_image_scale (GimpImage             *gimage,
		  gint                   new_width,
		  gint                   new_height,
                  GimpInterpolationType  interpolation_type,
                  GimpProgressFunc       progress_func,
                  gpointer               progress_data)
{
  GimpLayer *floating_layer;
  GimpItem  *item;
  GList     *list;
  GList     *remove = NULL;
  gint       old_width;
  gint       old_height;
  gdouble    img_scale_w = 1.0;
  gdouble    img_scale_h = 1.0;
  gint       progress_max;
  gint       progress_current = 1;

  g_return_if_fail (GIMP_IS_IMAGE (gimage));
  g_return_if_fail (new_width > 0 && new_height > 0);

  gimp_set_busy (gimage->gimp);

  progress_max = (gimage->channels->num_children +
                  gimage->layers->num_children   +
                  gimage->vectors->num_children  +
                  1 /* selection */);

  /*  Get the floating layer if one exists  */
  floating_layer = gimp_image_floating_sel (gimage);

  gimp_image_undo_group_start (gimage, GIMP_UNDO_GROUP_IMAGE_SCALE,
                               _("Scale Image"));

  /*  Relax the floating selection  */
  if (floating_layer)
    floating_sel_relax (floating_layer, TRUE);

  /*  Push the image size to the stack  */
  gimp_image_undo_push_image_size (gimage, NULL);

  /*  Set the new width and height  */

  old_width      = gimage->width;
  old_height     = gimage->height;
  gimage->width  = new_width;
  gimage->height = new_height;
  img_scale_w    = (gdouble) new_width  / (gdouble) old_width;
  img_scale_h    = (gdouble) new_height / (gdouble) old_height;

  /*  Scale all channels  */
  for (list = GIMP_LIST (gimage->channels)->list;
       list;
       list = g_list_next (list))
    {
      item = (GimpItem *) list->data;

      gimp_item_scale (item, new_width, new_height, 0, 0, interpolation_type);

      if (progress_func)
        (* progress_func) (0, progress_max, progress_current++, progress_data);
    }

  /*  Scale all vectors  */
  for (list = GIMP_LIST (gimage->vectors)->list;
       list;
       list = g_list_next (list))
    {
      item = (GimpItem *) list->data;

      gimp_item_scale (item, new_width, new_height, 0, 0, interpolation_type);

      if (progress_func)
        (* progress_func) (0, progress_max, progress_current++, progress_data);
    }

  /*  Don't forget the selection mask!  */
  gimp_item_scale (GIMP_ITEM (gimp_image_get_mask (gimage)),
                   new_width, new_height,
                   0, 0, interpolation_type);

  if (progress_func)
    (* progress_func) (0, progress_max, progress_current++, progress_data);

  /*  Scale all layers  */
  for (list = GIMP_LIST (gimage->layers)->list;
       list;
       list = g_list_next (list))
    {
      item = (GimpItem *) list->data;

      if (! gimp_item_scale_by_factors (item, img_scale_w, img_scale_h,
                                        interpolation_type))
	{
	  /* Since 0 < img_scale_w, img_scale_h, failure due to one or more
	   * vanishing scaled layer dimensions. Implicit delete implemented
	   * here. Upstream warning implemented in resize_check_layer_scaling(),
	   * which offers the user the chance to bail out.
	   */
          remove = g_list_prepend (remove, item);
        }

      if (progress_func)
        (* progress_func) (0, progress_max, progress_current++, progress_data);
    }

  /* We defer removing layers lost to scaling until now so as not to mix
   * the operations of iterating over and removal from gimage->layers.
   */
  remove = g_list_reverse (remove);

  for (list = remove; list; list = g_list_next (list))
    {
      GimpLayer *layer = list->data;

      gimp_image_remove_layer (gimage, layer);
    }

  g_list_free (remove);

  /*  Scale all Guides  */
  for (list = gimage->guides; list; list = g_list_next (list))
    {
      GimpGuide *guide = list->data;

      switch (guide->orientation)
	{
	case GIMP_ORIENTATION_HORIZONTAL:
	  gimp_image_undo_push_image_guide (gimage, NULL, guide);
	  guide->position = (guide->position * new_height) / old_height;
	  break;

	case GIMP_ORIENTATION_VERTICAL:
	  gimp_image_undo_push_image_guide (gimage, NULL, guide);
	  guide->position = (guide->position * new_width) / old_width;
	  break;

	default:
          break;
	}
    }

  /*  Rigor the floating selection  */
  if (floating_layer)
    floating_sel_rigor (floating_layer, TRUE);

  gimp_image_undo_group_end (gimage);

  gimp_viewable_size_changed (GIMP_VIEWABLE (gimage));

  gimp_unset_busy (gimage->gimp);
}

/**
 * gimp_image_scale_check:
 * @gimage:     A #GimpImage.
 * @new_width:  The new width.
 * @new_height: The new height.
 *
 * Inventory the layer list in gimage and check that it may be
 * scaled to @new_height and @new_width without problems.
 *
 * Return value: #GIMP_IMAGE_SCALE_OK if scaling the image will shrink none
 *               of its layers completely away, and the new image size
 *               is smaller than the maximum specified in the
 *               preferences.
 *               #GIMP_IMAGE_SCALE_TOO_SMALL if scaling would remove some
 *               existing layers.
 *               #GIMP_IMAGE_SCALE_TOO_BIG if the new image size would
 *               exceed the maximum specified in the preferences.
 **/
GimpImageScaleCheckType
gimp_image_scale_check (const GimpImage *gimage,
                        gint             new_width,
                        gint             new_height,
                        gint64          *new_memsize)
{
  GList  *list;
  gint64  current_size;
  gint64  fixed_size;
  gint64  undo_size;
  gint64  redo_size;
  gint64  scaled_size;
  gint64  new_size;

  g_return_val_if_fail (GIMP_IS_IMAGE (gimage), GIMP_IMAGE_SCALE_TOO_SMALL);
  g_return_val_if_fail (new_memsize != NULL, GIMP_IMAGE_SCALE_TOO_SMALL);

  current_size = gimp_object_get_memsize (GIMP_OBJECT (gimage), NULL);

  scaled_size =
    gimp_object_get_memsize (GIMP_OBJECT (gimage->layers), NULL)         +
    gimp_object_get_memsize (GIMP_OBJECT (gimage->channels), NULL)       +
    gimp_object_get_memsize (GIMP_OBJECT (gimage->selection_mask), NULL) +
    tile_manager_get_memsize (gimage->projection);

  undo_size = gimp_object_get_memsize (GIMP_OBJECT (gimage->undo_stack), NULL);
  redo_size = gimp_object_get_memsize (GIMP_OBJECT (gimage->redo_stack), NULL);

  fixed_size = current_size - scaled_size - undo_size - redo_size;

  new_size = (fixed_size  +
              scaled_size *
              ((gdouble) new_width  / gimp_image_get_width  (gimage)) *
              ((gdouble) new_height / gimp_image_get_height (gimage)));

  if (undo_size + scaled_size < gimage->gimp->config->undo_size)
    {
      new_size += undo_size + scaled_size;
    }
  else if (scaled_size < gimage->gimp->config->undo_size)
    {
      new_size += gimage->gimp->config->undo_size;
    }
  else
    {
      new_size += scaled_size;
    }

  *new_memsize = new_size;

  if (new_size > current_size &&
      new_size > GIMP_GUI_CONFIG (gimage->gimp->config)->max_new_image_size)
    return GIMP_IMAGE_SCALE_TOO_BIG;

  for (list = GIMP_LIST (gimage->layers)->list;
       list;
       list = g_list_next (list))
    {
      GimpItem *item = list->data;

      if (! gimp_item_check_scaling (item, new_width, new_height))
	return GIMP_IMAGE_SCALE_TOO_SMALL;
    }

  return GIMP_IMAGE_SCALE_OK;
}
