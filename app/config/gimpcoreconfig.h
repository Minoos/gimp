/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * GimpCoreConfig class
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org>
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

#ifndef __GIMP_CORE_CONFIG_H__
#define __GIMP_CORE_CONFIG_H__

#include "core/core-enums.h"

#include "config/gimpbaseconfig.h"


#define GIMP_TYPE_CORE_CONFIG            (gimp_core_config_get_type ())
#define GIMP_CORE_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_CORE_CONFIG, GimpCoreConfig))
#define GIMP_CORE_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_CORE_CONFIG, GimpCoreConfigClass))
#define GIMP_IS_CORE_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_CORE_CONFIG))
#define GIMP_IS_CORE_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_CORE_CONFIG))


typedef struct _GimpCoreConfigClass GimpCoreConfigClass;

struct _GimpCoreConfig
{
  GimpBaseConfig         parent_instance;

  GimpInterpolationType  interpolation_type;
  gchar                 *plug_in_path;
  gchar                 *module_path;
  gchar                 *environ_path;
  gchar                 *brush_path;
  gchar                 *brush_path_writable;
  gchar                 *pattern_path;
  gchar                 *pattern_path_writable;
  gchar                 *palette_path;
  gchar                 *palette_path_writable;
  gchar                 *gradient_path;
  gchar                 *gradient_path_writable;
  gchar                 *font_path;
  gchar                 *font_path_writable;
  gchar                 *default_brush;
  gchar                 *default_pattern;
  gchar                 *default_palette;
  gchar                 *default_gradient;
  gchar                 *default_font;
  gboolean               global_brush;
  gboolean               global_pattern;
  gboolean               global_palette;
  gboolean               global_gradient;
  gboolean               global_font;
  GimpTemplate          *default_image;
  GimpGrid              *default_grid;
  gint                   levels_of_undo;
  guint64                undo_size;
  GimpViewSize           undo_preview_size;
  gchar                 *plug_in_rc_path;
  gboolean               layer_previews;
  GimpViewSize           layer_preview_size;
  GimpThumbnailSize      thumbnail_size;
  guint64                thumbnail_filesize_limit;
  gboolean               install_cmap;
  gint                   min_colors;
};

struct _GimpCoreConfigClass
{
  GimpBaseConfigClass  parent_class;
};


GType  gimp_core_config_get_type (void) G_GNUC_CONST;


#endif /* GIMP_CORE_CONFIG_H__ */
