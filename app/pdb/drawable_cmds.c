/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2000 Spencer Kimball and Peter Mattis
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

/* NOTE: This file is autogenerated by pdbgen.pl. */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "core/core-types.h"
#include "procedural_db.h"

#include "appenums.h"
#include "core/core-types.h"
#include "core/gimpchannel.h"
#include "core/gimpdrawable-offset.h"
#include "core/gimpdrawable.h"
#include "core/gimpimage.h"
#include "core/gimplayer.h"
#include "core/gimplayermask.h"
#include "drawable.h"
#include "pdb_glue.h"
#include "temp_buf.h"
#include "tile.h"
#include "tile_manager.h"

static ProcRecord drawable_merge_shadow_proc;
static ProcRecord drawable_fill_proc;
static ProcRecord drawable_update_proc;
static ProcRecord drawable_mask_bounds_proc;
static ProcRecord drawable_image_proc;
static ProcRecord drawable_type_proc;
static ProcRecord drawable_has_alpha_proc;
static ProcRecord drawable_type_with_alpha_proc;
static ProcRecord drawable_is_rgb_proc;
static ProcRecord drawable_is_gray_proc;
static ProcRecord drawable_is_indexed_proc;
static ProcRecord drawable_bytes_proc;
static ProcRecord drawable_width_proc;
static ProcRecord drawable_height_proc;
static ProcRecord drawable_offsets_proc;
static ProcRecord drawable_is_layer_proc;
static ProcRecord drawable_is_layer_mask_proc;
static ProcRecord drawable_is_channel_proc;
static ProcRecord drawable_get_pixel_proc;
static ProcRecord drawable_set_pixel_proc;
static ProcRecord drawable_set_image_proc;
static ProcRecord drawable_thumbnail_proc;
static ProcRecord drawable_offset_proc;

void
register_drawable_procs (void)
{
  procedural_db_register (&drawable_merge_shadow_proc);
  procedural_db_register (&drawable_fill_proc);
  procedural_db_register (&drawable_update_proc);
  procedural_db_register (&drawable_mask_bounds_proc);
  procedural_db_register (&drawable_image_proc);
  procedural_db_register (&drawable_type_proc);
  procedural_db_register (&drawable_has_alpha_proc);
  procedural_db_register (&drawable_type_with_alpha_proc);
  procedural_db_register (&drawable_is_rgb_proc);
  procedural_db_register (&drawable_is_gray_proc);
  procedural_db_register (&drawable_is_indexed_proc);
  procedural_db_register (&drawable_bytes_proc);
  procedural_db_register (&drawable_width_proc);
  procedural_db_register (&drawable_height_proc);
  procedural_db_register (&drawable_offsets_proc);
  procedural_db_register (&drawable_is_layer_proc);
  procedural_db_register (&drawable_is_layer_mask_proc);
  procedural_db_register (&drawable_is_channel_proc);
  procedural_db_register (&drawable_get_pixel_proc);
  procedural_db_register (&drawable_set_pixel_proc);
  procedural_db_register (&drawable_set_image_proc);
  procedural_db_register (&drawable_thumbnail_proc);
  procedural_db_register (&drawable_offset_proc);
}

static Argument *
drawable_merge_shadow_invoker (Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gboolean undo;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  undo = args[1].value.pdb_int ? TRUE : FALSE;

  if (success)
    gimp_drawable_merge_shadow (drawable, undo);

  return procedural_db_return_args (&drawable_merge_shadow_proc, success);
}

static ProcArg drawable_merge_shadow_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  },
  {
    PDB_INT32,
    "undo",
    "Push merge to undo stack?"
  }
};

static ProcRecord drawable_merge_shadow_proc =
{
  "gimp_drawable_merge_shadow",
  "Merge the shadow buffer with the specified drawable.",
  "This procedure combines the contents of the image's shadow buffer (for temporary processing) with the specified drawable. The \"undo\" parameter specifies whether to add an undo step for the operation. Requesting no undo is useful for such applications as 'auto-apply'.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  2,
  drawable_merge_shadow_inargs,
  0,
  NULL,
  { { drawable_merge_shadow_invoker } }
};

static Argument *
drawable_fill_invoker (Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 fill_type;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  fill_type = args[1].value.pdb_int;
  if (fill_type < FOREGROUND_FILL || fill_type > NO_FILL)
    success = FALSE;

  if (success)
    drawable_fill (drawable, (GimpFillType) fill_type);

  return procedural_db_return_args (&drawable_fill_proc, success);
}

static ProcArg drawable_fill_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  },
  {
    PDB_INT32,
    "fill_type",
    "The type of fill: FG_IMAGE_FILL (0), BG_IMAGE_FILL (1), WHITE_IMAGE_FILL (2), TRANS_IMAGE_FILL (3), NO_IMAGE_FILL (4)"
  }
};

static ProcRecord drawable_fill_proc =
{
  "gimp_drawable_fill",
  "Fill the drawable with the specified fill mode.",
  "This procedure fills the drawable with the fill mode. If the fill mode is foreground the current foreground color is used. If the fill mode is background, the current background color is used. If the fill type is white, then white is used. Transparent fill only affects layers with an alpha channel, in which case the alpha channel is set to transparent. If the drawable has no alpha channel, it is filled to white. No fill leaves the drawable's contents undefined. This procedure is unlike the bucket fill tool because it fills regardless of a selection",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  2,
  drawable_fill_inargs,
  0,
  NULL,
  { { drawable_fill_invoker } }
};

static Argument *
drawable_update_invoker (Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 x;
  gint32 y;
  gint32 width;
  gint32 height;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  x = args[1].value.pdb_int;

  y = args[2].value.pdb_int;

  width = args[3].value.pdb_int;

  height = args[4].value.pdb_int;

  if (success)
    drawable_update (drawable, x, y, width, height);

  return procedural_db_return_args (&drawable_update_proc, success);
}

static ProcArg drawable_update_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  },
  {
    PDB_INT32,
    "x",
    "x coordinate of upper left corner of update region"
  },
  {
    PDB_INT32,
    "y",
    "y coordinate of upper left corner of update region"
  },
  {
    PDB_INT32,
    "width",
    "Width of update region"
  },
  {
    PDB_INT32,
    "height",
    "Height of update region"
  }
};

static ProcRecord drawable_update_proc =
{
  "gimp_drawable_update",
  "Update the specified region of the drawable.",
  "This procedure updates the specified region of the drawable. The (x, y) coordinate pair is relative to the drawable's origin, not to the image origin. Therefore, the entire drawable can be updated with: {x->0, y->0, w->width, h->height }.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  5,
  drawable_update_inargs,
  0,
  NULL,
  { { drawable_update_invoker } }
};

static Argument *
drawable_mask_bounds_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;
  gboolean non_empty = FALSE;
  gint32 x1;
  gint32 y1;
  gint32 x2;
  gint32 y2;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  if (success)
    non_empty = gimp_drawable_mask_bounds (drawable, &x1, &y1, &x2, &y2);

  return_args = procedural_db_return_args (&drawable_mask_bounds_proc, success);

  if (success)
    {
      return_args[1].value.pdb_int = non_empty;
      return_args[2].value.pdb_int = x1;
      return_args[3].value.pdb_int = y1;
      return_args[4].value.pdb_int = x2;
      return_args[5].value.pdb_int = y2;
    }

  return return_args;
}

static ProcArg drawable_mask_bounds_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_mask_bounds_outargs[] =
{
  {
    PDB_INT32,
    "non_empty",
    "TRUE if there is a selection"
  },
  {
    PDB_INT32,
    "x1",
    "x coordinate of the upper left corner of selection bounds"
  },
  {
    PDB_INT32,
    "y1",
    "y coordinate of the upper left corner of selection bounds"
  },
  {
    PDB_INT32,
    "x2",
    "x coordinate of the lower right corner of selection bounds"
  },
  {
    PDB_INT32,
    "y2",
    "y coordinate of the lower right corner of selection bounds"
  }
};

static ProcRecord drawable_mask_bounds_proc =
{
  "gimp_drawable_mask_bounds",
  "Find the bounding box of the current selection in relation to the specified drawable.",
  "This procedure returns the whether there is a selection. If there is one, the upper left and lower righthand corners of its bounding box are returned. These coordinates are specified relative to the drawable's origin, and bounded by the drawable's extents.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_mask_bounds_inargs,
  5,
  drawable_mask_bounds_outargs,
  { { drawable_mask_bounds_invoker } }
};

static Argument *
drawable_image_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;
  GimpImage *gimage = NULL;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  if (success)
    success = (gimage = gimp_drawable_gimage (drawable)) != NULL;

  return_args = procedural_db_return_args (&drawable_image_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_image_get_ID (gimage);

  return return_args;
}

static ProcArg drawable_image_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_image_outargs[] =
{
  {
    PDB_IMAGE,
    "image",
    "The drawable's image"
  }
};

static ProcRecord drawable_image_proc =
{
  "gimp_drawable_image",
  "Returns the drawable's image.",
  "This procedure returns the drawable's image.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_image_inargs,
  1,
  drawable_image_outargs,
  { { drawable_image_invoker } }
};

static Argument *
drawable_type_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_type_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_type (drawable);

  return return_args;
}

static ProcArg drawable_type_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_type_outargs[] =
{
  {
    PDB_INT32,
    "type",
    "The drawable's type: { RGB_IMAGE (0), RGBA_IMAGE (1), GRAY_IMAGE (2), GRAYA_IMAGE (3), INDEXED_IMAGE (4), INDEXEDA_IMAGE (5) }"
  }
};

static ProcRecord drawable_type_proc =
{
  "gimp_drawable_type",
  "Returns the drawable's type.",
  "This procedure returns the drawable's type.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_type_inargs,
  1,
  drawable_type_outargs,
  { { drawable_type_invoker } }
};

static Argument *
drawable_has_alpha_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_has_alpha_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_has_alpha (drawable);

  return return_args;
}

static ProcArg drawable_has_alpha_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_has_alpha_outargs[] =
{
  {
    PDB_INT32,
    "has_alpha",
    "Does the drawable have an alpha channel?"
  }
};

static ProcRecord drawable_has_alpha_proc =
{
  "gimp_drawable_has_alpha",
  "Returns non-zero if the drawable has an alpha channel.",
  "This procedure returns whether the specified drawable has an alpha channel. This can only be true for layers, and the associated type will be one of: { RGBA , GRAYA, INDEXEDA }.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_has_alpha_inargs,
  1,
  drawable_has_alpha_outargs,
  { { drawable_has_alpha_invoker } }
};

static Argument *
drawable_type_with_alpha_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_type_with_alpha_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_type_with_alpha (drawable);

  return return_args;
}

static ProcArg drawable_type_with_alpha_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_type_with_alpha_outargs[] =
{
  {
    PDB_INT32,
    "type_with_alpha",
    "The drawable's type with alpha: { RGBA_IMAGE (1), GRAYA_IMAGE (3), INDEXEDA_IMAGE (5) }"
  }
};

static ProcRecord drawable_type_with_alpha_proc =
{
  "gimp_drawable_type_with_alpha",
  "Returns the drawable's type with alpha.",
  "This procedure returns the drawable's type if an alpha channel were added. If the type is currently Gray, for instance, the returned type would be GrayA. If the drawable already has an alpha channel, the drawable's type is simply returned.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_type_with_alpha_inargs,
  1,
  drawable_type_with_alpha_outargs,
  { { drawable_type_with_alpha_invoker } }
};

static Argument *
drawable_is_rgb_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_is_rgb_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_is_rgb (drawable);

  return return_args;
}

static ProcArg drawable_is_rgb_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_is_rgb_outargs[] =
{
  {
    PDB_INT32,
    "is_rgb",
    "non-zero if the drawable is an RGB type"
  }
};

static ProcRecord drawable_is_rgb_proc =
{
  "gimp_drawable_is_rgb",
  "Returns whether the drawable is an RGB type.",
  "This procedure returns non-zero if the specified drawable is of type { RGB, RGBA }.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_is_rgb_inargs,
  1,
  drawable_is_rgb_outargs,
  { { drawable_is_rgb_invoker } }
};

static Argument *
drawable_is_gray_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_is_gray_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_is_gray (drawable);

  return return_args;
}

static ProcArg drawable_is_gray_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_is_gray_outargs[] =
{
  {
    PDB_INT32,
    "is_gray",
    "non-zero if the drawable is a grayscale type"
  }
};

static ProcRecord drawable_is_gray_proc =
{
  "gimp_drawable_is_gray",
  "Returns whether the drawable is a grayscale type.",
  "This procedure returns non-zero if the specified drawable is of type { Gray, GrayA }.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_is_gray_inargs,
  1,
  drawable_is_gray_outargs,
  { { drawable_is_gray_invoker } }
};

static Argument *
drawable_is_indexed_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_is_indexed_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_is_indexed (drawable);

  return return_args;
}

static ProcArg drawable_is_indexed_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_is_indexed_outargs[] =
{
  {
    PDB_INT32,
    "is_indexed",
    "non-zero if the drawable is an indexed type"
  }
};

static ProcRecord drawable_is_indexed_proc =
{
  "gimp_drawable_is_indexed",
  "Returns whether the drawable is an indexed type.",
  "This procedure returns non-zero if the specified drawable is of type { Indexed, IndexedA }.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_is_indexed_inargs,
  1,
  drawable_is_indexed_outargs,
  { { drawable_is_indexed_invoker } }
};

static Argument *
drawable_bytes_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_bytes_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_bytes (drawable);

  return return_args;
}

static ProcArg drawable_bytes_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_bytes_outargs[] =
{
  {
    PDB_INT32,
    "bytes",
    "Bytes per pixel"
  }
};

static ProcRecord drawable_bytes_proc =
{
  "gimp_drawable_bytes",
  "Returns the bytes per pixel.",
  "This procedure returns the number of bytes per pixel (or the number of channels) for the specified drawable.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_bytes_inargs,
  1,
  drawable_bytes_outargs,
  { { drawable_bytes_invoker } }
};

static Argument *
drawable_width_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_width_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_width (drawable);

  return return_args;
}

static ProcArg drawable_width_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_width_outargs[] =
{
  {
    PDB_INT32,
    "width",
    "Width of drawable"
  }
};

static ProcRecord drawable_width_proc =
{
  "gimp_drawable_width",
  "Returns the width of the drawable.",
  "This procedure returns the specified drawable's width in pixels.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_width_inargs,
  1,
  drawable_width_outargs,
  { { drawable_width_invoker } }
};

static Argument *
drawable_height_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_height_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_height (drawable);

  return return_args;
}

static ProcArg drawable_height_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_height_outargs[] =
{
  {
    PDB_INT32,
    "height",
    "Height of drawable"
  }
};

static ProcRecord drawable_height_proc =
{
  "gimp_drawable_height",
  "Returns the height of the drawable.",
  "This procedure returns the specified drawable's height in pixels.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_height_inargs,
  1,
  drawable_height_outargs,
  { { drawable_height_invoker } }
};

static Argument *
drawable_offsets_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;
  gint32 offset_x;
  gint32 offset_y;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  if (success)
    gimp_drawable_offsets (drawable, &offset_x, &offset_y);

  return_args = procedural_db_return_args (&drawable_offsets_proc, success);

  if (success)
    {
      return_args[1].value.pdb_int = offset_x;
      return_args[2].value.pdb_int = offset_y;
    }

  return return_args;
}

static ProcArg drawable_offsets_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_offsets_outargs[] =
{
  {
    PDB_INT32,
    "offset_x",
    "x offset of drawable"
  },
  {
    PDB_INT32,
    "offset_y",
    "y offset of drawable"
  }
};

static ProcRecord drawable_offsets_proc =
{
  "gimp_drawable_offsets",
  "Returns the offsets for the drawable.",
  "This procedure returns the specified drawable's offsets. This only makes sense if the drawable is a layer since channels are anchored. The offsets of a channel will be returned as 0.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_offsets_inargs,
  2,
  drawable_offsets_outargs,
  { { drawable_offsets_invoker } }
};

static Argument *
drawable_is_layer_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_is_layer_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_layer (drawable) ? TRUE : FALSE;

  return return_args;
}

static ProcArg drawable_is_layer_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_is_layer_outargs[] =
{
  {
    PDB_INT32,
    "layer",
    "Non-zero if the drawable is a layer"
  }
};

static ProcRecord drawable_is_layer_proc =
{
  "gimp_drawable_is_layer",
  "Returns whether the drawable is a layer.",
  "This procedure returns non-zero if the specified drawable is a layer.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_is_layer_inargs,
  1,
  drawable_is_layer_outargs,
  { { drawable_is_layer_invoker } }
};

static Argument *
drawable_is_layer_mask_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_is_layer_mask_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_layer_mask (drawable) ? TRUE : FALSE;

  return return_args;
}

static ProcArg drawable_is_layer_mask_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_is_layer_mask_outargs[] =
{
  {
    PDB_INT32,
    "layer_mask",
    "Non-zero if the drawable is a layer mask"
  }
};

static ProcRecord drawable_is_layer_mask_proc =
{
  "gimp_drawable_is_layer_mask",
  "Returns whether the drawable is a layer mask.",
  "This procedure returns non-zero if the specified drawable is a layer mask.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_is_layer_mask_inargs,
  1,
  drawable_is_layer_mask_outargs,
  { { drawable_is_layer_mask_invoker } }
};

static Argument *
drawable_is_channel_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  return_args = procedural_db_return_args (&drawable_is_channel_proc, success);

  if (success)
    return_args[1].value.pdb_int = gimp_drawable_channel (drawable) ? TRUE : FALSE;

  return return_args;
}

static ProcArg drawable_is_channel_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  }
};

static ProcArg drawable_is_channel_outargs[] =
{
  {
    PDB_INT32,
    "channel",
    "Non-zero if the drawable is a channel"
  }
};

static ProcRecord drawable_is_channel_proc =
{
  "gimp_drawable_is_channel",
  "Returns whether the drawable is a channel.",
  "This procedure returns non-zero if the specified drawable is a channel.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  1,
  drawable_is_channel_inargs,
  1,
  drawable_is_channel_outargs,
  { { drawable_is_channel_invoker } }
};

static Argument *
drawable_get_pixel_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;
  gint32 x;
  gint32 y;
  gint32 num_channels = 0;
  guint8 *pixel = NULL;
  guint8 *p;
  gint b;
  Tile *tile;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  x = args[1].value.pdb_int;
  if (x < 0)
    success = FALSE;

  y = args[2].value.pdb_int;
  if (y < 0)
    success = FALSE;

  if (success)
    {
      if (x < gimp_drawable_width (drawable) && y < gimp_drawable_height (drawable))
	{  
	  num_channels = gimp_drawable_bytes (drawable);
	  pixel = g_new (guint8, num_channels);
    
	  tile = tile_manager_get_tile (gimp_drawable_data (drawable), x, y,
					TRUE, TRUE);
    
	  x %= TILE_WIDTH;
	  y %= TILE_HEIGHT;
    
	  p = tile_data_pointer (tile, x, y);
	  for (b = 0; b < num_channels; b++)
	    pixel[b] = p[b];
    
	  tile_release (tile, FALSE);
	}
      else
	success = FALSE;
    }

  return_args = procedural_db_return_args (&drawable_get_pixel_proc, success);

  if (success)
    {
      return_args[1].value.pdb_int = num_channels;
      return_args[2].value.pdb_pointer = pixel;
    }

  return return_args;
}

static ProcArg drawable_get_pixel_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  },
  {
    PDB_INT32,
    "x_coord",
    "The x coordinate"
  },
  {
    PDB_INT32,
    "y_coord",
    "The y coordinate"
  }
};

static ProcArg drawable_get_pixel_outargs[] =
{
  {
    PDB_INT32,
    "num_channels",
    "The number of channels for the pixel"
  },
  {
    PDB_INT8ARRAY,
    "pixel",
    "The pixel value"
  }
};

static ProcRecord drawable_get_pixel_proc =
{
  "gimp_drawable_get_pixel",
  "Gets the value of the pixel at the specified coordinates.",
  "This procedure gets the pixel value at the specified coordinates. The 'num_channels' argument must always be equal to the bytes-per-pixel value for the specified drawable.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1997",
  PDB_INTERNAL,
  3,
  drawable_get_pixel_inargs,
  2,
  drawable_get_pixel_outargs,
  { { drawable_get_pixel_invoker } }
};

static Argument *
drawable_set_pixel_invoker (Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gint32 x;
  gint32 y;
  gint32 num_channels;
  guint8 *pixel;
  guint8 *p;
  gint b;
  Tile *tile;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  x = args[1].value.pdb_int;
  if (x < 0)
    success = FALSE;

  y = args[2].value.pdb_int;
  if (y < 0)
    success = FALSE;

  num_channels = args[3].value.pdb_int;

  pixel = (guint8 *) args[4].value.pdb_pointer;

  if (success)
    {
      if (x < gimp_drawable_width (drawable) &&
	  y < gimp_drawable_height (drawable) &&
	  num_channels == gimp_drawable_bytes (drawable))
	{
	  tile = tile_manager_get_tile (gimp_drawable_data (drawable), x, y,
					TRUE, TRUE);
    
	  x %= TILE_WIDTH;
	  y %= TILE_HEIGHT;
    
	  p = tile_data_pointer (tile, x, y);
	  for (b = 0; b < num_channels; b++)
	    *p++ = *pixel++;
    
	  tile_release (tile, TRUE);
	}
      else
	success = FALSE;
    }

  return procedural_db_return_args (&drawable_set_pixel_proc, success);
}

static ProcArg drawable_set_pixel_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  },
  {
    PDB_INT32,
    "x_coord",
    "The x coordinate"
  },
  {
    PDB_INT32,
    "y_coord",
    "The y coordinate"
  },
  {
    PDB_INT32,
    "num_channels",
    "The number of channels for the pixel"
  },
  {
    PDB_INT8ARRAY,
    "pixel",
    "The pixel value"
  }
};

static ProcRecord drawable_set_pixel_proc =
{
  "gimp_drawable_set_pixel",
  "Sets the value of the pixel at the specified coordinates.",
  "This procedure sets the pixel value at the specified coordinates. The 'num_channels' argument must always be equal to the bytes-per-pixel value for the spec ified drawable.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1997",
  PDB_INTERNAL,
  5,
  drawable_set_pixel_inargs,
  0,
  NULL,
  { { drawable_set_pixel_invoker } }
};

static Argument *
drawable_set_image_invoker (Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  GimpImage *gimage;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  gimage = gimp_image_get_by_ID (args[1].value.pdb_int);
  if (gimage == NULL)
    success = FALSE;

  if (success)
    gimp_drawable_set_gimage (drawable, gimage);

  return procedural_db_return_args (&drawable_set_image_proc, success);
}

static ProcArg drawable_set_image_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  },
  {
    PDB_IMAGE,
    "image",
    "The image"
  }
};

static ProcRecord drawable_set_image_proc =
{
  "gimp_drawable_set_image",
  "Set image where drawable belongs to.",
  "Set the image the drawable should be a part of (Use this before adding a drawable to another image).",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  PDB_INTERNAL,
  2,
  drawable_set_image_inargs,
  0,
  NULL,
  { { drawable_set_image_invoker } }
};

static Argument *
drawable_thumbnail_invoker (Argument *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpDrawable *drawable;
  gint32 req_width;
  gint32 req_height;
  gint32 width = 0;
  gint32 height = 0;
  gint32 bpp = 0;
  gint32 num_pixels = 0;
  guint8 *thumbnail_data = NULL;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  req_width = args[1].value.pdb_int;
  if (req_width <= 0)
    success = FALSE;

  req_height = args[2].value.pdb_int;
  if (req_height <= 0)
    success = FALSE;

  if (success)
    {
      TempBuf * buf;
      gint dwidth, dheight;
    
      if (req_width <= 128 && req_height <= 128)
	{        
	  /* Adjust the width/height ratio */
	  dwidth  = gimp_drawable_width (GIMP_DRAWABLE (drawable));
	  dheight = gimp_drawable_height (GIMP_DRAWABLE (drawable));
    
	  if (dwidth > dheight)
	    req_height = (req_width * dheight) / dwidth;
	  else
	    req_width = (req_height * dwidth) / dheight;
    
	  buf = gimp_viewable_get_preview (GIMP_VIEWABLE (drawable),
					   req_width, req_height);
    
	  num_pixels = buf->height * buf->width * buf->bytes;
	  thumbnail_data = g_new (guint8, num_pixels);
	  g_memmove (thumbnail_data, temp_buf_data (buf), num_pixels);
	  width = buf->width;        
	  height = buf->height;
	  bpp = buf->bytes;
	}
    }

  return_args = procedural_db_return_args (&drawable_thumbnail_proc, success);

  if (success)
    {
      return_args[1].value.pdb_int = width;
      return_args[2].value.pdb_int = height;
      return_args[3].value.pdb_int = bpp;
      return_args[4].value.pdb_int = num_pixels;
      return_args[5].value.pdb_pointer = thumbnail_data;
    }

  return return_args;
}

static ProcArg drawable_thumbnail_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable"
  },
  {
    PDB_INT32,
    "width",
    "The thumbnail width"
  },
  {
    PDB_INT32,
    "height",
    "The thumbnail height"
  }
};

static ProcArg drawable_thumbnail_outargs[] =
{
  {
    PDB_INT32,
    "width",
    "The previews width"
  },
  {
    PDB_INT32,
    "height",
    "The previews height"
  },
  {
    PDB_INT32,
    "bpp",
    "The previews bpp"
  },
  {
    PDB_INT32,
    "thumbnail_data_count",
    "The number of pixels in thumbnail data"
  },
  {
    PDB_INT8ARRAY,
    "thumbnail_data",
    "The thumbnail data"
  }
};

static ProcRecord drawable_thumbnail_proc =
{
  "gimp_drawable_thumbnail",
  "Get a thumbnail of a drawable.",
  "This function gets data from which a thumbnail of a drawable preview can be created. Maximum x or y dimension is 128 pixels. The pixels are returned in the RGB[A] format. The bpp return value gives the number of bytes in the image. The alpha channel also returned if the drawable has one.",
  "Andy Thomas",
  "Andy Thomas",
  "1999",
  PDB_INTERNAL,
  3,
  drawable_thumbnail_inargs,
  5,
  drawable_thumbnail_outargs,
  { { drawable_thumbnail_invoker } }
};

static Argument *
drawable_offset_invoker (Argument *args)
{
  gboolean success = TRUE;
  GimpDrawable *drawable;
  gboolean wrap_around;
  gint32 fill_type;
  gint32 offset_x;
  gint32 offset_y;

  drawable = gimp_drawable_get_by_ID (args[0].value.pdb_int);
  if (drawable == NULL)
    success = FALSE;

  wrap_around = args[1].value.pdb_int ? TRUE : FALSE;

  fill_type = args[2].value.pdb_int;
  if (fill_type < OFFSET_BACKGROUND || fill_type > OFFSET_TRANSPARENT)
    success = FALSE;

  offset_x = args[3].value.pdb_int;

  offset_y = args[4].value.pdb_int;

  if (success)
    {
      gimp_drawable_offset (drawable, wrap_around, fill_type, offset_x, offset_y);
    }

  return procedural_db_return_args (&drawable_offset_proc, success);
}

static ProcArg drawable_offset_inargs[] =
{
  {
    PDB_DRAWABLE,
    "drawable",
    "The drawable to offset"
  },
  {
    PDB_INT32,
    "wrap_around",
    "wrap image around or fill vacated regions"
  },
  {
    PDB_INT32,
    "fill_type",
    "fill vacated regions of drawable with background or transparent: OFFSET_BACKGROUND (0) or OFFSET_TRANSPARENT (1)"
  },
  {
    PDB_INT32,
    "offset_x",
    "offset by this amount in X direction"
  },
  {
    PDB_INT32,
    "offset_y",
    "offset by this amount in Y direction"
  }
};

static ProcRecord drawable_offset_proc =
{
  "gimp_drawable_offset",
  "Offset the drawable by the specified amounts in the X and Y directions",
  "This procedure offsets the specified drawable by the amounts specified by 'offset_x' and 'offset_y'. If 'wrap_around' is set to TRUE, then portions of the drawable which are offset out of bounds are wrapped around. Alternatively, the undefined regions of the drawable can be filled with transparency or the background color, as specified by the 'fill_type' parameter.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1997",
  PDB_INTERNAL,
  5,
  drawable_offset_inargs,
  0,
  NULL,
  { { drawable_offset_invoker } }
};
