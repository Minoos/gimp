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

#include "libgimpcolor/gimpcolor.h"

#include "core-types.h"

#include "gimpchannel.h"
#include "gimpchannel-select.h"
#include "gimpchannel-combine.h"
#include "gimpimage-contiguous-region.h"
#include "gimplayer.h"
#include "gimpscanconvert.h"

#include "vectors/gimpstroke.h"
#include "vectors/gimpvectors.h"

#include "gimp-intl.h"


/*  basic selection functions  */

void
gimp_channel_select_rectangle (GimpChannel    *channel,
                               gint            x,
                               gint            y,
                               gint            w,
                               gint            h,
                               GimpChannelOps  op,
                               gboolean        feather,
                               gdouble         feather_radius_x,
                               gdouble         feather_radius_y)
{
  g_return_if_fail (GIMP_IS_CHANNEL (channel));

  gimp_channel_push_undo (channel, _("Rect Select"));

  /*  if applicable, replace the current selection  */
  if (op == GIMP_CHANNEL_OP_REPLACE)
    gimp_channel_clear (channel, NULL, FALSE);

  /*  if feathering for rect, make a new mask with the
   *  rectangle and feather that with the old mask
   */
  if (feather || op == GIMP_CHANNEL_OP_INTERSECT)
    {
      GimpItem    *item;
      GimpChannel *add_on;

      item = GIMP_ITEM (channel);

      add_on = gimp_channel_new_mask (gimp_item_get_image (item),
                                      gimp_item_width (item),
                                      gimp_item_height (item));
      gimp_channel_combine_rect (add_on, GIMP_CHANNEL_OP_ADD, x, y, w, h);

      if (feather)
        gimp_channel_feather (add_on,
                              feather_radius_x,
                              feather_radius_y,
                              FALSE /* no undo */);

      gimp_channel_combine_mask (channel, add_on, op, 0, 0);
      g_object_unref (add_on);
    }
  else
    {
      gimp_channel_combine_rect (channel, op, x, y, w, h);
    }
}

void
gimp_channel_select_ellipse (GimpChannel    *channel,
                             gint            x,
                             gint            y,
                             gint            w,
                             gint            h,
                             GimpChannelOps  op,
                             gboolean        antialias,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y)
{
  g_return_if_fail (GIMP_IS_CHANNEL (channel));

  gimp_channel_push_undo (channel, _("Ellipse Select"));

  /*  if applicable, replace the current selection  */
  if (op == GIMP_CHANNEL_OP_REPLACE)
    gimp_channel_clear (channel, NULL, FALSE);

  /*  if feathering for rect, make a new mask with the
   *  rectangle and feather that with the old mask
   */
  if (feather || op == GIMP_CHANNEL_OP_INTERSECT)
    {
      GimpItem    *item;
      GimpChannel *add_on;

      item = GIMP_ITEM (channel);

      add_on = gimp_channel_new_mask (gimp_item_get_image (item),
                                      gimp_item_width (item),
                                      gimp_item_height (item));
      gimp_channel_combine_ellipse (add_on, GIMP_CHANNEL_OP_ADD,
                                    x, y, w, h, antialias);

      if (feather)
        gimp_channel_feather (add_on,
                              feather_radius_x,
                              feather_radius_y,
                              FALSE /* no undo */);

      gimp_channel_combine_mask (channel, add_on, op, 0, 0);
      g_object_unref (add_on);
    }
  else
    {
      gimp_channel_combine_ellipse (channel, op, x, y, w, h, antialias);
    }
}


/*  select by GimpScanConvert functions  */

void
gimp_channel_select_scan_convert (GimpChannel     *channel,
                                  const gchar     *undo_desc,
                                  GimpScanConvert *scan_convert,
                                  gint             offset_x,
                                  gint             offset_y,
                                  GimpChannelOps   op,
                                  gboolean         antialias,
                                  gboolean         feather,
                                  gdouble          feather_radius_x,
                                  gdouble          feather_radius_y)
{
  GimpItem    *item;
  GimpChannel *add_on;

  g_return_if_fail (GIMP_IS_CHANNEL (channel));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (scan_convert != NULL);

  gimp_channel_push_undo (channel, undo_desc);

  /*  if applicable, replace the current selection  */
  if (op == GIMP_CHANNEL_OP_REPLACE)
    gimp_channel_clear (channel, NULL, FALSE);

  item = GIMP_ITEM (channel);

  add_on = gimp_channel_new_mask (gimp_item_get_image (item),
                                  gimp_item_width (item),
                                  gimp_item_height (item));
  gimp_scan_convert_render (scan_convert,
                            gimp_drawable_data (GIMP_DRAWABLE (add_on)),
                            offset_x, offset_y, antialias);

  if (feather)
    gimp_channel_feather (add_on,
                          feather_radius_x,
                          feather_radius_y,
                          FALSE /* no undo */);

  gimp_channel_combine_mask (channel, add_on, op, 0, 0);
  g_object_unref (add_on);
}

void
gimp_channel_select_polygon (GimpChannel    *channel,
                             const gchar    *undo_desc,
                             gint            n_points,
                             GimpVector2    *points,
                             GimpChannelOps  op,
                             gboolean        antialias,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y)
{
  GimpScanConvert *scan_convert;

  g_return_if_fail (GIMP_IS_CHANNEL (channel));
  g_return_if_fail (undo_desc != NULL);

  scan_convert = gimp_scan_convert_new ();

  gimp_scan_convert_add_polyline (scan_convert, n_points, points, TRUE);

  gimp_channel_select_scan_convert (channel, undo_desc, scan_convert, 0, 0,
                                    op, antialias, feather,
                                    feather_radius_x, feather_radius_y);

  gimp_scan_convert_free (scan_convert);
}

void
gimp_channel_select_vectors (GimpChannel    *channel,
                             const gchar    *undo_desc,
                             GimpVectors    *vectors,
                             GimpChannelOps  op,
                             gboolean        antialias,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y)
{
  GimpScanConvert *scan_convert;
  GList           *stroke;
  gboolean         coords_added = FALSE;

  g_return_if_fail (GIMP_IS_CHANNEL (channel));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (GIMP_IS_VECTORS (vectors));

  scan_convert = gimp_scan_convert_new ();

  for (stroke = vectors->strokes; stroke; stroke = stroke->next)
    {
      GArray   *coords;
      gboolean  closed;

      coords = gimp_stroke_interpolate (GIMP_STROKE (stroke->data),
                                        1.0, &closed);

      if (coords && coords->len)
        {
          GimpVector2 *points;
          gint         i;

          points = g_new0 (GimpVector2, coords->len);

          for (i = 0; i < coords->len; i++)
            {
              points[i].x = g_array_index (coords, GimpCoords, i).x;
              points[i].y = g_array_index (coords, GimpCoords, i).y;
            }

          gimp_scan_convert_add_polyline (scan_convert, coords->len,
                                          points, TRUE);
          coords_added = TRUE;

          g_free (points);
        }

      if (coords)
        g_array_free (coords, TRUE);
    }

  if (coords_added)
    gimp_channel_select_scan_convert (channel, undo_desc, scan_convert, 0, 0,
                                      op, antialias, feather,
                                      feather_radius_x, feather_radius_y);

  gimp_scan_convert_free (scan_convert);
}


/*  select by GimpChannel functions  */

void
gimp_channel_select_channel (GimpChannel    *channel,
                             const gchar    *undo_desc,
                             GimpChannel    *add_on,
                             gint            offset_x,
                             gint            offset_y,
                             GimpChannelOps  op,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y)
{
  g_return_if_fail (GIMP_IS_CHANNEL (channel));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (GIMP_IS_CHANNEL (add_on));

  gimp_channel_push_undo (channel, undo_desc);

  /*  if applicable, replace the current selection  */
  if (op == GIMP_CHANNEL_OP_REPLACE)
    gimp_channel_clear (channel, NULL, FALSE);

  if (feather || op == GIMP_CHANNEL_OP_INTERSECT)
    {
      GimpItem    *item;
      GimpChannel *add_on2;

      item = GIMP_ITEM (channel);

      add_on2 = gimp_channel_new_mask (gimp_item_get_image (item),
                                       gimp_item_width (item),
                                       gimp_item_height (item));

      gimp_channel_combine_mask (add_on2, add_on, GIMP_CHANNEL_OP_ADD,
                                 offset_x, offset_y);

      if (feather)
        gimp_channel_feather (add_on2,
                              feather_radius_x,
                              feather_radius_y,
                              FALSE /* no undo */);

      gimp_channel_combine_mask (channel, add_on2, op, 0, 0);
      g_object_unref (add_on2);
    }
  else
    {
      gimp_channel_combine_mask (channel, add_on, op, offset_x, offset_y);
    }
}

void
gimp_channel_select_alpha (GimpChannel    *channel,
                           GimpLayer      *layer,
                           GimpChannelOps  op,
                           gboolean        feather,
                           gdouble         feather_radius_x,
                           gdouble         feather_radius_y)
{
  GimpItem    *item;
  GimpChannel *add_on;
  gint         off_x, off_y;

  g_return_if_fail (GIMP_IS_CHANNEL (channel));
  g_return_if_fail (GIMP_IS_LAYER (layer));

  item = GIMP_ITEM (channel);

  if (gimp_drawable_has_alpha (GIMP_DRAWABLE (layer)))
    {
      GimpRGB color;

      gimp_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);

      add_on = gimp_channel_new_from_alpha (gimp_item_get_image (item),
                                            layer, NULL, &color);
    }
  else
    {
      /*  no alpha is equivalent to completely opaque alpha,
       *  so simply select the whole layer's extents.  --mitch
       */
      add_on = gimp_channel_new_mask (gimp_item_get_image (item),
                                      gimp_item_width (GIMP_ITEM (layer)),
                                      gimp_item_height (GIMP_ITEM (layer)));
      gimp_channel_all (add_on, FALSE);
    }

  gimp_item_offsets (GIMP_ITEM (layer), &off_x, &off_y);

  gimp_channel_select_channel (channel, _("Alpha to Selection"), add_on,
                               off_x, off_y,
                               op, feather,
                               feather_radius_x,
                               feather_radius_y);
  g_object_unref (add_on);
}

void
gimp_channel_select_component (GimpChannel     *channel,
                               GimpChannelType  component,
                               GimpChannelOps   op,
                               gboolean         feather,
                               gdouble          feather_radius_x,
                               gdouble          feather_radius_y)
{
  GimpItem    *item;
  GimpChannel *add_on;
  GimpRGB      color;
  GEnumClass  *enum_class;
  GEnumValue  *enum_value;
  gchar       *undo_desc;

  g_return_if_fail (GIMP_IS_CHANNEL (channel));

  gimp_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);

  item = GIMP_ITEM (channel);

  add_on = gimp_channel_new_from_component (gimp_item_get_image (item),
                                            component, NULL, &color);

  if (feather)
    gimp_channel_feather (add_on,
                          feather_radius_x,
                          feather_radius_y,
                          FALSE /* no undo */);

  enum_class = g_type_class_ref (GIMP_TYPE_CHANNEL_TYPE);
  enum_value = g_enum_get_value (enum_class, component);
  g_type_class_unref (enum_class);

  undo_desc = g_strdup_printf (_("%s Channel to Selection"),
                               gettext (enum_value->value_name));

  gimp_channel_select_channel (channel, undo_desc, add_on,
                               0, 0, op,
                               FALSE, 0.0, 0.0);

  g_free (undo_desc);
  g_object_unref (add_on);
}

void
gimp_channel_select_fuzzy (GimpChannel    *channel,
                           GimpDrawable   *drawable,
                           gboolean        sample_merged,
                           gint            x,
                           gint            y,
                           gint            threshold,
                           gboolean        select_transparent,
                           GimpChannelOps  op,
                           gboolean        antialias,
                           gboolean        feather,
                           gdouble         feather_radius_x,
                           gdouble         feather_radius_y)
{
  GimpItem    *item;
  GimpChannel *add_on;
  gint         add_on_x = 0;
  gint         add_on_y = 0;

  g_return_if_fail (GIMP_IS_CHANNEL (channel));
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));

  item = GIMP_ITEM (channel);

  add_on = gimp_image_contiguous_region_by_seed (gimp_item_get_image (item),
                                                 drawable,
                                                 sample_merged,
                                                 antialias,
                                                 threshold,
                                                 select_transparent,
                                                 x, y);

  if (! sample_merged)
    gimp_item_offsets (GIMP_ITEM (drawable), &add_on_x, &add_on_y);

  gimp_channel_select_channel (channel, _("Fuzzy Select"),
                               add_on, add_on_x, add_on_y,
                               op,
                               feather,
                               feather_radius_x,
                               feather_radius_y);
  g_object_unref (add_on);
}

void
gimp_channel_select_by_color (GimpChannel    *channel,
                              GimpDrawable   *drawable,
                              gboolean        sample_merged,
                              const GimpRGB  *color,
                              gint            threshold,
                              gboolean        select_transparent,
                              GimpChannelOps  op,
                              gboolean        antialias,
                              gboolean        feather,
                              gdouble         feather_radius_x,
                              gdouble         feather_radius_y)
{
  GimpItem    *item;
  GimpChannel *add_on;
  gint         add_on_x = 0;
  gint         add_on_y = 0;

  g_return_if_fail (GIMP_IS_CHANNEL (channel));
  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));
  g_return_if_fail (color != NULL);

  item = GIMP_ITEM (channel);

  add_on = gimp_image_contiguous_region_by_color (gimp_item_get_image (item),
                                                  drawable,
                                                  sample_merged,
                                                  antialias,
                                                  threshold,
                                                  select_transparent,
                                                  color);

  if (! sample_merged)
    gimp_item_offsets (GIMP_ITEM (drawable), &add_on_x, &add_on_y);

  gimp_channel_select_channel (channel, _("Select by Color"),
                               add_on, add_on_x, add_on_y,
                               op,
                               feather,
                               feather_radius_x,
                               feather_radius_y);
  g_object_unref (add_on);
}
