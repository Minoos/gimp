/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * GimpConfigWriter
 * Copyright (C) 2003  Sven Neumann <sven@gimp.org>
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

#ifndef __GIMP_CONFIG_WRITER_H__
#define __GIMP_CONFIG_WRITER_H__


struct _GimpConfigWriter
{
  /*<  private  >*/
  gint      fd;
  gchar    *filename;
  gchar    *tmpname;
  GError   *error;
  GString  *buffer;
  gint      depth;
  gint      marker;
};


GimpConfigWriter * gimp_config_writer_new         (const gchar       *filename,
						   gboolean           safe,
						   const gchar       *header,
						   GError           **error);
GimpConfigWriter * gimp_config_writer_new_from_fd (gint               fd);

void               gimp_config_writer_open        (GimpConfigWriter  *writer,
						   const gchar       *name);
void               gimp_config_writer_print       (GimpConfigWriter  *writer,
						   const gchar       *string,
						   gint               len);
void               gimp_config_writer_printf      (GimpConfigWriter  *writer,
						   const gchar       *format,
						   ...);
void               gimp_config_writer_string      (GimpConfigWriter  *writer,
						   const gchar       *string);
void               gimp_config_writer_revert      (GimpConfigWriter  *writer);
void               gimp_config_writer_close       (GimpConfigWriter  *writer);

void               gimp_config_writer_linefeed    (GimpConfigWriter  *writer);
void               gimp_config_writer_comment     (GimpConfigWriter  *writer,
						   const gchar       *comment);

gboolean           gimp_config_writer_finish      (GimpConfigWriter  *writer,
						   const gchar       *footer,
						   GError           **error);


#endif /* __GIMP_CONFIG_WRITER_H__ */
