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


#include <gtk/gtk.h>

#include "libgimpbase/gimpbasetypes.h"

#include "pdb-types.h"
#include "gui/gui-types.h"
#include "procedural_db.h"

#include "core/gimp.h"
#include "core/gimpcontainer.h"
#include "core/gimpcontext.h"
#include "core/gimpdatafactory.h"
#include "gui/pattern-select.h"

static ProcRecord patterns_popup_proc;
static ProcRecord patterns_close_popup_proc;
static ProcRecord patterns_set_popup_proc;

void
register_pattern_select_procs (Gimp *gimp)
{
  procedural_db_register (gimp, &patterns_popup_proc);
  procedural_db_register (gimp, &patterns_close_popup_proc);
  procedural_db_register (gimp, &patterns_set_popup_proc);
}

static Argument *
patterns_popup_invoker (Gimp     *gimp,
                        Argument *args)
{
  gboolean success = TRUE;
  gchar *pattern_callback;
  gchar *popup_title;
  gchar *initial_pattern;
  ProcRecord *proc;

  pattern_callback = (gchar *) args[0].value.pdb_pointer;
  if (pattern_callback == NULL)
    success = FALSE;

  popup_title = (gchar *) args[1].value.pdb_pointer;
  if (popup_title == NULL)
    success = FALSE;

  initial_pattern = (gchar *) args[2].value.pdb_pointer;

  if (success)
    {
      if (! gimp->no_interface &&
	  (proc = procedural_db_lookup (gimp, pattern_callback)))
	{
	  pattern_select_new (gimp, popup_title,
			      initial_pattern,
			      pattern_callback);
	}
      else
	{
	  success = FALSE;
	}
    }

  return procedural_db_return_args (&patterns_popup_proc, success);
}

static ProcArg patterns_popup_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "pattern_callback",
    "The callback PDB proc to call when pattern selection is made"
  },
  {
    GIMP_PDB_STRING,
    "popup_title",
    "Title to give the pattern popup window"
  },
  {
    GIMP_PDB_STRING,
    "initial_pattern",
    "The name of the pattern to set as the first selected"
  }
};

static ProcRecord patterns_popup_proc =
{
  "gimp_patterns_popup",
  "Invokes the Gimp pattern selection.",
  "This procedure popups the pattern selection dialog.",
  "Andy Thomas",
  "Andy Thomas",
  "1998",
  GIMP_INTERNAL,
  3,
  patterns_popup_inargs,
  0,
  NULL,
  { { patterns_popup_invoker } }
};

static Argument *
patterns_close_popup_invoker (Gimp     *gimp,
                              Argument *args)
{
  gboolean success = TRUE;
  gchar *pattern_callback;
  ProcRecord *proc;
  PatternSelect *pattern_select;

  pattern_callback = (gchar *) args[0].value.pdb_pointer;
  if (pattern_callback == NULL)
    success = FALSE;

  if (success)
    {
      if (! gimp->no_interface &&
	  (proc = procedural_db_lookup (gimp, pattern_callback)) &&
	  (pattern_select = pattern_select_get_by_callback (pattern_callback)))
	{
	  pattern_select_free (pattern_select);
	}
      else
	{
	  success = FALSE;
	}
    }

  return procedural_db_return_args (&patterns_close_popup_proc, success);
}

static ProcArg patterns_close_popup_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "pattern_callback",
    "The name of the callback registered for this popup"
  }
};

static ProcRecord patterns_close_popup_proc =
{
  "gimp_patterns_close_popup",
  "Popdown the Gimp pattern selection.",
  "This procedure closes an opened pattern selection dialog.",
  "Andy Thomas",
  "Andy Thomas",
  "1998",
  GIMP_INTERNAL,
  1,
  patterns_close_popup_inargs,
  0,
  NULL,
  { { patterns_close_popup_invoker } }
};

static Argument *
patterns_set_popup_invoker (Gimp     *gimp,
                            Argument *args)
{
  gboolean success = TRUE;
  gchar *pattern_callback;
  gchar *pattern_name;
  ProcRecord *proc;
  PatternSelect *pattern_select;

  pattern_callback = (gchar *) args[0].value.pdb_pointer;
  if (pattern_callback == NULL)
    success = FALSE;

  pattern_name = (gchar *) args[1].value.pdb_pointer;
  if (pattern_name == NULL)
    success = FALSE;

  if (success)
    {
      if (! gimp->no_interface &&
	  (proc = procedural_db_lookup (gimp, pattern_callback)) &&
	  (pattern_select = pattern_select_get_by_callback (pattern_callback)))
	{
	  GimpPattern *active = (GimpPattern *)
	    gimp_container_get_child_by_name (gimp->pattern_factory->container,
					      pattern_name);
    
	  if (active)
	    {
	      gimp_context_set_pattern (pattern_select->context, active);
    
	      gtk_window_present (GTK_WINDOW (pattern_select->shell));
	    }
	  else
	    success = FALSE;
	}
      else
	success = FALSE;
    }

  return procedural_db_return_args (&patterns_set_popup_proc, success);
}

static ProcArg patterns_set_popup_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "pattern_callback",
    "The name of the callback registered for this popup"
  },
  {
    GIMP_PDB_STRING,
    "pattern_name",
    "The name of the pattern to set as selected"
  }
};

static ProcRecord patterns_set_popup_proc =
{
  "gimp_patterns_set_popup",
  "Sets the current pattern selection in a popup.",
  "Sets the current pattern selection in a popup.",
  "Andy Thomas",
  "Andy Thomas",
  "1998",
  GIMP_INTERNAL,
  2,
  patterns_set_popup_inargs,
  0,
  NULL,
  { { patterns_set_popup_invoker } }
};
