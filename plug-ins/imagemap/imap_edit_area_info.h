/*
 * This is a plug-in for the GIMP.
 *
 * Generates clickable image maps.
 *
 * Copyright (C) 1998-1999 Maurits Rijk  lpeek.mrijk@consunet.nl
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
 *
 */

#ifndef _IMAP_EDIT_AREA_INFO_H
#define _IMAP_EDIT_AREA_INFO_H

typedef struct AreaInfoDialog_t AreaInfoDialog_t;

#include "gtk/gtk.h"

#include "imap_default_dialog.h"
#include "imap_object.h"

struct AreaInfoDialog_t {
   DefaultDialog_t *dialog;
   Object_t 	   *obj;
   gboolean	    add;

   GtkWidget	   *notebook;
   GtkWidget	   *web_site;
   GtkWidget	   *ftp_site;
   GtkWidget	   *gopher;
   GtkWidget	   *other;
   GtkWidget	   *file;
   GtkWidget	   *wais;
   GtkWidget	   *telnet;
   GtkWidget	   *email;
   GtkWidget	   *url;
   GtkWidget	   *relative_link;
   GtkWidget	   *target;
   GtkWidget	   *comment;
   GtkWidget	   *mouse_over;
   GtkWidget	   *mouse_out;
   GtkWidget	   *focus;
   GtkWidget	   *blur;
   GtkWidget 	   *browse;
   gpointer	    infotab;
};

AreaInfoDialog_t *create_edit_area_info_dialog(Object_t *obj);
void edit_area_info_dialog_show(AreaInfoDialog_t *dialog, Object_t *obj,
				gboolean add);

#endif /* _IMAP_EDIT_AREA_INFO_H */
