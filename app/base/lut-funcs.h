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

#ifndef __LUT_FUNCS_H__
#define __LUT_FUNCS_H__


/* brightness contrast */
GimpLut * brightness_contrast_lut_new   (gdouble  brightness,
					 gdouble  contrast,
					 gint     n_channels);
void      brightness_contrast_lut_setup (GimpLut *lut,
					 gdouble  brightness,
					 gdouble  contrast,
					 gint     n_channels);

/* invert */
GimpLut * invert_lut_new                (gint     n_channels);
void      invert_lut_setup              (GimpLut *lut,
					 gint     n_channels);

/* add (or subtract) */
GimpLut * add_lut_new                   (gdouble  amount,
					 gint     n_channels);
void      add_lut_setup                 (GimpLut *lut,
					 gdouble  amount,
					 gint     n_channels);

/* intersect (MIN (pixel, value)) */
GimpLut * intersect_lut_new             (gdouble  value,
					 gint     n_channels);
void      intersect_lut_setup           (GimpLut *lut,
					 gdouble  value,
					 gint     n_channels);

/* threshold */
GimpLut * threshold_lut_new             (gdouble  value,
					 gint     n_channels);
void      threshold_lut_setup           (GimpLut *lut,
					 gdouble  value,
					 gint     n_channels);

/* levels */
GimpLut * levels_lut_new                (gdouble *gamma,
					 gint    *low_input,
					 gint    *high_input,
					 gint    *low_output,
					 gint    *high_output,
					 gint     n_channels);
void      levels_lut_setup              (GimpLut *lut,
					 gdouble *gamma,
					 gint    *low_input,
					 gint    *high_input,
					 gint    *low_output,
					 gint    *high_output,
					 gint     n_channels);

/* posterize */
GimpLut * posterize_lut_new             (gint     levels,
					 gint     n_channels);
void      posterize_lut_setup           (GimpLut *lut,
					 gint     levels,
					 gint     n_channels);

/* equalize histogram */
GimpLut * eq_histogram_lut_new          (GimpHistogram *histogram,
					 gint           n_channels);
void      eq_histogram_lut_setup        (GimpLut       *lut,
					 GimpHistogram *histogram,
					 gint           n_channels);


#endif /* __LUT_FUNCS_H__ */
