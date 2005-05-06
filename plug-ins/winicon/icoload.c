/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * GIMP Plug-in for Windows Icon files.
 * Copyright (C) 2002 Christian Kreibich <christian@whoop.org>.
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

#include <errno.h>
#include <string.h>

#include <glib/gstdio.h>

#include <libgimp/gimp.h>

/* #define ICO_DBG */

#include "main.h"

#include "libgimp/stdplugins-intl.h"


#define A_VAL(p) ((guchar *)(p))[3]
#define R_VAL(p) ((guchar *)(p))[2]
#define G_VAL(p) ((guchar *)(p))[1]
#define B_VAL(p) ((guchar *)(p))[0]

#define A_VAL_GIMP(p) ((guchar *)(p))[3]
#define R_VAL_GIMP(p) ((guchar *)(p))[0]
#define G_VAL_GIMP(p) ((guchar *)(p))[1]
#define B_VAL_GIMP(p) ((guchar *)(p))[2]


static gint       ico_read_int8  (FILE *fp, guint8 *data, gint count);
static gint       ico_read_int16 (FILE *fp, guint16 *data, gint count);
static gint       ico_read_int32 (FILE *fp, guint32 *data, gint count);
static gboolean   ico_init       (const gchar *filename, MsIcon *ico);
static void       ico_read_entry (MsIcon *ico, MsIconEntry* entry);
static void       ico_read_data  (MsIcon *ico, gint icon_num);
static void       ico_load       (MsIcon *ico);
static gint32     ico_to_gimp    (MsIcon *ico);


static gint
ico_read_int32 (FILE    *fp,
                guint32 *data,
                gint     count)
{
  gint i, total;

  total = count;
  if (count > 0)
    {
      ico_read_int8 (fp, (guint8 *) data, count * 4);
      for (i = 0; i < count; i++)
        data[i] = GUINT32_FROM_LE (data[i]);
    }

  return total * 4;
}


static gint
ico_read_int16 (FILE    *fp,
                guint16 *data,
                gint     count)
{
  gint i, total;

  total = count;
  if (count > 0)
    {
      ico_read_int8 (fp, (guint8 *) data, count * 2);
      for (i = 0; i < count; i++)
        data[i] = GUINT16_FROM_LE (data[i]);
    }

  return total * 2;
}


static gint
ico_read_int8 (FILE   *fp,
               guint8 *data,
               gint    count)
{
  gint total;
  gint bytes;

  total = count;
  while (count > 0)
    {
      bytes = fread ((gchar *) data, sizeof (gchar), count, fp);
      if (bytes <= 0) /* something bad happened */
        break;
      count -= bytes;
      data += bytes;
    }

  return total;
}


static gboolean
ico_init (const gchar *filename,
          MsIcon      *ico)
{
  memset (ico, 0, sizeof (MsIcon));

  if (! (ico->fp = g_fopen (filename, "r")))
    {
      g_message (_("Could not open '%s' for reading: %s"),
                 gimp_filename_to_utf8 (filename), g_strerror (errno));
      return FALSE;
    }

  ico->filename = filename;

  ico->cp += ico_read_int16 (ico->fp, &ico->reserved, 1);
  ico->cp += ico_read_int16 (ico->fp, &ico->resource_type, 1);

  /* Icon files use 1 as resource type, that's  what I wrote this for.
     From descriptions on the web it seems as if this loader should
     also be able to handle Win 3.11 - Win 95 cursor files (.cur),
     which use resource type 2. I haven't tested this though. */
  if (ico->reserved != 0 ||
      (ico->resource_type != 1 && ico->resource_type != 2))
    {
      ico_cleanup (ico);
      return FALSE;
    }

  return TRUE;
}


static void
ico_read_entry (MsIcon      *ico,
                MsIconEntry *entry)
{
  if (!ico || !entry)
    return;

  ico->cp += ico_read_int8 (ico->fp, &entry->width, 1);
  ico->cp += ico_read_int8 (ico->fp, &entry->height, 1);
  ico->cp += ico_read_int8 (ico->fp, &entry->num_colors, 1);
  ico->cp += ico_read_int8 (ico->fp, &entry->reserved, 1);

  ico->cp += ico_read_int16 (ico->fp, &entry->num_planes, 1);
  ico->cp += ico_read_int16 (ico->fp, &entry->bpp, 1);

  ico->cp += ico_read_int32 (ico->fp, &entry->size, 1);
  ico->cp += ico_read_int32 (ico->fp, &entry->offset, 1);

  D(("Read entry with w: "
     "%i, h: %i, num_colors: %i, bpp: %i, size %i, offset %i\n",
     entry->width, entry->height, entry->num_colors, entry->bpp,
     entry->size, entry->offset));
}


static void
ico_read_data (MsIcon *ico,
               gint    icon_num)
{
  MsIconData *data;
  MsIconEntry *entry;
  gint length;

  if (!ico)
    return;

  D(("Reading data for icon %i ------------------------------\n", icon_num));

  entry = &ico->icon_dir[icon_num];
  data = &ico->icon_data[icon_num];

  ico->cp = entry->offset;

  if (fseek (ico->fp, entry->offset, SEEK_SET) < 0)
    return;

  D(("  starting at offset %i\n", entry->offset));

  ico->cp += ico_read_int32 (ico->fp, &data->header_size, 1);
  ico->cp += ico_read_int32 (ico->fp, &data->width, 1);
  ico->cp += ico_read_int32 (ico->fp, &data->height, 1);

  ico->cp += ico_read_int16 (ico->fp, &data->planes, 1);
  ico->cp += ico_read_int16 (ico->fp, &data->bpp, 1);

  ico->cp += ico_read_int32 (ico->fp, &data->compression, 1);
  ico->cp += ico_read_int32 (ico->fp, &data->image_size, 1);
  ico->cp += ico_read_int32 (ico->fp, &data->x_res, 1);
  ico->cp += ico_read_int32 (ico->fp, &data->y_res, 1);
  ico->cp += ico_read_int32 (ico->fp, &data->used_clrs, 1);
  ico->cp += ico_read_int32 (ico->fp, &data->important_clrs, 1);

  D(("  header size %i, "
     "w %i, h %i, planes %i, size %i, bpp %i, used %i, imp %i.\n",
     data->header_size, data->width, data->height,
     data->planes, data->image_size, data->bpp,
     data->used_clrs, data->important_clrs));

  if (data->bpp <= 8)
    {
      if (data->used_clrs == 0)
        data->used_clrs = (1 << data->bpp);

      D(("  allocating a %i-slot palette for %i bpp.\n",
         data->used_clrs, data->bpp));

      data->palette = g_new0 (guint32, data->used_clrs);
      ico->cp += ico_read_int8 (ico->fp,
                                (guint8 *) data->palette, data->used_clrs * 4);
    }

  data->xor_map = ico_alloc_map (entry->width, entry->height,
                                 data->bpp, &length);
  ico->cp += ico_read_int8 (ico->fp, data->xor_map, length);
  D(("  length of xor_map: %i\n", length));

  /* Read in and_map. It's padded out to 32 bits per line: */
  data->and_map = ico_alloc_map (entry->width, entry->height, 1, &length);
  ico->cp += ico_read_int8 (ico->fp, data->and_map, length);
  D(("  length of and_map: %i\n", length));
}


static void
ico_load (MsIcon *ico)
{
  gint i;

  if (!ico)
    return;

  ico->cp += ico_read_int16 (ico->fp, &ico->icon_count, 1);
  ico->icon_dir  = g_new0 (MsIconEntry, ico->icon_count);
  ico->icon_data = g_new0 (MsIconData, ico->icon_count);

  D(("*** %s: Microsoft icon file, containing %i icon(s)\n",
         ico->filename, ico->icon_count));

  for (i = 0; i < ico->icon_count; i++)
    ico_read_entry (ico, &ico->icon_dir[i]);

  for (i = 0; i < ico->icon_count; i++)
    ico_read_data (ico, i);
}


gint
ico_get_bit_from_data (const guint8 *data,
                       gint          line_width,
                       gint          bit)
{
  gint line;
  gint width32;
  gint offset;
  gint result;

  /* width per line in multiples of 32 bits */
  width32 = (line_width % 32 == 0 ? line_width/32 : line_width/32 + 1);
  line = bit / line_width;
  offset = bit % line_width;

  result = (data[line * width32 * 4 + offset/8] & (1 << (7 - (bit % 8))));

  return (result ? 1 : 0);
}


gint
ico_get_nibble_from_data (const guint8 *data,
                          gint          line_width,
                          gint          nibble)
{
  gint line;
  gint width32;
  gint offset;
  gint result;

  /* width per line in multiples of 32 bits */
  width32 = (line_width % 8 == 0 ? line_width/8 : line_width/8 + 1);
  line = nibble / line_width;
  offset = nibble % line_width;

  result =
    (data[line * width32 * 4 + offset/2] & (0x0F << (4 * (1 - nibble % 2))));

  if (nibble % 2 == 0)
    result = result >> 4;

  return result;
}


gint
ico_get_byte_from_data (guint8 *data,
                        gint    line_width,
                        gint    byte)
{
  gint line;
  gint width32;
  gint offset;

  /* width per line in multiples of 32 bits */
  width32 = (line_width % 4 == 0 ? line_width / 4 : line_width / 4 + 1);
  line = byte / line_width;
  offset = byte % line_width;

  return data[line * width32 * 4 + offset];
}

static gint32
ico_load_layer (gint32  image,
                MsIcon *ico,
                gint    i)
{
  GimpDrawable *drawable;
  GimpPixelRgn  pixel_rgn;
  MsIconData   *idata;
  MsIconEntry  *ientry;
  guint8       *xor_map;
  guint8       *and_map;
  guint32      *palette;
  guint32      *dest_vec;
  gint32        layer;
  gchar         buf[MAXLEN];
  gint          x, y, w, h;

  idata = &ico->icon_data[i];
  ientry = &ico->icon_dir[i];
  xor_map = ico->icon_data[i].xor_map;
  and_map = ico->icon_data[i].and_map;
  palette = ico->icon_data[i].palette;
  w = ico->icon_dir[i].width;
  h = ico->icon_dir[i].height;

  gimp_progress_update ((gdouble) i / (gdouble) ico->icon_count);

  g_snprintf (buf, sizeof (buf), _("Icon #%i"), i + 1);

  layer = gimp_layer_new (image, buf, w, h,
                          GIMP_RGBA_IMAGE, 100, GIMP_NORMAL_MODE);

  gimp_image_add_layer (image, layer, i);
  drawable = gimp_drawable_get (layer);

  dest_vec = g_new (guint32, w * h);

  switch (idata->bpp)
    {
    case 1:
      for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
          {
            guint32 color = palette[ico_get_bit_from_data (xor_map,
                                                           w, y * w + x)];
            guint32 *dest = dest_vec + (h - 1 - y) * w + x;

            R_VAL_GIMP (dest) = R_VAL (&color);
            G_VAL_GIMP (dest) = G_VAL (&color);
            B_VAL_GIMP (dest) = B_VAL (&color);

            if (ico_get_bit_from_data (and_map, w, y * w + x))
              A_VAL_GIMP (dest) = 0;
            else
              A_VAL_GIMP (dest) = 255;
          }
      break;

    case 4:
      for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
          {
            guint32 color = palette[ico_get_nibble_from_data (xor_map,
                                                              w, y * w + x)];
            guint32 *dest = dest_vec + (h - 1 - y) * w + x;

            R_VAL_GIMP (dest) = R_VAL (&color);
            G_VAL_GIMP (dest) = G_VAL (&color);
            B_VAL_GIMP (dest) = B_VAL (&color);

            if (ico_get_bit_from_data (and_map, w, y * w + x))
              A_VAL_GIMP (dest) = 0;
            else
              A_VAL_GIMP (dest) = 255;
          }
      break;

    case 8:
      for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
          {
            guint32 color = palette[ico_get_byte_from_data (xor_map,
                                                            w, y * w + x)];
            guint32 *dest = dest_vec + (h - 1 - y) * w + x;

            R_VAL_GIMP (dest) = R_VAL (&color);
            G_VAL_GIMP (dest) = G_VAL (&color);
            B_VAL_GIMP (dest) = B_VAL (&color);

            if (ico_get_bit_from_data (and_map, w, y * w + x))
              A_VAL_GIMP (dest) = 0;
            else
              A_VAL_GIMP (dest) = 255;
          }
      break;

    default:
      {
        gint bytespp = idata->bpp/8;

        for (y = 0; y < h; y++)
          for (x = 0; x < w; x++)
            {
              guint32 *dest = dest_vec + (h - 1 - y) * w + x;

              B_VAL_GIMP (dest) = xor_map[(y * w + x) * bytespp];
              G_VAL_GIMP (dest) = xor_map[(y * w + x) * bytespp + 1];
              R_VAL_GIMP (dest) = xor_map[(y * w + x) * bytespp + 2];

              if (idata->bpp < 32)
                {
                  if (ico_get_bit_from_data (and_map, w, y * w + x))
                    A_VAL_GIMP (dest) = 0;
                  else
                    A_VAL_GIMP (dest) = 255;
                }
              else
                {
                  A_VAL_GIMP (dest) = xor_map[(y * w + x) * bytespp + 3];
                }
            }
      }
    }

  gimp_pixel_rgn_init (&pixel_rgn, drawable,
                       0, 0, drawable->width, drawable->height,
                       TRUE, FALSE);
  gimp_pixel_rgn_set_rect (&pixel_rgn, (const guchar *) dest_vec,
                           0, 0, drawable->width, drawable->height);

  g_free (dest_vec);

  gimp_drawable_detach (drawable);

  return layer;
}

static gint32
ico_to_gimp (MsIcon *ico)
{
  gint32 image;
  gint32 first_layer;
  gint   max_w, max_h;
  gint   i;

  /* Do a quick scan of the icons in the file to find the
     largest icon contained ... */

  max_w = 0; max_h = 0;

  for (i = 0; i < ico->icon_count; i++)
    {
      if (ico->icon_dir[i].width > max_w)
        max_w = ico->icon_dir[i].width;

      if (ico->icon_dir[i].height > max_h)
        max_h = ico->icon_dir[i].height;
    }

  /* Allocate the Gimp image */

  image = gimp_image_new (max_w, max_h, GIMP_RGB);
  gimp_image_set_filename (image, ico->filename);

  /* Scan icons again and set up a layer for each icon */
  first_layer = ico_load_layer (image, ico, 0);
  for (i = 1; i < ico->icon_count; i++)
    ico_load_layer (image, ico, i);

  gimp_image_set_active_layer (image, first_layer);

  D(("*** icon successfully loaded.\n\n"));

  return image;
}


gint32
ico_load_image (const gchar *filename)
{
  gint32  image_ID;
  MsIcon  ico;

  gimp_progress_init (NULL);
  gimp_progress_set_text (_("Opening '%s'..."),
                          gimp_filename_to_utf8 (filename));

  if (! ico_init (filename, &ico))
    return -1;

  ico_load (&ico);

  image_ID = ico_to_gimp (&ico);

  gimp_progress_update (1.0);

  ico_cleanup (&ico);

  return image_ID;
}

gint32
ico_load_thumbnail_image (const gchar *filename,
                          gint        *width,
                          gint        *height)
{
  /*  FIXME: implement thumbnail loading (bug #158191)  */
  return -1;
}
