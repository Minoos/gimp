/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpstock.c
 * Copyright (C) 2001 Michael Natterer <mitch@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gtk/gtk.h>
#include <gtk/gtkiconfactory.h>

#include "gimpstock.h"

#include "themes/Default/images/gimp-stock-pixbufs.h"

#include "libgimp/libgimp-intl.h"


static GtkIconFactory *gimp_stock_factory = NULL;


static GtkIconSet *
sized_with_same_fallback_icon_set_from_inline (const guchar *inline_data,
					       GtkIconSize   size)
{
  GtkIconSource *source;
  GtkIconSet    *set;
  GdkPixbuf     *pixbuf;

  source = gtk_icon_source_new ();

  gtk_icon_source_set_size (source, size);
  gtk_icon_source_set_size_wildcarded (source, FALSE);

  pixbuf = gdk_pixbuf_new_from_inline (-1, inline_data, FALSE, NULL);

  g_assert (pixbuf);

  gtk_icon_source_set_pixbuf (source, pixbuf);

  g_object_unref (G_OBJECT (pixbuf));

  set = gtk_icon_set_new ();

  gtk_icon_set_add_source (set, source);

  gtk_icon_source_set_size_wildcarded (source, TRUE);
  gtk_icon_set_add_source (set, source);

  gtk_icon_source_free (source);

  return set;
}

static void
add_sized_with_same_fallback (GtkIconFactory *factory,
			      const guchar   *inline_data,
			      GtkIconSize     size,
			      const gchar    *stock_id)
{
  GtkIconSet *set;
  
  set = sized_with_same_fallback_icon_set_from_inline (inline_data, size);
  
  gtk_icon_factory_add (factory, stock_id, set);

  gtk_icon_set_unref (set);
}

static GtkStockItem gimp_stock_items[] =
{
  { GIMP_STOCK_ANCHOR,       N_("Anchor"),            0, 0, "gimp-libgimp" },
  { GIMP_STOCK_DUPLICATE,    N_("Duplicate"),         0, 0, "gimp-libgimp" },
  { GIMP_STOCK_EDIT,         N_("Edit"),              0, 0, "gimp-libgimp" },
  { GIMP_STOCK_LINKED,       N_("Linked"),            0, 0, "gimp-libgimp" },
  { GIMP_STOCK_PASTE_AS_NEW, N_("Paste as New"),      0, 0, "gimp-libgimp" },
  { GIMP_STOCK_PASTE_INTO,   N_("Paste Into"),        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_RESET,        N_("_Reset"),            0, 0, "gimp-libgimp" },
  { GIMP_STOCK_VISIBLE,      N_("Visible"),           0, 0, "gimp-libgimp" },

  { GIMP_STOCK_HCHAIN,                   NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_HCHAIN_BROKEN,            NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_VCHAIN,                   NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_VCHAIN_BROKEN,            NULL,        0, 0, "gimp-libgimp" },

  { GIMP_STOCK_SELECTION_REPLACE,        NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_SELECTION_ADD,            NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_SELECTION_SUBTRACT,       NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_SELECTION_INTERSECT,      NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_SELECTION_STROKE,         NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_SELECTION_TO_CHANNEL,     NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_SELECTION_TO_PATH,        NULL,        0, 0, "gimp-libgimp" },

  { GIMP_STOCK_PATH_STROKE,              NULL,        0, 0, "gimp-libgimp" },

  { GIMP_STOCK_CONVERT_RGB,              NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_CONVERT_GRAYSCALE,        NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_CONVERT_INDEXED,          NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_MERGE_DOWN,               NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_RESHOW_FILTER,            NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_ROTATE_90,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_ROTATE_180,               NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_ROTATE_270,               NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_RESIZE,                   NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_SCALE,                    NULL,        0, 0, "gimp-libgimp" },

  { GIMP_STOCK_NAVIGATION,               NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_QMASK_OFF,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_QMASK_ON,                 NULL,        0, 0, "gimp-libgimp" },

  { GIMP_STOCK_TOOL_OPTIONS,             NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_INFO,                     NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_WARNING,                  NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_WILBER,                   NULL,        0, 0, "gimp-libgimp" },

  { GIMP_STOCK_TOOL_AIRBRUSH,            NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_BEZIER_SELECT,       NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_BLEND,               NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_BLUR,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_BRIGHTNESS_CONTRAST, NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_BUCKET_FILL,         NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_BY_COLOR_SELECT,     NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_CLONE,               NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_COLOR_BALANCE,       NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_COLOR_PICKER,        NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_CROP,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_CURVES,              NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_DODGE,               NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_ELLIPSE_SELECT,      NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_ERASER,              NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_FLIP,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_FREE_SELECT,         NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_FUZZY_SELECT,        NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_HISTOGRAM,           NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_HUE_SATURATION,      NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_INK,                 NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_ISCISSORS,           NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_LEVELS,              NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_MEASURE,             NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_MOVE,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_PAINTBRUSH,          NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_PATH,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_PENCIL,              NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_PERSPECTIVE,  N_("Transform"),    0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_POSTERIZE,           NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_RECT_SELECT,         NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_ROTATE,       N_("Rotate"),       0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_SCALE,        N_("Scale"),        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_SHEAR,        N_("Shear"),        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_SMUDGE,              NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_TEXT,                NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_THRESHOLD,           NULL,        0, 0, "gimp-libgimp" },
  { GIMP_STOCK_TOOL_ZOOM,                NULL,        0, 0, "gimp-libgimp" }
};

static struct
{
  const gchar   *stock_id;
  gconstpointer  inline_data;
}
gimp_stock_button_pixbufs[] =
{
  { GIMP_STOCK_ANCHOR,                   stock_anchor_16                   },
  { GIMP_STOCK_DUPLICATE,                stock_duplicate_16                },
  { GIMP_STOCK_EDIT,                     stock_edit_16                     },
  { GIMP_STOCK_LINKED,                   stock_linked_20                   },
  { GIMP_STOCK_PASTE_AS_NEW,             stock_paste_as_new_16             },
  { GIMP_STOCK_PASTE_INTO,               stock_paste_into_16               },
  { GIMP_STOCK_RESET,                    stock_reset_16                    },
  { GIMP_STOCK_VISIBLE,                  stock_eye_20                      },

  { GIMP_STOCK_HCHAIN,                   stock_hchain_24                   },
  { GIMP_STOCK_HCHAIN_BROKEN,            stock_hchain_broken_24            },
  { GIMP_STOCK_VCHAIN,                   stock_vchain_24                   },
  { GIMP_STOCK_VCHAIN_BROKEN,            stock_vchain_broken_24            },

  { GIMP_STOCK_SELECTION_REPLACE,        stock_selection_replace_16        },
  { GIMP_STOCK_SELECTION_ADD,            stock_selection_add_16            },
  { GIMP_STOCK_SELECTION_SUBTRACT,       stock_selection_subtract_16       },
  { GIMP_STOCK_SELECTION_INTERSECT,      stock_selection_intersect_16      },
  { GIMP_STOCK_SELECTION_STROKE,         stock_selection_stroke_16         },
  { GIMP_STOCK_SELECTION_TO_PATH,        stock_selection_to_path_16        },

  { GIMP_STOCK_PATH_STROKE,              stock_path_stroke_16              },

  { GIMP_STOCK_TOOL_AIRBRUSH,            stock_tool_airbrush_22            },
  { GIMP_STOCK_TOOL_BEZIER_SELECT,       stock_tool_bezier_select_22       },
  { GIMP_STOCK_TOOL_BLEND,               stock_tool_blend_22               },
  { GIMP_STOCK_TOOL_BLUR,                stock_tool_blur_22                },
  { GIMP_STOCK_TOOL_BRIGHTNESS_CONTRAST, stock_tool_brightness_contrast_22 },
  { GIMP_STOCK_TOOL_BUCKET_FILL,         stock_tool_bucket_fill_22         },
  { GIMP_STOCK_TOOL_BY_COLOR_SELECT,     stock_tool_by_color_select_22     },
  { GIMP_STOCK_TOOL_CLONE,               stock_tool_clone_22               },
  { GIMP_STOCK_TOOL_COLOR_BALANCE,       stock_tool_color_balance_22       },
  { GIMP_STOCK_TOOL_COLOR_PICKER,        stock_tool_color_picker_22        },
  { GIMP_STOCK_TOOL_CROP,                stock_tool_crop_22                },
  { GIMP_STOCK_TOOL_CURVES,              stock_tool_curves_22              },
  { GIMP_STOCK_TOOL_DODGE,               stock_tool_dodge_22               },
  { GIMP_STOCK_TOOL_ELLIPSE_SELECT,      stock_tool_ellipse_select_22      },
  { GIMP_STOCK_TOOL_ERASER,              stock_tool_eraser_22              },
  { GIMP_STOCK_TOOL_FLIP,                stock_tool_flip_22                },
  { GIMP_STOCK_TOOL_FREE_SELECT,         stock_tool_free_select_22         },
  { GIMP_STOCK_TOOL_FUZZY_SELECT,        stock_tool_fuzzy_select_22        },
  { GIMP_STOCK_TOOL_HISTOGRAM,           stock_tool_histogram_22           },
  { GIMP_STOCK_TOOL_HUE_SATURATION,      stock_tool_hue_saturation_22      },
  { GIMP_STOCK_TOOL_INK,                 stock_tool_ink_22                 },
  { GIMP_STOCK_TOOL_ISCISSORS,           stock_tool_iscissors_22           },
  { GIMP_STOCK_TOOL_LEVELS,              stock_tool_levels_22              },
  { GIMP_STOCK_TOOL_MEASURE,             stock_tool_measure_22             },
  { GIMP_STOCK_TOOL_MOVE,                stock_tool_move_22                },
  { GIMP_STOCK_TOOL_PAINTBRUSH,          stock_tool_paintbrush_22          },
  { GIMP_STOCK_TOOL_PATH,                stock_tool_path_22                },
  { GIMP_STOCK_TOOL_PENCIL,              stock_tool_pencil_22              },
  { GIMP_STOCK_TOOL_PERSPECTIVE,         stock_tool_perspective_22         },
  { GIMP_STOCK_TOOL_POSTERIZE,           stock_tool_posterize_22           },
  { GIMP_STOCK_TOOL_RECT_SELECT,         stock_tool_rect_select_22         },
  { GIMP_STOCK_TOOL_ROTATE,              stock_tool_rotate_22              },
  { GIMP_STOCK_TOOL_SCALE,               stock_tool_scale_22               },
  { GIMP_STOCK_TOOL_SHEAR,               stock_tool_shear_22               },
  { GIMP_STOCK_TOOL_SMUDGE,              stock_tool_smudge_22              },
  { GIMP_STOCK_TOOL_TEXT,                stock_tool_text_22                },
  { GIMP_STOCK_TOOL_THRESHOLD,           stock_tool_threshold_22           },
  { GIMP_STOCK_TOOL_ZOOM,                stock_tool_zoom_22                },

  { GIMP_STOCK_INFO,                     stock_info_24                     },
  { GIMP_STOCK_WARNING,                  stock_warning_24                  },
  { GIMP_STOCK_WILBER_EEK,               stock_wilber_eek_64               }
};

static struct
{
  const gchar   *stock_id;
  gconstpointer  inline_data;
}
gimp_stock_menu_pixbufs[] =
{
  { GIMP_STOCK_CONVERT_RGB,          stock_convert_rgb_16          },
  { GIMP_STOCK_CONVERT_GRAYSCALE,    stock_convert_grayscale_16    },
  { GIMP_STOCK_CONVERT_INDEXED,      stock_convert_indexed_16      },
  { GIMP_STOCK_MERGE_DOWN,           stock_merge_down_16           },
  { GIMP_STOCK_RESHOW_FILTER,        stock_reshow_filter_16        },
  { GIMP_STOCK_ROTATE_90,            stock_rotate_90_16            },
  { GIMP_STOCK_ROTATE_180,           stock_rotate_180_16           },
  { GIMP_STOCK_ROTATE_270,           stock_rotate_270_16           },
  { GIMP_STOCK_RESIZE,               stock_resize_16               },
  { GIMP_STOCK_SCALE,                stock_scale_16                },
  { GIMP_STOCK_SELECTION_TO_CHANNEL, stock_selection_to_channel_16 },

  { GIMP_STOCK_NAVIGATION,           stock_navigation_16           },
  { GIMP_STOCK_QMASK_OFF,            stock_qmask_off_16            },
  { GIMP_STOCK_QMASK_ON,             stock_qmask_on_16             },

  { GIMP_STOCK_DEFAULT_COLORS,       stock_default_colors_12       },
  { GIMP_STOCK_SWAP_COLORS,          stock_swap_colors_16          },

  { GIMP_STOCK_TOOL_OPTIONS,         stock_tool_options_16         },
  { GIMP_STOCK_INFO,                 stock_info_16                 },
  { GIMP_STOCK_WARNING,              stock_warning_16              },
  { GIMP_STOCK_WILBER,               stock_wilber_16               }
};


void
gimp_stock_init (void)
{
  static gboolean initialized = FALSE;

  gint i;

  if (initialized)
    return;

  gimp_stock_factory = gtk_icon_factory_new ();

  for (i = 0; i < G_N_ELEMENTS (gimp_stock_button_pixbufs); i++)
    {
      add_sized_with_same_fallback (gimp_stock_factory,
				    gimp_stock_button_pixbufs[i].inline_data,
				    GTK_ICON_SIZE_BUTTON,
				    gimp_stock_button_pixbufs[i].stock_id);
    }

  for (i = 0; i < G_N_ELEMENTS (gimp_stock_menu_pixbufs); i++)
    {
      add_sized_with_same_fallback (gimp_stock_factory,
				    gimp_stock_menu_pixbufs[i].inline_data,
				    GTK_ICON_SIZE_MENU,
				    gimp_stock_menu_pixbufs[i].stock_id);
    }

  gtk_icon_factory_add_default (gimp_stock_factory);

  gtk_stock_add_static (gimp_stock_items, G_N_ELEMENTS (gimp_stock_items));

  initialized = TRUE;
}
