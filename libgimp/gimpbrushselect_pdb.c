/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpbrushselect_pdb.c
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

gboolean
gimp_brushes_popup (gchar                *brush_callback,
		    gchar                *popup_title,
		    gchar                *initial_brush,
		    gdouble               opacity,
		    gint                  spacing,
		    GimpLayerModeEffects  paint_mode)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_brushes_popup",
				    &nreturn_vals,
				    GIMP_PDB_STRING, brush_callback,
				    GIMP_PDB_STRING, popup_title,
				    GIMP_PDB_STRING, initial_brush,
				    GIMP_PDB_FLOAT, opacity,
				    GIMP_PDB_INT32, spacing,
				    GIMP_PDB_INT32, paint_mode,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_brushes_close_popup (gchar *brush_callback)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_brushes_close_popup",
				    &nreturn_vals,
				    GIMP_PDB_STRING, brush_callback,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_brushes_set_popup (gchar                *brush_callback,
			gchar                *brush_name,
			gdouble               opacity,
			gint                  spacing,
			GimpLayerModeEffects  paint_mode)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_brushes_set_popup",
				    &nreturn_vals,
				    GIMP_PDB_STRING, brush_callback,
				    GIMP_PDB_STRING, brush_name,
				    GIMP_PDB_FLOAT, opacity,
				    GIMP_PDB_INT32, spacing,
				    GIMP_PDB_INT32, paint_mode,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
