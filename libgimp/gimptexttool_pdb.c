/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimptexttool_pdb.c
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

#include "gimp.h"

gint32
gimp_text_fontname (gint32        image_ID,
		    gint32        drawable_ID,
		    gdouble       x,
		    gdouble       y,
		    gchar        *text,
		    gint          border,
		    gboolean      antialias,
		    gdouble       size,
		    GimpSizeType  size_type,
		    gchar        *fontname)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 text_layer_ID = -1;

  return_vals = gimp_run_procedure ("gimp_text_fontname",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_FLOAT, x,
				    GIMP_PDB_FLOAT, y,
				    GIMP_PDB_STRING, text,
				    GIMP_PDB_INT32, border,
				    GIMP_PDB_INT32, antialias,
				    GIMP_PDB_FLOAT, size,
				    GIMP_PDB_INT32, size_type,
				    GIMP_PDB_STRING, fontname,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    text_layer_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return text_layer_ID;
}

gboolean
gimp_text_get_extents_fontname (gchar        *text,
				gdouble       size,
				GimpSizeType  size_type,
				gchar        *fontname,
				gint         *width,
				gint         *height,
				gint         *ascent,
				gint         *descent)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_text_get_extents_fontname",
				    &nreturn_vals,
				    GIMP_PDB_STRING, text,
				    GIMP_PDB_FLOAT, size,
				    GIMP_PDB_INT32, size_type,
				    GIMP_PDB_STRING, fontname,
				    GIMP_PDB_END);

  *width = 0;
  *height = 0;
  *ascent = 0;
  *descent = 0;

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  if (success)
    {
      *width = return_vals[1].data.d_int32;
      *height = return_vals[2].data.d_int32;
      *ascent = return_vals[3].data.d_int32;
      *descent = return_vals[4].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gint32
gimp_text (gint32        image_ID,
	   gint32        drawable_ID,
	   gdouble       x,
	   gdouble       y,
	   gchar        *text,
	   gint          border,
	   gboolean      antialias,
	   gdouble       size,
	   GimpSizeType  size_type,
	   gchar        *foundry,
	   gchar        *family,
	   gchar        *weight,
	   gchar        *slant,
	   gchar        *set_width,
	   gchar        *spacing,
	   gchar        *registry,
	   gchar        *encoding)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 text_layer_ID = -1;

  return_vals = gimp_run_procedure ("gimp_text",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_FLOAT, x,
				    GIMP_PDB_FLOAT, y,
				    GIMP_PDB_STRING, text,
				    GIMP_PDB_INT32, border,
				    GIMP_PDB_INT32, antialias,
				    GIMP_PDB_FLOAT, size,
				    GIMP_PDB_INT32, size_type,
				    GIMP_PDB_STRING, foundry,
				    GIMP_PDB_STRING, family,
				    GIMP_PDB_STRING, weight,
				    GIMP_PDB_STRING, slant,
				    GIMP_PDB_STRING, set_width,
				    GIMP_PDB_STRING, spacing,
				    GIMP_PDB_STRING, registry,
				    GIMP_PDB_STRING, encoding,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    text_layer_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return text_layer_ID;
}

gboolean
gimp_text_get_extents (gchar        *text,
		       gdouble       size,
		       GimpSizeType  size_type,
		       gchar        *foundry,
		       gchar        *family,
		       gchar        *weight,
		       gchar        *slant,
		       gchar        *set_width,
		       gchar        *spacing,
		       gchar        *registry,
		       gchar        *encoding,
		       gint         *width,
		       gint         *height,
		       gint         *ascent,
		       gint         *descent)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_text_get_extents",
				    &nreturn_vals,
				    GIMP_PDB_STRING, text,
				    GIMP_PDB_FLOAT, size,
				    GIMP_PDB_INT32, size_type,
				    GIMP_PDB_STRING, foundry,
				    GIMP_PDB_STRING, family,
				    GIMP_PDB_STRING, weight,
				    GIMP_PDB_STRING, slant,
				    GIMP_PDB_STRING, set_width,
				    GIMP_PDB_STRING, spacing,
				    GIMP_PDB_STRING, registry,
				    GIMP_PDB_STRING, encoding,
				    GIMP_PDB_END);

  *width = 0;
  *height = 0;
  *ascent = 0;
  *descent = 0;

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  if (success)
    {
      *width = return_vals[1].data.d_int32;
      *height = return_vals[2].data.d_int32;
      *ascent = return_vals[3].data.d_int32;
      *descent = return_vals[4].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
