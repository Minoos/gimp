/* LIBGIMP - The GIMP Library 
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball                
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

#ifndef __GIMP_COLOR_SPACE_H__
#define __GIMP_COLOR_SPACE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*  Color conversion routines  */


/*  GimpRGB function  */

void   gimp_rgb_to_hsv          (const GimpRGB *rgb,
				 GimpHSV       *hsv);

void   gimp_rgb_to_hsl          (const GimpRGB *rgb,
				 gdouble       *hue,
				 gdouble       *saturation,
				 gdouble       *lightness);

void   gimp_hsv_to_rgb          (const GimpHSV *hsv,
				 GimpRGB       *rgb);

void   gimp_hsl_to_rgb          (gdouble        hue,
				 gdouble        saturation,
				 gdouble        lightness,
				 GimpRGB       *rgb);

void   gimp_rgb_to_hwb          (const GimpRGB *rgb,
				 gdouble       *hue,
				 gdouble       *whiteness,
				 gdouble       *blackness);
void   gimp_hwb_to_rgb          (gdouble        hue,
				 gdouble        whiteness,
				 gdouble        blackness,
				 GimpRGB       *rgb);


/*  gint functions  */

void    gimp_rgb_to_hsv_int	(gint    *red         /* returns hue        */,
				 gint    *green       /* returns saturation */,
				 gint    *blue        /* returns value      */);
void    gimp_hsv_to_rgb_int	(gint    *hue         /* returns red        */,
				 gint    *saturation  /* returns green      */,
				 gint    *value       /* returns blue       */);

void    gimp_rgb_to_hls_int	(gint    *red         /* returns hue        */,
				 gint    *green       /* returns lightness  */,
				 gint    *blue        /* returns saturation */);
gint    gimp_rgb_to_l_int	(gint     red,
				 gint     green,
				 gint     blue);
void    gimp_hls_to_rgb_int	(gint    *hue         /* returns red        */,
				 gint    *lightness   /* returns green      */,
				 gint    *saturation  /* returns blue       */);


/*  gdouble functions  */

void    gimp_rgb_to_hsv_double	(gdouble *red         /* returns hue        */,
				 gdouble *green       /* returns saturation */,
				 gdouble *blue        /* returns value      */);
void    gimp_hsv_to_rgb_double	(gdouble *hue         /* returns red        */,
				 gdouble *saturation, /* returns green      */
				 gdouble *value       /* returns blue       */);

void    gimp_rgb_to_hsv4        (guchar  *hsv,
				 gdouble *red,
				 gdouble *green,
				 gdouble *blue);
void    gimp_hsv_to_rgb4        (guchar  *rgb,
				 gdouble  hue,
				 gdouble  saturation,
				 gdouble  value);


G_END_DECLS

#endif  /* __GIMP_COLOR_SPACE_H__ */
