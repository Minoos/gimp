/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpbrushselect_pdb.h
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

#ifndef __GIMP_BRUSH_SELECT_PDB_H__
#define __GIMP_BRUSH_SELECT_PDB_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


gboolean gimp_brushes_popup       (gchar                *brush_callback,
				   gchar                *popup_title,
				   gchar                *initial_brush,
				   gdouble               opacity,
				   gint                  spacing,
				   GimpLayerModeEffects  paint_mode);
gboolean gimp_brushes_close_popup (gchar                *brush_callback);
gboolean gimp_brushes_set_popup   (gchar                *brush_callback,
				   gchar                *brush_name,
				   gdouble               opacity,
				   gint                  spacing,
				   GimpLayerModeEffects  paint_mode);


G_END_DECLS

#endif /* __GIMP_BRUSH_SELECT_PDB_H__ */
