/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __GIMP_SMUDGE_TOOL_H__
#define __GIMP_SMUDGE_TOOL_H__


#include "gimppainttool.h"


typedef enum
{
  SMUDGE_TYPE_SMUDGE,
  SMUDGE_TYPE_STREAK 
} SmudgeType;


#define GIMP_TYPE_SMUDGE_TOOL            (gimp_smudge_tool_get_type ())
#define GIMP_SMUDGE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_SMUDGE_TOOL, GimpSmudgeTool))
#define GIMP_SMUDGE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_SMUDGE_TOOL, GimpSmudgeToolClass))
#define GIMP_IS_SMUDGE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_SMUDGE_TOOL))
#define GIMP_IS_SMUDGE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_SMUDGE_TOOL))
#define GIMP_SMUDGE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_SMUDGE_TOOL, GimpSmudgeToolClass))


typedef struct _GimpSmudgeTool      GimpSmudgeTool;
typedef struct _GimpSmudgeToolClass GimpSmudgeToolClass;

struct _GimpSmudgeTool
{
  GimpPaintTool parent_instance;
};

struct _GimpSmudgeToolClass
{
  GimpPaintToolClass parent_class;
};


void    gimp_smudge_tool_register (Gimp                     *gimp,
                                   GimpToolRegisterCallback  callback);

GType   gimp_smudge_tool_get_type (void) G_GNUC_CONST;


/* FIXME: this antique code doesn't follow the coding style */
gboolean   gimp_smudge_tool_non_gui          (GimpDrawable *drawable,
					      gdouble       rate,
					      gint          num_strokes,
					      gdouble      *stroke_array);
gboolean   gimp_smudge_tool_non_gui_default  (GimpDrawable *drawable,
					      gint          num_strokes,
					      gdouble      *stroke_array);


#endif  /*  __GIMP_SMUDGE_TOOL_H__  */
