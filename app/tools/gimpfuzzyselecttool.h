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

#ifndef __GIMP_FUZZY_SELECT_TOOL_H__
#define __GIMP_FUZZY_SELECT_TOOL_H__


#include "gimpselectiontool.h"


#define GIMP_TYPE_FUZZY_SELECT_TOOL            (gimp_fuzzy_select_tool_get_type ())
#define GIMP_FUZZY_SELECT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_FUZZY_SELECT_TOOL, GimpFuzzySelectTool))
#define GIMP_FUZZY_SELECT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_FUZZY_SELECT_TOOL, GimpFuzzySelectToolClass))
#define GIMP_IS_FUZZY_SELECT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_FUZZY_SELECT_TOOL))
#define GIMP_IS_FUZZY_SELECT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_FUZZY_SELECT_TOOL))
#define GIMP_FUZZY_SELECT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_FUZZY_SELECT_TOOL, GimpFuzzySelectToolClass))


typedef struct _GimpFuzzySelectTool      GimpFuzzySelectTool;
typedef struct _GimpFuzzySelectToolClass GimpFuzzySelectToolClass;

struct _GimpFuzzySelectTool
{
  GimpSelectionTool  parent_instance;

  gint               x, y;             /*  Point from which to execute seed fill   */
  gint               first_x;          /*                                          */
  gint               first_y;          /*  variables to keep track of sensitivity  */
  gdouble            first_threshold;  /*  initial value of threshold slider       */

  GimpChannel       *fuzzy_mask;
};

struct _GimpFuzzySelectToolClass
{
  GimpSelectionToolClass parent_class;
};


void    gimp_fuzzy_select_tool_register (Gimp                     *gimp,
                                         GimpToolRegisterCallback  callback);

GType   gimp_fuzzy_select_tool_get_type (void) G_GNUC_CONST;


#endif  /* __GIMP_FUZZY_SELECT_TOOL_H__ */
