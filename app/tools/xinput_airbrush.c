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
Hmm a little
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "appenv.h"
#include "drawable.h"
#include "draw_core.h"
#include "gimage_mask.h"
#include "gimprc.h"
#include "xinput_airbrush.h"
#include "paint_options.h"
#include "tool_options_ui.h"
#include "tools.h"
#include "undo.h"
#include "airbrush_blob.h"
#include "gdisplay.h"

#include "libgimp/gimpintl.h"

#include "tile.h"			/* ick. */

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif /* M_PI */

#define SUBSAMPLE 8

#define DIST_SMOOTHER_BUFFER 10
#define TIME_SMOOTHER_BUFFER 10

/*  the XinputAirbrush structures  */

typedef struct _XinputAirbrushTool XinputAirbrushTool;
struct _XinputAirbrushTool
  {
    DrawCore * core;         /*  Core select object             */
 
    AirBrush         * last_airbrush;
    AirBrushBlob     * last_airbrush_blob;	   /*  airbrush_blob for last cursor position  */
    AirBlob          * last_airblob;


    int        x1, y1;       /*  image space coordinate         */
    int        x2, y2;       /*  image space coords             */

    /* circular distance history buffer */
    gdouble    dt_buffer[DIST_SMOOTHER_BUFFER];
    gint       dt_index;

    /* circular timing history buffer */
    guint32    ts_buffer[TIME_SMOOTHER_BUFFER];
    gint       ts_index;

    /* Direction and center point */
    double     xcenter, ycenter;
    double     direction_abs;
    double     direction;
    double     c_direction;
    double     c_direction_abs;


    gdouble    last_time;    /*  previous time of a motion event      */
    gdouble    lastx, lasty; /*  previous position of a motion event  */
    gdouble    last_lastx, last_lasty; /* The previous previous position of a motion event i.e pos of the blob */

    gboolean   init_velocity;
    gboolean   init_prepre;

  };


typedef struct _XinputAirbrushOptions XinputAirbrushOptions;
struct _XinputAirbrushOptions
  {
    PaintOptions  paint_options;

    double        size;
    double        size_d;
    GtkObject    *size_w;

    double        sensitivity;
    double        sensitivity_d;
    GtkObject    *sensitivity_w;

    double        vel_sensitivity;
    double        vel_sensitivity_d;
    GtkObject    *vel_sensitivity_w;

    double        tilt_sensitivity;
    double        tilt_sensitivity_d;
    GtkObject    *tilt_sensitivity_w;


    double        tilt_angle;
    double        tilt_angle_d;
    GtkObject    *tilt_angle_w;

  };


/* the xinput_airbrush tool options  */
static XinputAirbrushOptions *xinput_airbrush_options = NULL;

/* local variables */

/*  undo blocks variables  */
static TileManager *  undo_tiles = NULL;

/* Tiles used to render the stroke at 1 byte/pp */
static TileManager *  canvas_tiles = NULL;

/* Flat buffer that is used to used to render the dirty region
 * for composition onto the destination drawable
 */
static TempBuf *  canvas_buf = NULL;


/*  local function prototypes  */

static void   xinput_airbrush_button_press   (Tool *, GdkEventButton *, gpointer);
static void   xinput_airbrush_button_release (Tool *, GdkEventButton *, gpointer);
static void   xinput_airbrush_motion         (Tool *, GdkEventMotion *, gpointer);
static void   xinput_airbrush_cursor_update  (Tool *, GdkEventMotion *, gpointer);
static void   xinput_airbrush_control        (Tool *, ToolAction,       gpointer);

static void    time_smoother_add    (XinputAirbrushTool* xinput_airbrush_tool, guint32 value);
static gdouble time_smoother_result (XinputAirbrushTool* xinput_airbrush_tool);
static void    time_smoother_init   (XinputAirbrushTool* xinput_airbrush_tool, guint32 initval);
static void    dist_smoother_add    (XinputAirbrushTool* xinput_airbrush_tool, gdouble value);
static gdouble dist_smoother_result (XinputAirbrushTool* xinput_airbrush_tool);
static void    dist_smoother_init   (XinputAirbrushTool* xinput_airbrush_tool, gdouble initval);

static void xinput_airbrush_init   (XinputAirbrushTool      *xinput_airbrush_tool,
                                    GimpDrawable *drawable,
                                    double        x,
				    double        y);

static void xinput_airbrush_finish (XinputAirbrushTool      *xinput_airbrush_tool,
                                    GimpDrawable *drawable,
                                    int           tool_id);

static void xinput_airbrush_cleanup (void);


/*Mask functions*/

static void calc_angle (AirBrushBlob *airbrush_blob,
			double xcenter,
			double ycenter);

static void calc_width (AirBrushBlob *airbrush_blob);


static void render_airbrush_line (AirBrushBlob *airbrush_blob,
                                  guchar *dest,
                                  int y,
                                  int width,
                                  XinputAirbrushTool *xinput_airbrush_tool);



static void make_single_mask (AirBrushBlob *airbrush_blob,
                                XinputAirbrushTool *xinput_airbrush_tool,
                                MaskBuf *dest);

static void make_stroke(AirBlob *airblob,
			XinputAirbrushTool *xinput_airbrush_tool,
			GimpDrawable *drawable,
			guint last_value,
			guint present_value);



static void make_mask(AirLine *airline,
		      MaskBuf *brush_mask,
		      guint value);




static void print_mask(MaskBuf *dest);




/*  Rendering functions  */

static void xinput_airbrush_set_paint_area  (XinputAirbrushTool      *xinput_airbrush_tool,
					     GimpDrawable *drawable,
					     int x,
					     int y,
					     int width,
					     int height);

static void xinput_airbrush_paste (XinputAirbrushTool      *xinput_airbrush_tool,
				   GimpDrawable *drawable,
				   MaskBuf *brush_mask,
				   int x,
				   int y,
				   int width,
				   int height);

static void xinput_airbrush_to_canvas_tiles (XinputAirbrushTool      *xinput_airbrush_tool,
					     MaskBuf *brush_mask,
					     int brush_opacity);

static void xinput_airbrush_canvas_tiles_to_canvas_buf(XinputAirbrushTool *xinput_airbrush_tool);

static void xinput_airbrush_set_undo_tiles  (GimpDrawable *drawable,
					     int           x,
					     int           y,
					     int           w,
					     int           h);

static void xinput_airbrush_set_canvas_tiles(int           x,
					     int           y,
					     int           w,
					     int           h);



/* Start of functions */


static void
  xinput_airbrush_options_reset (void)
  {
    XinputAirbrushOptions *options = xinput_airbrush_options;

    paint_options_reset ((PaintOptions *) options);

    gtk_adjustment_set_value (GTK_ADJUSTMENT (options->size_w),
                              options->size_d);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (options->sensitivity_w),
                              options->sensitivity_d);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (options->tilt_sensitivity_w),
                              options->tilt_sensitivity_d);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (options->vel_sensitivity_w),
                              options->vel_sensitivity_d);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (options->tilt_angle_w),
                              options->tilt_angle_d);
  }

static XinputAirbrushOptions *
  xinput_airbrush_options_new (void)
  {
    XinputAirbrushOptions *options;

    GtkWidget *table;
    GtkWidget *abox;
    GtkWidget *label;
    GtkWidget *slider;

    /*  the new xinput_airbrush tool options structure  */
    options = (XinputAirbrushOptions *) g_malloc (sizeof (XinputAirbrushOptions));
    paint_options_init ((PaintOptions *) options,
                        XINPUT_AIRBRUSH,
                        xinput_airbrush_options_reset);
    options->size             = options->size_d             = 4.4;
    options->sensitivity      = options->sensitivity_d      = 1.0;
    options->vel_sensitivity  = options->vel_sensitivity_d  = 0.8;
    options->tilt_sensitivity = options->tilt_sensitivity_d = 0.4;
    options->tilt_angle       = options->tilt_angle_d       = 0.0;

    /*the main table*/  

    table = gtk_table_new (6, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (((ToolOptions *) options)->main_vbox), table,
                        FALSE, FALSE, 0);

    /*size slider*/ 
    label = gtk_label_new (_("Size:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
    gtk_widget_show (label);

    options->size_w =
      gtk_adjustment_new (options->size_d, 0.0, 20.0, 1.0, 5.0, 0.0);
    slider = gtk_hscale_new (GTK_ADJUSTMENT (options->size_w));
    gtk_scale_set_value_pos (GTK_SCALE (slider), GTK_POS_TOP);
    gtk_table_attach_defaults (GTK_TABLE (table), slider, 1, 2, 0, 1);
    gtk_range_set_update_policy (GTK_RANGE (slider), GTK_UPDATE_DELAYED);
    gtk_signal_connect (GTK_OBJECT (options->size_w), "value_changed",
                        (GtkSignalFunc) tool_options_double_adjustment_update,
                        &options->size);
    gtk_widget_show (slider);

    /*sens slider*/
    label = gtk_label_new (_("Sensitivity:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
    gtk_widget_show (label);

    options->sensitivity_w =
      gtk_adjustment_new (options->sensitivity_d, 0.0, 1.0, 0.01, 0.1, 0.0);
    slider = gtk_hscale_new (GTK_ADJUSTMENT (options->sensitivity_w));
    gtk_scale_set_value_pos (GTK_SCALE (slider), GTK_POS_TOP);
    gtk_table_attach_defaults (GTK_TABLE (table), slider, 1, 2, 1, 2);
    gtk_range_set_update_policy (GTK_RANGE (slider), GTK_UPDATE_DELAYED);
    gtk_signal_connect (GTK_OBJECT (options->sensitivity_w), "value_changed",
                        (GtkSignalFunc) tool_options_double_adjustment_update,
                        &options->sensitivity);
    gtk_widget_show (slider);

    /*tilt sens slider*/
    label = gtk_label_new (_("Tilt"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show (label);

    label = gtk_label_new (_("Sensitivity:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show (label);

    abox = gtk_alignment_new (0.5, 1.0, 1.0, 0.0);
    gtk_table_attach (GTK_TABLE (table), abox, 1, 2, 2, 4,
                      GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
    gtk_widget_show (abox);

    options->tilt_sensitivity_w =
      gtk_adjustment_new (options->tilt_sensitivity_d, 0.0, 1.0, 0.01, 0.1, 0.0);
    slider = gtk_hscale_new (GTK_ADJUSTMENT (options->tilt_sensitivity_w));
    gtk_scale_set_value_pos (GTK_SCALE (slider), GTK_POS_TOP);
    gtk_container_add (GTK_CONTAINER (abox), slider);
    gtk_range_set_update_policy (GTK_RANGE (slider), GTK_UPDATE_DELAYED);
    gtk_signal_connect (GTK_OBJECT (options->tilt_sensitivity_w), "value_changed",
                        (GtkSignalFunc) tool_options_double_adjustment_update,
                        &options->tilt_sensitivity);
    gtk_widget_show (slider);


    /*velocity sens slider*/
    label = gtk_label_new (_("Speed"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show (label);

    label = gtk_label_new (_("Sensitivity:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show (label);

    abox = gtk_alignment_new (0.5, 1.0, 1.0, 0.0);
    gtk_table_attach (GTK_TABLE (table), abox, 1, 2, 4, 6,
                      GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
    gtk_widget_show (abox);

    options->vel_sensitivity_w =
      gtk_adjustment_new (options->vel_sensitivity_d, 0.0, 1.0, 0.01, 0.1, 0.0);
    slider = gtk_hscale_new (GTK_ADJUSTMENT (options->vel_sensitivity_w));
    gtk_scale_set_value_pos (GTK_SCALE (slider), GTK_POS_TOP);
    gtk_container_add (GTK_CONTAINER (abox), slider);
    gtk_range_set_update_policy (GTK_RANGE (slider), GTK_UPDATE_DELAYED);
    gtk_signal_connect (GTK_OBJECT (options->vel_sensitivity_w), "value_changed",
                        (GtkSignalFunc) tool_options_double_adjustment_update,
                        &options->vel_sensitivity);
    gtk_widget_show (slider);


    /*angle adjust slider*/ 
    label = gtk_label_new (_("Angle"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show (label);

    label = gtk_label_new (_("Adjust:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 7, 8,
                      GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show (label);

    abox = gtk_alignment_new (0.5, 1.0, 1.0, 0.0);
    gtk_table_attach (GTK_TABLE (table), abox, 1, 2, 6, 8,
                      GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
    gtk_widget_show (abox);

    options->tilt_angle_w =
      gtk_adjustment_new (options->tilt_angle_d, -90.0, 90.0, 1, 10.0, 0.0);
    slider = gtk_hscale_new (GTK_ADJUSTMENT (options->tilt_angle_w));
    gtk_scale_set_value_pos (GTK_SCALE (slider), GTK_POS_TOP);
    gtk_container_add (GTK_CONTAINER (abox), slider);
    gtk_range_set_update_policy (GTK_RANGE (slider), GTK_UPDATE_DELAYED);
    gtk_signal_connect (GTK_OBJECT (options->tilt_angle_w), "value_changed",
                        (GtkSignalFunc) tool_options_double_adjustment_update,
                        &options->tilt_angle);

  
    

    gtk_widget_show_all (table);

    return options;
  }



static AirBlob *
xinput_airbrush_pen_ellipse (XinputAirbrushTool      *xinput_airbrush_tool, gdouble x_center, gdouble y_center,
                               gdouble pressure, gdouble xtiltv, gdouble ytiltv)
{

    gdouble xtilt, ytilt;
    double height;
    double tanx, tany;
    double tanytop, tanxright, tanybot, tanxleft;
    double sprayangle;
    double distx, disty;
    double ytop, xright, ybot, xleft;



    /*Fix av tilt negative when it should be positive*/

    xtilt = xtiltv * -1.0;
    ytilt = ytiltv * -1.0;

    /*Values that should be adjust able*/

    height = 50.;
    sprayangle = M_PI/12;

    /*Tan of x and y tilt plus spray angles x r/l and y t/b tan*/

    tanx = tan(xtilt * M_PI / 2.0);
    tany = tan(ytilt * M_PI / 2.0);


    tanytop = tan((ytilt * M_PI / 2.0) + (sprayangle/2));
    tanxright = tan((xtilt * M_PI / 2.0) +  (sprayangle/2));
    tanybot = tan((ytilt * M_PI / 2.0) - (sprayangle/2));
    tanxleft = tan((xtilt * M_PI / 2.0) - (sprayangle/2));

    /* Offset from cursor due to tilt in x and y  depening on the hight*/

    distx = tanx * height;
    disty = tany * height;

    /* Offset from center of blob in all for axies */

    ytop =  fabs(fabs(tanytop * height) - fabs(disty));
    xright = fabs(fabs(tanxright * height) - fabs(distx));
    ybot = fabs(fabs(tanybot * height) - fabs(disty));
    xleft = fabs(fabs(tanxleft * height) - fabs(distx));

    x_center = x_center + distx;
    y_center = y_center + disty;


    xinput_airbrush_tool->xcenter=x_center;
    xinput_airbrush_tool->ycenter=y_center;
    xinput_airbrush_tool->direction_abs=atan2(ytiltv, xtiltv) + M_PI;
    xinput_airbrush_tool->direction=atan(ytiltv/xtiltv);
    xinput_airbrush_tool->c_direction=atan(ytiltv/xtilt);
    xinput_airbrush_tool->c_direction_abs=atan2(ytiltv, xtilt) + M_PI;

    return create_air_blob(x_center * SUBSAMPLE, y_center * SUBSAMPLE,
			   0., ytop * SUBSAMPLE, xright * SUBSAMPLE, 0.,
			   0., ybot * SUBSAMPLE, xleft * SUBSAMPLE, 0.,
			   (xinput_airbrush_tool->c_direction_abs - M_PI),
			   xinput_airbrush_tool->c_direction);
}

static void
xinput_airbrush_button_press (Tool           *tool,
			      GdkEventButton *bevent,
			      gpointer        gdisp_ptr)
{
  gdouble             x, y;
  GDisplay            *gdisp;
  XinputAirbrushTool  *xinput_airbrush_tool;
  GimpDrawable        *drawable;
  AirBlob             *airblob;
  AirLine             *airline;
  MaskBuf             *brush_mask;
  int                 width, height;
  int                 x_min, y_min;
  guint               value;



  gdisp = (GDisplay *) gdisp_ptr;
  xinput_airbrush_tool = (XinputAirbrushTool *) tool->private;
  
  /*  Keep the coordinates of the target  */
  gdisplay_untransform_coords_f (gdisp, bevent->x, bevent->y,
				 &x, &y, TRUE);
  drawable = gimage_active_drawable (gdisp->gimage);
  
  xinput_airbrush_init (xinput_airbrush_tool, drawable, x, y);
  
  tool->state = ACTIVE;
  tool->gdisp_ptr = gdisp_ptr;
  tool->paused_count = 0;

  /*  pause the current selection and grab the pointer  */
  gdisplays_selection_visibility (gdisp->gimage, SelectionPause);

  /* add motion memory if you press mod1 first ^ perfectmouse */
  if (((bevent->state & GDK_MOD1_MASK) != 0) != (perfectmouse != 0))
    gdk_pointer_grab (gdisp->canvas->window, FALSE,
		      GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
		      NULL, NULL, bevent->time);
  else
    gdk_pointer_grab (gdisp->canvas->window, FALSE,
		      GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON1_MOTION_MASK |
		      GDK_BUTTON_RELEASE_MASK,
		      NULL, NULL, bevent->time);

  tool->gdisp_ptr = gdisp_ptr;
  tool->state = ACTIVE;


  /* Get the AirBlob from xinput_airbrush_pen_ellipse */
  
  airblob = xinput_airbrush_pen_ellipse (xinput_airbrush_tool, x, y,
					 bevent->pressure, bevent->xtilt,
					 bevent->ytilt);
  

  /* Make the AirBlob mask */

  value = 150;
    
  airline = create_air_line(airblob);
  x_min = airline->min_x;
  y_min = airline->min_y;
  width = airline->width;
  height = airline->height;
  
  brush_mask =  mask_buf_new(width, height);
  
  make_mask(airline, brush_mask, value);

  /*print_mask(brush_mask);*/

    /*Paint the mask on the drawable*/

  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);

    /*Prepare for next stroke*/

  xinput_airbrush_tool->last_airblob = airblob;
    

  time_smoother_init (xinput_airbrush_tool, bevent->time);
  xinput_airbrush_tool->last_time = bevent->time;
  dist_smoother_init (xinput_airbrush_tool, 0.0);
  xinput_airbrush_tool->init_velocity = TRUE;
  xinput_airbrush_tool->init_prepre = FALSE;
  xinput_airbrush_tool->lastx = xinput_airbrush_tool->xcenter;
  xinput_airbrush_tool->lasty = xinput_airbrush_tool->ycenter;
  
  /*Free the maks_buf and airline*/
  
  mask_buf_free(brush_mask);
  g_free(airline);

  gdisplay_flush_now (gdisp);

}

static void
  xinput_airbrush_button_release (Tool           *tool,
                                  GdkEventButton *bevent,
                                  gpointer        gdisp_ptr)
  {
    GDisplay * gdisp;
    GImage * gimage;
    XinputAirbrushTool * xinput_airbrush_tool;

    gdisp = (GDisplay *) gdisp_ptr;
    gimage = gdisp->gimage;
    xinput_airbrush_tool = (XinputAirbrushTool *) tool->private;

    /*  resume the current selection and ungrab the pointer  */
    gdisplays_selection_visibility (gdisp->gimage, SelectionResume);

    gdk_pointer_ungrab (bevent->time);
    gdk_flush ();

    /*  Set tool state to inactive -- no longer painting */
    tool->state = INACTIVE;

    xinput_airbrush_finish (xinput_airbrush_tool, gimage_active_drawable (gdisp->gimage), tool->ID);
    gdisplays_flush ();
  }


static void
  dist_smoother_init (XinputAirbrushTool* xinput_airbrush_tool, gdouble initval)
  {
    gint i;

    xinput_airbrush_tool->dt_index = 0;

    for (i=0; i<DIST_SMOOTHER_BUFFER; i++)
      {
        xinput_airbrush_tool->dt_buffer[i] = initval;
      }
  }

static gdouble
  dist_smoother_result (XinputAirbrushTool* xinput_airbrush_tool)
  {
    gint i;
    gdouble result = 0.0;

    for (i=0; i<DIST_SMOOTHER_BUFFER; i++)
      {
        result += xinput_airbrush_tool->dt_buffer[i];
      }

    return (result / (gdouble)DIST_SMOOTHER_BUFFER);
  }

static void
  dist_smoother_add (XinputAirbrushTool* xinput_airbrush_tool, gdouble value)
  {
    xinput_airbrush_tool->dt_buffer[xinput_airbrush_tool->dt_index] = value;

    if ((++xinput_airbrush_tool->dt_index) == DIST_SMOOTHER_BUFFER)
      xinput_airbrush_tool->dt_index = 0;
  }


static void
  time_smoother_init (XinputAirbrushTool* xinput_airbrush_tool, guint32 initval)
  {
    gint i;

    xinput_airbrush_tool->ts_index = 0;

    for (i=0; i<TIME_SMOOTHER_BUFFER; i++)
      {
        xinput_airbrush_tool->ts_buffer[i] = initval;
      }
  }

static gdouble
  time_smoother_result (XinputAirbrushTool* xinput_airbrush_tool)
  {
    gint i;
    guint64 result = 0;

    for (i=0; i<TIME_SMOOTHER_BUFFER; i++)
      {
        result += xinput_airbrush_tool->ts_buffer[i];
      }

#ifdef _MSC_VER
    return (gdouble) (gint64) (result / TIME_SMOOTHER_BUFFER);
#else
    return (result / TIME_SMOOTHER_BUFFER);
#endif
  }

static void
  time_smoother_add (XinputAirbrushTool* xinput_airbrush_tool, guint32 value)
  {
    xinput_airbrush_tool->ts_buffer[xinput_airbrush_tool->ts_index] = value;

    if ((++xinput_airbrush_tool->ts_index) == TIME_SMOOTHER_BUFFER)
      xinput_airbrush_tool->ts_index = 0;
  }


static void
  xinput_airbrush_motion (Tool           *tool,
                          GdkEventMotion *mevent,
                          gpointer        gdisp_ptr)
  {
    GDisplay *gdisp;
    XinputAirbrushTool  *xinput_airbrush_tool;
    GimpDrawable *drawable;
    AirBlob *airblob;

    double x, y;
    double pressure;
    double velocity;
    double dist;
    gdouble lasttime, thistime;
    guint last_value, present_value;
    double a, b, c, A;

    gboolean turn_around;


    last_value = present_value = 150;

    gdisp = (GDisplay *) gdisp_ptr;
    xinput_airbrush_tool = (XinputAirbrushTool *) tool->private;

    gdisplay_untransform_coords_f (gdisp, mevent->x, mevent->y, &x, &y, TRUE);
    drawable = gimage_active_drawable (gdisp->gimage);

    pressure = mevent->pressure;

    airblob = xinput_airbrush_pen_ellipse (xinput_airbrush_tool, x, y, pressure, mevent->xtilt, mevent->ytilt);

    x = xinput_airbrush_tool->xcenter;
    y = xinput_airbrush_tool->ycenter;

    lasttime = xinput_airbrush_tool->last_time;

    time_smoother_add (xinput_airbrush_tool, mevent->time);
    thistime = xinput_airbrush_tool->last_time =
                 time_smoother_result(xinput_airbrush_tool);

    /* The time resolution on X-based GDK motion events is
       bloody awful, hence the use of the smoothing function.
       Sadly this also means that there is always the chance of
       having an indeterminite velocity since this event and
       the previous several may still appear to issue at the same
       instant. -ADM */

    if (thistime == lasttime)
      thistime = lasttime + 1;

    if (xinput_airbrush_tool->init_velocity)
      {
        dist_smoother_init (xinput_airbrush_tool, dist = sqrt((xinput_airbrush_tool->lastx-x)*(xinput_airbrush_tool->lastx-x) +
                            (xinput_airbrush_tool->lasty-y)*(xinput_airbrush_tool->lasty-y)));
        xinput_airbrush_tool->init_velocity = FALSE;
      }
    else
      {
        dist_smoother_add (xinput_airbrush_tool, sqrt((xinput_airbrush_tool->lastx-x)*(xinput_airbrush_tool->lastx-x) +
                           (xinput_airbrush_tool->lasty-y)*(xinput_airbrush_tool->lasty-y)));
        dist = dist_smoother_result(xinput_airbrush_tool);
      }

    turn_around = FALSE;

    if (xinput_airbrush_tool->init_prepre)
      {
        a = hypot(xinput_airbrush_tool->last_lastx - x, xinput_airbrush_tool->last_lasty - y);
        b = hypot(xinput_airbrush_tool->lastx - x,  xinput_airbrush_tool->lasty - y);
        c = hypot(xinput_airbrush_tool->last_lastx - xinput_airbrush_tool->lastx, xinput_airbrush_tool->last_lasty - xinput_airbrush_tool->lasty);

        /* Maybe fix the fact that a, b or c can be zero i.e that we had a perfectly strait turn around or continue*/
        A = acos((b*b + c*c - a*a)/2*b*c);
        /* 1.65806 RAD == 95deg*/
        if ( (A >= 0.0) && (A < 1.65806) )
          {
            turn_around = TRUE;
	    /* printf("TURN_AROUND\n");*/
          }

      }

    velocity = 10.0 * sqrt((dist) / (double)(thistime - lasttime));

    
    
    make_stroke(airblob, xinput_airbrush_tool, drawable, last_value, present_value);

    xinput_airbrush_tool->last_lastx = xinput_airbrush_tool->lastx;
    xinput_airbrush_tool->last_lasty = xinput_airbrush_tool->lasty;
    xinput_airbrush_tool->lastx = x;
    xinput_airbrush_tool->lasty = y;

    g_free(xinput_airbrush_tool->last_airblob);
    xinput_airbrush_tool->last_airblob = airblob;

    xinput_airbrush_tool->init_velocity = TRUE;
    xinput_airbrush_tool->init_prepre = TRUE;
    
    gdisplay_flush_now (gdisp);
  }

static void
  xinput_airbrush_cursor_update (Tool           *tool,
                                 GdkEventMotion *mevent,
                                 gpointer        gdisp_ptr)
  {
    GDisplay *gdisp;
    Layer *layer;
    GdkCursorType ctype = GDK_TOP_LEFT_ARROW;
    int x, y;

    gdisp = (GDisplay *) gdisp_ptr;

    gdisplay_untransform_coords (gdisp, mevent->x, mevent->y, &x, &y, FALSE, FALSE);
    if ((layer = gimage_get_active_layer (gdisp->gimage)))
      {
        int off_x, off_y;
        drawable_offsets (GIMP_DRAWABLE(layer), &off_x, &off_y);
        if (x >= off_x && y >= off_y &&
            x < (off_x + drawable_width (GIMP_DRAWABLE(layer))) &&
            y < (off_y + drawable_height (GIMP_DRAWABLE(layer))))
          {
            /*  One more test--is there a selected region?
             *  if so, is cursor inside?
             */
            if (gimage_mask_is_empty (gdisp->gimage))
              ctype = GDK_PENCIL;
            else if (gimage_mask_value (gdisp->gimage, x, y))
              ctype = GDK_PENCIL;
          }
      }
    gdisplay_install_tool_cursor (gdisp, ctype);
  }

static void
  xinput_airbrush_control (Tool       *tool,
                           ToolAction  action,
                           gpointer    gdisp_ptr)
  {
    XinputAirbrushTool *xinput_airbrush_tool;

    xinput_airbrush_tool = (XinputAirbrushTool *) tool->private;

    switch (action)
      {
      case PAUSE:
        draw_core_pause (xinput_airbrush_tool->core, tool);
        break;

      case RESUME:
        draw_core_resume (xinput_airbrush_tool->core, tool);
        break;

      case HALT:
        xinput_airbrush_cleanup ();
        break;

      default:
        break;
      }
  }

static void
  xinput_airbrush_init (XinputAirbrushTool *xinput_airbrush_tool, GimpDrawable *drawable,
                        double x, double y)
  {
    /*  free the block structures  */
    if (undo_tiles)
      tile_manager_destroy (undo_tiles);
    if (canvas_tiles)
      tile_manager_destroy (canvas_tiles);

    /*  Allocate the undo structure  */
    undo_tiles = tile_manager_new (drawable_width (drawable),
                                   drawable_height (drawable),
                                   drawable_bytes (drawable));

    /*  Allocate the canvas blocks structure  */
    canvas_tiles = tile_manager_new (drawable_width (drawable),
                                     drawable_height (drawable), 1);

    /*  Get the initial undo extents  */
    xinput_airbrush_tool->x1 = xinput_airbrush_tool->x2 = x;
    xinput_airbrush_tool->y1 = xinput_airbrush_tool->y2 = y;
  }

static void
  xinput_airbrush_finish (XinputAirbrushTool *xinput_airbrush_tool, GimpDrawable *drawable, int tool_id)
  {
    /*  push an undo  */
    drawable_apply_image (drawable, xinput_airbrush_tool->x1, xinput_airbrush_tool->y1,
                          xinput_airbrush_tool->x2, xinput_airbrush_tool->y2, undo_tiles, TRUE);
    undo_tiles = NULL;

    /*  invalidate the drawable--have to do it here, because
     *  it is not done during the actual painting.
     */
    drawable_invalidate_preview (drawable);
  }

static void
  xinput_airbrush_cleanup (void)
  {
    /*  CLEANUP  */
    /*  If the undo tiles exist, nuke them  */
    if (undo_tiles)
      {
        tile_manager_destroy (undo_tiles);
        undo_tiles = NULL;
      }

    /*  If the canvas blocks exist, nuke them  */
    if (canvas_tiles)
      {
        tile_manager_destroy (canvas_tiles);
        canvas_tiles = NULL;
      }

    /*  Free the temporary buffer if it exist  */
    if (canvas_buf)
      temp_buf_free (canvas_buf);
    canvas_buf = NULL;
  }

/*********************************
 *  Rendering functions          *
 *********************************/

/* Some of this stuff should probably be combined with the
 * code it was copied from in paint_core.c; but I wanted
 * to learn this stuff, so I've kept it simple.
 */


static void
make_stroke(AirBlob *airblob,
	    XinputAirbrushTool *xinput_airbrush_tool,
	    GimpDrawable *drawable,
	    guint last_value,
	    guint present_value)
{

  int steps;
  int j;


  int dx, dy;
  int x, y;


  int x0, x1, y0, y1;

  double x0d, x1d, y0d, y1d;

  int x_min, y_min, width, height; 
 
  guint ivalue;
  double lv, pv, mv;

  double dist;

  int slopeterm;

  AirBlob *last_airblob;
  AirBlob *trans_blob;
  AirLine *air_line;
  MaskBuf *brush_mask;
    

  x0 = xinput_airbrush_tool->lastx;
  x1 = xinput_airbrush_tool->xcenter;
  y0 = xinput_airbrush_tool->lasty;
  y1 = xinput_airbrush_tool->ycenter;

  x0d = xinput_airbrush_tool->lastx;
  x1d = xinput_airbrush_tool->xcenter;
  y0d = xinput_airbrush_tool->lasty;
  y1d = xinput_airbrush_tool->ycenter;

  last_airblob = xinput_airbrush_tool->last_airblob;

  steps = number_of_steps(x0, y0, x1, y1);


  dx = abs(x0 - x1);
  dy = abs(y0 - y1);
      
  lv = last_value;
  pv = present_value;
  mv = pv - lv;


  /*
    Yes I know this is bulky code :-),
    --> see comment on make_mask.
    
    But I wanted to keep it simple while I 
    implemented the tool :-). I will make it 
    more effective later on. And yes I could 
    look for the three special cases.
  */

  
  if (dy > dx)
    {
      /* Step in the y direction*/
      if (y0 < y1)
	{
	  /* 
	     Step from y0 to y1 
	  */
	  if(x1 < x0)
	    {
	      /*
		We have to x--
	      */
	      
	      x = x0;
	      y = y0;
		
	      j = 1;

	      slopeterm = dx;

	      while(y < y1)
		{
		  if (slopeterm > dy)
		    {
		      slopeterm -= dy;
		      x--;
		      y++;
		    }
		  else
		    {
		      slopeterm += dx;
		      y++;
 		    }
			  
		  j++;
		
		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);
		  
		}

	    }
	  else
	    {
	      /* 
		 We have to x++
	      */

	      x = x0;
	      y = y0;

	      slopeterm = dx;
	      
	      j = 1;
	      
	      while(y < y1)
		{
		  if (slopeterm > dy)
		    {
		      slopeterm -= dy;
		      x++;
		      y++;
		    }
		  else
		    {
		      slopeterm += dx;
		      y++;
		    }
		  
		  j++;
		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);

		}
	    }
	}
      else
	{
	  /* Step from y0 to y1 with neg steps */
	  if(x1 < x0)
	    {
	      /*
		We have to x--
	      */

	      x = x0;
	      y = y0;

	      slopeterm = dx;
		  
	      j = 1;
		  
	      while(y > y1)
		{
		  if (slopeterm > dy)
		    {
		      slopeterm -= dy;
		      x--;
		      y--;
		    }
		  else
		    {
		      slopeterm += dx;
		      y--;
		    }

		  j++;
		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);

		}
	    }
	  else
	    {
	      /* 
		 We have to x++
	      */
	      x = x0;
	      y = y0;

	      slopeterm = dx;
	      
	      j = 1;
		  
	      while(y > y1)
		{
		  if (slopeterm > dy)
		    {
		      slopeterm -= dy;
		      x++;
		      y--;
		    }
		  else
		    {
		      slopeterm += dx;
		      y--;
		    }

		  j++;
		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable,  brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);

		}
	    }
	}

    }
  else 
    {
      /* Step in the X direction*/
      if (x0 < x1)
	{
	  /* Step from x0 to x1 */
	  if(y1 < y0)
	    {
	      /*
		We have to y--
	      */
	      x = x0;
	      y = y0;

	      slopeterm = dy;
	      
	      j = 1;
	      
	      while(x < x1)
		{
		  if (slopeterm > dx)
		    {
		      slopeterm -= dx;
		      y--;
		      x++;
		    }
		  else
		    {
		      slopeterm += dy;
		      x++;
		    }
		  
		  j++;
		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);

		}
	      
	    }
	  else
	    {
	      /* 
		 We have to y++
	      */
	      x = x0;
	      y = y0;
	      
	      slopeterm = dy;
	      
	      j = 1;
	      
	      while(x < x1)
		{
		  if (slopeterm > dx)
		    {
		      slopeterm -= dx;
		      y++;
		      x++;
		    }
		  else
		    {
		      slopeterm += dy;
		      x++;
		    }

		  j++;
		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);

		}
	    }
	}
      else
	{
	  /* Step from x0 to x1 neg direction */
	  if(y1 < y0)
	    {
	      /*
		We have to y--
	      */
	      x = x0;
	      y = y0;

	      slopeterm = dy;
		  
	      j = 1;
		  
	      while(x > x1)
		{
		  if (slopeterm > dx)
		    {
		      slopeterm -= dx;
		      y--;
		      x--;
		    }
		  else
		    {
		      slopeterm += dy;
		      x--;
		    }

		  j++;
		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);

		}
	    }
	  else
	    {
	      /* 
		 We have to y++
	      */
	      x = x0;
	      y = y0;

	      slopeterm = dy;
	      
	      j = 1;
	      
	      while(x > x1)
		{
		  if (slopeterm > dx)
		    {
		      slopeterm -= dx;
		      y++;
		      x--;
		    }
		  else
		    {
		      slopeterm += dy;
		      x--;
		    }

		  j++;

		  dist = 1.0 - (double)j/(double)steps;			    
		  ivalue = lv + mv * dist;	    
	    
		  trans_blob = trans_air_blob(last_airblob, airblob, dist, x, y);
		  air_line = create_air_line(trans_blob);

		  x_min = air_line->min_x;
		  y_min = air_line->min_y;
		  width = air_line->width;
		  height = air_line->height;
		  
		  brush_mask =  mask_buf_new(width, height);
  
		  make_mask(air_line, brush_mask, ivalue);

		  /*Paint the mask on the drawable*/

		  xinput_airbrush_paste (xinput_airbrush_tool, drawable, brush_mask, x_min, y_min, width, height);
		  mask_buf_free(brush_mask);
		  g_free(air_line);
		  g_free(trans_blob);
		}

	    }
	}
    } 
}



/**************************************************
***************************************************
*/


static void
make_mask(AirLine *airline,
	  MaskBuf *dest,	  
	  guint value)
{

  int steps;
  int i;
  int j;


  int dx, dy;
  int x, y;

  int x0, x1, y0, y1;
 

  unsigned char *s;
  guint midvalue;
  double ivalue;

  int rowlength; 
  int slopeterm;


  rowlength = dest->width * dest->bytes; 

  for (i=0; i < airline->nlines ; i++)
    {

      steps = number_of_steps(airline->xcenter, airline->ycenter,
			      airline->line[i].x, airline->line[i].y);

      x0 = airline->xcenter - airline->min_x;
      x1 = airline->line[i].x - airline->min_x;

      y0 = airline->ycenter - airline->min_y;
      y1 = airline->line[i].y -airline->min_y;


      dx = abs(x0 - x1);
      dy = abs(y0 - y1);
      


      /*
	Yes I know this is bulky code :-),
	you could insted set the x direction
	to 1 or -1, and y to - or + rowside.
	Then set the first X and Y and of you 
	go.
	
	But I wanted to keep it simple while I 
	implemented the tool :-). I will make it 
	more effective later on. And yes I could 
	look for the three special cases.
      */

      /*  if (i >= 3)
	{
	  midvalue = 0;
	}
      else
      {*/
	  midvalue = value;
	  /*	}*/


      if (dy > dx)
	{
	   /* Step in the y direction*/
	  if (y0 < y1)
	    {
	      /* 
		 Step from y0 to y1 
	      */
	      if(x1 < x0)
		{
		  /*
		    We have to x--
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dx;
		  
		  j = 1;
		  
		  while(y < y1)
		    {
		      if (slopeterm > dy)
			{
			  slopeterm -= dy;
			  x--;
			  y++;
			}
		      else
			{
			  slopeterm += dx;
			  y++;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }

		}
	      else
		{
		  /* 
		     We have to x++
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dx;
		  
		  j = 1;
		  
		  while(y < y1)
		    {
		      if (slopeterm > dy)
			{
			  slopeterm -= dy;
			  x++;
			  y++;
			}
		      else
			{
			  slopeterm += dx;
			  y++;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }
		}
	    }
	  else
	    {
	      /* Step from y0 to y1 with neg steps */
	      if(x1 < x0)
		{
		  /*
		    We have to x--
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dx;
		  
		  j = 1;
		  
		  while(y > y1)
		    {
		      if (slopeterm > dy)
			{
			  slopeterm -= dy;
			  x--;
			  y--;
			}
		      else
			{
			  slopeterm += dx;
			  y--;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }
		}
	      else
		{
		  /* 
		     We have to x++
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dx;
		  
		  j = 1;
		  
		  while(y > y1)
		    {
		      if (slopeterm > dy)
			{
			  slopeterm -= dy;
			  x++;
			  y--;
			}
		      else
			{
			  slopeterm += dx;
			  y--;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }
		}
	    }

	}
      else 
	{
	  /* Step in the X direction*/
	  if (x0 < x1)
	    {
	      /* Step from x0 to x1 */
	      if(y1 < y0)
		{
		  /*
		    We have to y--
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dy;
		  
		  j = 1;
		  
		  while(x < x1)
		    {
		      if (slopeterm > dx)
			{
			  slopeterm -= dx;
			  y--;
			  x++;
			}
		      else
			{
			  slopeterm += dy;
			  x++;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }

		}
	      else
		{
		  /* 
		     We have to y++
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dy;
		  
		  j = 1;
		  
		  while(x < x1)
		    {
		      if (slopeterm > dx)
			{
			  slopeterm -= dx;
			  y++;
			  x++;
			}
		      else
			{
			  slopeterm += dy;
			  x++;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
  		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }
		}
	    }
	  else
	    {
	      /* Step from x0 to x1 neg direction */
	      if(y1 < y0)
		{
		  /*
		    We have to y--
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dy;
		  
		  j = 1;
		  
		  while(x > x1)
		    {
		      if (slopeterm > dx)
			{
			  slopeterm -= dx;
			  y--;
			  x--;
			}
		      else
			{
			  slopeterm += dy;
			  x--;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
  		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }
		}
	      else
		{
		  /* 
		     We have to y++
		  */
		  s = dest->data;

		  x = x0;
		  y = y0;

		  s += y * rowlength + x;
		  
		  *s = midvalue;
		  
		  slopeterm = dy;
		  
		  j = 1;
		  
		  while(x > x1)
		    {
		      if (slopeterm > dx)
			{
			  slopeterm -= dx;
			  y++;
			  x--;
			}
		      else
			{
			  slopeterm += dy;
			  x--;
			}

		      j++;

		      s = dest->data;
		      s += y * rowlength + x;
		      
  		      ivalue = value * (1.0 - (double)j/(double)steps);

		      *s = ivalue;
		    }

		}
	    }
	}
    }

 
}




static void
xinput_airbrush_set_paint_area (XinputAirbrushTool      *xinput_airbrush_tool,
                                GimpDrawable            *drawable,
				int x, int y, int width, int height)
{
    int iwidth, iheight;
    int x1, y1, x2, y2;
    int bytes;
    int dwidth, dheight;

    bytes = drawable_has_alpha (drawable) ?
            drawable_bytes (drawable) : drawable_bytes (drawable) + 1;

    dwidth = drawable_width (drawable);
    dheight = drawable_height (drawable);


    x1 = CLAMP (x,            0, dwidth);
    y1 = CLAMP (y,            0, dheight);
    x2 = CLAMP ((x + width),  0, dwidth);
    y2 = CLAMP ((y + height), 0, dheight);

    iwidth = MIN((x2 - x1),  width);
    iheight = MIN((y2 - y1), height);



    /*  configure the canvas buffer  */

    if ((x2 - x1) && (y2 - y1))
      canvas_buf = temp_buf_resize (canvas_buf, bytes, x1, y1,
                                    iwidth, iheight);




}


static void
render_airbrush_line (AirBrushBlob *airbrush_blob, guchar *dest,
                        int y, int width, XinputAirbrushTool *xinput_airbrush_tool)
{
  int i, j, k, l, m, g;
  
  int left, right;

  int brush_width;

  double x_dest;
  double xdist, ydist, dist, angle;
  double i_value;
  guchar value;



  if (airbrush_blob->height <= 6)
    {
      return;
    }
  left = 0;
  right = 0;

  j = y * SUBSAMPLE;
  g = y * SUBSAMPLE;

  for (i=0; i<SUBSAMPLE; i++, j++)
    {
      if (j >= airbrush_blob->height)
	{
	  if ( i == 0)
	    {
	      return;
	    }
	  else
	    {
	      left = airbrush_blob->data[g].left - airbrush_blob->min_x;
	      right = airbrush_blob->data[g].right - airbrush_blob->min_x;
	      break;
	    }
	}
      if ( i == SUBSAMPLE/2)
	{
	  left = airbrush_blob->data[j].left - airbrush_blob->min_x;
	  right = airbrush_blob->data[j].right - airbrush_blob->min_x;
	  break;
	}

    }

  brush_width = right - left;

  if (brush_width <= SUBSAMPLE*2)
    {
      return;
    }

  dest = dest + (left/SUBSAMPLE);

  for (i=4; i <= brush_width; i = i + SUBSAMPLE)
    {
      x_dest = (left + i)/SUBSAMPLE;
      xdist  =  (airbrush_blob->min_x/SUBSAMPLE) + x_dest - xinput_airbrush_tool->xcenter;
      ydist  =  xinput_airbrush_tool->ycenter - (airbrush_blob->y/SUBSAMPLE) - y;
      angle	 =  atan2(ydist, xdist) + M_PI;
      dist   =  hypot(xdist, ydist);
      for (j=1; j< (airbrush_blob->height - 1) ; j++)
	{
	  k = j - 1;
	  l = j + 1;
	  m = 0;
	  if ( (fabs(airbrush_blob->data[k].angle_right_abs - airbrush_blob->data[l].angle_right_abs) >
		fabs(airbrush_blob->data[k].angle_right_abs - angle)) &&
	       (fabs(airbrush_blob->data[k].angle_right_abs - airbrush_blob->data[l].angle_right_abs) >
		fabs(airbrush_blob->data[l].angle_right_abs - angle))
             )
	    {
	      i_value = MIN(1., (dist*SUBSAMPLE)/(airbrush_blob->data[j].dist_right));
	      value = 255 * (1 - i_value);
	      *dest = value;
	      dest++;
	      m = 1;
	      break;
	    }

	  else if ( (fabs(airbrush_blob->data[k].angle_left_abs - airbrush_blob->data[l].angle_left_abs) >
		     fabs(airbrush_blob->data[k].angle_left_abs - angle)) &&
		    (fabs(airbrush_blob->data[k].angle_left_abs - airbrush_blob->data[l].angle_left_abs) >
		     fabs(airbrush_blob->data[l].angle_left_abs - angle))
		  )
	    {
	      i_value = MIN(1., (dist*SUBSAMPLE)/(airbrush_blob->data[j].dist_left));
	      value = 255  * (1 - i_value);
	      *dest = value;
	      dest++;
	      m = 1;
	      break;
	    }
	  else if ((j == (airbrush_blob->height - 2)) && ( m == 0))
	    {
	      *dest = 0;
	      dest++;
	    }
	}
    }


}



static void
calc_width (AirBrushBlob *airbrush_blob)
{
  int i;
  int min_x, max_x;

  i = (airbrush_blob->height)/2;

  min_x = airbrush_blob->data[i].left;
  max_x = airbrush_blob->data[i].right;

  for (i=0; i< (airbrush_blob->height) ; i++)

    {

      if ( airbrush_blob->data[i].left <= airbrush_blob->data[i].right)
	{
	  min_x = MIN(airbrush_blob->data[i].left, min_x);
	}


      if ( airbrush_blob->data[i].left <= airbrush_blob->data[i].right)
	{
	  max_x = MAX(airbrush_blob->data[i].right, max_x );
	}
    }

  airbrush_blob->width = max_x - min_x;
  airbrush_blob->min_x = min_x;
  airbrush_blob->max_x = max_x;
}


static void
calc_angle (AirBrushBlob *airbrush_blob, double xcenter, double ycenter)
{


  int i;
  double y_center, x_center;

  double y_dist, x_dist;
  double left_ang, right_ang;
  double left_ang_abs, right_ang_abs;
  int min_x, max_x;



  x_center = xcenter * SUBSAMPLE;
  y_center = ycenter * SUBSAMPLE - airbrush_blob->y;

  i = (airbrush_blob->height)/2;

  min_x = airbrush_blob->data[i].left;
  max_x = airbrush_blob->data[i].right;

  for (i=0; i< (airbrush_blob->height) ; i++)

    {

      y_dist = y_center - i;
      x_dist = airbrush_blob->data[i].left - x_center;
      left_ang_abs = atan2(y_dist, x_dist) + M_PI;
      left_ang = atan(y_dist/x_dist);
      airbrush_blob->data[i].angle_left = left_ang;
      airbrush_blob->data[i].angle_left_abs = left_ang_abs;
      airbrush_blob->data[i].dist_left = hypot(x_dist, y_dist);
      if ( airbrush_blob->data[i].left <= airbrush_blob->data[i].right)
	{
	  min_x = MIN(airbrush_blob->data[i].left, min_x);
	}

      x_dist = airbrush_blob->data[i].right - x_center;
      right_ang_abs = atan2(y_dist, x_dist) + M_PI;
      right_ang = atan(y_dist/x_dist);
      airbrush_blob->data[i].angle_right = right_ang;
      airbrush_blob->data[i].angle_right_abs = right_ang_abs;
      airbrush_blob->data[i].dist_right = hypot(x_dist, y_dist);
      if ( airbrush_blob->data[i].left <= airbrush_blob->data[i].right)
	{
	  max_x = MAX(airbrush_blob->data[i].right, max_x );
	}



    }

  airbrush_blob->width = max_x - min_x;
  airbrush_blob->min_x = min_x;
  airbrush_blob->max_x = max_x;
  
  
}



static void
make_single_mask(AirBrushBlob *airbrush_blob, XinputAirbrushTool *xinput_airbrush_tool, MaskBuf *dest)
{
  unsigned char * s;
  int h;
  int i;

  h = dest->height;
  s = dest->data;

  for (i=0; i<h; i++)
    {
      render_airbrush_line(airbrush_blob, s,
			   i, dest->width, xinput_airbrush_tool);
      s += (dest->width * dest->bytes);
    }
  
}

static void
print_line (guchar *dest,
	    int width)
{
  
  int i;

  for (i=0; i < width; i++)
    {
      printf("%d       ", *dest);
      dest++;
    }
}

static void
print_mask(MaskBuf *dest)
{
  unsigned char * s;
  int h;
  int i;

  h = dest->height;
  s = dest->data;
  printf("\nBrush_Mask\n\n");
  for (i=0; i<h; i++)
    {
      print_line( s,
		  dest->width);
      s += (dest->width * dest->bytes);
      printf("\n");
    }
   printf("\n");
}



static void
xinput_airbrush_paste (XinputAirbrushTool      *xinput_airbrush_tool,
		       GimpDrawable *drawable,
		       MaskBuf *brush_mask,
		       int x,
		       int y,
		       int width,
		       int height)
{
  GImage *gimage;
  PixelRegion srcPR;
  int offx, offy;
  unsigned char col[MAX_CHANNELS];

 
  /*print_mask(brush_mask);*/


  if (! (gimage = drawable_gimage (drawable)))
    return;


  /* Get the the buffer */
  xinput_airbrush_set_paint_area (xinput_airbrush_tool, drawable, x, y, width, height);

  /* check to make sure there is actually a canvas to draw on */
  if (!canvas_buf)
    return;  
  
  gimage_get_foreground (gimage, drawable, col);

  /*  set the alpha channel  */
  col[canvas_buf->bytes - 1] = OPAQUE_OPACITY;

  /*  color the pixels  */
  color_pixels (temp_buf_data (canvas_buf), col,
		canvas_buf->width * canvas_buf->height, canvas_buf->bytes);

  /*  set undo blocks  */
  xinput_airbrush_set_undo_tiles (drawable,
				  canvas_buf->x, canvas_buf->y,
				  canvas_buf->width, canvas_buf->height);

  /*  initialize any invalid canvas tiles  */
  xinput_airbrush_set_canvas_tiles (canvas_buf->x, canvas_buf->y,
				    canvas_buf->width, canvas_buf->height);

  /*DON'T FORGETT THE 100 VALUE, I.E. THE BRUSH OPACITY WHICH SHOULD BE 255*/

  xinput_airbrush_to_canvas_tiles (xinput_airbrush_tool, brush_mask, 100);
  
  xinput_airbrush_canvas_tiles_to_canvas_buf(xinput_airbrush_tool);


  /*  initialize canvas buf source pixel regions  */
  srcPR.bytes = canvas_buf->bytes;
  srcPR.x = 0;
  srcPR.y = 0;
  srcPR.w = canvas_buf->width;
  srcPR.h = canvas_buf->height;
  srcPR.rowstride = canvas_buf->width * canvas_buf->bytes;
  srcPR.data = temp_buf_data (canvas_buf);


  /*  apply the paint area to the gimage  */
  gimage_apply_image (gimage, drawable, &srcPR,
		      FALSE,
		      (int) (gimp_context_get_opacity (NULL) * 255),
		      gimp_context_get_paint_mode (NULL),
		      undo_tiles,  /*  specify an alternative src1  */
		      canvas_buf->x, canvas_buf->y);

  /*  Update the undo extents  */
  xinput_airbrush_tool->x1 = MIN (xinput_airbrush_tool->x1, canvas_buf->x);
  xinput_airbrush_tool->y1 = MIN (xinput_airbrush_tool->y1, canvas_buf->y);
  xinput_airbrush_tool->x2 = MAX (xinput_airbrush_tool->x2, (canvas_buf->x + canvas_buf->width));
  xinput_airbrush_tool->y2 = MAX (xinput_airbrush_tool->y2, (canvas_buf->y + canvas_buf->height));

  /*  Update the gimage--it is important to call gdisplays_update_area
   *  instead of drawable_update because we don't want the drawable
   *  preview to be constantly invalidated
   */
  drawable_offsets (drawable, &offx, &offy);
  gdisplays_update_area (gimage, canvas_buf->x + offx, canvas_buf->y + offy,
                           canvas_buf->width, canvas_buf->height);

}



static void
xinput_airbrush_to_canvas_tiles (XinputAirbrushTool *xinput_airbrush_tool,
				 MaskBuf *brush_mask,
				 int brush_opacity)
{
  PixelRegion srcPR, maskPR;

  /*   combine the brush mask and the canvas tiles  */
  pixel_region_init (&srcPR, canvas_tiles,
		     canvas_buf->x, canvas_buf->y,
		     canvas_buf->width, canvas_buf->height, TRUE);

  maskPR.bytes = 1;
  maskPR.x = 0;
  maskPR.y = 0;
  maskPR.w = srcPR.w;
  maskPR.h = srcPR.h;
  maskPR.rowstride = maskPR.bytes * brush_mask->width;
  maskPR.data = mask_buf_data (brush_mask);

  /*  combine the mask to the canvas tiles  */
  combine_mask_and_region (&srcPR, &maskPR, brush_opacity);
}




static void
xinput_airbrush_canvas_tiles_to_canvas_buf(XinputAirbrushTool *xinput_airbrush_tool)
{
  PixelRegion srcPR, maskPR;

  /*  combine the canvas tiles and the canvas buf  */
  srcPR.bytes = canvas_buf->bytes;
  srcPR.x = 0;
  srcPR.y = 0;
  srcPR.w = canvas_buf->width;
  srcPR.h = canvas_buf->height;
  srcPR.rowstride = canvas_buf->width * canvas_buf->bytes;
  srcPR.data = temp_buf_data (canvas_buf);

  pixel_region_init (&maskPR, canvas_tiles,
		     canvas_buf->x, canvas_buf->y,
		     canvas_buf->width, canvas_buf->height, FALSE);

  /*  apply the canvas tiles to the canvas buf  */
  apply_mask_to_region (&srcPR, &maskPR, OPAQUE_OPACITY);
}



static void
xinput_airbrush_set_undo_tiles (GimpDrawable *drawable,
				int x, int y, int w, int h)
{
  int i, j;
  Tile *src_tile;
  Tile *dest_tile;

  for (i = y; i < (y + h); i += (TILE_HEIGHT - (i % TILE_HEIGHT)))
    {
      for (j = x; j < (x + w); j += (TILE_WIDTH - (j % TILE_WIDTH)))
	{
	  dest_tile = tile_manager_get_tile (undo_tiles, j, i, FALSE, FALSE);
	  if (tile_is_valid (dest_tile) == FALSE)
	    {
	      src_tile = tile_manager_get_tile (drawable_data (drawable), j, i, TRUE, FALSE);
	      tile_manager_map_tile (undo_tiles, j, i, src_tile);
	      tile_release (src_tile, FALSE);
	    }
	}
    }
}


static void
xinput_airbrush_set_canvas_tiles (int x, int y, int w, int h)
{
  int i, j;
  Tile *tile;

  for (i = y; i < (y + h); i += (TILE_HEIGHT - (i % TILE_HEIGHT)))
    {
      for (j = x; j < (x + w); j += (TILE_WIDTH - (j % TILE_WIDTH)))
	{
	  tile = tile_manager_get_tile (canvas_tiles, j, i, FALSE, FALSE);
	  if (tile_is_valid (tile) == FALSE)
	    {
	      tile = tile_manager_get_tile (canvas_tiles, j, i, TRUE, TRUE);
	      memset (tile_data_pointer (tile, 0, 0),
		      0,
		      tile_size (tile));
	      tile_release (tile, TRUE);
	    }
	}
    }
}

/**************************/
/*  Global xinput_airbrush functions  */
/**************************/

void
xinput_airbrush_no_draw (Tool *tool)
{
  return;
}

Tool *
tools_new_xinput_airbrush (void)
{
  Tool * tool;
  XinputAirbrushTool * private;

  /*  The tool options  */
  if (! xinput_airbrush_options)
    {
      xinput_airbrush_options = xinput_airbrush_options_new ();
      tools_register (XINPUT_AIRBRUSH, (ToolOptions *) xinput_airbrush_options);

      /*  press all default buttons  */
      xinput_airbrush_options_reset ();
    }

  tool = tools_new_tool (XINPUT_AIRBRUSH);
  private = g_new (XinputAirbrushTool, 1);

  private->core = draw_core_new (xinput_airbrush_no_draw);
  private->last_airbrush_blob = NULL;

  tool->private = private;

  tool->button_press_func   = xinput_airbrush_button_press;
  tool->button_release_func = xinput_airbrush_button_release;
  tool->motion_func         = xinput_airbrush_motion;
  tool->cursor_update_func  = xinput_airbrush_cursor_update;
  tool->control_func        = xinput_airbrush_control;

  return tool;
}

void
tools_free_xinput_airbrush(Tool *tool)
{
  XinputAirbrushTool * xinput_airbrush_tool;

  xinput_airbrush_tool = (XinputAirbrushTool *) tool->private;

  /*  Make sure the selection core is not visible  */
  if (tool->state == ACTIVE && xinput_airbrush_tool->core)
    draw_core_stop (xinput_airbrush_tool->core, tool);

  /*  Free the selection core  */
  if (xinput_airbrush_tool->core)
    draw_core_free (xinput_airbrush_tool->core);
  
  /*  Free the last airbrush_blob, if any */
  if (xinput_airbrush_tool->last_airbrush_blob)
    g_free (xinput_airbrush_tool->last_airbrush_blob);

  /*  Cleanup memory  */
  xinput_airbrush_cleanup ();

  /*  Free the paint core  */
  g_free (xinput_airbrush_tool);
}
