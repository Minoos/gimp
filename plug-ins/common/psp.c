/* GIMP plug-in to load and save Paint Shop Pro files (.PSP and .TUB)
 *
 * Copyright (C) 1999 Tor Lillqvist
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

/*
 *
 * Work in progress! Doesn't do anything at all useful yet.
 *
 * For a copy of the PSP file format documentation, surf to
 * http://www.jasc.com.
 *
 */

/* set to the level of debugging output you want, 0 for none */
#define PSP_DEBUG 2

/* the max number of layers that this plugin should try to load */
#define MAX_LAYERS 64

#define IFDBG(level) if (PSP_DEBUG >= level)

#include "config.h"

#include <glib.h>		/* We want glib.h first for some pretty obscure
				 * Win32 compilation issues.
				 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <gtk/gtk.h>

#include <zlib.h>

#include <libgimp/gimp.h>
#include <libgimp/stdplugins-intl.h>

#define gimp_message_printf(x) \
  G_STMT_START { \
    gchar *_t = g_strdup_printf x; \
    gimp_message (_t); \
    g_free (_t); \
  } G_STMT_END

/* The following was cut and pasted from the PSP file format
 * documentation version 3.0.(Minor stylistic changes done.)
 *
 * Note that the upcoming PSP version 6 writes PSP file format version
 * 4.0, but the documentation for that apparently isn't publicly
 * available (yet). The format is designed to be downward compatible,
 * however. The semantics of many of the additional fields and
 * block types can be deduced by reverse engineering.
 */

/*
 * To be on the safe side, here is the whole copyright notice from the
 * specification:
 *
 * The Paint Shop Pro File Format Specification (the Specification) is
 * copyright 1998 by Jasc Software, Inc. Jasc grants you a
 * nonexclusive license to use the Specification for the sole purposes
 * of developing software products(s) incorporating the
 * Specification. You are also granted the right to identify your
 * software product(s) as incorporating the Paint Shop Pro Format
 * (PSP) provided that your software in incorporating the
 * Specification complies with the terms, definitions, constraints and
 * specifications contained in the Specification and subject to the
 * following: DISCLAIMER OF WARRANTIES. THE SPECIFICATION IS PROVIDED
 * AS IS. JASC DISCLAIMS ALL OTHER WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 *
 * You are solely responsible for the selection, use, efficiency and
 * suitability of the Specification for your software products.  OTHER
 * WARRANTIES EXCLUDED. JASC SHALL NOT BE LIABLE FOR ANY DIRECT,
 * INDIRECT, CONSEQUENTIAL, EXEMPLARY, PUNITIVE OR INCIDENTAL DAMAGES
 * ARISING FROM ANY CAUSE EVEN IF JASC HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES. CERTAIN JURISDICTIONS DO NOT PERMIT
 * THE LIMITATION OR EXCLUSION OF INCIDENTAL DAMAGES, SO THIS
 * LIMITATION MAY NOT APPLY TO YOU.  IN NO EVENT WILL JASC BE LIABLE
 * FOR ANY AMOUNT GREATER THAN WHAT YOU ACTUALLY PAID FOR THE
 * SPECIFICATION. Should any warranties be found to exist, such
 * warranties shall be limited in duration to ninety (90) days
 * following the date you receive the Specification.
 *
 * Indemnification. By your inclusion of the Paint Shop Pro File
 * Format in your software product(s) you agree to indemnify and hold
 * Jasc Software, Inc. harmless from any and all claims of any kind or
 * nature made by any of your customers with respect to your software
 * product(s).
 *
 * Export Laws. You agree that you and your customers will not export
 * your software or Specification except in compliance with the laws
 * and regulations of the United States.
 *
 * US Government Restricted Rights. The Specification and any
 * accompanying materials are provided with Restricted Rights. Use,
 * duplication or disclosure by the Government is subject to
 * restrictions as set forth in subparagraph (c)(1)(ii) of The Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, or subparagraphs (c)(1) and (2) of the Commercial
 * Computer Software - Restricted Rights at 48 CFR 52.227-19, as
 * applicable. Contractor/manufacturer is Jasc Software, Inc., PO Box
 * 44997, Eden Prairie MN 55344.
 *
 * Jasc reserves the right to amend, modify, change, revoke or
 * withdraw the Specification at any time and from time to time. Jasc
 * shall have no obligation to support or maintain the Specification.  */

/* Block identifiers.
 */
typedef enum {
  PSP_IMAGE_BLOCK = 0,		/* General Image Attributes Block (main) */
  PSP_CREATOR_BLOCK,		/* Creator Data Block (main) */
  PSP_COLOR_BLOCK,		/* Color Palette Block (main and sub) */
  PSP_LAYER_START_BLOCK,	/* Layer Bank Block (main) */
  PSP_LAYER_BLOCK,		/* Layer Block (sub) */
  PSP_CHANNEL_BLOCK,		/* Channel Block (sub) */
  PSP_SELECTION_BLOCK,		/* Selection Block (main) */
  PSP_ALPHA_BANK_BLOCK,		/* Alpha Bank Block (main) */
  PSP_ALPHA_CHANNEL_BLOCK,	/* Alpha Channel Block (sub) */
  PSP_THUMBNAIL_BLOCK,		/* Thumbnail Block (main) */
  PSP_EXTENDED_DATA_BLOCK,	/* Extended Data Block (main) */
  PSP_TUBE_BLOCK		/* Picture Tube Data Block (main) */
} PSPBlockID;

/* Bitmap type.
 */
typedef enum {
  PSP_DIB_IMAGE = 0,		/* Layer color bitmap */
  PSP_DIB_TRANS_MASK,		/* Layer transparency mask bitmap */
  PSP_DIB_USER_MASK,		/* Layer user mask bitmap */
  PSP_DIB_SELECTION,		/* Selection mask bitmap */
  PSP_DIB_ALPHA_MASK,		/* Alpha channel mask bitmap */
  PSP_DIB_THUMBNAIL		/* Thumbnail bitmap */
} PSPDIBType;

/* Channel types.
 */
typedef enum {
  PSP_CHANNEL_COMPOSITE = 0,	/* Channel of single channel bitmap */
  PSP_CHANNEL_RED,		/* Red channel of 24 bit bitmap */
  PSP_CHANNEL_GREEN,		/* Green channel of 24 bit bitmap */
  PSP_CHANNEL_BLUE		/* Blue channel of 24 bit bitmap */
} PSPChannelType;

/* Possible metrics used to measure resolution.
 */
typedef enum {
  PSP_METRIC_UNDEFINED = 0,	/* Metric unknown */
  PSP_METRIC_INCH,		/* Resolution is in inches */
  PSP_METRIC_CM			/* Resolution is in centimeters */
} PSP_METRIC;


/* Possible types of compression.
 */
typedef enum {
  PSP_COMP_NONE = 0,		/* No compression */
  PSP_COMP_RLE,			/* RLE compression */
  PSP_COMP_LZ77			/* LZ77 compression */
} PSPCompression;

/* Picture tube placement mode.
 */
typedef enum {
  tpmRandom,			/* Place tube images in random intervals */
  tpmConstant			/* Place tube images in constant intervals */
} TubePlacementMode;

/* Picture tube selection mode.
 */
typedef enum {
  tsmRandom,			/* Randomly select the next image in  */
				/* tube to display */
  tsmIncremental,		/* Select each tube image in turn */
  tsmAngular,			/* Select image based on cursor direction */
  tsmPressure,			/* Select image based on pressure  */
				/* (from pressure-sensitive pad) */
  tsmVelocity			/* Select image based on cursor speed */
} TubeSelectionMode;

/* Extended data field types.
 */
typedef enum {
  PSP_XDATA_TRNS_INDEX = 0	/* Transparency index field */
} PSPExtendedDataID;

/* Creator field types.
 */
typedef enum {
  PSP_CRTR_FLD_TITLE = 0,	/* Image document title field */
  PSP_CRTR_FLD_CRT_DATE,	/* Creation date field */
  PSP_CRTR_FLD_MOD_DATE,	/* Modification date field */
  PSP_CRTR_FLD_ARTIST,		/* Artist name field */
  PSP_CRTR_FLD_CPYRGHT,		/* Copyright holder name field */
  PSP_CRTR_FLD_DESC,		/* Image document description field */
  PSP_CRTR_FLD_APP_ID,		/* Creating app id field */
  PSP_CRTR_FLD_APP_VER		/* Creating app version field */
} PSPCreatorFieldID;

/* Creator application identifiers.
 */
typedef enum {
  PSP_CREATOR_APP_UNKNOWN = 0,	/* Creator application unknown */
  PSP_CREATOR_APP_PAINT_SHOP_PRO /* Creator is Paint Shop Pro */
} PSPCreatorAppID;

/* Layer types.
 */
typedef enum {
  PSP_LAYER_NORMAL = 0,		/* Normal layer */
  PSP_LAYER_FLOATING_SELECTION	/* Floating selection layer */
} PSPLayerType;

/* Truth values.
 */
#if 0				/* FALSE and TRUE taken by GLib */
typedef enum {
  FALSE = 0,
  TRUE
} PSP_BOOLEAN;
#else
typedef gboolean PSP_BOOLEAN;
#endif

/* End of cut&paste from psp spec */

/* The following have been reverse engineered.
 * If a new version of the spec becomes available,
 * change to use the type and constant names from it.
 */
typedef enum {
  PSP_BLEND_NORMAL = 0,
  PSP_BLEND_DARKEN,
  PSP_BLEND_LIGHTEN,
  PSP_BLEND_HUE,
  PSP_BLEND_SATURATION,
  PSP_BLEND_COLOR,
  PSP_BLEND_LUMINANCE,
  PSP_BLEND_MULTIPLY,
  PSP_BLEND_SCREEN,
  PSP_BLEND_DISSOLVE,
  PSP_BLEND_OVERLAY,
  PSP_BLEND_HARD_LIGHT,
  PSP_BLEND_SOFT_LIGHT,
  PSP_BLEND_DIFFERENCE,
  PSP_BLEND_DODGE,
  PSP_BLEND_BURN,
  PSP_BLEND_EXCLUSION
} PSPLayerBlendModes;

/* End of reverse engineered types */

/* We store the various PSP data in own structures.
 * We cannot use structs intended to be direct copies of the file block
 * headers because of struct alignment issues.
 */
typedef struct
{
  guint32 width, height;
  gdouble resolution;
  PSPCompression compression;
  guint16 depth;
  gboolean greyscale;
  guint32 active_layer;
  guint16 layer_count;
} PSPimage;

/* Declare some local functions.
 */
static void   query      (void);
static void   run        (char    *name,
                          int      nparams,
                          GParam  *param,
                          int     *nreturn_vals,
                          GParam **return_vals);
static gint32 load_image (char   *filename);
static gint   save_image (char   *filename,
			  gint32  image_ID,
			  gint32  drawable_ID);

/* Various local variables...
 */
GPlugInInfo PLUG_IN_INFO =
{
  NULL,    /* init_proc */
  NULL,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};

static const gchar *prog_name = "PSP";

/* Save info  */
typedef struct
{
  PSPCompression compression;
} PSPSaveVals;

typedef struct
{
  gint  run;
} PSPSaveInterface;

static PSPSaveVals psvals =
{
  PSP_COMP_LZ77
};

static PSPSaveInterface psint =
{
  FALSE				/* run */
};

static guint16 major, minor;


MAIN()

static void
query ()
{
  static GParamDef load_args[] =
  {
    { PARAM_INT32, "run_mode", "Interactive, non-interactive" },
    { PARAM_STRING, "filename", "The name of the file to load" },
    { PARAM_STRING, "raw_filename", "The name of the file to load" },
  };
  static GParamDef load_return_vals[] =
  {
    { PARAM_IMAGE, "image", "Output image" },
  };
  static int nload_args = sizeof (load_args) / sizeof (load_args[0]);
  static int nload_return_vals = sizeof (load_return_vals) / sizeof (load_return_vals[0]);

  static GParamDef save_args[] =
  {
    { PARAM_INT32, "run_mode", "Interactive, non-interactive" },
    { PARAM_IMAGE, "image", "Input image" },
    { PARAM_DRAWABLE, "drawable", "Drawable to save" },
    { PARAM_STRING, "filename", "The name of the file to save the image in" },
    { PARAM_STRING, "raw_filename", "The name of the file to save the image in" },
    { PARAM_INT32, "compression", "Specify 0 for no compression, "
      "1 for RLE, and 2 for LZ77" }
  };
  static int nsave_args = sizeof (save_args) / sizeof (save_args[0]);

  INIT_I18N();

  gimp_install_procedure ("file_psp_load",
                          "loads images from the Paint Shop Pro PSP file format",
                          "This filter loads and saves images in "
			  "Paint Shop Pro's native PSP format. "
			  "These images may be of any type supported by GIMP, "
			  "with or without layers, layer masks, "
			  "or aux channels.",
                          "Tor Lillqvist",
                          "Tor Lillqvist",
                          "1999",
                          "<Load>/PSP",
			  NULL,
                          PROC_PLUG_IN,
                          nload_args, nload_return_vals,
                          load_args, load_return_vals);

  gimp_install_procedure ("file_psp_save",
                          "saves images in the Paint Shop Pro PSP file format",
                          "This filter loads and saves images in "
			  "Paint Shop Pro's native PSP format. "
			  "These images may be of any type supported by GIMP, "
			  "with or without layers, layer masks, "
			  "or aux channels.",
                          "Tor Lillqvist",
                          "Tor Lillqvist",
                          "1999",
                          "<Save>/PSP",
			  "RGB*, GRAY*, INDEXED*",
                          PROC_PLUG_IN,
                          nsave_args, 0,
                          save_args, NULL);

  gimp_register_magic_load_handler ("file_psp_load", "psp,tub", "",
				    "0,string,Paint Shop Pro Image File\n\032");
  gimp_register_save_handler ("file_psp_save", "psp,tub", "");
}

static void
save_toggle_update (GtkWidget *widget,
		    gpointer   data)
{
  int *toggle_val;

  toggle_val = (int *) data;

  if (GTK_TOGGLE_BUTTON (widget)->active)
    *toggle_val = TRUE;
  else
    *toggle_val = FALSE;
}

static void
save_close_callback (GtkWidget *widget,
		     gpointer   data)
{
  gtk_main_quit ();
}

static void
save_ok_callback (GtkWidget *widget,
		  gpointer   data)
{
  psint.run = TRUE;
  gtk_widget_destroy (GTK_WIDGET (data));
}

static gint
save_dialog ()
{
  GtkWidget *dlg;
  GtkWidget *button;
  GtkWidget *toggle;
  GtkWidget *frame;
  GtkWidget *toggle_vbox;
  GSList *group;
  gchar **argv;
  gint argc;
  gint use_none = (psvals.compression == PSP_COMP_NONE);
  gint use_rle = (psvals.compression == PSP_COMP_RLE);
  gint use_lz77 = (psvals.compression == PSP_COMP_LZ77);

  argc = 1;
  argv = g_new (gchar *, 1);
  argv[0] = g_strdup ("save");

  gtk_init (&argc, &argv);
  gtk_rc_parse (gimp_gtkrc ());

  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dlg), "Save as PSP");
  gtk_window_position (GTK_WINDOW (dlg), GTK_WIN_POS_MOUSE);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
		      (GtkSignalFunc) save_close_callback,
		      NULL);

  /*  Action area  */
  button = gtk_button_new_with_label ("OK");
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      (GtkSignalFunc) save_ok_callback,
                      dlg);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label ("Cancel");
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT (dlg));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  /*  file save type  */
  frame = gtk_frame_new ("Data Compression");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
  gtk_container_border_width (GTK_CONTAINER (frame), 10);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), frame, FALSE, TRUE, 0);
  toggle_vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_border_width (GTK_CONTAINER (toggle_vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), toggle_vbox);

  group = NULL;
  toggle = gtk_radio_button_new_with_label (group, "None");
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (toggle_vbox), toggle, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
		      (GtkSignalFunc) save_toggle_update,
		      &use_none);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), use_none);
  gtk_widget_show (toggle);

  toggle = gtk_radio_button_new_with_label (group, "RLE");
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (toggle_vbox), toggle, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
		      (GtkSignalFunc) save_toggle_update,
		      &use_rle);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), use_rle);
  gtk_widget_show (toggle);

  toggle = gtk_radio_button_new_with_label (group, "LZ77");
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (toggle_vbox), toggle, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
		      (GtkSignalFunc) save_toggle_update,
		      &use_lz77);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), use_lz77);
  gtk_widget_show (toggle);

  gtk_widget_show (toggle_vbox);
  gtk_widget_show (frame);

  gtk_widget_show (dlg);

  gtk_main ();
  gdk_flush ();

  if (use_none)
    psvals.compression = PSP_COMP_NONE;
  else if (use_rle)
    psvals.compression = PSP_COMP_RLE;
  else
    psvals.compression = PSP_COMP_LZ77;

  return psint.run;
}


static char *
block_name (int id)
{
  static char *block_names[] =
  {
    "IMAGE",
    "CREATOR",
    "COLOR",
    "LAYER_START",
    "LAYER",
    "CHANNEL",
    "SELECTION",
    "ALPHA_BANK",
    "ALPHA_CHANNEL",
    "THUMBNAIL",
    "EXTENDED_DATA",
    "TUBE"
  };
  static gchar *err_name = NULL;

  if (id >= 0 && id <= PSP_TUBE_BLOCK)
    return block_names[id];

  g_free (err_name);
  err_name = g_strdup_printf ("id=%d", id);

  return err_name;
}

static int
read_block_header (FILE *f,
		   guint32 *init_len,
		   guint32 *total_len)
{
  guchar buf[4];
  guint16 id;
  long header_start;
  guint32 len;

  IFDBG(2) header_start = ftell (f);

  if (fread (buf, 4, 1, f) < 1
      || fread (&id, 2, 1, f) < 1
      || fread (&len, 4, 1, f) < 1
      || (major < 4 && fread (total_len, 4, 1, f) < 1))
    {
      gimp_message ("PSP: Error reading block header");
      fclose (f);
      return -1;
    }
  if (memcmp (buf, "~BK\0", 4) != 0)
    {
      gimp_message_printf (("PSP: Invalid block header at %d", header_start));
      fclose (f);
      return -1;
    }

  IFDBG(2) gimp_message_printf (("PSP: %s at %d", block_name (id),
				 header_start));

  if (major < 4)
    {
      *init_len = GUINT32_FROM_LE (len);
      *total_len = GUINT32_FROM_LE (*total_len);
    }
  else
    {
      /* Version 4.0 seems to have dropped the initial data chunk length
       * field.
       */
      *total_len = *init_len = GUINT32_FROM_LE (len);
    }

  return id;
}

static int
read_general_image_attribute_block (FILE *f,
				    guint init_len,
				    guint total_len,
				    PSPimage *ia)
{
  char buf[6];
#ifdef G_HAVE_GINT64
  gint64 res[1];
#else
  guchar res[8];
#endif
  guchar metric;
  guint16 compression;
  guchar greyscale;

  if (init_len < 38 || total_len < 38)
    {
      gimp_message ("PSP: Invalid general image attribute chunk size");
      fclose (f);
      return -1;
    }

  if (major >= 4)
    fseek (f, 4, SEEK_CUR);
  
  if (fread (&ia->width, 4, 1, f) < 1
      || fread (&ia->height, 4, 1, f) < 1
      || fread (res, 8, 1, f) < 1
      || fread (&metric, 1, 1, f) < 1
      || fread (&compression, 1, 1, f) < 1
      || fread (&ia->depth, 2, 1, f) < 1
      || fread (buf, 2+4, 1, f) < 1 /* Skip plane and colour count */
      || fread (&greyscale, 1, 1, f) < 1
      || fread (buf, 4, 1, f) < 1 /* Skip total image size */
      || fread (&ia->active_layer, 4, 1, f) < 1
      || fread (&ia->layer_count, 2, 1, f) < 1)
    {
      gimp_message ("PSP: Error reading general image attribute block");
      fclose (f);
      return -1;
    }
  ia->height = GUINT32_FROM_LE (ia->height);
  ia->width = GUINT32_FROM_LE (ia->width);
  ia->width = GUINT32_FROM_LE (ia->width);

#ifdef G_HAVE_GINT64
  res[0] = GUINT64_FROM_LE (res[0]);
#else
#if G_BYTE_ORDER == G_BIG_ENDIAN
  {
    /* Swap bytes in the double */
    guchar t;
    t = res[0];
    res[0] = res[7];
    res[7] = t;
    t = res[1];
    res[1] = res[6];
    res[6] = t;
    t = res[2];
    res[2] = res[5];
    res[5] = t;
    t = res[3];
    res[3] = res[4];
    res[4] = t;
  }
#endif
#endif
  memcpy (&ia->resolution, res, 8);
  if (metric == PSP_METRIC_CM)
    ia->resolution /= 2.54;

  ia->compression = compression;

  return 0;
}

static int
try_fseek (FILE *f, long pos, int whence)
{
  if (fseek (f, pos, whence) < 0)
    {
      gimp_message ("PSP: Seek error");
      fclose (f);
      return -1;
    }
  return 0;
}

static int
read_creator_block (FILE *f,
		    gint image_ID,
		    guint total_len,
		    PSPimage *ia)
{
  long data_start;
  guchar buf[4];
  guint16 keyword;
  guint32 length;
  gchar *string;
  gchar *title = NULL, *artist = NULL, *copyright = NULL, *description = NULL;
  guint32 dword;
  guint32 cdate = 0, mdate = 0, appid, appver;
  GString *comment;
  Parasite *comment_parasite;

  data_start = ftell (f);
  comment = g_string_new (NULL);

  while (ftell (f) < data_start + total_len)
    {
      if (fread (buf, 4, 1, f) < 1
	  || fread (&keyword, 2, 1, f) < 1
	  || fread (&length, 4, 1, f) < 1)
	{
	  gimp_message ("PSP: Error reading creator keyword chunk");
	  fclose (f);
	  gimp_image_delete (image_ID);
	  return -1;
	}
      if (memcmp (buf, "~FL\0", 4) != 0)
	{
	  gimp_message ("PSP: Invalid keyword chunk header");
	  fclose (f);
	  gimp_image_delete (image_ID);
	  return -1;
	}
      keyword = GUINT16_FROM_LE (keyword);
      length = GUINT32_FROM_LE (length);
      switch (keyword)
	{
	case PSP_CRTR_FLD_TITLE:
	case PSP_CRTR_FLD_ARTIST:
	case PSP_CRTR_FLD_CPYRGHT:
	case PSP_CRTR_FLD_DESC:
	  string = g_malloc (length + 1);
	  if (fread (string, length, 1, f) < 1)
	    {
	      gimp_message ("PSP: Error reading creator keyword data");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	  switch (keyword)
	    {
	    case PSP_CRTR_FLD_TITLE:
	      g_free (title); title = string; break;
	    case PSP_CRTR_FLD_ARTIST:
	      g_free (artist); artist = string; break;
	    case PSP_CRTR_FLD_CPYRGHT:
	      g_free (copyright); copyright = string; break;
	    case PSP_CRTR_FLD_DESC:
	      g_free (description); description = string; break;
	    default:
	      g_free (string);
	    }
	  break;
	case PSP_CRTR_FLD_CRT_DATE:
	case PSP_CRTR_FLD_MOD_DATE:
	case PSP_CRTR_FLD_APP_ID:
	case PSP_CRTR_FLD_APP_VER:
	  if (fread (&dword, 4, 1, f) < 1)
	    {
	      gimp_message ("PSP: Error reading creator keyword data");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	  switch (keyword)
	    {
	    case PSP_CRTR_FLD_CRT_DATE:
	      cdate = dword; break;
	    case PSP_CRTR_FLD_MOD_DATE:
	      mdate = dword; break;
	    case PSP_CRTR_FLD_APP_ID:
	      appid = dword; break;
	    case PSP_CRTR_FLD_APP_VER:
	      appver = dword; break;
	    }
	  break;
	default:
	  if (try_fseek (f, length, SEEK_CUR) < 0)
	    {
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	  break;
	}
    }

  if (title)
    {
      g_string_append (comment, title);
      g_free (title);
      g_string_append (comment, "\n");
    }
  if (artist)
    {
      g_string_append (comment, artist);
      g_free (artist);
      g_string_append (comment, "\n");
    }
  if (copyright)
    {
      g_string_append (comment, "Copyright ");
      g_string_append (comment, copyright);
      g_free (copyright);
      g_string_append (comment, "\n");
    }
  if (description)
    {
      g_string_append (comment, description);
      g_free (description);
      g_string_append (comment, "\n");
    }
  if (comment->len > 0)
    comment_parasite = parasite_new("gimp-comment", PARASITE_PERSISTENT,
				    strlen (comment->str) + 1, comment->str);
  g_string_free (comment, FALSE);
}

static void inline
swab_rect (guint32 *rect)
{
  rect[0] = GUINT32_FROM_LE (rect[0]);
  rect[1] = GUINT32_FROM_LE (rect[1]);
  rect[2] = GUINT32_FROM_LE (rect[2]);
  rect[3] = GUINT32_FROM_LE (rect[3]);
}

static GLayerMode
gimp_layer_mode_from_psp_blend_mode (PSPLayerBlendModes mode)
{
  switch (mode)
    {
    case PSP_BLEND_NORMAL:
      return NORMAL_MODE;
    case PSP_BLEND_DARKEN:
      return DARKEN_ONLY_MODE;
    case PSP_BLEND_HUE:
      return HUE_MODE;
    case PSP_BLEND_SATURATION:
      return SATURATION_MODE;
    case PSP_BLEND_COLOR:
      return COLOR_MODE;
    case PSP_BLEND_LUMINANCE:
      return VALUE_MODE;	/* ??? */
    case PSP_BLEND_MULTIPLY:
      return MULTIPLY_MODE;
    case PSP_BLEND_SCREEN:
      return SCREEN_MODE;
    case PSP_BLEND_DISSOLVE:
      return DISSOLVE_MODE;
    case PSP_BLEND_OVERLAY:
      return OVERLAY_MODE;
    case PSP_BLEND_HARD_LIGHT:
    case PSP_BLEND_SOFT_LIGHT:
      return -1;
    case PSP_BLEND_DIFFERENCE:
      return DIFFERENCE_MODE;
    case PSP_BLEND_DODGE:
    case PSP_BLEND_BURN:
    case PSP_BLEND_EXCLUSION:
      return -1;		/* ??? */
    }
  return -1;
}

static char *
blend_mode_name (PSPLayerBlendModes mode)
{
  static char *blend_mode_names[] =
  {
    "NORMAL",
    "DARKEN",
    "LIGHTEN",
    "HUE",
    "SATURATION",
    "COLOR",
    "LUMINANCE",
    "MULTIPLY",
    "SCREEN",
    "DISSOLVE",
    "OVERLAY",
    "HARD_LIGHT",
    "SOFT_LIGHT",
    "DIFFERENCE",
    "DODGE",
    "BURN",
    "EXCLUSION"
  };
  static gchar *err_name = NULL;

  if (mode >= 0 && mode <= PSP_BLEND_EXCLUSION)
    return blend_mode_names[mode];

  g_free (err_name);
  err_name = g_strdup_printf ("%d", mode);

  return err_name;
}

static int
read_layer_block (FILE *f,
		  gint image_ID,
		  guint total_len,
		  PSPimage *ia)
{
  long block_start, sub_block_start, channel_start;
  int sub_id;
  guint32 sub_init_len, sub_total_len;
  guchar *name;
  guint16 namelen;
  guchar type, opacity, blend_mode, visibility, transparency_protected;
  guchar link_group_id, mask_linked, mask_disabled;
  guint32 image_rect[4], saved_image_rect[4], mask_rect[4], saved_mask_rect[4];
  gboolean width_bumped = FALSE, height_bumped = FALSE;
  guint16 bitmap_count, channel_count;
  guint32 layer_ID;
  GLayerMode layer_mode;
  guint32 channel_init_len, channel_total_len;
  guint32 compressed_len, uncompressed_len;
  guint16 bitmap_type, channel_type;

  block_start = ftell (f);

  while (ftell (f) < block_start + total_len)
    {
      /* Read the layer sub-block header */
      sub_id = read_block_header (f, &sub_init_len, &sub_total_len);
      if (sub_id == -1)
	{
	  gimp_image_delete (image_ID);
	  return -1;
	}
      if (sub_id != PSP_LAYER_BLOCK)
	{
	  gimp_message_printf (("PSP: Invalid layer sub-block %s, "
				"should be LAYER",
				block_name (sub_id)));
	  fclose (f);
	  gimp_image_delete (image_ID);
	  return -1;
	}

      sub_block_start = ftell (f);

      /* Read layer information chunk */
      if (major >= 4)
	{
	  if (fseek (f, 4, SEEK_CUR) < 0
	      || fread (&namelen, 2, 1, f) < 1
	      || ((namelen = GUINT16_FROM_LE (namelen)) && FALSE)
	      || (name = g_malloc (namelen + 1)) == NULL
	      || fread (name, namelen, 1, f) < 1
	      || fread (&type, 1, 1, f) < 1
	      || fread (&image_rect, 16, 1, f) < 1
	      || fread (&saved_image_rect, 16, 1, f) < 1
	      || fread (&opacity, 1, 1, f) < 1
	      || fread (&blend_mode, 1, 1, f) < 1
	      || fread (&visibility, 1, 1, f) < 1
	      || fread (&transparency_protected, 1, 1, f) < 1
	      || fread (&link_group_id, 1, 1, f) < 1
	      || fread (&mask_rect, 16, 1, f) < 1
	      || fread (&saved_mask_rect, 16, 1, f) < 1
	      || fread (&mask_linked, 1, 1, f) < 1
	      || fread (&mask_disabled, 1, 1, f) < 1
	      || fseek (f, 43, SEEK_CUR) < 0
	      || fread (&bitmap_count, 1, 1, f) < 1
	      || fread (&channel_count, 1, 1, f) < 1)
	    {
	      gimp_message ("PSP: Error reading layer information chunk");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	  name[namelen] = 0;
	  type = PSP_LAYER_NORMAL; /* ??? */
	}
      else
	{
	  name = g_malloc (257);
	  name[256] = 0;
	  if (fread (name, 256, 1, f) < 1
	      || fread (&type, 1, 1, f) < 1
	      || fread (&image_rect, 16, 1, f) < 1
	      || fread (&saved_image_rect, 16, 1, f) < 1
	      || fread (&opacity, 1, 1, f) < 1
	      || fread (&blend_mode, 1, 1, f) < 1
	      || fread (&visibility, 1, 1, f) < 1
	      || fread (&transparency_protected, 1, 1, f) < 1
	      || fread (&link_group_id, 1, 1, f) < 1
	      || fread (&mask_rect, 16, 1, f) < 1
	      || fread (&saved_mask_rect, 16, 1, f) < 1
	      || fread (&mask_linked, 1, 1, f) < 1
	      || fread (&mask_disabled, 1, 1, f) < 1
	      || fseek (f, 43, SEEK_CUR) < 0
	      || fread (&bitmap_count, 1, 1, f) < 1
	      || fread (&channel_count, 1, 1, f) < 1)
	    {
	      gimp_message ("PSP: Error reading layer information chunk");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	}

      if (type == PSP_LAYER_FLOATING_SELECTION)
	gimp_message ("PSP: Floating selection restored as normal layer");

      swab_rect (image_rect);
      swab_rect (saved_image_rect);
      swab_rect (mask_rect);
      swab_rect (saved_mask_rect);

      layer_mode = gimp_layer_mode_from_psp_blend_mode (blend_mode);
      if ((int) layer_mode == -1)
	{
	  gimp_message_printf (("PSP: Unsupported PSP layer blend mode %s "
				"for layer %s, setting layer invisible",
				blend_mode_name (blend_mode), name));
	  layer_mode = NORMAL_MODE;
	  visibility = FALSE;
	}

      IFDBG(2) gimp_message_printf
	(("PSP: layer: %s %dx%d (%dx%d) opacity %d blend_mode %d", name,
	  image_rect[2] - image_rect[0],
	  image_rect[3] - image_rect[1],
	  saved_image_rect[2] - saved_image_rect[0],
	  saved_image_rect[3] - saved_image_rect[1],
	  opacity, blend_mode));

      if (saved_image_rect[0] == saved_image_rect[2])
	{
	  saved_image_rect[2]++;
	  width_bumped = TRUE;
	}
      if (saved_image_rect[1] == saved_image_rect[3])
	{
	  saved_image_rect[3]++;
	  height_bumped = TRUE;
	}

      layer_ID = gimp_layer_new (image_ID, name,
				 saved_image_rect[2] - saved_image_rect[0],
				 saved_image_rect[3] - saved_image_rect[1],
				 /* XXX */
				 ia->greyscale ? GRAYA_IMAGE : RGBA_IMAGE,
				 opacity / 255.0,
				 /* XXX */
				 layer_mode);
      if (layer_ID == -1)
	{
	  gimp_message ("PSP: Error creating layer");
	  fclose (f);
	  gimp_image_delete (image_ID);
	  return -1;
	}

      if (saved_image_rect[0] != 0 || saved_image_rect[1] != 0)
	gimp_layer_set_offsets (layer_ID,
				saved_image_rect[0], saved_image_rect[1]);

      if (!visibility)
	gimp_layer_set_visible (layer_ID, FALSE);
      
      gimp_layer_set_preserve_transparency (layer_ID, transparency_protected);

      gimp_image_add_layer (image_ID, layer_ID, -1);

      if (try_fseek (f, sub_block_start + sub_init_len, SEEK_SET) < 0)
	{
	  gimp_image_delete (image_ID);
	  return -1;
	}

      /* Read the layer channel sub-blocks */
      while (ftell (f) < sub_block_start + sub_total_len)
	{
	  sub_id = read_block_header (f, &channel_init_len,
				      &channel_total_len);
	  if (sub_id == -1)
	    {
	      gimp_image_delete (image_ID);
	      return -1;
	    }

	  if (sub_id != PSP_CHANNEL_BLOCK)
	    {
	      gimp_message_printf (("PSP: Invalid layer sub-block %s, "
				    "should be CHANNEL",
				    block_name (sub_id)));
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }

	  channel_start = ftell (f);

	  if (fread (&compressed_len, 4, 1, f) < 1
	      || fread (&uncompressed_len, 4, 1, f) < 1
	      || fread (&bitmap_type, 2, 1, f) < 1
	      || fread (&channel_type, 2, 1, f) < 1)
	    {
	      gimp_message ("PSP: Error reading channel information chunk");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }

	  if (GUINT16_FROM_LE (bitmap_type) > PSP_DIB_USER_MASK)
	    {
	      gimp_message ("PSP: Invalid bitmap type "
			    "in channel information chunk");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }

	  if (GUINT16_FROM_LE (channel_type) > PSP_CHANNEL_BLUE)
	    {
	      gimp_message ("PSP: Invalid channel type "
			    "in channel information chunk");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }

	  if (try_fseek (f, channel_start + channel_init_len, SEEK_SET) < 0)
	    {
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	  
	  /* Read channel data */

	  

	  if (try_fseek (f, channel_start + channel_total_len, SEEK_SET) < 0)
	    {
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	}
    }
  if (try_fseek (f, block_start + total_len, SEEK_SET) < 0)
    {
      gimp_image_delete (image_ID);
      return -1;
    }
  return layer_ID;
}

static gint32
load_image (char *filename)
{
  FILE *f;
  struct stat st;
  char buf[32];
  PSPimage ia;
  guint32 block_init_len, block_total_len;
  long block_start;
  PSPBlockID id;
  gint block_number;

  GPixelRgn pixel_rgn;
  gint32 image_ID = -1;
  gint32 layer_ID;
  GDrawable *drawable;

  if (stat (filename, &st) == -1)
    return -1;

  f = fopen (filename, "rb");
  if (f == NULL)
    return -1;

  /* Read thePSP File Header */
  if (fread (buf, 32, 1, f) < 1
      || fread (&major, 2, 1, f) < 1
      || fread (&minor, 2, 1, f) < 1)
    {
      gimp_message ("PSP: Error reading file header");
      fclose (f);
      return -1;
    }
  if (memcmp (buf, "Paint Shop Pro Image File\n\032\0\0\0\0\0", 32) != 0)
    {
      gimp_message ("PSP: Incorrect file signature");
      fclose (f);
      return -1;
    }

  major = GUINT16_FROM_LE (major);
  minor = GUINT16_FROM_LE (minor);
  /* I only have the documentation for file format version 3.0,
   * but PSP 6 writes version 4.0. Let's hope it's backwards compatible.
   * Earlier versions probably don't have all the fields I expect
   * so don't accept those.
   */
  if (major < 3)
    {
      gimp_message_printf (("PSP: Unsupported PSP file format version "
			    "%d.%d, only knows 3.0 (and later?)",
			    major, minor));
      fclose (f);
      return -1;
    }
  else if (major == 3)
    ; /* OK */
  else if (major == 4 && minor == 0)
    gimp_message ("PSP: Warning: PSP file format version "
		  "4.0. Support for this format version "
		  "is based on reverse engineering, "
		  "as no documentation has been made available");
  else
    {
      gimp_message_printf (("PSP: Unsupported PSP file format version %d.%d",
			    major, minor));
      fclose (f);
      return -1;
    }

  /* Read all the blocks */
  block_number = 0;

  IFDBG(3) gimp_message_printf (("PSP: size = %d", st.st_size));
  while (ftell (f) != st.st_size
	 && (id = read_block_header (f, &block_init_len,
				     &block_total_len)) != -1)
    {
      block_start = ftell (f);

      if (id == PSP_IMAGE_BLOCK)
	{
	  if (block_number != 0)
	    {
	      gimp_message ("PSP: Duplicate General Image Attributes block");
	      fclose (f);
	      return -1;
	    }
	  if (read_general_image_attribute_block (f, block_init_len,
						  block_total_len, &ia) == -1)
	    return -1;

	  IFDBG(2) gimp_message_printf (("PSP: resolution: %d dpi "
					 "width: %d height %d",
					 (int) ia.resolution,
					 ia.width, ia.height));

	  image_ID = gimp_image_new (ia.width, ia.height,
				     ia.greyscale ? GRAY : RGB);
	  if (image_ID == -1)
	    return -1;

	  gimp_image_set_filename (image_ID, filename);
	  gimp_image_set_resolution (image_ID, (int) ia.resolution,
				     (int) ia.resolution);
	}
      else
	{
	  if (block_number == 0)
	    {
	      gimp_message ("PSP: Missing General Image Attributes block");
	      fclose (f);
	      gimp_image_delete (image_ID);
	      return -1;
	    }
	  switch (id)
	    {
	    case PSP_CREATOR_BLOCK:
	      if (read_creator_block (f, image_ID, block_total_len, &ia) == -1)
		return -1;
	      break;
	    case PSP_COLOR_BLOCK:
	      break;		/* Not yet implemented */
	    case PSP_LAYER_START_BLOCK:
	      if (read_layer_block (f, image_ID, block_total_len, &ia) == -1)
		return -1;
	      break;
	    case PSP_SELECTION_BLOCK:
	      break;		/* Not yet implemented */
	    case PSP_ALPHA_BANK_BLOCK:
	      break;		/* Not yet implemented */
	    case PSP_THUMBNAIL_BLOCK:
	      break;		/* Not yet implemented */
	    case PSP_EXTENDED_DATA_BLOCK:
	      break;		/* Not yet implemented */
	    case PSP_TUBE_BLOCK:
	      break;		/* Not yet implemented */

	    case PSP_LAYER_BLOCK:
	    case PSP_CHANNEL_BLOCK:
	    case PSP_ALPHA_CHANNEL_BLOCK:
	      gimp_message_printf (("PSP: Sub-block %s should not occur "
				    "at main level of file",
				    block_name (id)));
	      break;

	    default:
	      gimp_message_printf (("PSP: Unrecognized block id %d", id));
	      break;
	    }
	}

      if (block_start + block_total_len >= st.st_size)
	break;

      if (try_fseek (f, block_start + block_total_len, SEEK_SET) < 0)
	{
	  gimp_image_delete (image_ID);
	  return -1;
	}
      block_number++;
    }

  if (id == -1)
    {
      fclose (f);
      gimp_image_delete (image_ID);
      return -1;
    }

  fclose (f);

  return image_ID;
}

static gint
save_image (char   *filename,
	    gint32  image_ID,
	    gint32  drawable_ID)
{
  return 42;
}

static void
run (char    *name,
     int      nparams,
     GParam  *param,
     int     *nreturn_vals,
     GParam **return_vals)
{
  static GParam values[2];
  GRunModeType run_mode;
  GStatusType status = STATUS_SUCCESS;
  gint32 image_ID;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals = values;
  values[0].type = PARAM_STATUS;
  values[0].data.d_status = STATUS_CALLING_ERROR;

  if (strcmp (name, "file_psp_load") == 0)
    {
      image_ID = load_image (param[1].data.d_string);

      if (image_ID != -1)
	{
	  *nreturn_vals = 2;
	  values[0].data.d_status = STATUS_SUCCESS;
	  values[1].type = PARAM_IMAGE;
	  values[1].data.d_image = image_ID;
	}
      else
	{
	  values[0].data.d_status = STATUS_EXECUTION_ERROR;
	}
    }
  else if (strcmp (name, "file_psp_save") == 0)
    {
      switch (run_mode)
	{
	case RUN_INTERACTIVE:
	  /*  Possibly retrieve data  */
	  gimp_get_data ("file_pnm_save", &psvals);

	  /*  First acquire information with a dialog  */
	  if (! save_dialog ())
	    return;
	  break;

	case RUN_NONINTERACTIVE:
	  /*  Make sure all the arguments are there!  */
	  if (nparams != 6
	      || param[5].data.d_int32 < 0
	      || param[5].data.d_int32 > PSP_COMP_LZ77)
	    status = STATUS_CALLING_ERROR;
	  if (status == STATUS_SUCCESS)
	    {
	      psvals.compression = (param[5].data.d_int32) ? TRUE : FALSE;
	    }

	case RUN_WITH_LAST_VALS:
	  gimp_get_data ("file_psp_save", &psvals);
	  break;

	default:
	  break;
	}

      if (status == STATUS_SUCCESS)
	{
	  if (save_image (param[3].data.d_string, param[1].data.d_int32, param[2].data.d_int32))
	    {
	      gimp_set_data ("file_psp_save", &psvals, sizeof (PSPSaveVals));

	      status = STATUS_SUCCESS;
	    }
	  else
	    status = STATUS_EXECUTION_ERROR;
	}

      values[0].data.d_status = status;
    }
}
