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

#include "config.h"

#include <gtk/gtk.h>

#include "libgimpmath/gimpmath.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#ifdef __GNUC__
#warning FIXME #include "gui/gui-types.h"
#endif
#include "gui/gui-types.h"

#include "core/gimpimage.h"
#include "core/gimpdrawable-transform.h"
#include "core/gimpdrawable-transform-utils.h"
#include "core/gimptoolinfo.h"

#include "widgets/gimpviewabledialog.h"

#include "display/gimpdisplay.h"

#include "gui/info-dialog.h"

#include "gimpperspectivetool.h"
#include "transform_options.h"

#include "libgimp/gimpintl.h"


/*  local function prototypes  */

static void          gimp_perspective_tool_class_init (GimpPerspectiveToolClass *klass);
static void          gimp_perspective_tool_init       (GimpPerspectiveTool      *perspective_tool);

static TileManager * gimp_perspective_tool_transform  (GimpTransformTool *transform_tool,
                                                       GimpDisplay       *gdisp,
                                                       TransformState     state);

static void          perspective_tool_recalc          (GimpTransformTool *tr_tool,
                                                       GimpDisplay       *gdisp);
static void          perspective_tool_motion          (GimpTransformTool *tr_tool,
                                                       GimpDisplay       *gdisp);
static void          perspective_info_update          (GimpTransformTool *tr_tool);


/*  storage for information dialog fields  */
static gchar  matrix_row_buf [3][MAX_INFO_BUF];

static GimpTransformToolClass *parent_class = NULL;


/*  public functions  */

void 
gimp_perspective_tool_register (GimpToolRegisterCallback  callback,
                                gpointer                  data)
{
  (* callback) (GIMP_TYPE_PERSPECTIVE_TOOL,
                transform_options_new,
                FALSE,
                "gimp-perspective-tool",
                _("Perspective Tool"),
                _("Change perspective of the layer or selection"),
                N_("/Tools/Transform Tools/Perspective"), "<shift>P",
                NULL, "tools/perspective.html",
                GIMP_STOCK_TOOL_PERSPECTIVE,
                data);
}

GType
gimp_perspective_tool_get_type (void)
{
  static GType tool_type = 0;

  if (! tool_type)
    {
      static const GTypeInfo tool_info =
      {
        sizeof (GimpPerspectiveToolClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_perspective_tool_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data     */
	sizeof (GimpPerspectiveTool),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_perspective_tool_init,
      };

      tool_type = g_type_register_static (GIMP_TYPE_TRANSFORM_TOOL,
					  "GimpPerspectiveTool", 
                                          &tool_info, 0);
    }

  return tool_type;
}

static void
gimp_perspective_tool_class_init (GimpPerspectiveToolClass *klass)
{
  GimpTransformToolClass *trans_class;

  trans_class = GIMP_TRANSFORM_TOOL_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  trans_class->transform = gimp_perspective_tool_transform;
}

static void
gimp_perspective_tool_init (GimpPerspectiveTool *perspective_tool)
{
  GimpTool *tool;

  tool = GIMP_TOOL (perspective_tool);

  gimp_tool_control_set_tool_cursor (tool->control,
                                     GIMP_PERSPECTIVE_TOOL_CURSOR);
}

static TileManager *
gimp_perspective_tool_transform (GimpTransformTool  *transform_tool,
				 GimpDisplay        *gdisp,
				 TransformState      state)
{
  switch (state)
    {
    case TRANSFORM_INIT:
      if (! transform_tool->info_dialog)
	{
	  transform_tool->info_dialog =
	    info_dialog_new (NULL,
                             _("Perspective Transform"), "perspective",
                             GIMP_STOCK_TOOL_PERSPECTIVE,
                             _("Perspective Transform Information"),
			     gimp_standard_help_func,
			     "tools/transform_perspective.html");

          gimp_transform_tool_info_dialog_connect (transform_tool,
                                                   GIMP_STOCK_TOOL_PERSPECTIVE);

	  info_dialog_add_label (transform_tool->info_dialog, _("Matrix:"),
				 matrix_row_buf[0]);
	  info_dialog_add_label (transform_tool->info_dialog, "",
                                 matrix_row_buf[1]);
	  info_dialog_add_label (transform_tool->info_dialog, "",
                                 matrix_row_buf[2]);
	}

      gimp_viewable_dialog_set_viewable (GIMP_VIEWABLE_DIALOG (transform_tool->info_dialog->shell),
                                         GIMP_VIEWABLE (gimp_image_active_drawable (gdisp->gimage)));

      gtk_widget_set_sensitive (GTK_WIDGET (transform_tool->info_dialog->shell),
                                TRUE);

      transform_tool->trans_info[X0] = (gdouble) transform_tool->x1;
      transform_tool->trans_info[Y0] = (gdouble) transform_tool->y1;
      transform_tool->trans_info[X1] = (gdouble) transform_tool->x2;
      transform_tool->trans_info[Y1] = (gdouble) transform_tool->y1;
      transform_tool->trans_info[X2] = (gdouble) transform_tool->x1;
      transform_tool->trans_info[Y2] = (gdouble) transform_tool->y2;
      transform_tool->trans_info[X3] = (gdouble) transform_tool->x2;
      transform_tool->trans_info[Y3] = (gdouble) transform_tool->y2;
      break;

    case TRANSFORM_MOTION:
      perspective_tool_motion (transform_tool, gdisp);
      perspective_tool_recalc (transform_tool, gdisp);
      break;

    case TRANSFORM_RECALC:
      perspective_tool_recalc (transform_tool, gdisp);
      break;

    case TRANSFORM_FINISH:
      return gimp_transform_tool_transform_tiles (transform_tool,
                                                  _("Perspective..."));
      break;
    }

  return NULL;
}

static void
perspective_info_update (GimpTransformTool *transform_tool)
{
  gint i;
  
  for (i = 0; i < 3; i++)
    {
      gchar *p = matrix_row_buf[i];
      gint   j;

      for (j = 0; j < 3; j++)
	{
	  p += g_snprintf (p, MAX_INFO_BUF - (p - matrix_row_buf[i]),
			   "%10.3g", transform_tool->transform[i][j]);
	}
    }

  info_dialog_update (transform_tool->info_dialog);
  info_dialog_popup (transform_tool->info_dialog);
}

static void
perspective_tool_motion (GimpTransformTool *transform_tool,
			 GimpDisplay       *gdisp)
{
  gdouble diff_x, diff_y;

  diff_x = transform_tool->curx - transform_tool->lastx;
  diff_y = transform_tool->cury - transform_tool->lasty;

  switch (transform_tool->function)
    {
    case TRANSFORM_HANDLE_1:
      transform_tool->trans_info[X0] += diff_x;
      transform_tool->trans_info[Y0] += diff_y;
      break;
    case TRANSFORM_HANDLE_2:
      transform_tool->trans_info[X1] += diff_x;
      transform_tool->trans_info[Y1] += diff_y;
      break;
    case TRANSFORM_HANDLE_3:
      transform_tool->trans_info[X2] += diff_x;
      transform_tool->trans_info[Y2] += diff_y;
      break;
    case TRANSFORM_HANDLE_4:
      transform_tool->trans_info[X3] += diff_x;
      transform_tool->trans_info[Y3] += diff_y;
      break;
    case TRANSFORM_HANDLE_CENTER:
      transform_tool->trans_info[X0] += diff_x;
      transform_tool->trans_info[Y0] += diff_y;
      transform_tool->trans_info[X1] += diff_x;
      transform_tool->trans_info[Y1] += diff_y;
      transform_tool->trans_info[X2] += diff_x;
      transform_tool->trans_info[Y2] += diff_y;
      transform_tool->trans_info[X3] += diff_x;
      transform_tool->trans_info[Y3] += diff_y;
      break;
    default:
      break;
    }
}

static void
perspective_tool_recalc (GimpTransformTool *transform_tool,
			 GimpDisplay       *gdisp)
{
  gimp_drawable_transform_matrix_perspective (transform_tool->x1,
                                              transform_tool->y1,
                                              transform_tool->x2,
                                              transform_tool->y2,
                                              transform_tool->trans_info[X0],
                                              transform_tool->trans_info[Y0],
                                              transform_tool->trans_info[X1],
                                              transform_tool->trans_info[Y1],
                                              transform_tool->trans_info[X2],
                                              transform_tool->trans_info[Y2],
                                              transform_tool->trans_info[X3],
                                              transform_tool->trans_info[Y3],
                                              transform_tool->transform);

  /*  transform the bounding box  */
  gimp_transform_tool_transform_bounding_box (transform_tool);

  /*  update the information dialog  */
  perspective_info_update (transform_tool);
}
