/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimppreviewrenderertextlayer.c
 * Copyright (C) 2003 Michael Natterer <mitch@gimp.org>
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

#include "widgets-types.h"

#include "core/gimpviewable.h"

#include "gimppreviewrenderertextlayer.h"


static void   gimp_preview_renderer_text_layer_class_init (GimpPreviewRendererTextLayerClass *klass);
static void   gimp_preview_renderer_text_layer_init       (GimpPreviewRendererTextLayer      *renderer);

static void   gimp_preview_renderer_text_layer_render     (GimpPreviewRenderer *renderer,
                                                           GtkWidget           *widget);


static GimpPreviewRendererDrawableClass *parent_class = NULL;


GType
gimp_preview_renderer_text_layer_get_type (void)
{
  static GType renderer_type = 0;

  if (! renderer_type)
    {
      static const GTypeInfo renderer_info =
      {
        sizeof (GimpPreviewRendererTextLayerClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gimp_preview_renderer_text_layer_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GimpPreviewRendererTextLayer),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gimp_preview_renderer_text_layer_init,
      };

      renderer_type = g_type_register_static (GIMP_TYPE_PREVIEW_RENDERER_DRAWABLE,
                                              "GimpPreviewRendererTextLayer",
                                              &renderer_info, 0);
    }

  return renderer_type;
}

static void
gimp_preview_renderer_text_layer_class_init (GimpPreviewRendererTextLayerClass *klass)
{
  GimpPreviewRendererClass *renderer_class;

  renderer_class = GIMP_PREVIEW_RENDERER_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  renderer_class->render = gimp_preview_renderer_text_layer_render;
}

static void
gimp_preview_renderer_text_layer_init (GimpPreviewRendererTextLayer *renderer)
{
}

static void
gimp_preview_renderer_text_layer_render (GimpPreviewRenderer *renderer,
                                         GtkWidget           *widget)
{
  const gchar *stock_id;

  stock_id = gimp_viewable_get_stock_id (renderer->viewable);

  gimp_preview_renderer_default_render_stock (renderer, widget, stock_id);
}
