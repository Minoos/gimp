/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpdisplay_pdb.c
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

#include "gimp.h"

/**
 * gimp_display_new:
 * @image_ID: The image.
 *
 * Create a new display for the specified image.
 *
 * Creates a new display for the specified image. If the image already
 * has a display, another is added. Multiple displays are handled
 * transparently by the GIMP. The newly created display is returned and
 * can be subsequently destroyed with a call to 'gimp-display-delete'.
 * This procedure only makes sense for use with the GIMP UI.
 *
 * Returns: The new display.
 */
gint32
gimp_display_new (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 display_ID = -1;

  return_vals = gimp_run_procedure ("gimp_display_new",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    display_ID = return_vals[1].data.d_display;

  gimp_destroy_params (return_vals, nreturn_vals);

  return display_ID;
}

/**
 * gimp_display_delete:
 * @display_ID: The display to delete.
 *
 * Delete the specified display.
 *
 * This procedure removes the specified display. If this is the last
 * remaining display for the underlying image, then the image is
 * deleted also.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_display_delete (gint32 display_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_display_delete",
				    &nreturn_vals,
				    GIMP_PDB_DISPLAY, display_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_displays_flush:
 *
 * Flush all internal changes to the user interface
 *
 * This procedure takes no arguments and returns nothing except a
 * success status. Its purpose is to flush all pending updates of image
 * manipulations to the user interface. It should be called whenever
 * appropriate.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_displays_flush (void)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_displays_flush",
				    &nreturn_vals,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_displays_reconnect:
 * @old_image_ID: The old image (should have at least one display).
 * @new_image_ID: The new image (must not have a display).
 *
 * Reconnect displays from one image to another image.
 *
 * This procedure connects all displays of the old_image to the
 * new_image. If the new_image already has a display the reconnect is
 * not performed and the procedure returns without success. You should
 * rarely need to use this function.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_displays_reconnect (gint32 old_image_ID,
			 gint32 new_image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_displays_reconnect",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, old_image_ID,
				    GIMP_PDB_IMAGE, new_image_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
