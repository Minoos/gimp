/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpunit_pdb.h
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

#ifndef __GIMP_UNIT_PDB_H__
#define __GIMP_UNIT_PDB_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* For information look into the C source or the html documentation */


gint     _gimp_unit_get_number_of_units          (void);
gint     _gimp_unit_get_number_of_built_in_units (void);
GimpUnit _gimp_unit_new                          (gchar    *identifier,
						  gdouble   factor,
						  gint      digits,
						  gchar    *symbol,
						  gchar    *abbreviation,
						  gchar    *singular,
						  gchar    *plural);
gboolean _gimp_unit_get_deletion_flag            (GimpUnit  unit_id);
gboolean _gimp_unit_set_deletion_flag            (GimpUnit  unit_id,
						  gboolean  deletion_flag);
gchar*   _gimp_unit_get_identifier               (GimpUnit  unit_id);
gdouble  _gimp_unit_get_factor                   (GimpUnit  unit_id);
gint     _gimp_unit_get_digits                   (GimpUnit  unit_id);
gchar*   _gimp_unit_get_symbol                   (GimpUnit  unit_id);
gchar*   _gimp_unit_get_abbreviation             (GimpUnit  unit_id);
gchar*   _gimp_unit_get_singular                 (GimpUnit  unit_id);
gchar*   _gimp_unit_get_plural                   (GimpUnit  unit_id);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GIMP_UNIT_PDB_H__ */
