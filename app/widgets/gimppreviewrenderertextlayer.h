/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimppreviewrenderertextlayer.h
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

#ifndef __GIMP_PREVIEW_RENDERER_TEXT_LAYER_H__
#define __GIMP_PREVIEW_RENDERER_TEXT_LAYER_H__

#include "gimppreviewrendererdrawable.h"

#define GIMP_TYPE_PREVIEW_RENDERER_TEXT_LAYER            (gimp_preview_renderer_text_layer_get_type ())
#define GIMP_PREVIEW_RENDERER_TEXT_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_PREVIEW_RENDERER_TEXT_LAYER, GimpPreviewRendererTextLayer))
#define GIMP_PREVIEW_RENDERER_TEXT_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_PREVIEW_RENDERER_TEXT_LAYER, GimpPreviewRendererTextLayerClass))
#define GIMP_IS_PREVIEW_RENDERER_TEXT_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, GIMP_TYPE_PREVIEW_RENDERER_TEXT_LAYER))
#define GIMP_IS_PREVIEW_RENDERER_TEXT_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_PREVIEW_RENDERER_TEXT_LAYER))
#define GIMP_PREVIEW_RENDERER_TEXT_LAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_PREVIEW_RENDERER_TEXT_LAYER, GimpPreviewRendererTextLayerClass))


typedef struct _GimpPreviewRendererTextLayerClass  GimpPreviewRendererTextLayerClass;

struct _GimpPreviewRendererTextLayer
{
  GimpPreviewRendererDrawable  parent_instance;
};

struct _GimpPreviewRendererTextLayerClass
{
  GimpPreviewRendererDrawableClass  parent_class;
};


GType   gimp_preview_renderer_text_layer_get_type (void) G_GNUC_CONST;


#endif /* __GIMP_PREVIEW_RENDERER_TEXT_LAYER_H__ */
