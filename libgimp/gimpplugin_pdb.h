/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpplugin_pdb.h
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

#ifndef __GIMP_PLUG_IN_PDB_H__
#define __GIMP_PLUG_IN_PDB_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* For information look into the C source or the html documentation */


gboolean gimp_progress_init          (gchar   *message);
gboolean gimp_progress_update        (gdouble  percentage);
gchar*   gimp_temp_PDB_name          (void);
gboolean gimp_plugin_domain_register (gchar   *domain_name,
				      gchar   *domain_path);
gboolean gimp_plugin_help_register   (gchar   *help_path);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GIMP_PLUG_IN_PDB_H__ */
