/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpdrawable_pdb.c
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* NOTE: This file is autogenerated by pdbgen.pl */

#include "config.h"

#include <string.h>

#include "gimp.h"

/**
 * gimp_drawable_merge_shadow:
 * @drawable_ID: The drawable.
 * @undo: Push merge to undo stack?
 *
 * Merge the shadow buffer with the specified drawable.
 *
 * This procedure combines the contents of the image's shadow buffer
 * (for temporary processing) with the specified drawable. The \"undo\"
 * parameter specifies whether to add an undo step for the operation.
 * Requesting no undo is useful for such applications as 'auto-apply'.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_drawable_merge_shadow (gint32   drawable_ID,
			    gboolean undo)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_drawable_merge_shadow",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_INT32, undo,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_drawable_fill:
 * @drawable_ID: The drawable.
 * @fill_type: The type of fill.
 *
 * Fill the drawable with the specified fill mode.
 *
 * This procedure fills the drawable with the fill mode. If the fill
 * mode is foreground the current foreground color is used. If the fill
 * mode is background, the current background color is used. If the
 * fill type is white, then white is used. Transparent fill only
 * affects layers with an alpha channel, in which case the alpha
 * channel is set to transparent. If the drawable has no alpha channel,
 * it is filled to white. No fill leaves the drawable's contents
 * undefined. This procedure is unlike the bucket fill tool because it
 * fills regardless of a selection
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_drawable_fill (gint32       drawable_ID,
		    GimpFillType fill_type)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_drawable_fill",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_INT32, fill_type,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_drawable_update:
 * @drawable_ID: The drawable.
 * @x: x coordinate of upper left corner of update region.
 * @y: y coordinate of upper left corner of update region.
 * @width: Width of update region.
 * @height: Height of update region.
 *
 * Update the specified region of the drawable.
 *
 * This procedure updates the specified region of the drawable. The (x,
 * y) coordinate pair is relative to the drawable's origin, not to the
 * image origin. Therefore, the entire drawable can be updated with:
 * {x-&gt;0, y-&gt;0, w-&gt;width, h-&gt;height }.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_drawable_update (gint32 drawable_ID,
		      gint   x,
		      gint   y,
		      gint   width,
		      gint   height)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_drawable_update",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_INT32, x,
				    GIMP_PDB_INT32, y,
				    GIMP_PDB_INT32, width,
				    GIMP_PDB_INT32, height,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_drawable_mask_bounds:
 * @drawable_ID: The drawable.
 * @x1: x coordinate of the upper left corner of selection bounds.
 * @y1: y coordinate of the upper left corner of selection bounds.
 * @x2: x coordinate of the lower right corner of selection bounds.
 * @y2: y coordinate of the lower right corner of selection bounds.
 *
 * Find the bounding box of the current selection in relation to the
 * specified drawable.
 *
 * This procedure returns the whether there is a selection. If there is
 * one, the upper left and lower righthand corners of its bounding box
 * are returned. These coordinates are specified relative to the
 * drawable's origin, and bounded by the drawable's extents. Please
 * note that the pixel specified by the lower righthand coordinate of
 * the bounding box is not part of the selection. The selection ends at
 * the upper left corner of this pixel. This means the width of the
 * selection can be calculated as (x2 - x1), its height as (y2 - y1).
 *
 * Returns: TRUE if there is a selection.
 */
gboolean
gimp_drawable_mask_bounds (gint32  drawable_ID,
			   gint   *x1,
			   gint   *y1,
			   gint   *x2,
			   gint   *y2)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean non_empty = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_mask_bounds",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      non_empty = return_vals[1].data.d_int32;
      *x1 = return_vals[2].data.d_int32;
      *y1 = return_vals[3].data.d_int32;
      *x2 = return_vals[4].data.d_int32;
      *y2 = return_vals[5].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return non_empty;
}

/**
 * gimp_drawable_image:
 * @drawable_ID: The drawable.
 *
 * Returns the drawable's image.
 *
 * This procedure returns the drawable's image.
 *
 * Returns: The drawable's image.
 */
gint32
gimp_drawable_image (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 image_ID = -1;

  return_vals = gimp_run_procedure ("gimp_drawable_image",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    image_ID = return_vals[1].data.d_image;

  gimp_destroy_params (return_vals, nreturn_vals);

  return image_ID;
}

/**
 * gimp_drawable_type:
 * @drawable_ID: The drawable.
 *
 * Returns the drawable's type.
 *
 * This procedure returns the drawable's type.
 *
 * Returns: The drawable's type.
 */
GimpImageType
gimp_drawable_type (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  GimpImageType type = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_type",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    type = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return type;
}

/**
 * gimp_drawable_has_alpha:
 * @drawable_ID: The drawable.
 *
 * Returns non-zero if the drawable has an alpha channel.
 *
 * This procedure returns whether the specified drawable has an alpha
 * channel. This can only be true for layers, and the associated type
 * will be one of: { RGBA , GRAYA, INDEXEDA }.
 *
 * Returns: Does the drawable have an alpha channel?
 */
gboolean
gimp_drawable_has_alpha (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean has_alpha = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_has_alpha",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    has_alpha = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return has_alpha;
}

/**
 * gimp_drawable_type_with_alpha:
 * @drawable_ID: The drawable.
 *
 * Returns the drawable's type with alpha.
 *
 * This procedure returns the drawable's type if an alpha channel were
 * added. If the type is currently Gray, for instance, the returned
 * type would be GrayA. If the drawable already has an alpha channel,
 * the drawable's type is simply returned.
 *
 * Returns: The drawable's type with alpha.
 */
GimpImageType
gimp_drawable_type_with_alpha (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  GimpImageType type_with_alpha = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_type_with_alpha",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    type_with_alpha = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return type_with_alpha;
}

/**
 * gimp_drawable_is_rgb:
 * @drawable_ID: The drawable.
 *
 * Returns whether the drawable is an RGB type.
 *
 * This procedure returns non-zero if the specified drawable is of type
 * { RGB, RGBA }.
 *
 * Returns: non-zero if the drawable is an RGB type.
 */
gboolean
gimp_drawable_is_rgb (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean is_rgb = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_rgb",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    is_rgb = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return is_rgb;
}

/**
 * gimp_drawable_is_gray:
 * @drawable_ID: The drawable.
 *
 * Returns whether the drawable is a grayscale type.
 *
 * This procedure returns non-zero if the specified drawable is of type
 * { Gray, GrayA }.
 *
 * Returns: non-zero if the drawable is a grayscale type.
 */
gboolean
gimp_drawable_is_gray (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean is_gray = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_gray",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    is_gray = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return is_gray;
}

/**
 * gimp_drawable_is_indexed:
 * @drawable_ID: The drawable.
 *
 * Returns whether the drawable is an indexed type.
 *
 * This procedure returns non-zero if the specified drawable is of type
 * { Indexed, IndexedA }.
 *
 * Returns: non-zero if the drawable is an indexed type.
 */
gboolean
gimp_drawable_is_indexed (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean is_indexed = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_indexed",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    is_indexed = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return is_indexed;
}

/**
 * gimp_drawable_bytes:
 * @drawable_ID: The drawable.
 *
 * Returns the bytes per pixel.
 *
 * This procedure returns the number of bytes per pixel (or the number
 * of channels) for the specified drawable.
 *
 * Returns: Bytes per pixel.
 */
gint
gimp_drawable_bytes (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint bytes = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_bytes",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    bytes = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return bytes;
}

/**
 * gimp_drawable_width:
 * @drawable_ID: The drawable.
 *
 * Returns the width of the drawable.
 *
 * This procedure returns the specified drawable's width in pixels.
 *
 * Returns: Width of drawable.
 */
gint
gimp_drawable_width (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint width = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_width",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    width = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return width;
}

/**
 * gimp_drawable_height:
 * @drawable_ID: The drawable.
 *
 * Returns the height of the drawable.
 *
 * This procedure returns the specified drawable's height in pixels.
 *
 * Returns: Height of drawable.
 */
gint
gimp_drawable_height (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint height = 0;

  return_vals = gimp_run_procedure ("gimp_drawable_height",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    height = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return height;
}

/**
 * gimp_drawable_offsets:
 * @drawable_ID: The drawable.
 * @offset_x: x offset of drawable.
 * @offset_y: y offset of drawable.
 *
 * Returns the offsets for the drawable.
 *
 * This procedure returns the specified drawable's offsets. This only
 * makes sense if the drawable is a layer since channels are anchored.
 * The offsets of a channel will be returned as 0.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_drawable_offsets (gint32  drawable_ID,
		       gint   *offset_x,
		       gint   *offset_y)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_drawable_offsets",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  *offset_x = 0;
  *offset_y = 0;

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  if (success)
    {
      *offset_x = return_vals[1].data.d_int32;
      *offset_y = return_vals[2].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_drawable_is_layer:
 * @drawable_ID: The drawable.
 *
 * Returns whether the drawable is a layer.
 *
 * This procedure returns non-zero if the specified drawable is a
 * layer.
 *
 * Returns: Non-zero if the drawable is a layer.
 */
gboolean
gimp_drawable_is_layer (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean layer = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_layer",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer;
}

/**
 * gimp_drawable_is_layer_mask:
 * @drawable_ID: The drawable.
 *
 * Returns whether the drawable is a layer mask.
 *
 * This procedure returns non-zero if the specified drawable is a layer
 * mask.
 *
 * Returns: Non-zero if the drawable is a layer mask.
 */
gboolean
gimp_drawable_is_layer_mask (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean layer_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_layer_mask",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_mask;
}

/**
 * gimp_drawable_is_channel:
 * @drawable_ID: The drawable.
 *
 * Returns whether the drawable is a channel.
 *
 * This procedure returns non-zero if the specified drawable is a
 * channel.
 *
 * Returns: Non-zero if the drawable is a channel.
 */
gboolean
gimp_drawable_is_channel (gint32 drawable_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean channel = FALSE;

  return_vals = gimp_run_procedure ("gimp_drawable_is_channel",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    channel = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return channel;
}

/**
 * _gimp_drawable_thumbnail:
 * @drawable_ID: The drawable.
 * @width: The thumbnail width.
 * @height: The thumbnail height.
 * @ret_width: The previews width.
 * @ret_height: The previews height.
 * @bpp: The previews bpp.
 * @thumbnail_data_count: The number of pixels in thumbnail data.
 * @thumbnail_data: The thumbnail data.
 *
 * Get a thumbnail of a drawable.
 *
 * This function gets data from which a thumbnail of a drawable preview
 * can be created. Maximum x or y dimension is 128 pixels. The pixels
 * are returned in the RGB[A] format. The bpp return value gives the
 * number of bytes in the image. The alpha channel is also returned if
 * the drawable has one.
 *
 * Returns: TRUE on success.
 */
gboolean
_gimp_drawable_thumbnail (gint32   drawable_ID,
			  gint     width,
			  gint     height,
			  gint    *ret_width,
			  gint    *ret_height,
			  gint    *bpp,
			  gint    *thumbnail_data_count,
			  guint8 **thumbnail_data)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_drawable_thumbnail",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_INT32, width,
				    GIMP_PDB_INT32, height,
				    GIMP_PDB_END);

  *ret_width = 0;
  *ret_height = 0;
  *bpp = 0;
  *thumbnail_data_count = 0;
  *thumbnail_data = NULL;

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  if (success)
    {
      *ret_width = return_vals[1].data.d_int32;
      *ret_height = return_vals[2].data.d_int32;
      *bpp = return_vals[3].data.d_int32;
      *thumbnail_data_count = return_vals[4].data.d_int32;
      *thumbnail_data = g_new (guint8, *thumbnail_data_count);
      memcpy (*thumbnail_data, return_vals[5].data.d_int8array,
	      *thumbnail_data_count * sizeof (guint8));
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_drawable_offset:
 * @drawable_ID: The drawable to offset.
 * @wrap_around: wrap image around or fill vacated regions.
 * @fill_type: fill vacated regions of drawable with background or transparent.
 * @offset_x: offset by this amount in X direction.
 * @offset_y: offset by this amount in Y direction.
 *
 * Offset the drawable by the specified amounts in the X and Y
 * directions
 *
 * This procedure offsets the specified drawable by the amounts
 * specified by 'offset_x' and 'offset_y'. If 'wrap_around' is set to
 * TRUE, then portions of the drawable which are offset out of bounds
 * are wrapped around. Alternatively, the undefined regions of the
 * drawable can be filled with transparency or the background color, as
 * specified by the 'fill_type' parameter.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_drawable_offset (gint32         drawable_ID,
		      gboolean       wrap_around,
		      GimpOffsetType fill_type,
		      gint           offset_x,
		      gint           offset_y)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_drawable_offset",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_INT32, wrap_around,
				    GIMP_PDB_INT32, fill_type,
				    GIMP_PDB_INT32, offset_x,
				    GIMP_PDB_INT32, offset_y,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
