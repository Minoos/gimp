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

#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "base/boundary.h"

#include "core/gimpchannel.h"
#include "core/gimpchannel-select.h"
#include "core/gimplayer-floating-sel.h"
#include "core/gimpimage.h"
#include "core/gimppickable.h"
#include "core/gimptoolinfo.h"
#include "core/gimp-utils.h"

#include "widgets/gimpdialogfactory.h"
#include "widgets/gimphelp-ids.h"
#include "widgets/gimpviewabledialog.h"
#include "widgets/gimpwidgets-utils.h"

#include "display/gimpcanvas.h"
#include "display/gimpdisplay.h"
#include "display/gimpdisplayshell.h"
#include "display/gimpdisplayshell-transform.h"

#include "gimpselectiontool.h"
#include "gimpselectionoptions.h"
#include "gimprectangletool.h"
#include "gimprectangleoptions.h"
#include "gimpnewrectselecttool.h"
#include "gimpnewrectselectoptions.h"
#include "gimptoolcontrol.h"

#include "gimp-intl.h"


static void     gimp_new_rect_select_tool_rectangle_tool_iface_init (GimpRectangleToolInterface *iface);

static GObject *
                gimp_new_rect_select_tool_constructor    (GType            type,
                                                          guint            n_params,
                                                          GObjectConstructParam *params);
static void     gimp_new_rect_select_tool_finalize       (GObject         *object);
static void     gimp_new_rect_select_tool_control        (GimpTool        *tool,
                                                          GimpToolAction   action,
                                                          GimpDisplay     *gdisp);
static void     gimp_new_rect_select_tool_button_press   (GimpTool        *tool,
                                                          GimpCoords      *coords,
                                                          guint32          time,
                                                          GdkModifierType  state,
                                                          GimpDisplay     *gdisp);
static void     gimp_new_rect_select_tool_button_release (GimpTool        *tool,
                                                          GimpCoords      *coords,
                                                          guint32          time,
                                                          GdkModifierType  state,
                                                          GimpDisplay     *gdisp);
static void     gimp_new_rect_select_tool_oper_update    (GimpTool        *tool,
                                                          GimpCoords      *coords,
                                                          GdkModifierType  state,
                                                          gboolean         proximity,
                                                          GimpDisplay     *gdisp);
static gboolean gimp_new_rect_select_tool_execute        (GimpRectangleTool *rect_tool,
                                                          gint             x,
                                                          gint             y,
                                                          gint             w,
                                                          gint             h);
static void     gimp_new_rect_select_tool_real_rect_select (GimpNewRectSelectTool *rect_select,
                                                            gint                   x,
                                                            gint                   y,
                                                            gint                   w,
                                                            gint                   h);


G_DEFINE_TYPE_WITH_CODE (GimpNewRectSelectTool, gimp_new_rect_select_tool,
                         GIMP_TYPE_SELECTION_TOOL,
                         G_IMPLEMENT_INTERFACE (GIMP_TYPE_RECTANGLE_TOOL,
                                                gimp_new_rect_select_tool_rectangle_tool_iface_init));

#define parent_class gimp_new_rect_select_tool_parent_class


void
gimp_new_rect_select_tool_register (GimpToolRegisterCallback  callback,
                                    gpointer                  data)
{
  (* callback) (GIMP_TYPE_NEW_RECT_SELECT_TOOL,
                GIMP_TYPE_NEW_RECT_SELECT_OPTIONS,
                gimp_new_rect_select_options_gui,
                0,
                "gimp-new-rect-select-tool",
                _("New Rect Select"),
                _("Select a Rectangular part of an image"),
                N_("_New Rect Select"), NULL,
                NULL, GIMP_HELP_TOOL_RECT_SELECT,
                GIMP_STOCK_TOOL_RECT_SELECT,
                data);
}

static void
gimp_new_rect_select_tool_class_init (GimpNewRectSelectToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  GimpToolClass     *tool_class      = GIMP_TOOL_CLASS (klass);
  GimpDrawToolClass *draw_tool_class = GIMP_DRAW_TOOL_CLASS (klass);

  object_class->constructor  = gimp_new_rect_select_tool_constructor;
  object_class->dispose      = gimp_rectangle_tool_dispose;
  object_class->finalize     = gimp_new_rect_select_tool_finalize;
  object_class->set_property = gimp_rectangle_tool_set_property;
  object_class->get_property = gimp_rectangle_tool_get_property;

  gimp_rectangle_tool_install_properties (object_class);

  tool_class->initialize     = gimp_rectangle_tool_initialize;
  tool_class->control        = gimp_new_rect_select_tool_control;
  tool_class->button_press   = gimp_new_rect_select_tool_button_press;
  tool_class->button_release = gimp_new_rect_select_tool_button_release;
  tool_class->motion         = gimp_rectangle_tool_motion;
  tool_class->key_press      = gimp_rectangle_tool_key_press;
  tool_class->modifier_key   = gimp_rectangle_tool_modifier_key;
  tool_class->oper_update    = gimp_new_rect_select_tool_oper_update;
  tool_class->cursor_update  = gimp_rectangle_tool_cursor_update;

  draw_tool_class->draw      = gimp_rectangle_tool_draw;

  klass->rect_select         = gimp_new_rect_select_tool_real_rect_select;
}

static void
gimp_new_rect_select_tool_init (GimpNewRectSelectTool *new_rect_select_tool)
{
  GimpTool          *tool      = GIMP_TOOL (new_rect_select_tool);

  gimp_tool_control_set_tool_cursor (tool->control, GIMP_TOOL_CURSOR_RECT_SELECT);
}

static void
gimp_new_rect_select_tool_rectangle_tool_iface_init (GimpRectangleToolInterface *iface)
{
  iface->execute = gimp_new_rect_select_tool_execute;
}

static GObject *
gimp_new_rect_select_tool_constructor (GType                  type,
                                       guint                  n_params,
                                       GObjectConstructParam *params)
{
  GObject *object;

  object = G_OBJECT_CLASS (parent_class)->constructor (type, n_params, params);

  gimp_rectangle_tool_constructor (object);

  return object;
}

static void
gimp_new_rect_select_tool_finalize (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_new_rect_select_tool_control (GimpTool       *tool,
                                   GimpToolAction  action,
                                   GimpDisplay    *gdisp)
{
  GimpRectangleTool *rectangle = GIMP_RECTANGLE_TOOL (tool);

  switch (action)
    {
    case PAUSE:
      break;

    case RESUME:
      gimp_rectangle_tool_configure (rectangle);
      break;

    case HALT:
      gimp_rectangle_tool_response (NULL, GIMP_RECTANGLE_MODE_EXECUTE,
                                    rectangle);
      break;

    default:
      break;
    }

  GIMP_TOOL_CLASS (parent_class)->control (tool, action, gdisp);
}

static void
gimp_new_rect_select_tool_button_press (GimpTool        *tool,
                                        GimpCoords      *coords,
                                        guint32          time,
                                        GdkModifierType  state,
                                        GimpDisplay     *gdisp)
{
  if (tool->gdisp && gdisp != tool->gdisp)
    gimp_rectangle_tool_response (NULL, GIMP_RECTANGLE_MODE_EXECUTE,
                                  GIMP_RECTANGLE_TOOL (tool));

  gimp_rectangle_tool_button_press (tool, coords, time, state, gdisp);
}

static void
gimp_new_rect_select_tool_button_release (GimpTool        *tool,
                                          GimpCoords      *coords,
                                          guint32          time,
                                          GdkModifierType  state,
                                          GimpDisplay     *gdisp)
{
  gimp_tool_pop_status (tool, gdisp);
  gimp_tool_push_status (tool, gdisp,
                         _("Click or press enter to create the selection."));

  gimp_rectangle_tool_button_release (tool, coords, time, state, gdisp);
}

static void
gimp_new_rect_select_tool_oper_update (GimpTool        *tool,
                                       GimpCoords      *coords,
                                       GdkModifierType  state,
                                       gboolean         proximity,
                                       GimpDisplay     *gdisp)
{
  gimp_rectangle_tool_oper_update (tool, coords, state, proximity, gdisp);

  GIMP_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               gdisp);
}

/*
 * This function is called if the user clicks and releases the left
 * button without moving it.  There are four things we might want
 * to do here:
 * 1) If there is an existing rectangle and we are inside it, we
 *    convert it into a selection.
 * 2) If there is an existing rectangle and we are outside it, we
 *    clear it.
 * 3) If there is no rectangle and we are inside the selection, we
 *    create a rectangle from the selection bounds.
 * 4) If there is no rectangle and we are outside the selection,
 *    we clear the selection.
 */
static gboolean
gimp_new_rect_select_tool_execute (GimpRectangleTool  *rectangle,
                                   gint                x,
                                   gint                y,
                                   gint                w,
                                   gint                h)
{
  GimpTool              *tool        = GIMP_TOOL (rectangle);
  GimpNewRectSelectTool *rect_select = GIMP_NEW_RECT_SELECT_TOOL (rectangle);
  GimpSelectionOptions  *options;
  GimpImage             *gimage;
  gint                   max_x, max_y;
  gboolean               rectangle_exists;
  gboolean               selected;
  gint                   val;
  GimpChannel           *selection_mask;
  gint                   x1, y1;
  gint                   x2, y2;
  gint                   pressx, pressy;
  guchar                *val_ptr;

  options = GIMP_SELECTION_OPTIONS (tool->tool_info->tool_options);

  gimp_tool_pop_status (tool, tool->gdisp);

  gimage = tool->gdisp->gimage;
  max_x = gimage->width;
  max_y = gimage->height;
  selection_mask = gimp_image_get_mask (gimage);

  rectangle_exists = (x <= max_x && y <= max_y &&
                      x + w >= 0 && y + h >= 0 &&
                      w > 0 && h > 0);

  if (x < 0)
    {
      w += x;
      x = 0;
    }

  if (y < 0)
    {
      h += y;
      y = 0;
    }

  if (x + w > max_x)
    w = max_x - x;

  if (y + h > max_y)
    h = max_y - y;

  /* if rectangle exists, turn it into a selection */
  if (rectangle_exists)
    {
      GIMP_NEW_RECT_SELECT_TOOL_GET_CLASS (rect_select)->rect_select (rect_select, x, y, w, h);

      return TRUE;
    }

  g_object_get (rectangle,
                "pressx", &pressx,
                "pressy", &pressy,
                NULL);

  if ((val_ptr = gimp_pickable_get_color_at (GIMP_PICKABLE (selection_mask),
                                             pressx,
                                             pressy)))
    val = *val_ptr;
  else
    val = 0;

  selected = (val > 127);

  /* if point clicked is inside selection, set rectangle to  */
  /* edges of marching ants.                                 */
  if (selected)
    {
      GimpChannel    *selection_mask = gimp_image_get_mask (gimage);
      const BoundSeg *segs_in;
      const BoundSeg *segs_out;
      gint            n_segs_in;
      gint            n_segs_out;

      if (gimp_channel_boundary (selection_mask, &segs_in, &segs_out,
                                 &n_segs_in, &n_segs_out,
                                 0, 0, 0, 0))
        {
          x1 = y1 = x2 = y2 = 0;

          if (n_segs_in > 0)
            {
              gint i;

              x1 = segs_in[0].x1;
              x2 = segs_in[0].x1;
              y1 = segs_in[0].y1;
              y2 = segs_in[0].y1;

              for (i = 1; i < n_segs_in; i++)
                {
                  x1 = MIN (x1, segs_in[i].x1);
                  x2 = MAX (x2, segs_in[i].x1);
                  y1 = MIN (y1, segs_in[i].y1);
                  y2 = MAX (y2, segs_in[i].y1);
                }
            }

          g_object_set (rectangle,
                        "x1", x1,
                        "y1", y1,
                        "x2", x2,
                        "y2", y2,
                        NULL);
        }
      else
        {
          g_object_set (rectangle,
                        "x1", 0,
                        "y1", 0,
                        "x2", gimage->width,
                        "y2", gimage->height,
                        NULL);
        }

      g_object_set (rectangle, "function", RECT_MOVING, NULL);

      return FALSE;
    }

  /* otherwise clear the selection */
  gimp_channel_clear (selection_mask, NULL, TRUE);

  return TRUE;
}

static void
gimp_new_rect_select_tool_real_rect_select (GimpNewRectSelectTool *rect_select,
                                            gint                   x,
                                            gint                   y,
                                            gint                   w,
                                            gint                   h)
{
  GimpTool             *tool     = GIMP_TOOL (rect_select);
  GimpSelectionOptions *options;

  options = GIMP_SELECTION_OPTIONS (tool->tool_info->tool_options);

  gimp_channel_select_rectangle (gimp_image_get_mask (tool->gdisp->gimage),
                                 x, y, w, h,
                                 options->operation,
                                 options->feather,
                                 options->feather_radius,
                                 options->feather_radius);
}
