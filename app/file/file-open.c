/* The GIMP -- an image manipulation program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib-object.h>

#include "core/core-types.h"

#include "core/gimp.h"
#include "core/gimpimage.h"
#include "core/gimpdocuments.h"

#include "file-open.h"
#include "file-utils.h"

#include "plug_in.h"
#include "undo.h"

#include "libgimp/gimpintl.h"


/*  public functions  */

GimpImage *
file_open_image (Gimp              *gimp,
		 const gchar       *filename,
		 const gchar       *raw_filename,
		 const gchar       *open_mode,
		 PlugInProcDef     *file_proc,
		 RunModeType        run_mode,
		 GimpPDBStatusType *status)
{
  ProcRecord    *proc;
  Argument      *args;
  Argument      *return_vals;
  gint           gimage_id;
  gint           i;
  struct stat    statbuf;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), NULL);
  g_return_val_if_fail (status != NULL, NULL);

  *status = GIMP_PDB_CANCEL;  /* inhibits error messages by caller */

  if (! file_proc)
    file_proc = file_proc_find (gimp->load_procs, filename);

  if (! file_proc)
    {
      /*  no errors when making thumbnails  */
      if (run_mode == RUN_INTERACTIVE)
	g_message (_("%s failed.\n"
		     "%s: Unknown file type."),
		   open_mode, filename);

      return NULL;
    }

  /* check if we are opening a file */
  if (stat (filename, &statbuf) == 0)
    {
      if (! (statbuf.st_mode & S_IFREG))
	{
	  /*  no errors when making thumbnails  */
	  if (run_mode == RUN_INTERACTIVE)
	    g_message (_("%s failed.\n"
			 "%s is not a regular file."),
		       open_mode, filename);

	  return NULL;
	}

      if (access (filename, R_OK) != 0)
	{
	  /*  no errors when making thumbnails  */
	  if (run_mode == RUN_INTERACTIVE)
	    g_message (_("%s failed.\n"
			 "%s: %s."),
		       open_mode, filename, g_strerror (errno));

	  return NULL;
	}
    }

  proc = &file_proc->db_info;

  args = g_new0 (Argument, proc->num_args);

  for (i = 0; i < proc->num_args; i++)
    args[i].arg_type = proc->args[i].arg_type;

  args[0].value.pdb_int     = run_mode;
  args[1].value.pdb_pointer = (gchar *) filename;
  args[2].value.pdb_pointer = (gchar *) raw_filename;

  return_vals = procedural_db_execute (gimp, proc->name, args);

  *status   = return_vals[0].value.pdb_int;
  gimage_id = return_vals[1].value.pdb_int;

  procedural_db_destroy_args (return_vals, proc->num_values);
  g_free (args);

  if (*status == GIMP_PDB_SUCCESS && gimage_id != -1)
    {
      return gimp_image_get_by_ID (gimp, gimage_id);
    }

  return NULL;
}

GimpPDBStatusType
file_open_with_display (Gimp        *gimp,
                        const gchar *filename)
{
  g_return_val_if_fail (GIMP_IS_GIMP (gimp), GIMP_PDB_CALLING_ERROR);

  return file_open_with_proc_and_display (gimp, filename, filename, NULL);
}

GimpPDBStatusType
file_open_with_proc_and_display (Gimp          *gimp,
                                 const gchar   *filename,
                                 const gchar   *raw_filename,
                                 PlugInProcDef *file_proc)
{
  GimpImage         *gimage;
  gchar             *absolute;
  GimpPDBStatusType  status;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), GIMP_PDB_CALLING_ERROR);

  if ((gimage = file_open_image (gimp,
				 filename,
				 raw_filename,
				 _("Open"),
				 file_proc,
				 RUN_INTERACTIVE,
				 &status)) != NULL)
    {
      /* enable & clear all undo steps */
      gimp_image_undo_enable (gimage);

      /* set the image to clean  */
      gimp_image_clean_all (gimage);

      gimp_create_display (gimage->gimp, gimage, 0x0101);

      g_object_unref (G_OBJECT (gimage));

      absolute = file_open_absolute_filename (gimp, filename);

      gimp_documents_add (gimp, filename);

      g_free (absolute);
    }

  return status;
}

gchar *
file_open_absolute_filename (Gimp        *gimp,
                             const gchar *name)
{
  PlugInProcDef *proc;
  GSList        *procs;
  GSList        *prefixes;
  gchar         *absolute;
  gchar         *current;

  g_return_val_if_fail (GIMP_IS_GIMP (gimp), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  /*  check for prefixes like http or ftp  */
  for (procs = gimp->load_procs; procs; procs = g_slist_next (procs))
    {
      proc = (PlugInProcDef *)procs->data;

      for (prefixes = proc->prefixes_list;
	   prefixes;
	   prefixes = g_slist_next (prefixes))
	{
	  if (strncmp (name, prefixes->data, strlen (prefixes->data)) == 0)
	    return g_strdup (name);
	}
     }

  if (g_path_is_absolute (name))
    return g_strdup (name);

  current = g_get_current_dir ();
  absolute = g_build_filename (current, name, NULL);
  g_free (current);

  return absolute;
}
