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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__


void   tools_default_colors_cmd_callback    (GtkWidget *widget,
					     gpointer   data);
void   tools_swap_colors_cmd_callback       (GtkWidget *widget,
					     gpointer   data);
void   tools_swap_contexts_cmd_callback     (GtkWidget *widget,
					     gpointer   data);
void   tools_select_cmd_callback            (GtkWidget *widget,
					     gpointer   data,
					     guint      action);

void   filters_repeat_cmd_callback          (GtkWidget *widget,
					     gpointer   data,
					     guint      action);

void   help_help_cmd_callback               (GtkWidget *widget,
					     gpointer   data);
void   help_context_help_cmd_callback       (GtkWidget *widget,
					     gpointer   data);


#endif /* __COMMANDS_H__ */
