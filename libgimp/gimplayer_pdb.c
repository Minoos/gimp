/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimplayer_pdb.c
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
_gimp_layer_new (gint32                image_ID,
		 gint                  width,
		 gint                  height,
		 GimpImageType         type,
		 gchar                *name,
		 gdouble               opacity,
		 GimpLayerModeEffects  mode)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 layer_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_new",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, width,
				    GIMP_PDB_INT32, height,
				    GIMP_PDB_INT32, type,
				    GIMP_PDB_STRING, name,
				    GIMP_PDB_FLOAT, opacity,
				    GIMP_PDB_INT32, mode,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_ID;
}

gint32
_gimp_layer_copy (gint32   layer_ID,
		  gboolean add_alpha)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 layer_copy_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_copy",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, add_alpha,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer_copy_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_copy_ID;
}

gint32
gimp_layer_create_mask (gint32          layer_ID,
			GimpAddMaskType mask_type)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 mask_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_create_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, mask_type,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    mask_ID = return_vals[1].data.d_layer_mask;

  gimp_destroy_params (return_vals, nreturn_vals);

  return mask_ID;
}

gboolean
gimp_layer_scale (gint32   layer_ID,
		  gint     new_width,
		  gint     new_height,
		  gboolean local_origin)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_scale",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, new_width,
				    GIMP_PDB_INT32, new_height,
				    GIMP_PDB_INT32, local_origin,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_resize (gint32 layer_ID,
		   gint   new_width,
		   gint   new_height,
		   gint   offx,
		   gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_resize",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, new_width,
				    GIMP_PDB_INT32, new_height,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_delete (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_delete",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_translate (gint32 layer_ID,
		      gint   offx,
		      gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_translate",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_add_alpha (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_add_alpha",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_set_offsets (gint32 layer_ID,
			gint   offx,
			gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_offsets",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gint32
gimp_layer_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 mask_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    mask_ID = return_vals[1].data.d_channel;

  gimp_destroy_params (return_vals, nreturn_vals);

  return mask_ID;
}

gboolean
gimp_layer_is_floating_sel (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean is_floating_sel = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_is_floating_sel",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    is_floating_sel = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return is_floating_sel;
}

gchar *
gimp_layer_get_name (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gchar *name = NULL;

  return_vals = gimp_run_procedure ("gimp_layer_get_name",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    name = g_strdup (return_vals[1].data.d_string);

  gimp_destroy_params (return_vals, nreturn_vals);

  return name;
}

gboolean
gimp_layer_set_name (gint32  layer_ID,
		     gchar  *name)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_name",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_STRING, name,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_get_visible (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean visible = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_visible",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    visible = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return visible;
}

gboolean
gimp_layer_set_visible (gint32   layer_ID,
			gboolean visible)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_visible",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, visible,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_get_preserve_trans (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean preserve_trans = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_preserve_trans",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    preserve_trans = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return preserve_trans;
}

gboolean
gimp_layer_set_preserve_trans (gint32   layer_ID,
			       gboolean preserve_trans)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_preserve_trans",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, preserve_trans,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_get_apply_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean apply_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_apply_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    apply_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return apply_mask;
}

gboolean
gimp_layer_set_apply_mask (gint32   layer_ID,
			   gboolean apply_mask)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_apply_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, apply_mask,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_get_show_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean show_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_show_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    show_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return show_mask;
}

gboolean
gimp_layer_set_show_mask (gint32   layer_ID,
			  gboolean show_mask)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_show_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, show_mask,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_get_edit_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean edit_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_edit_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    edit_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return edit_mask;
}

gboolean
gimp_layer_set_edit_mask (gint32   layer_ID,
			  gboolean edit_mask)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_edit_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, edit_mask,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gdouble
gimp_layer_get_opacity (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gdouble opacity = 0;

  return_vals = gimp_run_procedure ("gimp_layer_get_opacity",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    opacity = return_vals[1].data.d_float;

  gimp_destroy_params (return_vals, nreturn_vals);

  return opacity;
}

gboolean
gimp_layer_set_opacity (gint32  layer_ID,
			gdouble opacity)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_opacity",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_FLOAT, opacity,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

GimpLayerModeEffects
gimp_layer_get_mode (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  GimpLayerModeEffects mode = 0;

  return_vals = gimp_run_procedure ("gimp_layer_get_mode",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    mode = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return mode;
}

gboolean
gimp_layer_set_mode (gint32               layer_ID,
		     GimpLayerModeEffects mode)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_mode",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, mode,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gboolean
gimp_layer_get_linked (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean linked = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_linked",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    linked = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return linked;
}

gboolean
gimp_layer_set_linked (gint32   layer_ID,
		       gboolean linked)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_linked",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, linked,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

gint
gimp_layer_get_tattoo (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint tattoo = 0;

  return_vals = gimp_run_procedure ("gimp_layer_get_tattoo",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    tattoo = return_vals[1].data.d_tattoo;

  gimp_destroy_params (return_vals, nreturn_vals);

  return tattoo;
}

gboolean
gimp_layer_set_tattoo (gint32 layer_ID,
		       gint   tattoo)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_tattoo",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, tattoo,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
