/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1999 Adrian Likins and Tor Lillqvist
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
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#include <glib-object.h>

#ifdef G_OS_WIN32
#include <io.h>
#endif

#include "libgimpmath/gimpmath.h"
#include "libgimpbase/gimpparasiteio.h"

#include "core-types.h"

#include "gimpbrush.h"
#include "gimpbrush-header.h"
#include "gimpbrushpipe.h"
#include "gimppattern-header.h"

#include "libgimp/gimpintl.h"


static void        gimp_brush_pipe_class_init       (GimpBrushPipeClass *klass);
static void        gimp_brush_pipe_init             (GimpBrushPipe      *pipe);

static void        gimp_brush_pipe_finalize         (GObject    *object);

static gsize       gimp_brush_pipe_get_memsize      (GimpObject *object);

static GimpBrush * gimp_brush_pipe_select_brush     (GimpBrush  *brush,
                                                     GimpCoords *last_coords,
                                                     GimpCoords *cur_coords);
static gboolean    gimp_brush_pipe_want_null_motion (GimpBrush  *brush,
                                                     GimpCoords *last_coords,
                                                     GimpCoords *cur_coords);


static GimpBrushClass *parent_class = NULL;


GType
gimp_brush_pipe_get_type (void)
{
  static GType brush_type = 0;

  if (! brush_type)
    {
      static const GTypeInfo brush_info =
      {
        sizeof (GimpBrushPipeClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_brush_pipe_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (GimpBrushPipe),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_brush_pipe_init,
      };

      brush_type = g_type_register_static (GIMP_TYPE_BRUSH,
					   "GimpBrushPipe", 
					   &brush_info, 0);
    }

  return brush_type;
}

static void
gimp_brush_pipe_class_init (GimpBrushPipeClass *klass)
{
  GObjectClass    *object_class;
  GimpObjectClass *gimp_object_class;
  GimpBrushClass  *brush_class;

  object_class      = G_OBJECT_CLASS (klass);
  gimp_object_class = GIMP_OBJECT_CLASS (klass);
  brush_class       = GIMP_BRUSH_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize         = gimp_brush_pipe_finalize;

  gimp_object_class->get_memsize = gimp_brush_pipe_get_memsize;

  brush_class->select_brush      = gimp_brush_pipe_select_brush;
  brush_class->want_null_motion  = gimp_brush_pipe_want_null_motion;
}

static void
gimp_brush_pipe_init (GimpBrushPipe *pipe)
{
  pipe->current   = NULL;
  pipe->dimension = 0;
  pipe->rank      = NULL;
  pipe->stride    = NULL;
  pipe->nbrushes  = 0;
  pipe->brushes   = NULL;
  pipe->select    = NULL;
  pipe->index     = NULL;
}

static void
gimp_brush_pipe_finalize (GObject *object)
{
  GimpBrushPipe *pipe;

  g_return_if_fail (GIMP_IS_BRUSH_PIPE (object));

  pipe = GIMP_BRUSH_PIPE (object);

  if (pipe->rank)
    {
      g_free (pipe->rank);
      pipe->rank = NULL;
    }
  if (pipe->stride)
    {
      g_free (pipe->stride);
      pipe->stride = NULL;
    }

  if (pipe->brushes)
    {
      gint i;

      for (i = 0; i < pipe->nbrushes; i++)
	if (pipe->brushes[i])
	  g_object_unref (G_OBJECT (pipe->brushes[i]));

      g_free (pipe->brushes);
      pipe->brushes = NULL;
    }

  if (pipe->select)
    {
      g_free (pipe->select);
      pipe->select = NULL;
    }
  if (pipe->index)
    {
      g_free (pipe->index);
      pipe->index = NULL;
    }

  GIMP_BRUSH (pipe)->mask   = NULL;
  GIMP_BRUSH (pipe)->pixmap = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gsize
gimp_brush_pipe_get_memsize (GimpObject *object)
{
  GimpBrushPipe *pipe;
  gsize          memsize = 0;
  gint           i;

  pipe = GIMP_BRUSH_PIPE (object);

  memsize += pipe->dimension * (sizeof (gint) /* rank   */ +
                                sizeof (gint) /* stride */ +
                                sizeof (PipeSelectModes));

  for (i = 0; i < pipe->nbrushes; i++)
    {
      memsize += gimp_object_get_memsize (GIMP_OBJECT (pipe->brushes[i]));
    }

  return memsize + GIMP_OBJECT_CLASS (parent_class)->get_memsize (object);
}

static GimpBrush *
gimp_brush_pipe_select_brush (GimpBrush  *brush,
                              GimpCoords *last_coords,
                              GimpCoords *cur_coords)
{
  GimpBrushPipe *pipe;
  gint           i, brushix, ix;
  gdouble        angle;

  pipe = GIMP_BRUSH_PIPE (brush);

  if (pipe->nbrushes == 1)
    return GIMP_BRUSH (pipe->current);

  brushix = 0;
  for (i = 0; i < pipe->dimension; i++)
    {
      switch (pipe->select[i])
	{
	case PIPE_SELECT_INCREMENTAL:
	  ix = (pipe->index[i] + 1) % pipe->rank[i];
	  break;
	case PIPE_SELECT_ANGULAR:
	  angle = atan2 (cur_coords->y - last_coords->y,
			 cur_coords->x - last_coords->x);
	  /* Offset angle to be compatible with PSP tubes */
	  angle += G_PI_2;
	  /* Map it to the [0..2*G_PI) interval */
	  if (angle < 0)
	    angle += 2.0 * G_PI;
	  else if (angle > 2.0 * G_PI)
	    angle -= 2.0 * G_PI;
	  ix = RINT (angle / (2.0 * G_PI) * pipe->rank[i]);
	  break;
	case PIPE_SELECT_RANDOM:
	  /* This probably isn't the right way */
	  ix = rand () % pipe->rank[i];
	  break;
	case PIPE_SELECT_PRESSURE:
	  ix = RINT (cur_coords->pressure * (pipe->rank[i] - 1));
	  break;
	case PIPE_SELECT_TILT_X:
	  ix = RINT (cur_coords->xtilt / 2.0 * pipe->rank[i]) + pipe->rank[i]/2;
	  break;
	case PIPE_SELECT_TILT_Y:
	  ix = RINT (cur_coords->ytilt / 2.0 * pipe->rank[i]) + pipe->rank[i]/2;
	  break;
	case PIPE_SELECT_CONSTANT:
	default:
	  ix = pipe->index[i];
	  break;
	}
      pipe->index[i] = CLAMP (ix, 0, pipe->rank[i]-1);
      brushix += pipe->stride[i] * pipe->index[i];
    }

  /* Make sure is inside bounds */
  brushix = CLAMP (brushix, 0, pipe->nbrushes-1);

  pipe->current = pipe->brushes[brushix];

  return GIMP_BRUSH (pipe->current);
}

static gboolean
gimp_brush_pipe_want_null_motion (GimpBrush  *brush,
                                  GimpCoords *last_coords,
                                  GimpCoords *cur_coords)
{
  GimpBrushPipe *pipe;
  gint           i;

  pipe = GIMP_BRUSH_PIPE (brush);

  if (pipe->nbrushes == 1)
    return TRUE;

  for (i = 0; i < pipe->dimension; i++)
    if (pipe->select[i] == PIPE_SELECT_ANGULAR)
      return FALSE;

  return TRUE;
}

GimpData *
gimp_brush_pipe_load (const gchar *filename)
{
  GimpBrushPipe     *pipe = NULL;
  GimpPixPipeParams  params;
  gint               i;
  gint               num_of_brushes = 0;
  gint               totalcells;
  gchar             *paramstring;
  GString           *buffer;
  gchar              c;
  gint               fd;

  g_return_val_if_fail (filename != NULL, NULL);

  fd = open (filename, O_RDONLY | _O_BINARY);
  if (fd == -1)
    {
      g_message (_("Could not open file '%s'"), filename);
      return NULL;
    }

  /* The file format starts with a painfully simple text header */

  /*  get the name  */
  buffer = g_string_new (NULL);
  while (read (fd, &c, 1) == 1 && c != '\n' && buffer->len < 1024)
    g_string_append_c (buffer, c);
    
  if (buffer->len > 0 && buffer->len < 1024)
    {
      pipe = GIMP_BRUSH_PIPE (g_object_new (GIMP_TYPE_BRUSH_PIPE, NULL));

      if (g_utf8_validate (buffer->str, buffer->len, NULL))
        {
          gimp_object_set_name (GIMP_OBJECT (pipe), buffer->str);
        }
      else
        {
          g_message (_("Invalid UTF-8 string in brush file '%s'."), 
                     filename);
          gimp_object_set_name (GIMP_OBJECT (pipe), _("Unnamed"));
        }
    }
  g_string_free (buffer, TRUE);

  if (!pipe)
    {
      g_message (_("Fatal parsing error:\nBrush pipe file '%s' is corrupt."), 
		 filename);
      close (fd);
      return NULL;
    }

  /*  get the number of brushes  */
  buffer = g_string_new (NULL);
  while (read (fd, &c, 1) == 1 && c != '\n' && buffer->len < 1024)
    g_string_append_c (buffer, c);

  if (buffer->len > 0 && buffer->len < 1024)
    {
      num_of_brushes = strtol (buffer->str, &paramstring, 10);
    }

  if (num_of_brushes < 1)
    {
      g_message (_("Fatal parsing error:\nBrush pipe file '%s' is corrupt."), 
		 filename);
      close (fd);
      g_object_unref (G_OBJECT (pipe));
      g_string_free (buffer, TRUE);
      return NULL;
    }

  while (*paramstring && g_ascii_isspace (*paramstring))
    paramstring++;

  if (*paramstring)
    {
      gimp_pixpipe_params_init (&params);
      gimp_pixpipe_params_parse (paramstring, &params);

      pipe->dimension = params.dim;
      pipe->rank      = g_new0 (gint, pipe->dimension);
      pipe->select    = g_new0 (PipeSelectModes, pipe->dimension);
      pipe->index     = g_new0 (gint, pipe->dimension);

      /* placement is not used at all ?? */
      if (params.free_placement_string)
	g_free (params.placement);

      for (i = 0; i < pipe->dimension; i++)
	{
	  pipe->rank[i] = params.rank[i];
	  if (strcmp (params.selection[i], "incremental") == 0)
	    pipe->select[i] = PIPE_SELECT_INCREMENTAL;
	  else if (strcmp (params.selection[i], "angular") == 0)
	    pipe->select[i] = PIPE_SELECT_ANGULAR;
	  else if (strcmp (params.selection[i], "velocity") == 0)
	    pipe->select[i] = PIPE_SELECT_VELOCITY;
	  else if (strcmp (params.selection[i], "random") == 0)
	    pipe->select[i] = PIPE_SELECT_RANDOM;
	  else if (strcmp (params.selection[i], "pressure") == 0)
	    pipe->select[i] = PIPE_SELECT_PRESSURE;
	  else if (strcmp (params.selection[i], "xtilt") == 0)
	    pipe->select[i] = PIPE_SELECT_TILT_X;
	  else if (strcmp (params.selection[i], "ytilt") == 0)
	    pipe->select[i] = PIPE_SELECT_TILT_Y;
	  else
	    pipe->select[i] = PIPE_SELECT_CONSTANT;
	  if (params.free_selection_string)
	    g_free (params.selection[i]);
	  pipe->index[i] = 0;
	}
    }
  else
    {
      pipe->dimension = 1;
      pipe->rank      = g_new (gint, 1);
      pipe->rank[0]   = num_of_brushes;
      pipe->select    = g_new (PipeSelectModes, 1);
      pipe->select[0] = PIPE_SELECT_INCREMENTAL;
      pipe->index     = g_new (gint, 1);
      pipe->index[0]  = 0;
    }

  g_string_free (buffer, TRUE);

  totalcells = 1;		/* Not all necessarily present, maybe */
  for (i = 0; i < pipe->dimension; i++)
    totalcells *= pipe->rank[i];
  pipe->stride = g_new0 (gint, pipe->dimension);
  for (i = 0; i < pipe->dimension; i++)
    {
      if (i == 0)
	pipe->stride[i] = totalcells / pipe->rank[i];
      else
	pipe->stride[i] = pipe->stride[i-1] / pipe->rank[i];
    }
  g_assert (pipe->stride[pipe->dimension-1] == 1);

  pipe->brushes = g_new0 (GimpBrush *, num_of_brushes);

  while (pipe->nbrushes < num_of_brushes)
    {
      pipe->brushes[pipe->nbrushes] = gimp_brush_load_brush (fd, filename);

      if (pipe->brushes[pipe->nbrushes])
	{
	  gimp_object_set_name (GIMP_OBJECT (pipe->brushes[pipe->nbrushes]),
				NULL);
	}
      else
	{
	  g_message (_("Fatal parsing error:\n"
                       "Brush pipe file '%s' is corrupt."), filename);
	  close (fd);
	  g_object_unref (G_OBJECT (pipe));
	  return NULL;
	}
  
      pipe->nbrushes++;
    }

  /* Current brush is the first one. */
  pipe->current = pipe->brushes[0];

  gimp_data_set_filename (GIMP_DATA (pipe), filename);

  /*  just to satisfy the code that relies on this crap  */
  GIMP_BRUSH (pipe)->spacing  = pipe->current->spacing;
  GIMP_BRUSH (pipe)->x_axis   = pipe->current->x_axis;
  GIMP_BRUSH (pipe)->y_axis   = pipe->current->y_axis;
  GIMP_BRUSH (pipe)->mask     = pipe->current->mask;
  GIMP_BRUSH (pipe)->pixmap   = pipe->current->pixmap;

  close (fd);

  return GIMP_DATA (pipe);
}
