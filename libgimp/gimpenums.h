/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1999 Peter Mattis and Spencer Kimball
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* NOTE: This file is autogenerated by enumcode.pl */

#ifndef __GIMP_ENUMS_H__
#define __GIMP_ENUMS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
  GIMP_WHITE_MASK,
  GIMP_BLACK_MASK,
  GIMP_ALPHA_MASK,
  GIMP_SELECTION_MASK,
  GIMP_INV_SELECTION_MASK
} GimpAddMaskType;

typedef enum
{
  GIMP_FG_BG_RGB,
  GIMP_FG_BG_HSV,
  GIMP_FG_TRANS,
  GIMP_CUSTOM
} GimpBlendMode;

typedef enum
{
  GIMP_HARD,
  GIMP_SOFT,
  GIMP_PRESSURE
} GimpBrushApplicationMode;

typedef enum
{
  GIMP_FG_BUCKET_FILL,
  GIMP_BG_BUCKET_FILL,
  GIMP_PATTERN_BUCKET_FILL
} GimpBucketFillMode;

typedef enum
{
  GIMP_VALUE_LUT,
  GIMP_RED_LUT,
  GIMP_GREEN_LUT,
  GIMP_BLUE_LUT,
  GIMP_ALPHA_LUT
} GimpChannelLutType;

typedef enum
{
  GIMP_ADD,
  GIMP_SUB,
  GIMP_REPLACE,
  GIMP_INTERSECT
} GimpChannelOps;

typedef enum
{
  GIMP_RED_CHANNEL,
  GIMP_GREEN_CHANNEL,
  GIMP_BLUE_CHANNEL,
  GIMP_GRAY_CHANNEL,
  GIMP_INDEXED_CHANNEL,
  GIMP_ALPHA_CHANNEL
} GimpChannelType;

typedef enum
{
  GIMP_IMAGE_CLONE,
  GIMP_PATTERN_CLONE
} GimpCloneType;

typedef enum
{
  GIMP_NO_DITHER,
  GIMP_FS_DITHER,
  GIMP_FSLOWBLEED_DITHER,
  GIMP_FIXED_DITHER,
  GIMP_NODESTRUCT_DITHER
} GimpConvertDitherType;

typedef enum
{
  GIMP_MAKE_PALETTE,
  GIMP_REUSE_PALETTE,
  GIMP_WEB_PALETTE,
  GIMP_MONO_PALETTE,
  GIMP_CUSTOM_PALETTE
} GimpConvertPaletteType;

typedef enum
{
  GIMP_NORMAL_CONVOL,
  GIMP_ABSOLUTE_CONVOL,
  GIMP_NEGATIVE_CONVOL
} GimpConvolutionType;

typedef enum
{
  GIMP_BLUR_CONVOLVE,
  GIMP_SHARPEN_CONVOLVE,
  GIMP_CUSTOM_CONVOLVE
} GimpConvolveType;

typedef enum
{
  GIMP_DODGEBURN_HIGHLIGHTS,
  GIMP_DODGEBURN_MIDTONES,
  GIMP_DODGEBURN_SHADOWS
} GimpDodgeBurnMode;

typedef enum
{
  GIMP_DODGE,
  GIMP_BURN
} GimpDodgeBurnType;

typedef enum
{
  GIMP_FG_IMAGE_FILL,
  GIMP_BG_IMAGE_FILL,
  GIMP_WHITE_IMAGE_FILL,
  GIMP_TRANS_IMAGE_FILL,
  GIMP_NO_IMAGE_FILL
} GimpFillType;

typedef enum
{
  GIMP_RGB,
  GIMP_GRAY,
  GIMP_INDEXED
} GimpImageBaseType;

typedef enum
{
  GIMP_RGB_IMAGE,
  GIMP_RGBA_IMAGE,
  GIMP_GRAY_IMAGE,
  GIMP_GRAYA_IMAGE,
  GIMP_INDEXED_IMAGE,
  GIMP_INDEXEDA_IMAGE
} GimpImageType;

typedef enum
{
  GIMP_OFFSET_BACKGROUND,
  GIMP_OFFSET_TRANSPARENT
} GimpOffsetType;

typedef enum
{
  GIMP_ONCE_FORWARD,
  GIMP_ONCE_BACKWARDS,
  GIMP_LOOP_SAWTOOTH,
  GIMP_LOOP_TRIANGLE,
  GIMP_ONCE_END_COLOR
} GimpGradientPaintMode;

typedef enum
{
  GIMP_LINEAR,
  GIMP_BILINEAR,
  GIMP_RADIAL,
  GIMP_SQUARE,
  GIMP_CONICAL_SYMMETRIC,
  GIMP_CONICAL_ASYMMETRIC,
  GIMP_SHAPEBURST_ANGULAR,
  GIMP_SHAPEBURST_SPHERICAL,
  GIMP_SHAPEBURST_DIMPLED,
  GIMP_SPIRAL_CLOCKWISE,
  GIMP_SPIRAL_ANTICLOCKWISE
} GimpGradientType;

typedef enum
{
  GIMP_ALL_HUES,
  GIMP_RED_HUES,
  GIMP_YELLOW_HUES,
  GIMP_GREEN_HUES,
  GIMP_CYAN_HUES,
  GIMP_BLUE_HUES,
  GIMP_MAGENTA_HUES
} GimpHueRange;

typedef enum
{
  GIMP_LINEAR_INTERPOLATION,
  GIMP_CUBIC_INTERPOLATION,
  GIMP_NEAREST_NEIGHBOR_INTERPOLATION
} GimpInterpolationType;

typedef enum
{
  GIMP_NORMAL_MODE,
  GIMP_DISSOLVE_MODE,
  GIMP_BEHIND_MODE,
  GIMP_MULTIPLY_MODE,
  GIMP_SCREEN_MODE,
  GIMP_OVERLAY_MODE,
  GIMP_DIFFERENCE_MODE,
  GIMP_ADDITION_MODE,
  GIMP_SUBTRACT_MODE,
  GIMP_DARKEN_ONLY_MODE,
  GIMP_LIGHTEN_ONLY_MODE,
  GIMP_HUE_MODE,
  GIMP_SATURATION_MODE,
  GIMP_COLOR_MODE,
  GIMP_VALUE_MODE,
  GIMP_DIVIDE_MODE,
  GIMP_DODGE_MODE,
  GIMP_BURN_MODE,
  GIMP_HARDLIGHT_MODE
} GimpLayerModeEffects;

typedef enum
{
  GIMP_APPLY,
  GIMP_DISCARD
} GimpMaskApplyMode;

typedef enum
{
  GIMP_EXPAND_AS_NECESSARY,
  GIMP_CLIP_TO_IMAGE,
  GIMP_CLIP_TO_BOTTOM_LAYER,
  GIMP_FLATTEN_IMAGE
} GimpMergeType;

typedef enum
{
  GIMP_MESSAGE_BOX,
  GIMP_CONSOLE,
  GIMP_ERROR_CONSOLE
} GimpMessageHandlerType;

typedef enum
{
  GIMP_HORIZONTAL,
  GIMP_VERTICAL,
  GIMP_UNKNOWN
} GimpOrientationType;

typedef enum
{
  GIMP_PDB_INT32,
  GIMP_PDB_INT16,
  GIMP_PDB_INT8,
  GIMP_PDB_FLOAT,
  GIMP_PDB_STRING,
  GIMP_PDB_INT32ARRAY,
  GIMP_PDB_INT16ARRAY,
  GIMP_PDB_INT8ARRAY,
  GIMP_PDB_FLOATARRAY,
  GIMP_PDB_STRINGARRAY,
  GIMP_PDB_COLOR,
  GIMP_PDB_REGION,
  GIMP_PDB_DISPLAY,
  GIMP_PDB_IMAGE,
  GIMP_PDB_LAYER,
  GIMP_PDB_CHANNEL,
  GIMP_PDB_DRAWABLE,
  GIMP_PDB_SELECTION,
  GIMP_PDB_BOUNDARY,
  GIMP_PDB_PATH,
  GIMP_PDB_PARASITE,
  GIMP_PDB_STATUS,
  GIMP_PDB_END
} GimpPDBArgType;

typedef enum
{
  GIMP_INTERNAL,
  GIMP_PLUGIN,
  GIMP_EXTENSION,
  GIMP_TEMPORARY
} GimpPDBProcType;

typedef enum
{
  GIMP_PDB_EXECUTION_ERROR,
  GIMP_PDB_CALLING_ERROR,
  GIMP_PDB_PASS_THROUGH,
  GIMP_PDB_SUCCESS,
  GIMP_PDB_CANCEL
} GimpPDBStatusType;

typedef enum
{
  GIMP_CONTINUOUS,
  GIMP_INCREMENTAL
} GimpPaintApplicationMode;

typedef enum
{
  GIMP_REPEAT_NONE,
  GIMP_REPEAT_SAWTOOTH,
  GIMP_REPEAT_TRIANGULAR
} GimpRepeatMode;

typedef enum
{
  GIMP_RUN_INTERACTIVE,
  GIMP_RUN_NONINTERACTIVE,
  GIMP_RUN_WITH_LAST_VALS
} GimpRunModeType;

typedef enum
{
  GIMP_PIXELS,
  GIMP_POINTS
} GimpSizeType;

typedef enum
{
  GIMP_STACK_TRACE_NEVER,
  GIMP_STACK_TRACE_QUERY,
  GIMP_STACK_TRACE_ALWAYS
} GimpStackTraceMode;

typedef enum
{
  GIMP_SHADOWS,
  GIMP_MIDTONES,
  GIMP_HIGHLIGHTS
} GimpTransferMode;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GIMP_ENUMS_H__ */
