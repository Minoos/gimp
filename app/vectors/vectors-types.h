/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * vectors-types.h
 * Copyright (C) 2002 Simon Budig  <simon@gimp.org>
 *
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

#ifndef __VECTORS_TYPES_H__
#define __VECTORS_TYPES_H__

#include "core/core-types.h"

typedef struct _GimpAnchor GimpAnchor;

typedef enum
{
  ANCHOR_HANDLE,
  CONTROL_HANDLE,
} GimpAnchorType;

typedef enum
{
  EXTEND_SIMPLE,
  EXTEND_EDITABLE,
} GimpVectorExtendMode;

typedef struct _GimpStroke            GimpStroke;
typedef struct _GimpStrokeClass       GimpStrokeClass;
typedef struct _GimpBezierStroke      GimpBezierStroke;
typedef struct _GimpBezierStrokeClass GimpBezierStrokeClass;

typedef struct _GimpBezier        GimpBezier;
typedef struct _GimpBezierClass   GimpBezierClass;
typedef struct _GimpVectors       GimpVectors;
typedef struct _GimpVectorsClass  GimpVectorsClass;

#endif /* __VECTORS_TYPES_H__ */
