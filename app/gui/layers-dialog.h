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

#ifndef __LAYERS_DIALOG_H__
#define __LAYERS_DIALOG_H__


GtkWidget * layers_dialog_create (void);
void        layers_dialog_free   (void);
void        layers_dialog_update (GimpImage *gimage);
void        layers_dialog_flush  (void);
void        layers_dialog_clear  (void);


void   render_fs_preview                 (GtkWidget *widget,
					  GdkPixmap *pixmap);
void   render_preview                    (TempBuf   *preview_buf,
					  GtkWidget *preview_widget,
					  gint       width,
					  gint       height,
					  gint       channel);
void   layers_dialog_invalidate_previews (GimpImage *gimage);


#endif /* __LAYERS_DIALOG_H__ */
