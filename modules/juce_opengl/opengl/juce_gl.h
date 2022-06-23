/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

// This file was generated automatically using data from the opengl-registry
// https://github.com/KhronosGroup/OpenGL-Registry

#pragma once

// IMPORTANT! Include this before any platform gl files.

// The portion of this file between the line reading BEGIN_GLEW_LICENSE
// and the line reading END_GLEW_LICENSE is taken from the GLEW library.
// We thank the maintainers of GLEW for sharing their code under a permissive
// license.
// The original license of this portion of the file is as follows:

// BEGIN_GLEW_LICENSE

/*
    The OpenGL Extension Wrangler Library
    Copyright (C) 2002-2007, Milan Ikits <milan ikits[]ieee org>
    Copyright (C) 2002-2007, Marcelo E. Magallon <mmagallo[]debian org>
    Copyright (C) 2002, Lev Povalahev
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * The name of the author may be used to endorse or promote products
      derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
    THE POSSIBILITY OF SUCH DAMAGE.


    Mesa 3-D graphics library
    Version:  7.0

    Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    Copyright (c) 2007 The Khronos Group Inc.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and/or associated documentation files (the
    "Materials"), to deal in the Materials without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Materials, and to
    permit persons to whom the Materials are furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Materials.

    THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#if defined (__gl_h_) || defined (__GL_H__) || defined (_GL_H) || defined (__gl_gl_h_) || defined (__X_GL_H)
#error gl.h included before juce_gl.h
#endif
#if defined (__gl2_h_)
#error gl2.h included before juce_gl.h
#endif
#if defined (__gltypes_h_)
#error gltypes.h included before juce_gl.h
#endif
#if defined (__REGAL_H__)
#error Regal.h included before juce_gl.h
#endif
#if defined (__glext_h_) || defined (__GLEXT_H_) || defined (__gl_glext_h_)
#error glext.h included before juce_gl.h
#endif
#if defined (__gl_ATI_h_)
#error glATI.h included before juce_gl.h
#endif

#define __gl_h_
#define __gl2_h_
#define __GL_H__
#define _GL_H
#define __gl_gl_h_
#define __gltypes_h_
#define __REGAL_H__
#define __X_GL_H
#define __glext_h_
#define __GLEXT_H_
#define __gl_glext_h_
#define __gl_ATI_h_

// END_GLEW_LICENSE

#include <juce_core/system/juce_CompilerWarnings.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

#include "juce_khrplatform.h"
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef khronos_int8_t GLbyte;
typedef khronos_uint8_t GLubyte;
typedef khronos_int16_t GLshort;
typedef khronos_uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef khronos_int32_t GLclampx;
typedef int GLsizei;
typedef khronos_float_t GLfloat;
typedef khronos_float_t GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglClientBufferEXT;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
#ifdef __APPLE__
typedef void *GLhandleARB;
#else
typedef unsigned int GLhandleARB;
#endif
typedef khronos_uint16_t GLhalf;
typedef khronos_uint16_t GLhalfARB;
typedef khronos_int32_t GLfixed;
typedef khronos_intptr_t GLintptr;
typedef khronos_intptr_t GLintptrARB;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_ssize_t GLsizeiptrARB;
typedef khronos_int64_t GLint64;
typedef khronos_int64_t GLint64EXT;
typedef khronos_uint64_t GLuint64;
typedef khronos_uint64_t GLuint64EXT;
typedef struct __GLsync *GLsync;
struct _cl_context;
struct _cl_event;
typedef void ( *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void ( *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void ( *GLDEBUGPROCKHR)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void ( *GLDEBUGPROCAMD)(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,void *userParam);
typedef unsigned short GLhalfNV;
typedef GLintptr GLvdpauSurfaceNV;
typedef void ( *GLVULKANPROCNV)(void);

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace juce
{
namespace gl
{

#ifndef GL_VERSION_1_0
#define GL_VERSION_1_0 1
enum : GLenum
{
    GL_DEPTH_BUFFER_BIT                                     = 0x00000100,
    GL_STENCIL_BUFFER_BIT                                   = 0x00000400,
    GL_COLOR_BUFFER_BIT                                     = 0x00004000,
    GL_FALSE                                                = 0,
    GL_TRUE                                                 = 1,
    GL_POINTS                                               = 0x0000,
    GL_LINES                                                = 0x0001,
    GL_LINE_LOOP                                            = 0x0002,
    GL_LINE_STRIP                                           = 0x0003,
    GL_TRIANGLES                                            = 0x0004,
    GL_TRIANGLE_STRIP                                       = 0x0005,
    GL_TRIANGLE_FAN                                         = 0x0006,
    GL_QUADS                                                = 0x0007,
    GL_NEVER                                                = 0x0200,
    GL_LESS                                                 = 0x0201,
    GL_EQUAL                                                = 0x0202,
    GL_LEQUAL                                               = 0x0203,
    GL_GREATER                                              = 0x0204,
    GL_NOTEQUAL                                             = 0x0205,
    GL_GEQUAL                                               = 0x0206,
    GL_ALWAYS                                               = 0x0207,
    GL_ZERO                                                 = 0,
    GL_ONE                                                  = 1,
    GL_SRC_COLOR                                            = 0x0300,
    GL_ONE_MINUS_SRC_COLOR                                  = 0x0301,
    GL_SRC_ALPHA                                            = 0x0302,
    GL_ONE_MINUS_SRC_ALPHA                                  = 0x0303,
    GL_DST_ALPHA                                            = 0x0304,
    GL_ONE_MINUS_DST_ALPHA                                  = 0x0305,
    GL_DST_COLOR                                            = 0x0306,
    GL_ONE_MINUS_DST_COLOR                                  = 0x0307,
    GL_SRC_ALPHA_SATURATE                                   = 0x0308,
    GL_NONE                                                 = 0,
    GL_FRONT_LEFT                                           = 0x0400,
    GL_FRONT_RIGHT                                          = 0x0401,
    GL_BACK_LEFT                                            = 0x0402,
    GL_BACK_RIGHT                                           = 0x0403,
    GL_FRONT                                                = 0x0404,
    GL_BACK                                                 = 0x0405,
    GL_LEFT                                                 = 0x0406,
    GL_RIGHT                                                = 0x0407,
    GL_FRONT_AND_BACK                                       = 0x0408,
    GL_NO_ERROR                                             = 0,
    GL_INVALID_ENUM                                         = 0x0500,
    GL_INVALID_VALUE                                        = 0x0501,
    GL_INVALID_OPERATION                                    = 0x0502,
    GL_OUT_OF_MEMORY                                        = 0x0505,
    GL_CW                                                   = 0x0900,
    GL_CCW                                                  = 0x0901,
    GL_POINT_SIZE                                           = 0x0B11,
    GL_POINT_SIZE_RANGE                                     = 0x0B12,
    GL_POINT_SIZE_GRANULARITY                               = 0x0B13,
    GL_LINE_SMOOTH                                          = 0x0B20,
    GL_LINE_WIDTH                                           = 0x0B21,
    GL_LINE_WIDTH_RANGE                                     = 0x0B22,
    GL_LINE_WIDTH_GRANULARITY                               = 0x0B23,
    GL_POLYGON_MODE                                         = 0x0B40,
    GL_POLYGON_SMOOTH                                       = 0x0B41,
    GL_CULL_FACE                                            = 0x0B44,
    GL_CULL_FACE_MODE                                       = 0x0B45,
    GL_FRONT_FACE                                           = 0x0B46,
    GL_DEPTH_RANGE                                          = 0x0B70,
    GL_DEPTH_TEST                                           = 0x0B71,
    GL_DEPTH_WRITEMASK                                      = 0x0B72,
    GL_DEPTH_CLEAR_VALUE                                    = 0x0B73,
    GL_DEPTH_FUNC                                           = 0x0B74,
    GL_STENCIL_TEST                                         = 0x0B90,
    GL_STENCIL_CLEAR_VALUE                                  = 0x0B91,
    GL_STENCIL_FUNC                                         = 0x0B92,
    GL_STENCIL_VALUE_MASK                                   = 0x0B93,
    GL_STENCIL_FAIL                                         = 0x0B94,
    GL_STENCIL_PASS_DEPTH_FAIL                              = 0x0B95,
    GL_STENCIL_PASS_DEPTH_PASS                              = 0x0B96,
    GL_STENCIL_REF                                          = 0x0B97,
    GL_STENCIL_WRITEMASK                                    = 0x0B98,
    GL_VIEWPORT                                             = 0x0BA2,
    GL_DITHER                                               = 0x0BD0,
    GL_BLEND_DST                                            = 0x0BE0,
    GL_BLEND_SRC                                            = 0x0BE1,
    GL_BLEND                                                = 0x0BE2,
    GL_LOGIC_OP_MODE                                        = 0x0BF0,
    GL_DRAW_BUFFER                                          = 0x0C01,
    GL_READ_BUFFER                                          = 0x0C02,
    GL_SCISSOR_BOX                                          = 0x0C10,
    GL_SCISSOR_TEST                                         = 0x0C11,
    GL_COLOR_CLEAR_VALUE                                    = 0x0C22,
    GL_COLOR_WRITEMASK                                      = 0x0C23,
    GL_DOUBLEBUFFER                                         = 0x0C32,
    GL_STEREO                                               = 0x0C33,
    GL_LINE_SMOOTH_HINT                                     = 0x0C52,
    GL_POLYGON_SMOOTH_HINT                                  = 0x0C53,
    GL_UNPACK_SWAP_BYTES                                    = 0x0CF0,
    GL_UNPACK_LSB_FIRST                                     = 0x0CF1,
    GL_UNPACK_ROW_LENGTH                                    = 0x0CF2,
    GL_UNPACK_SKIP_ROWS                                     = 0x0CF3,
    GL_UNPACK_SKIP_PIXELS                                   = 0x0CF4,
    GL_UNPACK_ALIGNMENT                                     = 0x0CF5,
    GL_PACK_SWAP_BYTES                                      = 0x0D00,
    GL_PACK_LSB_FIRST                                       = 0x0D01,
    GL_PACK_ROW_LENGTH                                      = 0x0D02,
    GL_PACK_SKIP_ROWS                                       = 0x0D03,
    GL_PACK_SKIP_PIXELS                                     = 0x0D04,
    GL_PACK_ALIGNMENT                                       = 0x0D05,
    GL_MAX_TEXTURE_SIZE                                     = 0x0D33,
    GL_MAX_VIEWPORT_DIMS                                    = 0x0D3A,
    GL_SUBPIXEL_BITS                                        = 0x0D50,
    GL_TEXTURE_1D                                           = 0x0DE0,
    GL_TEXTURE_2D                                           = 0x0DE1,
    GL_TEXTURE_WIDTH                                        = 0x1000,
    GL_TEXTURE_HEIGHT                                       = 0x1001,
    GL_TEXTURE_BORDER_COLOR                                 = 0x1004,
    GL_DONT_CARE                                            = 0x1100,
    GL_FASTEST                                              = 0x1101,
    GL_NICEST                                               = 0x1102,
    GL_BYTE                                                 = 0x1400,
    GL_UNSIGNED_BYTE                                        = 0x1401,
    GL_SHORT                                                = 0x1402,
    GL_UNSIGNED_SHORT                                       = 0x1403,
    GL_INT                                                  = 0x1404,
    GL_UNSIGNED_INT                                         = 0x1405,
    GL_FLOAT                                                = 0x1406,
    GL_STACK_OVERFLOW                                       = 0x0503,
    GL_STACK_UNDERFLOW                                      = 0x0504,
    GL_CLEAR                                                = 0x1500,
    GL_AND                                                  = 0x1501,
    GL_AND_REVERSE                                          = 0x1502,
    GL_COPY                                                 = 0x1503,
    GL_AND_INVERTED                                         = 0x1504,
    GL_NOOP                                                 = 0x1505,
    GL_XOR                                                  = 0x1506,
    GL_OR                                                   = 0x1507,
    GL_NOR                                                  = 0x1508,
    GL_EQUIV                                                = 0x1509,
    GL_INVERT                                               = 0x150A,
    GL_OR_REVERSE                                           = 0x150B,
    GL_COPY_INVERTED                                        = 0x150C,
    GL_OR_INVERTED                                          = 0x150D,
    GL_NAND                                                 = 0x150E,
    GL_SET                                                  = 0x150F,
    GL_TEXTURE                                              = 0x1702,
    GL_COLOR                                                = 0x1800,
    GL_DEPTH                                                = 0x1801,
    GL_STENCIL                                              = 0x1802,
    GL_STENCIL_INDEX                                        = 0x1901,
    GL_DEPTH_COMPONENT                                      = 0x1902,
    GL_RED                                                  = 0x1903,
    GL_GREEN                                                = 0x1904,
    GL_BLUE                                                 = 0x1905,
    GL_ALPHA                                                = 0x1906,
    GL_RGB                                                  = 0x1907,
    GL_RGBA                                                 = 0x1908,
    GL_POINT                                                = 0x1B00,
    GL_LINE                                                 = 0x1B01,
    GL_FILL                                                 = 0x1B02,
    GL_KEEP                                                 = 0x1E00,
    GL_REPLACE                                              = 0x1E01,
    GL_INCR                                                 = 0x1E02,
    GL_DECR                                                 = 0x1E03,
    GL_VENDOR                                               = 0x1F00,
    GL_RENDERER                                             = 0x1F01,
    GL_VERSION                                              = 0x1F02,
    GL_EXTENSIONS                                           = 0x1F03,
    GL_NEAREST                                              = 0x2600,
    GL_LINEAR                                               = 0x2601,
    GL_NEAREST_MIPMAP_NEAREST                               = 0x2700,
    GL_LINEAR_MIPMAP_NEAREST                                = 0x2701,
    GL_NEAREST_MIPMAP_LINEAR                                = 0x2702,
    GL_LINEAR_MIPMAP_LINEAR                                 = 0x2703,
    GL_TEXTURE_MAG_FILTER                                   = 0x2800,
    GL_TEXTURE_MIN_FILTER                                   = 0x2801,
    GL_TEXTURE_WRAP_S                                       = 0x2802,
    GL_TEXTURE_WRAP_T                                       = 0x2803,
    GL_REPEAT                                               = 0x2901,
    GL_CURRENT_BIT                                          = 0x00000001,
    GL_POINT_BIT                                            = 0x00000002,
    GL_LINE_BIT                                             = 0x00000004,
    GL_POLYGON_BIT                                          = 0x00000008,
    GL_POLYGON_STIPPLE_BIT                                  = 0x00000010,
    GL_PIXEL_MODE_BIT                                       = 0x00000020,
    GL_LIGHTING_BIT                                         = 0x00000040,
    GL_FOG_BIT                                              = 0x00000080,
    GL_ACCUM_BUFFER_BIT                                     = 0x00000200,
    GL_VIEWPORT_BIT                                         = 0x00000800,
    GL_TRANSFORM_BIT                                        = 0x00001000,
    GL_ENABLE_BIT                                           = 0x00002000,
    GL_HINT_BIT                                             = 0x00008000,
    GL_EVAL_BIT                                             = 0x00010000,
    GL_LIST_BIT                                             = 0x00020000,
    GL_TEXTURE_BIT                                          = 0x00040000,
    GL_SCISSOR_BIT                                          = 0x00080000,
    GL_ALL_ATTRIB_BITS                                      = 0xFFFFFFFF,
    GL_QUAD_STRIP                                           = 0x0008,
    GL_POLYGON                                              = 0x0009,
    GL_ACCUM                                                = 0x0100,
    GL_LOAD                                                 = 0x0101,
    GL_RETURN                                               = 0x0102,
    GL_MULT                                                 = 0x0103,
    GL_ADD                                                  = 0x0104,
    GL_AUX0                                                 = 0x0409,
    GL_AUX1                                                 = 0x040A,
    GL_AUX2                                                 = 0x040B,
    GL_AUX3                                                 = 0x040C,
    GL_2D                                                   = 0x0600,
    GL_3D                                                   = 0x0601,
    GL_3D_COLOR                                             = 0x0602,
    GL_3D_COLOR_TEXTURE                                     = 0x0603,
    GL_4D_COLOR_TEXTURE                                     = 0x0604,
    GL_PASS_THROUGH_TOKEN                                   = 0x0700,
    GL_POINT_TOKEN                                          = 0x0701,
    GL_LINE_TOKEN                                           = 0x0702,
    GL_POLYGON_TOKEN                                        = 0x0703,
    GL_BITMAP_TOKEN                                         = 0x0704,
    GL_DRAW_PIXEL_TOKEN                                     = 0x0705,
    GL_COPY_PIXEL_TOKEN                                     = 0x0706,
    GL_LINE_RESET_TOKEN                                     = 0x0707,
    GL_EXP                                                  = 0x0800,
    GL_EXP2                                                 = 0x0801,
    GL_COEFF                                                = 0x0A00,
    GL_ORDER                                                = 0x0A01,
    GL_DOMAIN                                               = 0x0A02,
    GL_PIXEL_MAP_I_TO_I                                     = 0x0C70,
    GL_PIXEL_MAP_S_TO_S                                     = 0x0C71,
    GL_PIXEL_MAP_I_TO_R                                     = 0x0C72,
    GL_PIXEL_MAP_I_TO_G                                     = 0x0C73,
    GL_PIXEL_MAP_I_TO_B                                     = 0x0C74,
    GL_PIXEL_MAP_I_TO_A                                     = 0x0C75,
    GL_PIXEL_MAP_R_TO_R                                     = 0x0C76,
    GL_PIXEL_MAP_G_TO_G                                     = 0x0C77,
    GL_PIXEL_MAP_B_TO_B                                     = 0x0C78,
    GL_PIXEL_MAP_A_TO_A                                     = 0x0C79,
    GL_CURRENT_COLOR                                        = 0x0B00,
    GL_CURRENT_INDEX                                        = 0x0B01,
    GL_CURRENT_NORMAL                                       = 0x0B02,
    GL_CURRENT_TEXTURE_COORDS                               = 0x0B03,
    GL_CURRENT_RASTER_COLOR                                 = 0x0B04,
    GL_CURRENT_RASTER_INDEX                                 = 0x0B05,
    GL_CURRENT_RASTER_TEXTURE_COORDS                        = 0x0B06,
    GL_CURRENT_RASTER_POSITION                              = 0x0B07,
    GL_CURRENT_RASTER_POSITION_VALID                        = 0x0B08,
    GL_CURRENT_RASTER_DISTANCE                              = 0x0B09,
    GL_POINT_SMOOTH                                         = 0x0B10,
    GL_LINE_STIPPLE                                         = 0x0B24,
    GL_LINE_STIPPLE_PATTERN                                 = 0x0B25,
    GL_LINE_STIPPLE_REPEAT                                  = 0x0B26,
    GL_LIST_MODE                                            = 0x0B30,
    GL_MAX_LIST_NESTING                                     = 0x0B31,
    GL_LIST_BASE                                            = 0x0B32,
    GL_LIST_INDEX                                           = 0x0B33,
    GL_POLYGON_STIPPLE                                      = 0x0B42,
    GL_EDGE_FLAG                                            = 0x0B43,
    GL_LIGHTING                                             = 0x0B50,
    GL_LIGHT_MODEL_LOCAL_VIEWER                             = 0x0B51,
    GL_LIGHT_MODEL_TWO_SIDE                                 = 0x0B52,
    GL_LIGHT_MODEL_AMBIENT                                  = 0x0B53,
    GL_SHADE_MODEL                                          = 0x0B54,
    GL_COLOR_MATERIAL_FACE                                  = 0x0B55,
    GL_COLOR_MATERIAL_PARAMETER                             = 0x0B56,
    GL_COLOR_MATERIAL                                       = 0x0B57,
    GL_FOG                                                  = 0x0B60,
    GL_FOG_INDEX                                            = 0x0B61,
    GL_FOG_DENSITY                                          = 0x0B62,
    GL_FOG_START                                            = 0x0B63,
    GL_FOG_END                                              = 0x0B64,
    GL_FOG_MODE                                             = 0x0B65,
    GL_FOG_COLOR                                            = 0x0B66,
    GL_ACCUM_CLEAR_VALUE                                    = 0x0B80,
    GL_MATRIX_MODE                                          = 0x0BA0,
    GL_NORMALIZE                                            = 0x0BA1,
    GL_MODELVIEW_STACK_DEPTH                                = 0x0BA3,
    GL_PROJECTION_STACK_DEPTH                               = 0x0BA4,
    GL_TEXTURE_STACK_DEPTH                                  = 0x0BA5,
    GL_MODELVIEW_MATRIX                                     = 0x0BA6,
    GL_PROJECTION_MATRIX                                    = 0x0BA7,
    GL_TEXTURE_MATRIX                                       = 0x0BA8,
    GL_ATTRIB_STACK_DEPTH                                   = 0x0BB0,
    GL_ALPHA_TEST                                           = 0x0BC0,
    GL_ALPHA_TEST_FUNC                                      = 0x0BC1,
    GL_ALPHA_TEST_REF                                       = 0x0BC2,
    GL_LOGIC_OP                                             = 0x0BF1,
    GL_AUX_BUFFERS                                          = 0x0C00,
    GL_INDEX_CLEAR_VALUE                                    = 0x0C20,
    GL_INDEX_WRITEMASK                                      = 0x0C21,
    GL_INDEX_MODE                                           = 0x0C30,
    GL_RGBA_MODE                                            = 0x0C31,
    GL_RENDER_MODE                                          = 0x0C40,
    GL_PERSPECTIVE_CORRECTION_HINT                          = 0x0C50,
    GL_POINT_SMOOTH_HINT                                    = 0x0C51,
    GL_FOG_HINT                                             = 0x0C54,
    GL_TEXTURE_GEN_S                                        = 0x0C60,
    GL_TEXTURE_GEN_T                                        = 0x0C61,
    GL_TEXTURE_GEN_R                                        = 0x0C62,
    GL_TEXTURE_GEN_Q                                        = 0x0C63,
    GL_PIXEL_MAP_I_TO_I_SIZE                                = 0x0CB0,
    GL_PIXEL_MAP_S_TO_S_SIZE                                = 0x0CB1,
    GL_PIXEL_MAP_I_TO_R_SIZE                                = 0x0CB2,
    GL_PIXEL_MAP_I_TO_G_SIZE                                = 0x0CB3,
    GL_PIXEL_MAP_I_TO_B_SIZE                                = 0x0CB4,
    GL_PIXEL_MAP_I_TO_A_SIZE                                = 0x0CB5,
    GL_PIXEL_MAP_R_TO_R_SIZE                                = 0x0CB6,
    GL_PIXEL_MAP_G_TO_G_SIZE                                = 0x0CB7,
    GL_PIXEL_MAP_B_TO_B_SIZE                                = 0x0CB8,
    GL_PIXEL_MAP_A_TO_A_SIZE                                = 0x0CB9,
    GL_MAP_COLOR                                            = 0x0D10,
    GL_MAP_STENCIL                                          = 0x0D11,
    GL_INDEX_SHIFT                                          = 0x0D12,
    GL_INDEX_OFFSET                                         = 0x0D13,
    GL_RED_SCALE                                            = 0x0D14,
    GL_RED_BIAS                                             = 0x0D15,
    GL_ZOOM_X                                               = 0x0D16,
    GL_ZOOM_Y                                               = 0x0D17,
    GL_GREEN_SCALE                                          = 0x0D18,
    GL_GREEN_BIAS                                           = 0x0D19,
    GL_BLUE_SCALE                                           = 0x0D1A,
    GL_BLUE_BIAS                                            = 0x0D1B,
    GL_ALPHA_SCALE                                          = 0x0D1C,
    GL_ALPHA_BIAS                                           = 0x0D1D,
    GL_DEPTH_SCALE                                          = 0x0D1E,
    GL_DEPTH_BIAS                                           = 0x0D1F,
    GL_MAX_EVAL_ORDER                                       = 0x0D30,
    GL_MAX_LIGHTS                                           = 0x0D31,
    GL_MAX_CLIP_PLANES                                      = 0x0D32,
    GL_MAX_PIXEL_MAP_TABLE                                  = 0x0D34,
    GL_MAX_ATTRIB_STACK_DEPTH                               = 0x0D35,
    GL_MAX_MODELVIEW_STACK_DEPTH                            = 0x0D36,
    GL_MAX_NAME_STACK_DEPTH                                 = 0x0D37,
    GL_MAX_PROJECTION_STACK_DEPTH                           = 0x0D38,
    GL_MAX_TEXTURE_STACK_DEPTH                              = 0x0D39,
    GL_INDEX_BITS                                           = 0x0D51,
    GL_RED_BITS                                             = 0x0D52,
    GL_GREEN_BITS                                           = 0x0D53,
    GL_BLUE_BITS                                            = 0x0D54,
    GL_ALPHA_BITS                                           = 0x0D55,
    GL_DEPTH_BITS                                           = 0x0D56,
    GL_STENCIL_BITS                                         = 0x0D57,
    GL_ACCUM_RED_BITS                                       = 0x0D58,
    GL_ACCUM_GREEN_BITS                                     = 0x0D59,
    GL_ACCUM_BLUE_BITS                                      = 0x0D5A,
    GL_ACCUM_ALPHA_BITS                                     = 0x0D5B,
    GL_NAME_STACK_DEPTH                                     = 0x0D70,
    GL_AUTO_NORMAL                                          = 0x0D80,
    GL_MAP1_COLOR_4                                         = 0x0D90,
    GL_MAP1_INDEX                                           = 0x0D91,
    GL_MAP1_NORMAL                                          = 0x0D92,
    GL_MAP1_TEXTURE_COORD_1                                 = 0x0D93,
    GL_MAP1_TEXTURE_COORD_2                                 = 0x0D94,
    GL_MAP1_TEXTURE_COORD_3                                 = 0x0D95,
    GL_MAP1_TEXTURE_COORD_4                                 = 0x0D96,
    GL_MAP1_VERTEX_3                                        = 0x0D97,
    GL_MAP1_VERTEX_4                                        = 0x0D98,
    GL_MAP2_COLOR_4                                         = 0x0DB0,
    GL_MAP2_INDEX                                           = 0x0DB1,
    GL_MAP2_NORMAL                                          = 0x0DB2,
    GL_MAP2_TEXTURE_COORD_1                                 = 0x0DB3,
    GL_MAP2_TEXTURE_COORD_2                                 = 0x0DB4,
    GL_MAP2_TEXTURE_COORD_3                                 = 0x0DB5,
    GL_MAP2_TEXTURE_COORD_4                                 = 0x0DB6,
    GL_MAP2_VERTEX_3                                        = 0x0DB7,
    GL_MAP2_VERTEX_4                                        = 0x0DB8,
    GL_MAP1_GRID_DOMAIN                                     = 0x0DD0,
    GL_MAP1_GRID_SEGMENTS                                   = 0x0DD1,
    GL_MAP2_GRID_DOMAIN                                     = 0x0DD2,
    GL_MAP2_GRID_SEGMENTS                                   = 0x0DD3,
    GL_TEXTURE_COMPONENTS                                   = 0x1003,
    GL_TEXTURE_BORDER                                       = 0x1005,
    GL_AMBIENT                                              = 0x1200,
    GL_DIFFUSE                                              = 0x1201,
    GL_SPECULAR                                             = 0x1202,
    GL_POSITION                                             = 0x1203,
    GL_SPOT_DIRECTION                                       = 0x1204,
    GL_SPOT_EXPONENT                                        = 0x1205,
    GL_SPOT_CUTOFF                                          = 0x1206,
    GL_CONSTANT_ATTENUATION                                 = 0x1207,
    GL_LINEAR_ATTENUATION                                   = 0x1208,
    GL_QUADRATIC_ATTENUATION                                = 0x1209,
    GL_COMPILE                                              = 0x1300,
    GL_COMPILE_AND_EXECUTE                                  = 0x1301,
    GL_2_BYTES                                              = 0x1407,
    GL_3_BYTES                                              = 0x1408,
    GL_4_BYTES                                              = 0x1409,
    GL_EMISSION                                             = 0x1600,
    GL_SHININESS                                            = 0x1601,
    GL_AMBIENT_AND_DIFFUSE                                  = 0x1602,
    GL_COLOR_INDEXES                                        = 0x1603,
    GL_MODELVIEW                                            = 0x1700,
    GL_PROJECTION                                           = 0x1701,
    GL_COLOR_INDEX                                          = 0x1900,
    GL_LUMINANCE                                            = 0x1909,
    GL_LUMINANCE_ALPHA                                      = 0x190A,
    GL_BITMAP                                               = 0x1A00,
    GL_RENDER                                               = 0x1C00,
    GL_FEEDBACK                                             = 0x1C01,
    GL_SELECT                                               = 0x1C02,
    GL_FLAT                                                 = 0x1D00,
    GL_SMOOTH                                               = 0x1D01,
    GL_S                                                    = 0x2000,
    GL_T                                                    = 0x2001,
    GL_R                                                    = 0x2002,
    GL_Q                                                    = 0x2003,
    GL_MODULATE                                             = 0x2100,
    GL_DECAL                                                = 0x2101,
    GL_TEXTURE_ENV_MODE                                     = 0x2200,
    GL_TEXTURE_ENV_COLOR                                    = 0x2201,
    GL_TEXTURE_ENV                                          = 0x2300,
    GL_EYE_LINEAR                                           = 0x2400,
    GL_OBJECT_LINEAR                                        = 0x2401,
    GL_SPHERE_MAP                                           = 0x2402,
    GL_TEXTURE_GEN_MODE                                     = 0x2500,
    GL_OBJECT_PLANE                                         = 0x2501,
    GL_EYE_PLANE                                            = 0x2502,
    GL_CLAMP                                                = 0x2900,
    GL_CLIP_PLANE0                                          = 0x3000,
    GL_CLIP_PLANE1                                          = 0x3001,
    GL_CLIP_PLANE2                                          = 0x3002,
    GL_CLIP_PLANE3                                          = 0x3003,
    GL_CLIP_PLANE4                                          = 0x3004,
    GL_CLIP_PLANE5                                          = 0x3005,
    GL_LIGHT0                                               = 0x4000,
    GL_LIGHT1                                               = 0x4001,
    GL_LIGHT2                                               = 0x4002,
    GL_LIGHT3                                               = 0x4003,
    GL_LIGHT4                                               = 0x4004,
    GL_LIGHT5                                               = 0x4005,
    GL_LIGHT6                                               = 0x4006,
    GL_LIGHT7                                               = 0x4007,
};
extern void         (KHRONOS_APIENTRY* const& glCullFace) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glFrontFace) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glHint) (GLenum target, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glLineWidth) (GLfloat width);
extern void         (KHRONOS_APIENTRY* const& glPointSize) (GLfloat size);
extern void         (KHRONOS_APIENTRY* const& glPolygonMode) (GLenum face, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameteri) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glTexParameteriv) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glDrawBuffer) (GLenum buf);
extern void         (KHRONOS_APIENTRY* const& glClear) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glClearColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void         (KHRONOS_APIENTRY* const& glClearStencil) (GLint s);
extern void         (KHRONOS_APIENTRY* const& glClearDepth) (GLdouble depth);
extern void         (KHRONOS_APIENTRY* const& glStencilMask) (GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void         (KHRONOS_APIENTRY* const& glDepthMask) (GLboolean flag);
extern void         (KHRONOS_APIENTRY* const& glDisable) (GLenum cap);
extern void         (KHRONOS_APIENTRY* const& glEnable) (GLenum cap);
extern void         (KHRONOS_APIENTRY* const& glFinish) ();
extern void         (KHRONOS_APIENTRY* const& glFlush) ();
extern void         (KHRONOS_APIENTRY* const& glBlendFunc) (GLenum sfactor, GLenum dfactor);
extern void         (KHRONOS_APIENTRY* const& glLogicOp) (GLenum opcode);
extern void         (KHRONOS_APIENTRY* const& glStencilFunc) (GLenum func, GLint ref, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
extern void         (KHRONOS_APIENTRY* const& glDepthFunc) (GLenum func);
extern void         (KHRONOS_APIENTRY* const& glPixelStoref) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPixelStorei) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glReadBuffer) (GLenum src);
extern void         (KHRONOS_APIENTRY* const& glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetBooleanv) (GLenum pname, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glGetDoublev) (GLenum pname, GLdouble *data);
extern GLenum       (KHRONOS_APIENTRY* const& glGetError) ();
extern void         (KHRONOS_APIENTRY* const& glGetFloatv) (GLenum pname, GLfloat *data);
extern void         (KHRONOS_APIENTRY* const& glGetIntegerv) (GLenum pname, GLint *data);
extern const GLubyte * (KHRONOS_APIENTRY* const& glGetString) (GLenum name);
extern void         (KHRONOS_APIENTRY* const& glGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnabled) (GLenum cap);
extern void         (KHRONOS_APIENTRY* const& glDepthRange) (GLdouble n, GLdouble f);
extern void         (KHRONOS_APIENTRY* const& glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glNewList) (GLuint list, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glEndList) ();
extern void         (KHRONOS_APIENTRY* const& glCallList) (GLuint list);
extern void         (KHRONOS_APIENTRY* const& glCallLists) (GLsizei n, GLenum type, const void *lists);
extern void         (KHRONOS_APIENTRY* const& glDeleteLists) (GLuint list, GLsizei range);
extern GLuint       (KHRONOS_APIENTRY* const& glGenLists) (GLsizei range);
extern void         (KHRONOS_APIENTRY* const& glListBase) (GLuint base);
extern void         (KHRONOS_APIENTRY* const& glBegin) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBitmap) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern void         (KHRONOS_APIENTRY* const& glColor3b) (GLbyte red, GLbyte green, GLbyte blue);
extern void         (KHRONOS_APIENTRY* const& glColor3bv) (const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glColor3d) (GLdouble red, GLdouble green, GLdouble blue);
extern void         (KHRONOS_APIENTRY* const& glColor3dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glColor3f) (GLfloat red, GLfloat green, GLfloat blue);
extern void         (KHRONOS_APIENTRY* const& glColor3fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glColor3i) (GLint red, GLint green, GLint blue);
extern void         (KHRONOS_APIENTRY* const& glColor3iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glColor3s) (GLshort red, GLshort green, GLshort blue);
extern void         (KHRONOS_APIENTRY* const& glColor3sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glColor3ub) (GLubyte red, GLubyte green, GLubyte blue);
extern void         (KHRONOS_APIENTRY* const& glColor3ubv) (const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glColor3ui) (GLuint red, GLuint green, GLuint blue);
extern void         (KHRONOS_APIENTRY* const& glColor3uiv) (const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glColor3us) (GLushort red, GLushort green, GLushort blue);
extern void         (KHRONOS_APIENTRY* const& glColor3usv) (const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glColor4b) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4bv) (const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glColor4d) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glColor4i) (GLint red, GLint green, GLint blue, GLint alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glColor4s) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4ubv) (const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glColor4ui) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4uiv) (const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glColor4us) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4usv) (const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glEdgeFlag) (GLboolean flag);
extern void         (KHRONOS_APIENTRY* const& glEdgeFlagv) (const GLboolean *flag);
extern void         (KHRONOS_APIENTRY* const& glEnd) ();
extern void         (KHRONOS_APIENTRY* const& glIndexd) (GLdouble c);
extern void         (KHRONOS_APIENTRY* const& glIndexdv) (const GLdouble *c);
extern void         (KHRONOS_APIENTRY* const& glIndexf) (GLfloat c);
extern void         (KHRONOS_APIENTRY* const& glIndexfv) (const GLfloat *c);
extern void         (KHRONOS_APIENTRY* const& glIndexi) (GLint c);
extern void         (KHRONOS_APIENTRY* const& glIndexiv) (const GLint *c);
extern void         (KHRONOS_APIENTRY* const& glIndexs) (GLshort c);
extern void         (KHRONOS_APIENTRY* const& glIndexsv) (const GLshort *c);
extern void         (KHRONOS_APIENTRY* const& glNormal3b) (GLbyte nx, GLbyte ny, GLbyte nz);
extern void         (KHRONOS_APIENTRY* const& glNormal3bv) (const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glNormal3d) (GLdouble nx, GLdouble ny, GLdouble nz);
extern void         (KHRONOS_APIENTRY* const& glNormal3dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
extern void         (KHRONOS_APIENTRY* const& glNormal3fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glNormal3i) (GLint nx, GLint ny, GLint nz);
extern void         (KHRONOS_APIENTRY* const& glNormal3iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glNormal3s) (GLshort nx, GLshort ny, GLshort nz);
extern void         (KHRONOS_APIENTRY* const& glNormal3sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2d) (GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2f) (GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2i) (GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2s) (GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3d) (GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3f) (GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3i) (GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3s) (GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4i) (GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glRectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern void         (KHRONOS_APIENTRY* const& glRectdv) (const GLdouble *v1, const GLdouble *v2);
extern void         (KHRONOS_APIENTRY* const& glRectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern void         (KHRONOS_APIENTRY* const& glRectfv) (const GLfloat *v1, const GLfloat *v2);
extern void         (KHRONOS_APIENTRY* const& glRecti) (GLint x1, GLint y1, GLint x2, GLint y2);
extern void         (KHRONOS_APIENTRY* const& glRectiv) (const GLint *v1, const GLint *v2);
extern void         (KHRONOS_APIENTRY* const& glRects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern void         (KHRONOS_APIENTRY* const& glRectsv) (const GLshort *v1, const GLshort *v2);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1d) (GLdouble s);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1f) (GLfloat s);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1i) (GLint s);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1s) (GLshort s);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2d) (GLdouble s, GLdouble t);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2f) (GLfloat s, GLfloat t);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2i) (GLint s, GLint t);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2s) (GLshort s, GLshort t);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3d) (GLdouble s, GLdouble t, GLdouble r);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3f) (GLfloat s, GLfloat t, GLfloat r);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3i) (GLint s, GLint t, GLint r);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3s) (GLshort s, GLshort t, GLshort r);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4d) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4f) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4i) (GLint s, GLint t, GLint r, GLint q);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4s) (GLshort s, GLshort t, GLshort r, GLshort q);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertex2d) (GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glVertex2dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertex2f) (GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glVertex2fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertex2i) (GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glVertex2iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertex2s) (GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glVertex2sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertex3d) (GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glVertex3dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertex3f) (GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glVertex3fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertex3i) (GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glVertex3iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertex3s) (GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glVertex3sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertex4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glVertex4dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertex4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glVertex4fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertex4i) (GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glVertex4iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertex4s) (GLshort x, GLshort y, GLshort z, GLshort w);
extern void         (KHRONOS_APIENTRY* const& glVertex4sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glClipPlane) (GLenum plane, const GLdouble *equation);
extern void         (KHRONOS_APIENTRY* const& glColorMaterial) (GLenum face, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glFogf) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glFogfv) (GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glFogi) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glFogiv) (GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glLightf) (GLenum light, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glLightfv) (GLenum light, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glLighti) (GLenum light, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glLightiv) (GLenum light, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glLightModelf) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glLightModelfv) (GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glLightModeli) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glLightModeliv) (GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glLineStipple) (GLint factor, GLushort pattern);
extern void         (KHRONOS_APIENTRY* const& glMaterialf) (GLenum face, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glMaterialfv) (GLenum face, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glMateriali) (GLenum face, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glMaterialiv) (GLenum face, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glPolygonStipple) (const GLubyte *mask);
extern void         (KHRONOS_APIENTRY* const& glShadeModel) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glTexEnvf) (GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glTexEnvfv) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glTexEnvi) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glTexEnviv) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexGend) (GLenum coord, GLenum pname, GLdouble param);
extern void         (KHRONOS_APIENTRY* const& glTexGendv) (GLenum coord, GLenum pname, const GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glTexGenf) (GLenum coord, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glTexGenfv) (GLenum coord, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glTexGeni) (GLenum coord, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glTexGeniv) (GLenum coord, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glFeedbackBuffer) (GLsizei size, GLenum type, GLfloat *buffer);
extern void         (KHRONOS_APIENTRY* const& glSelectBuffer) (GLsizei size, GLuint *buffer);
extern GLint        (KHRONOS_APIENTRY* const& glRenderMode) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glInitNames) ();
extern void         (KHRONOS_APIENTRY* const& glLoadName) (GLuint name);
extern void         (KHRONOS_APIENTRY* const& glPassThrough) (GLfloat token);
extern void         (KHRONOS_APIENTRY* const& glPopName) ();
extern void         (KHRONOS_APIENTRY* const& glPushName) (GLuint name);
extern void         (KHRONOS_APIENTRY* const& glClearAccum) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void         (KHRONOS_APIENTRY* const& glClearIndex) (GLfloat c);
extern void         (KHRONOS_APIENTRY* const& glIndexMask) (GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glAccum) (GLenum op, GLfloat value);
extern void         (KHRONOS_APIENTRY* const& glPopAttrib) ();
extern void         (KHRONOS_APIENTRY* const& glPushAttrib) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glMap1d) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
extern void         (KHRONOS_APIENTRY* const& glMap1f) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
extern void         (KHRONOS_APIENTRY* const& glMap2d) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
extern void         (KHRONOS_APIENTRY* const& glMap2f) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
extern void         (KHRONOS_APIENTRY* const& glMapGrid1d) (GLint un, GLdouble u1, GLdouble u2);
extern void         (KHRONOS_APIENTRY* const& glMapGrid1f) (GLint un, GLfloat u1, GLfloat u2);
extern void         (KHRONOS_APIENTRY* const& glMapGrid2d) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern void         (KHRONOS_APIENTRY* const& glMapGrid2f) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord1d) (GLdouble u);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord1dv) (const GLdouble *u);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord1f) (GLfloat u);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord1fv) (const GLfloat *u);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord2d) (GLdouble u, GLdouble v);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord2dv) (const GLdouble *u);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord2f) (GLfloat u, GLfloat v);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord2fv) (const GLfloat *u);
extern void         (KHRONOS_APIENTRY* const& glEvalMesh1) (GLenum mode, GLint i1, GLint i2);
extern void         (KHRONOS_APIENTRY* const& glEvalPoint1) (GLint i);
extern void         (KHRONOS_APIENTRY* const& glEvalMesh2) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern void         (KHRONOS_APIENTRY* const& glEvalPoint2) (GLint i, GLint j);
extern void         (KHRONOS_APIENTRY* const& glAlphaFunc) (GLenum func, GLfloat ref);
extern void         (KHRONOS_APIENTRY* const& glPixelZoom) (GLfloat xfactor, GLfloat yfactor);
extern void         (KHRONOS_APIENTRY* const& glPixelTransferf) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPixelTransferi) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glPixelMapfv) (GLenum map, GLsizei mapsize, const GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glPixelMapuiv) (GLenum map, GLsizei mapsize, const GLuint *values);
extern void         (KHRONOS_APIENTRY* const& glPixelMapusv) (GLenum map, GLsizei mapsize, const GLushort *values);
extern void         (KHRONOS_APIENTRY* const& glCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern void         (KHRONOS_APIENTRY* const& glDrawPixels) (GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetClipPlane) (GLenum plane, GLdouble *equation);
extern void         (KHRONOS_APIENTRY* const& glGetLightfv) (GLenum light, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetLightiv) (GLenum light, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMapdv) (GLenum target, GLenum query, GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glGetMapfv) (GLenum target, GLenum query, GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glGetMapiv) (GLenum target, GLenum query, GLint *v);
extern void         (KHRONOS_APIENTRY* const& glGetMaterialfv) (GLenum face, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMaterialiv) (GLenum face, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetPixelMapfv) (GLenum map, GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glGetPixelMapuiv) (GLenum map, GLuint *values);
extern void         (KHRONOS_APIENTRY* const& glGetPixelMapusv) (GLenum map, GLushort *values);
extern void         (KHRONOS_APIENTRY* const& glGetPolygonStipple) (GLubyte *mask);
extern void         (KHRONOS_APIENTRY* const& glGetTexEnvfv) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexEnviv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexGendv) (GLenum coord, GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexGenfv) (GLenum coord, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexGeniv) (GLenum coord, GLenum pname, GLint *params);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsList) (GLuint list);
extern void         (KHRONOS_APIENTRY* const& glFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void         (KHRONOS_APIENTRY* const& glLoadIdentity) ();
extern void         (KHRONOS_APIENTRY* const& glLoadMatrixf) (const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glLoadMatrixd) (const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMode) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glMultMatrixf) (const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMultMatrixd) (const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void         (KHRONOS_APIENTRY* const& glPopMatrix) ();
extern void         (KHRONOS_APIENTRY* const& glPushMatrix) ();
extern void         (KHRONOS_APIENTRY* const& glRotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glScaled) (GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glScalef) (GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glTranslated) (GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glTranslatef) (GLfloat x, GLfloat y, GLfloat z);
#endif

#ifndef GL_VERSION_1_1
#define GL_VERSION_1_1 1
enum : GLenum
{
    GL_COLOR_LOGIC_OP                                       = 0x0BF2,
    GL_POLYGON_OFFSET_UNITS                                 = 0x2A00,
    GL_POLYGON_OFFSET_POINT                                 = 0x2A01,
    GL_POLYGON_OFFSET_LINE                                  = 0x2A02,
    GL_POLYGON_OFFSET_FILL                                  = 0x8037,
    GL_POLYGON_OFFSET_FACTOR                                = 0x8038,
    GL_TEXTURE_BINDING_1D                                   = 0x8068,
    GL_TEXTURE_BINDING_2D                                   = 0x8069,
    GL_TEXTURE_INTERNAL_FORMAT                              = 0x1003,
    GL_TEXTURE_RED_SIZE                                     = 0x805C,
    GL_TEXTURE_GREEN_SIZE                                   = 0x805D,
    GL_TEXTURE_BLUE_SIZE                                    = 0x805E,
    GL_TEXTURE_ALPHA_SIZE                                   = 0x805F,
    GL_DOUBLE                                               = 0x140A,
    GL_PROXY_TEXTURE_1D                                     = 0x8063,
    GL_PROXY_TEXTURE_2D                                     = 0x8064,
    GL_R3_G3_B2                                             = 0x2A10,
    GL_RGB4                                                 = 0x804F,
    GL_RGB5                                                 = 0x8050,
    GL_RGB8                                                 = 0x8051,
    GL_RGB10                                                = 0x8052,
    GL_RGB12                                                = 0x8053,
    GL_RGB16                                                = 0x8054,
    GL_RGBA2                                                = 0x8055,
    GL_RGBA4                                                = 0x8056,
    GL_RGB5_A1                                              = 0x8057,
    GL_RGBA8                                                = 0x8058,
    GL_RGB10_A2                                             = 0x8059,
    GL_RGBA12                                               = 0x805A,
    GL_RGBA16                                               = 0x805B,
    GL_CLIENT_PIXEL_STORE_BIT                               = 0x00000001,
    GL_CLIENT_VERTEX_ARRAY_BIT                              = 0x00000002,
    GL_CLIENT_ALL_ATTRIB_BITS                               = 0xFFFFFFFF,
    GL_VERTEX_ARRAY_POINTER                                 = 0x808E,
    GL_NORMAL_ARRAY_POINTER                                 = 0x808F,
    GL_COLOR_ARRAY_POINTER                                  = 0x8090,
    GL_INDEX_ARRAY_POINTER                                  = 0x8091,
    GL_TEXTURE_COORD_ARRAY_POINTER                          = 0x8092,
    GL_EDGE_FLAG_ARRAY_POINTER                              = 0x8093,
    GL_FEEDBACK_BUFFER_POINTER                              = 0x0DF0,
    GL_SELECTION_BUFFER_POINTER                             = 0x0DF3,
    GL_CLIENT_ATTRIB_STACK_DEPTH                            = 0x0BB1,
    GL_INDEX_LOGIC_OP                                       = 0x0BF1,
    GL_MAX_CLIENT_ATTRIB_STACK_DEPTH                        = 0x0D3B,
    GL_FEEDBACK_BUFFER_SIZE                                 = 0x0DF1,
    GL_FEEDBACK_BUFFER_TYPE                                 = 0x0DF2,
    GL_SELECTION_BUFFER_SIZE                                = 0x0DF4,
    GL_VERTEX_ARRAY                                         = 0x8074,
    GL_NORMAL_ARRAY                                         = 0x8075,
    GL_COLOR_ARRAY                                          = 0x8076,
    GL_INDEX_ARRAY                                          = 0x8077,
    GL_TEXTURE_COORD_ARRAY                                  = 0x8078,
    GL_EDGE_FLAG_ARRAY                                      = 0x8079,
    GL_VERTEX_ARRAY_SIZE                                    = 0x807A,
    GL_VERTEX_ARRAY_TYPE                                    = 0x807B,
    GL_VERTEX_ARRAY_STRIDE                                  = 0x807C,
    GL_NORMAL_ARRAY_TYPE                                    = 0x807E,
    GL_NORMAL_ARRAY_STRIDE                                  = 0x807F,
    GL_COLOR_ARRAY_SIZE                                     = 0x8081,
    GL_COLOR_ARRAY_TYPE                                     = 0x8082,
    GL_COLOR_ARRAY_STRIDE                                   = 0x8083,
    GL_INDEX_ARRAY_TYPE                                     = 0x8085,
    GL_INDEX_ARRAY_STRIDE                                   = 0x8086,
    GL_TEXTURE_COORD_ARRAY_SIZE                             = 0x8088,
    GL_TEXTURE_COORD_ARRAY_TYPE                             = 0x8089,
    GL_TEXTURE_COORD_ARRAY_STRIDE                           = 0x808A,
    GL_EDGE_FLAG_ARRAY_STRIDE                               = 0x808C,
    GL_TEXTURE_LUMINANCE_SIZE                               = 0x8060,
    GL_TEXTURE_INTENSITY_SIZE                               = 0x8061,
    GL_TEXTURE_PRIORITY                                     = 0x8066,
    GL_TEXTURE_RESIDENT                                     = 0x8067,
    GL_ALPHA4                                               = 0x803B,
    GL_ALPHA8                                               = 0x803C,
    GL_ALPHA12                                              = 0x803D,
    GL_ALPHA16                                              = 0x803E,
    GL_LUMINANCE4                                           = 0x803F,
    GL_LUMINANCE8                                           = 0x8040,
    GL_LUMINANCE12                                          = 0x8041,
    GL_LUMINANCE16                                          = 0x8042,
    GL_LUMINANCE4_ALPHA4                                    = 0x8043,
    GL_LUMINANCE6_ALPHA2                                    = 0x8044,
    GL_LUMINANCE8_ALPHA8                                    = 0x8045,
    GL_LUMINANCE12_ALPHA4                                   = 0x8046,
    GL_LUMINANCE12_ALPHA12                                  = 0x8047,
    GL_LUMINANCE16_ALPHA16                                  = 0x8048,
    GL_INTENSITY                                            = 0x8049,
    GL_INTENSITY4                                           = 0x804A,
    GL_INTENSITY8                                           = 0x804B,
    GL_INTENSITY12                                          = 0x804C,
    GL_INTENSITY16                                          = 0x804D,
    GL_V2F                                                  = 0x2A20,
    GL_V3F                                                  = 0x2A21,
    GL_C4UB_V2F                                             = 0x2A22,
    GL_C4UB_V3F                                             = 0x2A23,
    GL_C3F_V3F                                              = 0x2A24,
    GL_N3F_V3F                                              = 0x2A25,
    GL_C4F_N3F_V3F                                          = 0x2A26,
    GL_T2F_V3F                                              = 0x2A27,
    GL_T4F_V4F                                              = 0x2A28,
    GL_T2F_C4UB_V3F                                         = 0x2A29,
    GL_T2F_C3F_V3F                                          = 0x2A2A,
    GL_T2F_N3F_V3F                                          = 0x2A2B,
    GL_T2F_C4F_N3F_V3F                                      = 0x2A2C,
    GL_T4F_C4F_N3F_V4F                                      = 0x2A2D,
};
extern void         (KHRONOS_APIENTRY* const& glDrawArrays) (GLenum mode, GLint first, GLsizei count);
extern void         (KHRONOS_APIENTRY* const& glDrawElements) (GLenum mode, GLsizei count, GLenum type, const void *indices);
extern void         (KHRONOS_APIENTRY* const& glGetPointerv) (GLenum pname, void **params);
extern void         (KHRONOS_APIENTRY* const& glPolygonOffset) (GLfloat factor, GLfloat units);
extern void         (KHRONOS_APIENTRY* const& glCopyTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glBindTexture) (GLenum target, GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glDeleteTextures) (GLsizei n, const GLuint *textures);
extern void         (KHRONOS_APIENTRY* const& glGenTextures) (GLsizei n, GLuint *textures);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTexture) (GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glArrayElement) (GLint i);
extern void         (KHRONOS_APIENTRY* const& glColorPointer) (GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glDisableClientState) (GLenum array);
extern void         (KHRONOS_APIENTRY* const& glEdgeFlagPointer) (GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glEnableClientState) (GLenum array);
extern void         (KHRONOS_APIENTRY* const& glIndexPointer) (GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glInterleavedArrays) (GLenum format, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glNormalPointer) (GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glVertexPointer) (GLint size, GLenum type, GLsizei stride, const void *pointer);
extern GLboolean    (KHRONOS_APIENTRY* const& glAreTexturesResident) (GLsizei n, const GLuint *textures, GLboolean *residences);
extern void         (KHRONOS_APIENTRY* const& glPrioritizeTextures) (GLsizei n, const GLuint *textures, const GLfloat *priorities);
extern void         (KHRONOS_APIENTRY* const& glIndexub) (GLubyte c);
extern void         (KHRONOS_APIENTRY* const& glIndexubv) (const GLubyte *c);
extern void         (KHRONOS_APIENTRY* const& glPopClientAttrib) ();
extern void         (KHRONOS_APIENTRY* const& glPushClientAttrib) (GLbitfield mask);
#endif

#ifndef GL_VERSION_1_2
#define GL_VERSION_1_2 1
enum : GLenum
{
    GL_UNSIGNED_BYTE_3_3_2                                  = 0x8032,
    GL_UNSIGNED_SHORT_4_4_4_4                               = 0x8033,
    GL_UNSIGNED_SHORT_5_5_5_1                               = 0x8034,
    GL_UNSIGNED_INT_8_8_8_8                                 = 0x8035,
    GL_UNSIGNED_INT_10_10_10_2                              = 0x8036,
    GL_TEXTURE_BINDING_3D                                   = 0x806A,
    GL_PACK_SKIP_IMAGES                                     = 0x806B,
    GL_PACK_IMAGE_HEIGHT                                    = 0x806C,
    GL_UNPACK_SKIP_IMAGES                                   = 0x806D,
    GL_UNPACK_IMAGE_HEIGHT                                  = 0x806E,
    GL_TEXTURE_3D                                           = 0x806F,
    GL_PROXY_TEXTURE_3D                                     = 0x8070,
    GL_TEXTURE_DEPTH                                        = 0x8071,
    GL_TEXTURE_WRAP_R                                       = 0x8072,
    GL_MAX_3D_TEXTURE_SIZE                                  = 0x8073,
    GL_UNSIGNED_BYTE_2_3_3_REV                              = 0x8362,
    GL_UNSIGNED_SHORT_5_6_5                                 = 0x8363,
    GL_UNSIGNED_SHORT_5_6_5_REV                             = 0x8364,
    GL_UNSIGNED_SHORT_4_4_4_4_REV                           = 0x8365,
    GL_UNSIGNED_SHORT_1_5_5_5_REV                           = 0x8366,
    GL_UNSIGNED_INT_8_8_8_8_REV                             = 0x8367,
    GL_UNSIGNED_INT_2_10_10_10_REV                          = 0x8368,
    GL_BGR                                                  = 0x80E0,
    GL_BGRA                                                 = 0x80E1,
    GL_MAX_ELEMENTS_VERTICES                                = 0x80E8,
    GL_MAX_ELEMENTS_INDICES                                 = 0x80E9,
    GL_CLAMP_TO_EDGE                                        = 0x812F,
    GL_TEXTURE_MIN_LOD                                      = 0x813A,
    GL_TEXTURE_MAX_LOD                                      = 0x813B,
    GL_TEXTURE_BASE_LEVEL                                   = 0x813C,
    GL_TEXTURE_MAX_LEVEL                                    = 0x813D,
    GL_SMOOTH_POINT_SIZE_RANGE                              = 0x0B12,
    GL_SMOOTH_POINT_SIZE_GRANULARITY                        = 0x0B13,
    GL_SMOOTH_LINE_WIDTH_RANGE                              = 0x0B22,
    GL_SMOOTH_LINE_WIDTH_GRANULARITY                        = 0x0B23,
    GL_ALIASED_LINE_WIDTH_RANGE                             = 0x846E,
    GL_RESCALE_NORMAL                                       = 0x803A,
    GL_LIGHT_MODEL_COLOR_CONTROL                            = 0x81F8,
    GL_SINGLE_COLOR                                         = 0x81F9,
    GL_SEPARATE_SPECULAR_COLOR                              = 0x81FA,
    GL_ALIASED_POINT_SIZE_RANGE                             = 0x846D,
};
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
extern void         (KHRONOS_APIENTRY* const& glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#endif

#ifndef GL_VERSION_1_3
#define GL_VERSION_1_3 1
enum : GLenum
{
    GL_TEXTURE0                                             = 0x84C0,
    GL_TEXTURE1                                             = 0x84C1,
    GL_TEXTURE2                                             = 0x84C2,
    GL_TEXTURE3                                             = 0x84C3,
    GL_TEXTURE4                                             = 0x84C4,
    GL_TEXTURE5                                             = 0x84C5,
    GL_TEXTURE6                                             = 0x84C6,
    GL_TEXTURE7                                             = 0x84C7,
    GL_TEXTURE8                                             = 0x84C8,
    GL_TEXTURE9                                             = 0x84C9,
    GL_TEXTURE10                                            = 0x84CA,
    GL_TEXTURE11                                            = 0x84CB,
    GL_TEXTURE12                                            = 0x84CC,
    GL_TEXTURE13                                            = 0x84CD,
    GL_TEXTURE14                                            = 0x84CE,
    GL_TEXTURE15                                            = 0x84CF,
    GL_TEXTURE16                                            = 0x84D0,
    GL_TEXTURE17                                            = 0x84D1,
    GL_TEXTURE18                                            = 0x84D2,
    GL_TEXTURE19                                            = 0x84D3,
    GL_TEXTURE20                                            = 0x84D4,
    GL_TEXTURE21                                            = 0x84D5,
    GL_TEXTURE22                                            = 0x84D6,
    GL_TEXTURE23                                            = 0x84D7,
    GL_TEXTURE24                                            = 0x84D8,
    GL_TEXTURE25                                            = 0x84D9,
    GL_TEXTURE26                                            = 0x84DA,
    GL_TEXTURE27                                            = 0x84DB,
    GL_TEXTURE28                                            = 0x84DC,
    GL_TEXTURE29                                            = 0x84DD,
    GL_TEXTURE30                                            = 0x84DE,
    GL_TEXTURE31                                            = 0x84DF,
    GL_ACTIVE_TEXTURE                                       = 0x84E0,
    GL_MULTISAMPLE                                          = 0x809D,
    GL_SAMPLE_ALPHA_TO_COVERAGE                             = 0x809E,
    GL_SAMPLE_ALPHA_TO_ONE                                  = 0x809F,
    GL_SAMPLE_COVERAGE                                      = 0x80A0,
    GL_SAMPLE_BUFFERS                                       = 0x80A8,
    GL_SAMPLES                                              = 0x80A9,
    GL_SAMPLE_COVERAGE_VALUE                                = 0x80AA,
    GL_SAMPLE_COVERAGE_INVERT                               = 0x80AB,
    GL_TEXTURE_CUBE_MAP                                     = 0x8513,
    GL_TEXTURE_BINDING_CUBE_MAP                             = 0x8514,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X                          = 0x8515,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X                          = 0x8516,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y                          = 0x8517,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                          = 0x8518,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z                          = 0x8519,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                          = 0x851A,
    GL_PROXY_TEXTURE_CUBE_MAP                               = 0x851B,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE                            = 0x851C,
    GL_COMPRESSED_RGB                                       = 0x84ED,
    GL_COMPRESSED_RGBA                                      = 0x84EE,
    GL_TEXTURE_COMPRESSION_HINT                             = 0x84EF,
    GL_TEXTURE_COMPRESSED_IMAGE_SIZE                        = 0x86A0,
    GL_TEXTURE_COMPRESSED                                   = 0x86A1,
    GL_NUM_COMPRESSED_TEXTURE_FORMATS                       = 0x86A2,
    GL_COMPRESSED_TEXTURE_FORMATS                           = 0x86A3,
    GL_CLAMP_TO_BORDER                                      = 0x812D,
    GL_CLIENT_ACTIVE_TEXTURE                                = 0x84E1,
    GL_MAX_TEXTURE_UNITS                                    = 0x84E2,
    GL_TRANSPOSE_MODELVIEW_MATRIX                           = 0x84E3,
    GL_TRANSPOSE_PROJECTION_MATRIX                          = 0x84E4,
    GL_TRANSPOSE_TEXTURE_MATRIX                             = 0x84E5,
    GL_TRANSPOSE_COLOR_MATRIX                               = 0x84E6,
    GL_MULTISAMPLE_BIT                                      = 0x20000000,
    GL_NORMAL_MAP                                           = 0x8511,
    GL_REFLECTION_MAP                                       = 0x8512,
    GL_COMPRESSED_ALPHA                                     = 0x84E9,
    GL_COMPRESSED_LUMINANCE                                 = 0x84EA,
    GL_COMPRESSED_LUMINANCE_ALPHA                           = 0x84EB,
    GL_COMPRESSED_INTENSITY                                 = 0x84EC,
    GL_COMBINE                                              = 0x8570,
    GL_COMBINE_RGB                                          = 0x8571,
    GL_COMBINE_ALPHA                                        = 0x8572,
    GL_SOURCE0_RGB                                          = 0x8580,
    GL_SOURCE1_RGB                                          = 0x8581,
    GL_SOURCE2_RGB                                          = 0x8582,
    GL_SOURCE0_ALPHA                                        = 0x8588,
    GL_SOURCE1_ALPHA                                        = 0x8589,
    GL_SOURCE2_ALPHA                                        = 0x858A,
    GL_OPERAND0_RGB                                         = 0x8590,
    GL_OPERAND1_RGB                                         = 0x8591,
    GL_OPERAND2_RGB                                         = 0x8592,
    GL_OPERAND0_ALPHA                                       = 0x8598,
    GL_OPERAND1_ALPHA                                       = 0x8599,
    GL_OPERAND2_ALPHA                                       = 0x859A,
    GL_RGB_SCALE                                            = 0x8573,
    GL_ADD_SIGNED                                           = 0x8574,
    GL_INTERPOLATE                                          = 0x8575,
    GL_SUBTRACT                                             = 0x84E7,
    GL_CONSTANT                                             = 0x8576,
    GL_PRIMARY_COLOR                                        = 0x8577,
    GL_PREVIOUS                                             = 0x8578,
    GL_DOT3_RGB                                             = 0x86AE,
    GL_DOT3_RGBA                                            = 0x86AF,
};
extern void         (KHRONOS_APIENTRY* const& glActiveTexture) (GLenum texture);
extern void         (KHRONOS_APIENTRY* const& glSampleCoverage) (GLfloat value, GLboolean invert);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glGetCompressedTexImage) (GLenum target, GLint level, void *img);
extern void         (KHRONOS_APIENTRY* const& glClientActiveTexture) (GLenum texture);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1d) (GLenum target, GLdouble s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1dv) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1f) (GLenum target, GLfloat s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1fv) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1i) (GLenum target, GLint s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1iv) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1s) (GLenum target, GLshort s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1sv) (GLenum target, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2d) (GLenum target, GLdouble s, GLdouble t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2dv) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2f) (GLenum target, GLfloat s, GLfloat t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2fv) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2i) (GLenum target, GLint s, GLint t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2iv) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2s) (GLenum target, GLshort s, GLshort t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2sv) (GLenum target, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3d) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3dv) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3f) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3fv) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3i) (GLenum target, GLint s, GLint t, GLint r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3iv) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3s) (GLenum target, GLshort s, GLshort t, GLshort r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3sv) (GLenum target, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4d) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4dv) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4f) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4fv) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4i) (GLenum target, GLint s, GLint t, GLint r, GLint q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4iv) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4s) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4sv) (GLenum target, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glLoadTransposeMatrixf) (const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glLoadTransposeMatrixd) (const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMultTransposeMatrixf) (const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMultTransposeMatrixd) (const GLdouble *m);
#endif

#ifndef GL_VERSION_1_4
#define GL_VERSION_1_4 1
enum : GLenum
{
    GL_BLEND_DST_RGB                                        = 0x80C8,
    GL_BLEND_SRC_RGB                                        = 0x80C9,
    GL_BLEND_DST_ALPHA                                      = 0x80CA,
    GL_BLEND_SRC_ALPHA                                      = 0x80CB,
    GL_POINT_FADE_THRESHOLD_SIZE                            = 0x8128,
    GL_DEPTH_COMPONENT16                                    = 0x81A5,
    GL_DEPTH_COMPONENT24                                    = 0x81A6,
    GL_DEPTH_COMPONENT32                                    = 0x81A7,
    GL_MIRRORED_REPEAT                                      = 0x8370,
    GL_MAX_TEXTURE_LOD_BIAS                                 = 0x84FD,
    GL_TEXTURE_LOD_BIAS                                     = 0x8501,
    GL_INCR_WRAP                                            = 0x8507,
    GL_DECR_WRAP                                            = 0x8508,
    GL_TEXTURE_DEPTH_SIZE                                   = 0x884A,
    GL_TEXTURE_COMPARE_MODE                                 = 0x884C,
    GL_TEXTURE_COMPARE_FUNC                                 = 0x884D,
    GL_POINT_SIZE_MIN                                       = 0x8126,
    GL_POINT_SIZE_MAX                                       = 0x8127,
    GL_POINT_DISTANCE_ATTENUATION                           = 0x8129,
    GL_GENERATE_MIPMAP                                      = 0x8191,
    GL_GENERATE_MIPMAP_HINT                                 = 0x8192,
    GL_FOG_COORDINATE_SOURCE                                = 0x8450,
    GL_FOG_COORDINATE                                       = 0x8451,
    GL_FRAGMENT_DEPTH                                       = 0x8452,
    GL_CURRENT_FOG_COORDINATE                               = 0x8453,
    GL_FOG_COORDINATE_ARRAY_TYPE                            = 0x8454,
    GL_FOG_COORDINATE_ARRAY_STRIDE                          = 0x8455,
    GL_FOG_COORDINATE_ARRAY_POINTER                         = 0x8456,
    GL_FOG_COORDINATE_ARRAY                                 = 0x8457,
    GL_COLOR_SUM                                            = 0x8458,
    GL_CURRENT_SECONDARY_COLOR                              = 0x8459,
    GL_SECONDARY_COLOR_ARRAY_SIZE                           = 0x845A,
    GL_SECONDARY_COLOR_ARRAY_TYPE                           = 0x845B,
    GL_SECONDARY_COLOR_ARRAY_STRIDE                         = 0x845C,
    GL_SECONDARY_COLOR_ARRAY_POINTER                        = 0x845D,
    GL_SECONDARY_COLOR_ARRAY                                = 0x845E,
    GL_TEXTURE_FILTER_CONTROL                               = 0x8500,
    GL_DEPTH_TEXTURE_MODE                                   = 0x884B,
    GL_COMPARE_R_TO_TEXTURE                                 = 0x884E,
    GL_BLEND_COLOR                                          = 0x8005,
    GL_BLEND_EQUATION                                       = 0x8009,
    GL_CONSTANT_COLOR                                       = 0x8001,
    GL_ONE_MINUS_CONSTANT_COLOR                             = 0x8002,
    GL_CONSTANT_ALPHA                                       = 0x8003,
    GL_ONE_MINUS_CONSTANT_ALPHA                             = 0x8004,
    GL_FUNC_ADD                                             = 0x8006,
    GL_FUNC_REVERSE_SUBTRACT                                = 0x800B,
    GL_FUNC_SUBTRACT                                        = 0x800A,
    GL_MIN                                                  = 0x8007,
    GL_MAX                                                  = 0x8008,
};
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparate) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArrays) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElements) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
extern void         (KHRONOS_APIENTRY* const& glPointParameterf) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPointParameterfv) (GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glPointParameteri) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glPointParameteriv) (GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glFogCoordf) (GLfloat coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoordfv) (const GLfloat *coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoordd) (GLdouble coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoorddv) (const GLdouble *coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoordPointer) (GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3b) (GLbyte red, GLbyte green, GLbyte blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3bv) (const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3d) (GLdouble red, GLdouble green, GLdouble blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3f) (GLfloat red, GLfloat green, GLfloat blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3i) (GLint red, GLint green, GLint blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3s) (GLshort red, GLshort green, GLshort blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3ub) (GLubyte red, GLubyte green, GLubyte blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3ubv) (const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3ui) (GLuint red, GLuint green, GLuint blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3uiv) (const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3us) (GLushort red, GLushort green, GLushort blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3usv) (const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColorPointer) (GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2d) (GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2f) (GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2i) (GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2s) (GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3d) (GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3dv) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3f) (GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3fv) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3i) (GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3iv) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3s) (GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3sv) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glBlendColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void         (KHRONOS_APIENTRY* const& glBlendEquation) (GLenum mode);
#endif

#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1
enum : GLenum
{
    GL_BUFFER_SIZE                                          = 0x8764,
    GL_BUFFER_USAGE                                         = 0x8765,
    GL_QUERY_COUNTER_BITS                                   = 0x8864,
    GL_CURRENT_QUERY                                        = 0x8865,
    GL_QUERY_RESULT                                         = 0x8866,
    GL_QUERY_RESULT_AVAILABLE                               = 0x8867,
    GL_ARRAY_BUFFER                                         = 0x8892,
    GL_ELEMENT_ARRAY_BUFFER                                 = 0x8893,
    GL_ARRAY_BUFFER_BINDING                                 = 0x8894,
    GL_ELEMENT_ARRAY_BUFFER_BINDING                         = 0x8895,
    GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING                   = 0x889F,
    GL_READ_ONLY                                            = 0x88B8,
    GL_WRITE_ONLY                                           = 0x88B9,
    GL_READ_WRITE                                           = 0x88BA,
    GL_BUFFER_ACCESS                                        = 0x88BB,
    GL_BUFFER_MAPPED                                        = 0x88BC,
    GL_BUFFER_MAP_POINTER                                   = 0x88BD,
    GL_STREAM_DRAW                                          = 0x88E0,
    GL_STREAM_READ                                          = 0x88E1,
    GL_STREAM_COPY                                          = 0x88E2,
    GL_STATIC_DRAW                                          = 0x88E4,
    GL_STATIC_READ                                          = 0x88E5,
    GL_STATIC_COPY                                          = 0x88E6,
    GL_DYNAMIC_DRAW                                         = 0x88E8,
    GL_DYNAMIC_READ                                         = 0x88E9,
    GL_DYNAMIC_COPY                                         = 0x88EA,
    GL_SAMPLES_PASSED                                       = 0x8914,
    GL_SRC1_ALPHA                                           = 0x8589,
    GL_VERTEX_ARRAY_BUFFER_BINDING                          = 0x8896,
    GL_NORMAL_ARRAY_BUFFER_BINDING                          = 0x8897,
    GL_COLOR_ARRAY_BUFFER_BINDING                           = 0x8898,
    GL_INDEX_ARRAY_BUFFER_BINDING                           = 0x8899,
    GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING                   = 0x889A,
    GL_EDGE_FLAG_ARRAY_BUFFER_BINDING                       = 0x889B,
    GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING                 = 0x889C,
    GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING                  = 0x889D,
    GL_WEIGHT_ARRAY_BUFFER_BINDING                          = 0x889E,
    GL_FOG_COORD_SRC                                        = 0x8450,
    GL_FOG_COORD                                            = 0x8451,
    GL_CURRENT_FOG_COORD                                    = 0x8453,
    GL_FOG_COORD_ARRAY_TYPE                                 = 0x8454,
    GL_FOG_COORD_ARRAY_STRIDE                               = 0x8455,
    GL_FOG_COORD_ARRAY_POINTER                              = 0x8456,
    GL_FOG_COORD_ARRAY                                      = 0x8457,
    GL_FOG_COORD_ARRAY_BUFFER_BINDING                       = 0x889D,
    GL_SRC0_RGB                                             = 0x8580,
    GL_SRC1_RGB                                             = 0x8581,
    GL_SRC2_RGB                                             = 0x8582,
    GL_SRC0_ALPHA                                           = 0x8588,
    GL_SRC2_ALPHA                                           = 0x858A,
};
extern void         (KHRONOS_APIENTRY* const& glGenQueries) (GLsizei n, GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glDeleteQueries) (GLsizei n, const GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsQuery) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glBeginQuery) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glEndQuery) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetQueryiv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectiv) (GLuint id, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glBindBuffer) (GLenum target, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glDeleteBuffers) (GLsizei n, const GLuint *buffers);
extern void         (KHRONOS_APIENTRY* const& glGenBuffers) (GLsizei n, GLuint *buffers);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsBuffer) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glBufferData) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern void         (KHRONOS_APIENTRY* const& glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
extern void         (KHRONOS_APIENTRY* const& glGetBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, void *data);
extern void *       (KHRONOS_APIENTRY* const& glMapBuffer) (GLenum target, GLenum access);
extern GLboolean    (KHRONOS_APIENTRY* const& glUnmapBuffer) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetBufferParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetBufferPointerv) (GLenum target, GLenum pname, void **params);
#endif

#ifndef GL_VERSION_2_0
#define GL_VERSION_2_0 1
enum : GLenum
{
    GL_BLEND_EQUATION_RGB                                   = 0x8009,
    GL_VERTEX_ATTRIB_ARRAY_ENABLED                          = 0x8622,
    GL_VERTEX_ATTRIB_ARRAY_SIZE                             = 0x8623,
    GL_VERTEX_ATTRIB_ARRAY_STRIDE                           = 0x8624,
    GL_VERTEX_ATTRIB_ARRAY_TYPE                             = 0x8625,
    GL_CURRENT_VERTEX_ATTRIB                                = 0x8626,
    GL_VERTEX_PROGRAM_POINT_SIZE                            = 0x8642,
    GL_VERTEX_ATTRIB_ARRAY_POINTER                          = 0x8645,
    GL_STENCIL_BACK_FUNC                                    = 0x8800,
    GL_STENCIL_BACK_FAIL                                    = 0x8801,
    GL_STENCIL_BACK_PASS_DEPTH_FAIL                         = 0x8802,
    GL_STENCIL_BACK_PASS_DEPTH_PASS                         = 0x8803,
    GL_MAX_DRAW_BUFFERS                                     = 0x8824,
    GL_DRAW_BUFFER0                                         = 0x8825,
    GL_DRAW_BUFFER1                                         = 0x8826,
    GL_DRAW_BUFFER2                                         = 0x8827,
    GL_DRAW_BUFFER3                                         = 0x8828,
    GL_DRAW_BUFFER4                                         = 0x8829,
    GL_DRAW_BUFFER5                                         = 0x882A,
    GL_DRAW_BUFFER6                                         = 0x882B,
    GL_DRAW_BUFFER7                                         = 0x882C,
    GL_DRAW_BUFFER8                                         = 0x882D,
    GL_DRAW_BUFFER9                                         = 0x882E,
    GL_DRAW_BUFFER10                                        = 0x882F,
    GL_DRAW_BUFFER11                                        = 0x8830,
    GL_DRAW_BUFFER12                                        = 0x8831,
    GL_DRAW_BUFFER13                                        = 0x8832,
    GL_DRAW_BUFFER14                                        = 0x8833,
    GL_DRAW_BUFFER15                                        = 0x8834,
    GL_BLEND_EQUATION_ALPHA                                 = 0x883D,
    GL_MAX_VERTEX_ATTRIBS                                   = 0x8869,
    GL_VERTEX_ATTRIB_ARRAY_NORMALIZED                       = 0x886A,
    GL_MAX_TEXTURE_IMAGE_UNITS                              = 0x8872,
    GL_FRAGMENT_SHADER                                      = 0x8B30,
    GL_VERTEX_SHADER                                        = 0x8B31,
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS                      = 0x8B49,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS                        = 0x8B4A,
    GL_MAX_VARYING_FLOATS                                   = 0x8B4B,
    GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                       = 0x8B4C,
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS                     = 0x8B4D,
    GL_SHADER_TYPE                                          = 0x8B4F,
    GL_FLOAT_VEC2                                           = 0x8B50,
    GL_FLOAT_VEC3                                           = 0x8B51,
    GL_FLOAT_VEC4                                           = 0x8B52,
    GL_INT_VEC2                                             = 0x8B53,
    GL_INT_VEC3                                             = 0x8B54,
    GL_INT_VEC4                                             = 0x8B55,
    GL_BOOL                                                 = 0x8B56,
    GL_BOOL_VEC2                                            = 0x8B57,
    GL_BOOL_VEC3                                            = 0x8B58,
    GL_BOOL_VEC4                                            = 0x8B59,
    GL_FLOAT_MAT2                                           = 0x8B5A,
    GL_FLOAT_MAT3                                           = 0x8B5B,
    GL_FLOAT_MAT4                                           = 0x8B5C,
    GL_SAMPLER_1D                                           = 0x8B5D,
    GL_SAMPLER_2D                                           = 0x8B5E,
    GL_SAMPLER_3D                                           = 0x8B5F,
    GL_SAMPLER_CUBE                                         = 0x8B60,
    GL_SAMPLER_1D_SHADOW                                    = 0x8B61,
    GL_SAMPLER_2D_SHADOW                                    = 0x8B62,
    GL_DELETE_STATUS                                        = 0x8B80,
    GL_COMPILE_STATUS                                       = 0x8B81,
    GL_LINK_STATUS                                          = 0x8B82,
    GL_VALIDATE_STATUS                                      = 0x8B83,
    GL_INFO_LOG_LENGTH                                      = 0x8B84,
    GL_ATTACHED_SHADERS                                     = 0x8B85,
    GL_ACTIVE_UNIFORMS                                      = 0x8B86,
    GL_ACTIVE_UNIFORM_MAX_LENGTH                            = 0x8B87,
    GL_SHADER_SOURCE_LENGTH                                 = 0x8B88,
    GL_ACTIVE_ATTRIBUTES                                    = 0x8B89,
    GL_ACTIVE_ATTRIBUTE_MAX_LENGTH                          = 0x8B8A,
    GL_FRAGMENT_SHADER_DERIVATIVE_HINT                      = 0x8B8B,
    GL_SHADING_LANGUAGE_VERSION                             = 0x8B8C,
    GL_CURRENT_PROGRAM                                      = 0x8B8D,
    GL_POINT_SPRITE_COORD_ORIGIN                            = 0x8CA0,
    GL_LOWER_LEFT                                           = 0x8CA1,
    GL_UPPER_LEFT                                           = 0x8CA2,
    GL_STENCIL_BACK_REF                                     = 0x8CA3,
    GL_STENCIL_BACK_VALUE_MASK                              = 0x8CA4,
    GL_STENCIL_BACK_WRITEMASK                               = 0x8CA5,
    GL_VERTEX_PROGRAM_TWO_SIDE                              = 0x8643,
    GL_POINT_SPRITE                                         = 0x8861,
    GL_COORD_REPLACE                                        = 0x8862,
    GL_MAX_TEXTURE_COORDS                                   = 0x8871,
};
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparate) (GLenum modeRGB, GLenum modeAlpha);
extern void         (KHRONOS_APIENTRY* const& glDrawBuffers) (GLsizei n, const GLenum *bufs);
extern void         (KHRONOS_APIENTRY* const& glStencilOpSeparate) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
extern void         (KHRONOS_APIENTRY* const& glStencilFuncSeparate) (GLenum face, GLenum func, GLint ref, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilMaskSeparate) (GLenum face, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glAttachShader) (GLuint program, GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glBindAttribLocation) (GLuint program, GLuint index, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glCompileShader) (GLuint shader);
extern GLuint       (KHRONOS_APIENTRY* const& glCreateProgram) ();
extern GLuint       (KHRONOS_APIENTRY* const& glCreateShader) (GLenum type);
extern void         (KHRONOS_APIENTRY* const& glDeleteProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glDeleteShader) (GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glDetachShader) (GLuint program, GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glDisableVertexAttribArray) (GLuint index);
extern void         (KHRONOS_APIENTRY* const& glEnableVertexAttribArray) (GLuint index);
extern void         (KHRONOS_APIENTRY* const& glGetActiveAttrib) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniform) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetAttachedShaders) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
extern GLint        (KHRONOS_APIENTRY* const& glGetAttribLocation) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramInfoLog) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void         (KHRONOS_APIENTRY* const& glGetShaderiv) (GLuint shader, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetShaderInfoLog) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void         (KHRONOS_APIENTRY* const& glGetShaderSource) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
extern GLint        (KHRONOS_APIENTRY* const& glGetUniformLocation) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetUniformfv) (GLuint program, GLint location, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetUniformiv) (GLuint program, GLint location, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribdv) (GLuint index, GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribfv) (GLuint index, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribiv) (GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribPointerv) (GLuint index, GLenum pname, void **pointer);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsProgram) (GLuint program);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsShader) (GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glLinkProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glShaderSource) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
extern void         (KHRONOS_APIENTRY* const& glUseProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glUniform1f) (GLint location, GLfloat v0);
extern void         (KHRONOS_APIENTRY* const& glUniform2f) (GLint location, GLfloat v0, GLfloat v1);
extern void         (KHRONOS_APIENTRY* const& glUniform3f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glUniform4f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void         (KHRONOS_APIENTRY* const& glUniform1i) (GLint location, GLint v0);
extern void         (KHRONOS_APIENTRY* const& glUniform2i) (GLint location, GLint v0, GLint v1);
extern void         (KHRONOS_APIENTRY* const& glUniform3i) (GLint location, GLint v0, GLint v1, GLint v2);
extern void         (KHRONOS_APIENTRY* const& glUniform4i) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void         (KHRONOS_APIENTRY* const& glUniform1fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform1iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glValidateProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1d) (GLuint index, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1f) (GLuint index, GLfloat x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1s) (GLuint index, GLshort x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1sv) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2d) (GLuint index, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2f) (GLuint index, GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2s) (GLuint index, GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2sv) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3d) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3f) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3s) (GLuint index, GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3sv) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4Nbv) (GLuint index, const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4Niv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4Nsv) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4Nub) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4Nubv) (GLuint index, const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4Nuiv) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4Nusv) (GLuint index, const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4bv) (GLuint index, const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4d) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4f) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4iv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4s) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4sv) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4ubv) (GLuint index, const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4uiv) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4usv) (GLuint index, const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribPointer) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
#endif

#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1
enum : GLenum
{
    GL_PIXEL_PACK_BUFFER                                    = 0x88EB,
    GL_PIXEL_UNPACK_BUFFER                                  = 0x88EC,
    GL_PIXEL_PACK_BUFFER_BINDING                            = 0x88ED,
    GL_PIXEL_UNPACK_BUFFER_BINDING                          = 0x88EF,
    GL_FLOAT_MAT2x3                                         = 0x8B65,
    GL_FLOAT_MAT2x4                                         = 0x8B66,
    GL_FLOAT_MAT3x2                                         = 0x8B67,
    GL_FLOAT_MAT3x4                                         = 0x8B68,
    GL_FLOAT_MAT4x2                                         = 0x8B69,
    GL_FLOAT_MAT4x3                                         = 0x8B6A,
    GL_SRGB                                                 = 0x8C40,
    GL_SRGB8                                                = 0x8C41,
    GL_SRGB_ALPHA                                           = 0x8C42,
    GL_SRGB8_ALPHA8                                         = 0x8C43,
    GL_COMPRESSED_SRGB                                      = 0x8C48,
    GL_COMPRESSED_SRGB_ALPHA                                = 0x8C49,
    GL_CURRENT_RASTER_SECONDARY_COLOR                       = 0x845F,
    GL_SLUMINANCE_ALPHA                                     = 0x8C44,
    GL_SLUMINANCE8_ALPHA8                                   = 0x8C45,
    GL_SLUMINANCE                                           = 0x8C46,
    GL_SLUMINANCE8                                          = 0x8C47,
    GL_COMPRESSED_SLUMINANCE                                = 0x8C4A,
    GL_COMPRESSED_SLUMINANCE_ALPHA                          = 0x8C4B,
};
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif

#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1
enum : GLenum
{
    GL_COMPARE_REF_TO_TEXTURE                               = 0x884E,
    GL_CLIP_DISTANCE0                                       = 0x3000,
    GL_CLIP_DISTANCE1                                       = 0x3001,
    GL_CLIP_DISTANCE2                                       = 0x3002,
    GL_CLIP_DISTANCE3                                       = 0x3003,
    GL_CLIP_DISTANCE4                                       = 0x3004,
    GL_CLIP_DISTANCE5                                       = 0x3005,
    GL_CLIP_DISTANCE6                                       = 0x3006,
    GL_CLIP_DISTANCE7                                       = 0x3007,
    GL_MAX_CLIP_DISTANCES                                   = 0x0D32,
    GL_MAJOR_VERSION                                        = 0x821B,
    GL_MINOR_VERSION                                        = 0x821C,
    GL_NUM_EXTENSIONS                                       = 0x821D,
    GL_CONTEXT_FLAGS                                        = 0x821E,
    GL_COMPRESSED_RED                                       = 0x8225,
    GL_COMPRESSED_RG                                        = 0x8226,
    GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT                  = 0x00000001,
    GL_RGBA32F                                              = 0x8814,
    GL_RGB32F                                               = 0x8815,
    GL_RGBA16F                                              = 0x881A,
    GL_RGB16F                                               = 0x881B,
    GL_VERTEX_ATTRIB_ARRAY_INTEGER                          = 0x88FD,
    GL_MAX_ARRAY_TEXTURE_LAYERS                             = 0x88FF,
    GL_MIN_PROGRAM_TEXEL_OFFSET                             = 0x8904,
    GL_MAX_PROGRAM_TEXEL_OFFSET                             = 0x8905,
    GL_CLAMP_READ_COLOR                                     = 0x891C,
    GL_FIXED_ONLY                                           = 0x891D,
    GL_MAX_VARYING_COMPONENTS                               = 0x8B4B,
    GL_TEXTURE_1D_ARRAY                                     = 0x8C18,
    GL_PROXY_TEXTURE_1D_ARRAY                               = 0x8C19,
    GL_TEXTURE_2D_ARRAY                                     = 0x8C1A,
    GL_PROXY_TEXTURE_2D_ARRAY                               = 0x8C1B,
    GL_TEXTURE_BINDING_1D_ARRAY                             = 0x8C1C,
    GL_TEXTURE_BINDING_2D_ARRAY                             = 0x8C1D,
    GL_R11F_G11F_B10F                                       = 0x8C3A,
    GL_UNSIGNED_INT_10F_11F_11F_REV                         = 0x8C3B,
    GL_RGB9_E5                                              = 0x8C3D,
    GL_UNSIGNED_INT_5_9_9_9_REV                             = 0x8C3E,
    GL_TEXTURE_SHARED_SIZE                                  = 0x8C3F,
    GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH                = 0x8C76,
    GL_TRANSFORM_FEEDBACK_BUFFER_MODE                       = 0x8C7F,
    GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS           = 0x8C80,
    GL_TRANSFORM_FEEDBACK_VARYINGS                          = 0x8C83,
    GL_TRANSFORM_FEEDBACK_BUFFER_START                      = 0x8C84,
    GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                       = 0x8C85,
    GL_PRIMITIVES_GENERATED                                 = 0x8C87,
    GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN                = 0x8C88,
    GL_RASTERIZER_DISCARD                                   = 0x8C89,
    GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS        = 0x8C8A,
    GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS              = 0x8C8B,
    GL_INTERLEAVED_ATTRIBS                                  = 0x8C8C,
    GL_SEPARATE_ATTRIBS                                     = 0x8C8D,
    GL_TRANSFORM_FEEDBACK_BUFFER                            = 0x8C8E,
    GL_TRANSFORM_FEEDBACK_BUFFER_BINDING                    = 0x8C8F,
    GL_RGBA32UI                                             = 0x8D70,
    GL_RGB32UI                                              = 0x8D71,
    GL_RGBA16UI                                             = 0x8D76,
    GL_RGB16UI                                              = 0x8D77,
    GL_RGBA8UI                                              = 0x8D7C,
    GL_RGB8UI                                               = 0x8D7D,
    GL_RGBA32I                                              = 0x8D82,
    GL_RGB32I                                               = 0x8D83,
    GL_RGBA16I                                              = 0x8D88,
    GL_RGB16I                                               = 0x8D89,
    GL_RGBA8I                                               = 0x8D8E,
    GL_RGB8I                                                = 0x8D8F,
    GL_RED_INTEGER                                          = 0x8D94,
    GL_GREEN_INTEGER                                        = 0x8D95,
    GL_BLUE_INTEGER                                         = 0x8D96,
    GL_RGB_INTEGER                                          = 0x8D98,
    GL_RGBA_INTEGER                                         = 0x8D99,
    GL_BGR_INTEGER                                          = 0x8D9A,
    GL_BGRA_INTEGER                                         = 0x8D9B,
    GL_SAMPLER_1D_ARRAY                                     = 0x8DC0,
    GL_SAMPLER_2D_ARRAY                                     = 0x8DC1,
    GL_SAMPLER_1D_ARRAY_SHADOW                              = 0x8DC3,
    GL_SAMPLER_2D_ARRAY_SHADOW                              = 0x8DC4,
    GL_SAMPLER_CUBE_SHADOW                                  = 0x8DC5,
    GL_UNSIGNED_INT_VEC2                                    = 0x8DC6,
    GL_UNSIGNED_INT_VEC3                                    = 0x8DC7,
    GL_UNSIGNED_INT_VEC4                                    = 0x8DC8,
    GL_INT_SAMPLER_1D                                       = 0x8DC9,
    GL_INT_SAMPLER_2D                                       = 0x8DCA,
    GL_INT_SAMPLER_3D                                       = 0x8DCB,
    GL_INT_SAMPLER_CUBE                                     = 0x8DCC,
    GL_INT_SAMPLER_1D_ARRAY                                 = 0x8DCE,
    GL_INT_SAMPLER_2D_ARRAY                                 = 0x8DCF,
    GL_UNSIGNED_INT_SAMPLER_1D                              = 0x8DD1,
    GL_UNSIGNED_INT_SAMPLER_2D                              = 0x8DD2,
    GL_UNSIGNED_INT_SAMPLER_3D                              = 0x8DD3,
    GL_UNSIGNED_INT_SAMPLER_CUBE                            = 0x8DD4,
    GL_UNSIGNED_INT_SAMPLER_1D_ARRAY                        = 0x8DD6,
    GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                        = 0x8DD7,
    GL_QUERY_WAIT                                           = 0x8E13,
    GL_QUERY_NO_WAIT                                        = 0x8E14,
    GL_QUERY_BY_REGION_WAIT                                 = 0x8E15,
    GL_QUERY_BY_REGION_NO_WAIT                              = 0x8E16,
    GL_BUFFER_ACCESS_FLAGS                                  = 0x911F,
    GL_BUFFER_MAP_LENGTH                                    = 0x9120,
    GL_BUFFER_MAP_OFFSET                                    = 0x9121,
    GL_DEPTH_COMPONENT32F                                   = 0x8CAC,
    GL_DEPTH32F_STENCIL8                                    = 0x8CAD,
    GL_FLOAT_32_UNSIGNED_INT_24_8_REV                       = 0x8DAD,
    GL_INVALID_FRAMEBUFFER_OPERATION                        = 0x0506,
    GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING                = 0x8210,
    GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE                = 0x8211,
    GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE                      = 0x8212,
    GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE                    = 0x8213,
    GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE                     = 0x8214,
    GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE                    = 0x8215,
    GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE                    = 0x8216,
    GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE                  = 0x8217,
    GL_FRAMEBUFFER_DEFAULT                                  = 0x8218,
    GL_FRAMEBUFFER_UNDEFINED                                = 0x8219,
    GL_DEPTH_STENCIL_ATTACHMENT                             = 0x821A,
    GL_MAX_RENDERBUFFER_SIZE                                = 0x84E8,
    GL_DEPTH_STENCIL                                        = 0x84F9,
    GL_UNSIGNED_INT_24_8                                    = 0x84FA,
    GL_DEPTH24_STENCIL8                                     = 0x88F0,
    GL_TEXTURE_STENCIL_SIZE                                 = 0x88F1,
    GL_TEXTURE_RED_TYPE                                     = 0x8C10,
    GL_TEXTURE_GREEN_TYPE                                   = 0x8C11,
    GL_TEXTURE_BLUE_TYPE                                    = 0x8C12,
    GL_TEXTURE_ALPHA_TYPE                                   = 0x8C13,
    GL_TEXTURE_DEPTH_TYPE                                   = 0x8C16,
    GL_UNSIGNED_NORMALIZED                                  = 0x8C17,
    GL_FRAMEBUFFER_BINDING                                  = 0x8CA6,
    GL_DRAW_FRAMEBUFFER_BINDING                             = 0x8CA6,
    GL_RENDERBUFFER_BINDING                                 = 0x8CA7,
    GL_READ_FRAMEBUFFER                                     = 0x8CA8,
    GL_DRAW_FRAMEBUFFER                                     = 0x8CA9,
    GL_READ_FRAMEBUFFER_BINDING                             = 0x8CAA,
    GL_RENDERBUFFER_SAMPLES                                 = 0x8CAB,
    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE                   = 0x8CD0,
    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME                   = 0x8CD1,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL                 = 0x8CD2,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE         = 0x8CD3,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER                 = 0x8CD4,
    GL_FRAMEBUFFER_COMPLETE                                 = 0x8CD5,
    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT                    = 0x8CD6,
    GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT            = 0x8CD7,
    GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER                   = 0x8CDB,
    GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER                   = 0x8CDC,
    GL_FRAMEBUFFER_UNSUPPORTED                              = 0x8CDD,
    GL_MAX_COLOR_ATTACHMENTS                                = 0x8CDF,
    GL_COLOR_ATTACHMENT0                                    = 0x8CE0,
    GL_COLOR_ATTACHMENT1                                    = 0x8CE1,
    GL_COLOR_ATTACHMENT2                                    = 0x8CE2,
    GL_COLOR_ATTACHMENT3                                    = 0x8CE3,
    GL_COLOR_ATTACHMENT4                                    = 0x8CE4,
    GL_COLOR_ATTACHMENT5                                    = 0x8CE5,
    GL_COLOR_ATTACHMENT6                                    = 0x8CE6,
    GL_COLOR_ATTACHMENT7                                    = 0x8CE7,
    GL_COLOR_ATTACHMENT8                                    = 0x8CE8,
    GL_COLOR_ATTACHMENT9                                    = 0x8CE9,
    GL_COLOR_ATTACHMENT10                                   = 0x8CEA,
    GL_COLOR_ATTACHMENT11                                   = 0x8CEB,
    GL_COLOR_ATTACHMENT12                                   = 0x8CEC,
    GL_COLOR_ATTACHMENT13                                   = 0x8CED,
    GL_COLOR_ATTACHMENT14                                   = 0x8CEE,
    GL_COLOR_ATTACHMENT15                                   = 0x8CEF,
    GL_COLOR_ATTACHMENT16                                   = 0x8CF0,
    GL_COLOR_ATTACHMENT17                                   = 0x8CF1,
    GL_COLOR_ATTACHMENT18                                   = 0x8CF2,
    GL_COLOR_ATTACHMENT19                                   = 0x8CF3,
    GL_COLOR_ATTACHMENT20                                   = 0x8CF4,
    GL_COLOR_ATTACHMENT21                                   = 0x8CF5,
    GL_COLOR_ATTACHMENT22                                   = 0x8CF6,
    GL_COLOR_ATTACHMENT23                                   = 0x8CF7,
    GL_COLOR_ATTACHMENT24                                   = 0x8CF8,
    GL_COLOR_ATTACHMENT25                                   = 0x8CF9,
    GL_COLOR_ATTACHMENT26                                   = 0x8CFA,
    GL_COLOR_ATTACHMENT27                                   = 0x8CFB,
    GL_COLOR_ATTACHMENT28                                   = 0x8CFC,
    GL_COLOR_ATTACHMENT29                                   = 0x8CFD,
    GL_COLOR_ATTACHMENT30                                   = 0x8CFE,
    GL_COLOR_ATTACHMENT31                                   = 0x8CFF,
    GL_DEPTH_ATTACHMENT                                     = 0x8D00,
    GL_STENCIL_ATTACHMENT                                   = 0x8D20,
    GL_FRAMEBUFFER                                          = 0x8D40,
    GL_RENDERBUFFER                                         = 0x8D41,
    GL_RENDERBUFFER_WIDTH                                   = 0x8D42,
    GL_RENDERBUFFER_HEIGHT                                  = 0x8D43,
    GL_RENDERBUFFER_INTERNAL_FORMAT                         = 0x8D44,
    GL_STENCIL_INDEX1                                       = 0x8D46,
    GL_STENCIL_INDEX4                                       = 0x8D47,
    GL_STENCIL_INDEX8                                       = 0x8D48,
    GL_STENCIL_INDEX16                                      = 0x8D49,
    GL_RENDERBUFFER_RED_SIZE                                = 0x8D50,
    GL_RENDERBUFFER_GREEN_SIZE                              = 0x8D51,
    GL_RENDERBUFFER_BLUE_SIZE                               = 0x8D52,
    GL_RENDERBUFFER_ALPHA_SIZE                              = 0x8D53,
    GL_RENDERBUFFER_DEPTH_SIZE                              = 0x8D54,
    GL_RENDERBUFFER_STENCIL_SIZE                            = 0x8D55,
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE                   = 0x8D56,
    GL_MAX_SAMPLES                                          = 0x8D57,
    GL_INDEX                                                = 0x8222,
    GL_TEXTURE_LUMINANCE_TYPE                               = 0x8C14,
    GL_TEXTURE_INTENSITY_TYPE                               = 0x8C15,
    GL_FRAMEBUFFER_SRGB                                     = 0x8DB9,
    GL_HALF_FLOAT                                           = 0x140B,
    GL_MAP_READ_BIT                                         = 0x0001,
    GL_MAP_WRITE_BIT                                        = 0x0002,
    GL_MAP_INVALIDATE_RANGE_BIT                             = 0x0004,
    GL_MAP_INVALIDATE_BUFFER_BIT                            = 0x0008,
    GL_MAP_FLUSH_EXPLICIT_BIT                               = 0x0010,
    GL_MAP_UNSYNCHRONIZED_BIT                               = 0x0020,
    GL_COMPRESSED_RED_RGTC1                                 = 0x8DBB,
    GL_COMPRESSED_SIGNED_RED_RGTC1                          = 0x8DBC,
    GL_COMPRESSED_RG_RGTC2                                  = 0x8DBD,
    GL_COMPRESSED_SIGNED_RG_RGTC2                           = 0x8DBE,
    GL_RG                                                   = 0x8227,
    GL_RG_INTEGER                                           = 0x8228,
    GL_R8                                                   = 0x8229,
    GL_R16                                                  = 0x822A,
    GL_RG8                                                  = 0x822B,
    GL_RG16                                                 = 0x822C,
    GL_R16F                                                 = 0x822D,
    GL_R32F                                                 = 0x822E,
    GL_RG16F                                                = 0x822F,
    GL_RG32F                                                = 0x8230,
    GL_R8I                                                  = 0x8231,
    GL_R8UI                                                 = 0x8232,
    GL_R16I                                                 = 0x8233,
    GL_R16UI                                                = 0x8234,
    GL_R32I                                                 = 0x8235,
    GL_R32UI                                                = 0x8236,
    GL_RG8I                                                 = 0x8237,
    GL_RG8UI                                                = 0x8238,
    GL_RG16I                                                = 0x8239,
    GL_RG16UI                                               = 0x823A,
    GL_RG32I                                                = 0x823B,
    GL_RG32UI                                               = 0x823C,
    GL_VERTEX_ARRAY_BINDING                                 = 0x85B5,
    GL_CLAMP_VERTEX_COLOR                                   = 0x891A,
    GL_CLAMP_FRAGMENT_COLOR                                 = 0x891B,
    GL_ALPHA_INTEGER                                        = 0x8D97,
};
extern void         (KHRONOS_APIENTRY* const& glColorMaski) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern void         (KHRONOS_APIENTRY* const& glGetBooleani_v) (GLenum target, GLuint index, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glGetIntegeri_v) (GLenum target, GLuint index, GLint *data);
extern void         (KHRONOS_APIENTRY* const& glEnablei) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisablei) (GLenum target, GLuint index);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnabledi) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glBeginTransformFeedback) (GLenum primitiveMode);
extern void         (KHRONOS_APIENTRY* const& glEndTransformFeedback) ();
extern void         (KHRONOS_APIENTRY* const& glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackVaryings) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
extern void         (KHRONOS_APIENTRY* const& glGetTransformFeedbackVarying) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glClampColor) (GLenum target, GLenum clamp);
extern void         (KHRONOS_APIENTRY* const& glBeginConditionalRender) (GLuint id, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glEndConditionalRender) ();
extern void         (KHRONOS_APIENTRY* const& glVertexAttribIPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribIiv) (GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribIuiv) (GLuint index, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1i) (GLuint index, GLint x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2i) (GLuint index, GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3i) (GLuint index, GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4i) (GLuint index, GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1ui) (GLuint index, GLuint x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2ui) (GLuint index, GLuint x, GLuint y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3ui) (GLuint index, GLuint x, GLuint y, GLuint z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4ui) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1iv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2iv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3iv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4iv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1uiv) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2uiv) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3uiv) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4uiv) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4bv) (GLuint index, const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4sv) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4ubv) (GLuint index, const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4usv) (GLuint index, const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glGetUniformuiv) (GLuint program, GLint location, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glBindFragDataLocation) (GLuint program, GLuint color, const GLchar *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetFragDataLocation) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glUniform1ui) (GLint location, GLuint v0);
extern void         (KHRONOS_APIENTRY* const& glUniform2ui) (GLint location, GLuint v0, GLuint v1);
extern void         (KHRONOS_APIENTRY* const& glUniform3ui) (GLint location, GLuint v0, GLuint v1, GLuint v2);
extern void         (KHRONOS_APIENTRY* const& glUniform4ui) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern void         (KHRONOS_APIENTRY* const& glUniform1uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glTexParameterIiv) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameterIuiv) (GLenum target, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIiv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIuiv) (GLenum target, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glClearBufferiv) (GLenum buffer, GLint drawbuffer, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glClearBufferuiv) (GLenum buffer, GLint drawbuffer, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glClearBufferfv) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glClearBufferfi) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
extern const GLubyte * (KHRONOS_APIENTRY* const& glGetStringi) (GLenum name, GLuint index);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsRenderbuffer) (GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glBindRenderbuffer) (GLenum target, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glDeleteRenderbuffers) (GLsizei n, const GLuint *renderbuffers);
extern void         (KHRONOS_APIENTRY* const& glGenRenderbuffers) (GLsizei n, GLuint *renderbuffers);
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetRenderbufferParameteriv) (GLenum target, GLenum pname, GLint *params);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsFramebuffer) (GLuint framebuffer);
extern void         (KHRONOS_APIENTRY* const& glBindFramebuffer) (GLenum target, GLuint framebuffer);
extern void         (KHRONOS_APIENTRY* const& glDeleteFramebuffers) (GLsizei n, const GLuint *framebuffers);
extern void         (KHRONOS_APIENTRY* const& glGenFramebuffers) (GLsizei n, GLuint *framebuffers);
extern GLenum       (KHRONOS_APIENTRY* const& glCheckFramebufferStatus) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture1D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture3D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
extern void         (KHRONOS_APIENTRY* const& glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferAttachmentParameteriv) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGenerateMipmap) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureLayer) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern void *       (KHRONOS_APIENTRY* const& glMapBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern void         (KHRONOS_APIENTRY* const& glFlushMappedBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length);
extern void         (KHRONOS_APIENTRY* const& glBindVertexArray) (GLuint array);
extern void         (KHRONOS_APIENTRY* const& glDeleteVertexArrays) (GLsizei n, const GLuint *arrays);
extern void         (KHRONOS_APIENTRY* const& glGenVertexArrays) (GLsizei n, GLuint *arrays);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsVertexArray) (GLuint array);
#endif

#ifndef GL_VERSION_3_1
#define GL_VERSION_3_1 1
enum : GLenum
{
    GL_SAMPLER_2D_RECT                                      = 0x8B63,
    GL_SAMPLER_2D_RECT_SHADOW                               = 0x8B64,
    GL_SAMPLER_BUFFER                                       = 0x8DC2,
    GL_INT_SAMPLER_2D_RECT                                  = 0x8DCD,
    GL_INT_SAMPLER_BUFFER                                   = 0x8DD0,
    GL_UNSIGNED_INT_SAMPLER_2D_RECT                         = 0x8DD5,
    GL_UNSIGNED_INT_SAMPLER_BUFFER                          = 0x8DD8,
    GL_TEXTURE_BUFFER                                       = 0x8C2A,
    GL_MAX_TEXTURE_BUFFER_SIZE                              = 0x8C2B,
    GL_TEXTURE_BINDING_BUFFER                               = 0x8C2C,
    GL_TEXTURE_BUFFER_DATA_STORE_BINDING                    = 0x8C2D,
    GL_TEXTURE_RECTANGLE                                    = 0x84F5,
    GL_TEXTURE_BINDING_RECTANGLE                            = 0x84F6,
    GL_PROXY_TEXTURE_RECTANGLE                              = 0x84F7,
    GL_MAX_RECTANGLE_TEXTURE_SIZE                           = 0x84F8,
    GL_R8_SNORM                                             = 0x8F94,
    GL_RG8_SNORM                                            = 0x8F95,
    GL_RGB8_SNORM                                           = 0x8F96,
    GL_RGBA8_SNORM                                          = 0x8F97,
    GL_R16_SNORM                                            = 0x8F98,
    GL_RG16_SNORM                                           = 0x8F99,
    GL_RGB16_SNORM                                          = 0x8F9A,
    GL_RGBA16_SNORM                                         = 0x8F9B,
    GL_SIGNED_NORMALIZED                                    = 0x8F9C,
    GL_PRIMITIVE_RESTART                                    = 0x8F9D,
    GL_PRIMITIVE_RESTART_INDEX                              = 0x8F9E,
    GL_COPY_READ_BUFFER                                     = 0x8F36,
    GL_COPY_WRITE_BUFFER                                    = 0x8F37,
    GL_UNIFORM_BUFFER                                       = 0x8A11,
    GL_UNIFORM_BUFFER_BINDING                               = 0x8A28,
    GL_UNIFORM_BUFFER_START                                 = 0x8A29,
    GL_UNIFORM_BUFFER_SIZE                                  = 0x8A2A,
    GL_MAX_VERTEX_UNIFORM_BLOCKS                            = 0x8A2B,
    GL_MAX_GEOMETRY_UNIFORM_BLOCKS                          = 0x8A2C,
    GL_MAX_FRAGMENT_UNIFORM_BLOCKS                          = 0x8A2D,
    GL_MAX_COMBINED_UNIFORM_BLOCKS                          = 0x8A2E,
    GL_MAX_UNIFORM_BUFFER_BINDINGS                          = 0x8A2F,
    GL_MAX_UNIFORM_BLOCK_SIZE                               = 0x8A30,
    GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS               = 0x8A31,
    GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS             = 0x8A32,
    GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS             = 0x8A33,
    GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT                      = 0x8A34,
    GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH                 = 0x8A35,
    GL_ACTIVE_UNIFORM_BLOCKS                                = 0x8A36,
    GL_UNIFORM_TYPE                                         = 0x8A37,
    GL_UNIFORM_SIZE                                         = 0x8A38,
    GL_UNIFORM_NAME_LENGTH                                  = 0x8A39,
    GL_UNIFORM_BLOCK_INDEX                                  = 0x8A3A,
    GL_UNIFORM_OFFSET                                       = 0x8A3B,
    GL_UNIFORM_ARRAY_STRIDE                                 = 0x8A3C,
    GL_UNIFORM_MATRIX_STRIDE                                = 0x8A3D,
    GL_UNIFORM_IS_ROW_MAJOR                                 = 0x8A3E,
    GL_UNIFORM_BLOCK_BINDING                                = 0x8A3F,
    GL_UNIFORM_BLOCK_DATA_SIZE                              = 0x8A40,
    GL_UNIFORM_BLOCK_NAME_LENGTH                            = 0x8A41,
    GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                        = 0x8A42,
    GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES                 = 0x8A43,
    GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER            = 0x8A44,
    GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER          = 0x8A45,
    GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER          = 0x8A46,
    GL_INVALID_INDEX                                        = 0xFFFFFFFF,
};
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstanced) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstanced) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
extern void         (KHRONOS_APIENTRY* const& glTexBuffer) (GLenum target, GLenum internalformat, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glPrimitiveRestartIndex) (GLuint index);
extern void         (KHRONOS_APIENTRY* const& glCopyBufferSubData) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glGetUniformIndices) (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformsiv) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformName) (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
extern GLuint       (KHRONOS_APIENTRY* const& glGetUniformBlockIndex) (GLuint program, const GLchar *uniformBlockName);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformBlockiv) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformBlockName) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
extern void         (KHRONOS_APIENTRY* const& glUniformBlockBinding) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
#endif

#ifndef GL_VERSION_3_2
#define GL_VERSION_3_2 1
enum : GLenum
{
    GL_CONTEXT_CORE_PROFILE_BIT                             = 0x00000001,
    GL_CONTEXT_COMPATIBILITY_PROFILE_BIT                    = 0x00000002,
    GL_LINES_ADJACENCY                                      = 0x000A,
    GL_LINE_STRIP_ADJACENCY                                 = 0x000B,
    GL_TRIANGLES_ADJACENCY                                  = 0x000C,
    GL_TRIANGLE_STRIP_ADJACENCY                             = 0x000D,
    GL_PROGRAM_POINT_SIZE                                   = 0x8642,
    GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS                     = 0x8C29,
    GL_FRAMEBUFFER_ATTACHMENT_LAYERED                       = 0x8DA7,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS                 = 0x8DA8,
    GL_GEOMETRY_SHADER                                      = 0x8DD9,
    GL_GEOMETRY_VERTICES_OUT                                = 0x8916,
    GL_GEOMETRY_INPUT_TYPE                                  = 0x8917,
    GL_GEOMETRY_OUTPUT_TYPE                                 = 0x8918,
    GL_MAX_GEOMETRY_UNIFORM_COMPONENTS                      = 0x8DDF,
    GL_MAX_GEOMETRY_OUTPUT_VERTICES                         = 0x8DE0,
    GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS                 = 0x8DE1,
    GL_MAX_VERTEX_OUTPUT_COMPONENTS                         = 0x9122,
    GL_MAX_GEOMETRY_INPUT_COMPONENTS                        = 0x9123,
    GL_MAX_GEOMETRY_OUTPUT_COMPONENTS                       = 0x9124,
    GL_MAX_FRAGMENT_INPUT_COMPONENTS                        = 0x9125,
    GL_CONTEXT_PROFILE_MASK                                 = 0x9126,
    GL_DEPTH_CLAMP                                          = 0x864F,
    GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION             = 0x8E4C,
    GL_FIRST_VERTEX_CONVENTION                              = 0x8E4D,
    GL_LAST_VERTEX_CONVENTION                               = 0x8E4E,
    GL_PROVOKING_VERTEX                                     = 0x8E4F,
    GL_TEXTURE_CUBE_MAP_SEAMLESS                            = 0x884F,
    GL_MAX_SERVER_WAIT_TIMEOUT                              = 0x9111,
    GL_OBJECT_TYPE                                          = 0x9112,
    GL_SYNC_CONDITION                                       = 0x9113,
    GL_SYNC_STATUS                                          = 0x9114,
    GL_SYNC_FLAGS                                           = 0x9115,
    GL_SYNC_FENCE                                           = 0x9116,
    GL_SYNC_GPU_COMMANDS_COMPLETE                           = 0x9117,
    GL_UNSIGNALED                                           = 0x9118,
    GL_SIGNALED                                             = 0x9119,
    GL_ALREADY_SIGNALED                                     = 0x911A,
    GL_TIMEOUT_EXPIRED                                      = 0x911B,
    GL_CONDITION_SATISFIED                                  = 0x911C,
    GL_WAIT_FAILED                                          = 0x911D,
};
enum : GLuint64
{
    GL_TIMEOUT_IGNORED                                      = 0xFFFFFFFFFFFFFFFF,
};
enum : GLenum
{
    GL_SYNC_FLUSH_COMMANDS_BIT                              = 0x00000001,
    GL_SAMPLE_POSITION                                      = 0x8E50,
    GL_SAMPLE_MASK                                          = 0x8E51,
    GL_SAMPLE_MASK_VALUE                                    = 0x8E52,
    GL_MAX_SAMPLE_MASK_WORDS                                = 0x8E59,
    GL_TEXTURE_2D_MULTISAMPLE                               = 0x9100,
    GL_PROXY_TEXTURE_2D_MULTISAMPLE                         = 0x9101,
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY                         = 0x9102,
    GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY                   = 0x9103,
    GL_TEXTURE_BINDING_2D_MULTISAMPLE                       = 0x9104,
    GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY                 = 0x9105,
    GL_TEXTURE_SAMPLES                                      = 0x9106,
    GL_TEXTURE_FIXED_SAMPLE_LOCATIONS                       = 0x9107,
    GL_SAMPLER_2D_MULTISAMPLE                               = 0x9108,
    GL_INT_SAMPLER_2D_MULTISAMPLE                           = 0x9109,
    GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE                  = 0x910A,
    GL_SAMPLER_2D_MULTISAMPLE_ARRAY                         = 0x910B,
    GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY                     = 0x910C,
    GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY            = 0x910D,
    GL_MAX_COLOR_TEXTURE_SAMPLES                            = 0x910E,
    GL_MAX_DEPTH_TEXTURE_SAMPLES                            = 0x910F,
    GL_MAX_INTEGER_SAMPLES                                  = 0x9110,
};
extern void         (KHRONOS_APIENTRY* const& glDrawElementsBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElementsBaseVertex) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsBaseVertex) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
extern void         (KHRONOS_APIENTRY* const& glProvokingVertex) (GLenum mode);
extern GLsync       (KHRONOS_APIENTRY* const& glFenceSync) (GLenum condition, GLbitfield flags);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsSync) (GLsync sync);
extern void         (KHRONOS_APIENTRY* const& glDeleteSync) (GLsync sync);
extern GLenum       (KHRONOS_APIENTRY* const& glClientWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void         (KHRONOS_APIENTRY* const& glWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void         (KHRONOS_APIENTRY* const& glGetInteger64v) (GLenum pname, GLint64 *data);
extern void         (KHRONOS_APIENTRY* const& glGetSynciv) (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values);
extern void         (KHRONOS_APIENTRY* const& glGetInteger64i_v) (GLenum target, GLuint index, GLint64 *data);
extern void         (KHRONOS_APIENTRY* const& glGetBufferParameteri64v) (GLenum target, GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture) (GLenum target, GLenum attachment, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glTexImage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glTexImage3DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glGetMultisamplefv) (GLenum pname, GLuint index, GLfloat *val);
extern void         (KHRONOS_APIENTRY* const& glSampleMaski) (GLuint maskNumber, GLbitfield mask);
#endif

#ifndef GL_VERSION_3_3
#define GL_VERSION_3_3 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_DIVISOR                          = 0x88FE,
    GL_SRC1_COLOR                                           = 0x88F9,
    GL_ONE_MINUS_SRC1_COLOR                                 = 0x88FA,
    GL_ONE_MINUS_SRC1_ALPHA                                 = 0x88FB,
    GL_MAX_DUAL_SOURCE_DRAW_BUFFERS                         = 0x88FC,
    GL_ANY_SAMPLES_PASSED                                   = 0x8C2F,
    GL_SAMPLER_BINDING                                      = 0x8919,
    GL_RGB10_A2UI                                           = 0x906F,
    GL_TEXTURE_SWIZZLE_R                                    = 0x8E42,
    GL_TEXTURE_SWIZZLE_G                                    = 0x8E43,
    GL_TEXTURE_SWIZZLE_B                                    = 0x8E44,
    GL_TEXTURE_SWIZZLE_A                                    = 0x8E45,
    GL_TEXTURE_SWIZZLE_RGBA                                 = 0x8E46,
    GL_TIME_ELAPSED                                         = 0x88BF,
    GL_TIMESTAMP                                            = 0x8E28,
    GL_INT_2_10_10_10_REV                                   = 0x8D9F,
};
extern void         (KHRONOS_APIENTRY* const& glBindFragDataLocationIndexed) (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetFragDataIndex) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGenSamplers) (GLsizei count, GLuint *samplers);
extern void         (KHRONOS_APIENTRY* const& glDeleteSamplers) (GLsizei count, const GLuint *samplers);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsSampler) (GLuint sampler);
extern void         (KHRONOS_APIENTRY* const& glBindSampler) (GLuint unit, GLuint sampler);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameteri) (GLuint sampler, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameteriv) (GLuint sampler, GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterf) (GLuint sampler, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterfv) (GLuint sampler, GLenum pname, const GLfloat *param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIiv) (GLuint sampler, GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIuiv) (GLuint sampler, GLenum pname, const GLuint *param);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameteriv) (GLuint sampler, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIiv) (GLuint sampler, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterfv) (GLuint sampler, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIuiv) (GLuint sampler, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glQueryCounter) (GLuint id, GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjecti64v) (GLuint id, GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectui64v) (GLuint id, GLenum pname, GLuint64 *params);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribDivisor) (GLuint index, GLuint divisor);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP1ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP1uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP2ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP2uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP3ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP3uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP4ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribP4uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glVertexP2ui) (GLenum type, GLuint value);
extern void         (KHRONOS_APIENTRY* const& glVertexP2uiv) (GLenum type, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glVertexP3ui) (GLenum type, GLuint value);
extern void         (KHRONOS_APIENTRY* const& glVertexP3uiv) (GLenum type, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glVertexP4ui) (GLenum type, GLuint value);
extern void         (KHRONOS_APIENTRY* const& glVertexP4uiv) (GLenum type, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP1ui) (GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP1uiv) (GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP2ui) (GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP2uiv) (GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP3ui) (GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP3uiv) (GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP4ui) (GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoordP4uiv) (GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP1ui) (GLenum texture, GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP1uiv) (GLenum texture, GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP2ui) (GLenum texture, GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP2uiv) (GLenum texture, GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP3ui) (GLenum texture, GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP3uiv) (GLenum texture, GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP4ui) (GLenum texture, GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordP4uiv) (GLenum texture, GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glNormalP3ui) (GLenum type, GLuint coords);
extern void         (KHRONOS_APIENTRY* const& glNormalP3uiv) (GLenum type, const GLuint *coords);
extern void         (KHRONOS_APIENTRY* const& glColorP3ui) (GLenum type, GLuint color);
extern void         (KHRONOS_APIENTRY* const& glColorP3uiv) (GLenum type, const GLuint *color);
extern void         (KHRONOS_APIENTRY* const& glColorP4ui) (GLenum type, GLuint color);
extern void         (KHRONOS_APIENTRY* const& glColorP4uiv) (GLenum type, const GLuint *color);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColorP3ui) (GLenum type, GLuint color);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColorP3uiv) (GLenum type, const GLuint *color);
#endif

#ifndef GL_VERSION_4_0
#define GL_VERSION_4_0 1
enum : GLenum
{
    GL_SAMPLE_SHADING                                       = 0x8C36,
    GL_MIN_SAMPLE_SHADING_VALUE                             = 0x8C37,
    GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET                    = 0x8E5E,
    GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET                    = 0x8E5F,
    GL_TEXTURE_CUBE_MAP_ARRAY                               = 0x9009,
    GL_TEXTURE_BINDING_CUBE_MAP_ARRAY                       = 0x900A,
    GL_PROXY_TEXTURE_CUBE_MAP_ARRAY                         = 0x900B,
    GL_SAMPLER_CUBE_MAP_ARRAY                               = 0x900C,
    GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW                        = 0x900D,
    GL_INT_SAMPLER_CUBE_MAP_ARRAY                           = 0x900E,
    GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY                  = 0x900F,
    GL_DRAW_INDIRECT_BUFFER                                 = 0x8F3F,
    GL_DRAW_INDIRECT_BUFFER_BINDING                         = 0x8F43,
    GL_GEOMETRY_SHADER_INVOCATIONS                          = 0x887F,
    GL_MAX_GEOMETRY_SHADER_INVOCATIONS                      = 0x8E5A,
    GL_MIN_FRAGMENT_INTERPOLATION_OFFSET                    = 0x8E5B,
    GL_MAX_FRAGMENT_INTERPOLATION_OFFSET                    = 0x8E5C,
    GL_FRAGMENT_INTERPOLATION_OFFSET_BITS                   = 0x8E5D,
    GL_MAX_VERTEX_STREAMS                                   = 0x8E71,
    GL_DOUBLE_VEC2                                          = 0x8FFC,
    GL_DOUBLE_VEC3                                          = 0x8FFD,
    GL_DOUBLE_VEC4                                          = 0x8FFE,
    GL_DOUBLE_MAT2                                          = 0x8F46,
    GL_DOUBLE_MAT3                                          = 0x8F47,
    GL_DOUBLE_MAT4                                          = 0x8F48,
    GL_DOUBLE_MAT2x3                                        = 0x8F49,
    GL_DOUBLE_MAT2x4                                        = 0x8F4A,
    GL_DOUBLE_MAT3x2                                        = 0x8F4B,
    GL_DOUBLE_MAT3x4                                        = 0x8F4C,
    GL_DOUBLE_MAT4x2                                        = 0x8F4D,
    GL_DOUBLE_MAT4x3                                        = 0x8F4E,
    GL_ACTIVE_SUBROUTINES                                   = 0x8DE5,
    GL_ACTIVE_SUBROUTINE_UNIFORMS                           = 0x8DE6,
    GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS                  = 0x8E47,
    GL_ACTIVE_SUBROUTINE_MAX_LENGTH                         = 0x8E48,
    GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH                 = 0x8E49,
    GL_MAX_SUBROUTINES                                      = 0x8DE7,
    GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS                     = 0x8DE8,
    GL_NUM_COMPATIBLE_SUBROUTINES                           = 0x8E4A,
    GL_COMPATIBLE_SUBROUTINES                               = 0x8E4B,
    GL_PATCHES                                              = 0x000E,
    GL_PATCH_VERTICES                                       = 0x8E72,
    GL_PATCH_DEFAULT_INNER_LEVEL                            = 0x8E73,
    GL_PATCH_DEFAULT_OUTER_LEVEL                            = 0x8E74,
    GL_TESS_CONTROL_OUTPUT_VERTICES                         = 0x8E75,
    GL_TESS_GEN_MODE                                        = 0x8E76,
    GL_TESS_GEN_SPACING                                     = 0x8E77,
    GL_TESS_GEN_VERTEX_ORDER                                = 0x8E78,
    GL_TESS_GEN_POINT_MODE                                  = 0x8E79,
    GL_ISOLINES                                             = 0x8E7A,
    GL_FRACTIONAL_ODD                                       = 0x8E7B,
    GL_FRACTIONAL_EVEN                                      = 0x8E7C,
    GL_MAX_PATCH_VERTICES                                   = 0x8E7D,
    GL_MAX_TESS_GEN_LEVEL                                   = 0x8E7E,
    GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS                  = 0x8E7F,
    GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS               = 0x8E80,
    GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS                 = 0x8E81,
    GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS              = 0x8E82,
    GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS                   = 0x8E83,
    GL_MAX_TESS_PATCH_COMPONENTS                            = 0x8E84,
    GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS             = 0x8E85,
    GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS                = 0x8E86,
    GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS                      = 0x8E89,
    GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS                   = 0x8E8A,
    GL_MAX_TESS_CONTROL_INPUT_COMPONENTS                    = 0x886C,
    GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS                 = 0x886D,
    GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS         = 0x8E1E,
    GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS      = 0x8E1F,
    GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER      = 0x84F0,
    GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER   = 0x84F1,
    GL_TESS_EVALUATION_SHADER                               = 0x8E87,
    GL_TESS_CONTROL_SHADER                                  = 0x8E88,
    GL_TRANSFORM_FEEDBACK                                   = 0x8E22,
    GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED                     = 0x8E23,
    GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE                     = 0x8E24,
    GL_TRANSFORM_FEEDBACK_BINDING                           = 0x8E25,
    GL_MAX_TRANSFORM_FEEDBACK_BUFFERS                       = 0x8E70,
};
extern void         (KHRONOS_APIENTRY* const& glMinSampleShading) (GLfloat value);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationi) (GLuint buf, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparatei) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
extern void         (KHRONOS_APIENTRY* const& glBlendFunci) (GLuint buf, GLenum src, GLenum dst);
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparatei) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern void         (KHRONOS_APIENTRY* const& glDrawArraysIndirect) (GLenum mode, const void *indirect);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect);
extern void         (KHRONOS_APIENTRY* const& glUniform1d) (GLint location, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glUniform2d) (GLint location, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glUniform3d) (GLint location, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glUniform4d) (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glUniform1dv) (GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2dv) (GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3dv) (GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4dv) (GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x3dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x4dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x2dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x4dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x2dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x3dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glGetUniformdv) (GLuint program, GLint location, GLdouble *params);
extern GLint        (KHRONOS_APIENTRY* const& glGetSubroutineUniformLocation) (GLuint program, GLenum shadertype, const GLchar *name);
extern GLuint       (KHRONOS_APIENTRY* const& glGetSubroutineIndex) (GLuint program, GLenum shadertype, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetActiveSubroutineUniformiv) (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
extern void         (KHRONOS_APIENTRY* const& glGetActiveSubroutineUniformName) (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetActiveSubroutineName) (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glUniformSubroutinesuiv) (GLenum shadertype, GLsizei count, const GLuint *indices);
extern void         (KHRONOS_APIENTRY* const& glGetUniformSubroutineuiv) (GLenum shadertype, GLint location, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramStageiv) (GLuint program, GLenum shadertype, GLenum pname, GLint *values);
extern void         (KHRONOS_APIENTRY* const& glPatchParameteri) (GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glPatchParameterfv) (GLenum pname, const GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glBindTransformFeedback) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDeleteTransformFeedbacks) (GLsizei n, const GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glGenTransformFeedbacks) (GLsizei n, GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTransformFeedback) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glPauseTransformFeedback) ();
extern void         (KHRONOS_APIENTRY* const& glResumeTransformFeedback) ();
extern void         (KHRONOS_APIENTRY* const& glDrawTransformFeedback) (GLenum mode, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDrawTransformFeedbackStream) (GLenum mode, GLuint id, GLuint stream);
extern void         (KHRONOS_APIENTRY* const& glBeginQueryIndexed) (GLenum target, GLuint index, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glEndQueryIndexed) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glGetQueryIndexediv) (GLenum target, GLuint index, GLenum pname, GLint *params);
#endif

#ifndef GL_VERSION_4_1
#define GL_VERSION_4_1 1
enum : GLenum
{
    GL_FIXED                                                = 0x140C,
    GL_IMPLEMENTATION_COLOR_READ_TYPE                       = 0x8B9A,
    GL_IMPLEMENTATION_COLOR_READ_FORMAT                     = 0x8B9B,
    GL_LOW_FLOAT                                            = 0x8DF0,
    GL_MEDIUM_FLOAT                                         = 0x8DF1,
    GL_HIGH_FLOAT                                           = 0x8DF2,
    GL_LOW_INT                                              = 0x8DF3,
    GL_MEDIUM_INT                                           = 0x8DF4,
    GL_HIGH_INT                                             = 0x8DF5,
    GL_SHADER_COMPILER                                      = 0x8DFA,
    GL_SHADER_BINARY_FORMATS                                = 0x8DF8,
    GL_NUM_SHADER_BINARY_FORMATS                            = 0x8DF9,
    GL_MAX_VERTEX_UNIFORM_VECTORS                           = 0x8DFB,
    GL_MAX_VARYING_VECTORS                                  = 0x8DFC,
    GL_MAX_FRAGMENT_UNIFORM_VECTORS                         = 0x8DFD,
    GL_RGB565                                               = 0x8D62,
    GL_PROGRAM_BINARY_RETRIEVABLE_HINT                      = 0x8257,
    GL_PROGRAM_BINARY_LENGTH                                = 0x8741,
    GL_NUM_PROGRAM_BINARY_FORMATS                           = 0x87FE,
    GL_PROGRAM_BINARY_FORMATS                               = 0x87FF,
    GL_VERTEX_SHADER_BIT                                    = 0x00000001,
    GL_FRAGMENT_SHADER_BIT                                  = 0x00000002,
    GL_GEOMETRY_SHADER_BIT                                  = 0x00000004,
    GL_TESS_CONTROL_SHADER_BIT                              = 0x00000008,
    GL_TESS_EVALUATION_SHADER_BIT                           = 0x00000010,
    GL_ALL_SHADER_BITS                                      = 0xFFFFFFFF,
    GL_PROGRAM_SEPARABLE                                    = 0x8258,
    GL_ACTIVE_PROGRAM                                       = 0x8259,
    GL_PROGRAM_PIPELINE_BINDING                             = 0x825A,
    GL_MAX_VIEWPORTS                                        = 0x825B,
    GL_VIEWPORT_SUBPIXEL_BITS                               = 0x825C,
    GL_VIEWPORT_BOUNDS_RANGE                                = 0x825D,
    GL_LAYER_PROVOKING_VERTEX                               = 0x825E,
    GL_VIEWPORT_INDEX_PROVOKING_VERTEX                      = 0x825F,
    GL_UNDEFINED_VERTEX                                     = 0x8260,
};
extern void         (KHRONOS_APIENTRY* const& glReleaseShaderCompiler) ();
extern void         (KHRONOS_APIENTRY* const& glShaderBinary) (GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length);
extern void         (KHRONOS_APIENTRY* const& glGetShaderPrecisionFormat) (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
extern void         (KHRONOS_APIENTRY* const& glDepthRangef) (GLfloat n, GLfloat f);
extern void         (KHRONOS_APIENTRY* const& glClearDepthf) (GLfloat d);
extern void         (KHRONOS_APIENTRY* const& glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
extern void         (KHRONOS_APIENTRY* const& glProgramBinary) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
extern void         (KHRONOS_APIENTRY* const& glProgramParameteri) (GLuint program, GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glUseProgramStages) (GLuint pipeline, GLbitfield stages, GLuint program);
extern void         (KHRONOS_APIENTRY* const& glActiveShaderProgram) (GLuint pipeline, GLuint program);
extern GLuint       (KHRONOS_APIENTRY* const& glCreateShaderProgramv) (GLenum type, GLsizei count, const GLchar *const*strings);
extern void         (KHRONOS_APIENTRY* const& glBindProgramPipeline) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glDeleteProgramPipelines) (GLsizei n, const GLuint *pipelines);
extern void         (KHRONOS_APIENTRY* const& glGenProgramPipelines) (GLsizei n, GLuint *pipelines);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsProgramPipeline) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glGetProgramPipelineiv) (GLuint pipeline, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1i) (GLuint program, GLint location, GLint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1f) (GLuint program, GLint location, GLfloat v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1d) (GLuint program, GLint location, GLdouble v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ui) (GLuint program, GLint location, GLuint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2i) (GLuint program, GLint location, GLint v0, GLint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2f) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2d) (GLuint program, GLint location, GLdouble v0, GLdouble v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ui) (GLuint program, GLint location, GLuint v0, GLuint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3d) (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4d) (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x3dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x2dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x4dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x2dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x4dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x3dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glValidateProgramPipeline) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glGetProgramPipelineInfoLog) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1d) (GLuint index, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2d) (GLuint index, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3d) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4d) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4dv) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribLPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribLdv) (GLuint index, GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glViewportArrayv) (GLuint first, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glViewportIndexedf) (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
extern void         (KHRONOS_APIENTRY* const& glViewportIndexedfv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glScissorArrayv) (GLuint first, GLsizei count, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glScissorIndexed) (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glScissorIndexedv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glDepthRangeArrayv) (GLuint first, GLsizei count, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glDepthRangeIndexed) (GLuint index, GLdouble n, GLdouble f);
extern void         (KHRONOS_APIENTRY* const& glGetFloati_v) (GLenum target, GLuint index, GLfloat *data);
extern void         (KHRONOS_APIENTRY* const& glGetDoublei_v) (GLenum target, GLuint index, GLdouble *data);
#endif

#ifndef GL_VERSION_4_2
#define GL_VERSION_4_2 1
enum : GLenum
{
    GL_COPY_READ_BUFFER_BINDING                             = 0x8F36,
    GL_COPY_WRITE_BUFFER_BINDING                            = 0x8F37,
    GL_TRANSFORM_FEEDBACK_ACTIVE                            = 0x8E24,
    GL_TRANSFORM_FEEDBACK_PAUSED                            = 0x8E23,
    GL_UNPACK_COMPRESSED_BLOCK_WIDTH                        = 0x9127,
    GL_UNPACK_COMPRESSED_BLOCK_HEIGHT                       = 0x9128,
    GL_UNPACK_COMPRESSED_BLOCK_DEPTH                        = 0x9129,
    GL_UNPACK_COMPRESSED_BLOCK_SIZE                         = 0x912A,
    GL_PACK_COMPRESSED_BLOCK_WIDTH                          = 0x912B,
    GL_PACK_COMPRESSED_BLOCK_HEIGHT                         = 0x912C,
    GL_PACK_COMPRESSED_BLOCK_DEPTH                          = 0x912D,
    GL_PACK_COMPRESSED_BLOCK_SIZE                           = 0x912E,
    GL_NUM_SAMPLE_COUNTS                                    = 0x9380,
    GL_MIN_MAP_BUFFER_ALIGNMENT                             = 0x90BC,
    GL_ATOMIC_COUNTER_BUFFER                                = 0x92C0,
    GL_ATOMIC_COUNTER_BUFFER_BINDING                        = 0x92C1,
    GL_ATOMIC_COUNTER_BUFFER_START                          = 0x92C2,
    GL_ATOMIC_COUNTER_BUFFER_SIZE                           = 0x92C3,
    GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE                      = 0x92C4,
    GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS         = 0x92C5,
    GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES  = 0x92C6,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER    = 0x92C7,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER = 0x92C8,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER = 0x92C9,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER  = 0x92CA,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER  = 0x92CB,
    GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS                    = 0x92CC,
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS              = 0x92CD,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS           = 0x92CE,
    GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS                  = 0x92CF,
    GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS                  = 0x92D0,
    GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS                  = 0x92D1,
    GL_MAX_VERTEX_ATOMIC_COUNTERS                           = 0x92D2,
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS                     = 0x92D3,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS                  = 0x92D4,
    GL_MAX_GEOMETRY_ATOMIC_COUNTERS                         = 0x92D5,
    GL_MAX_FRAGMENT_ATOMIC_COUNTERS                         = 0x92D6,
    GL_MAX_COMBINED_ATOMIC_COUNTERS                         = 0x92D7,
    GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE                       = 0x92D8,
    GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS                   = 0x92DC,
    GL_ACTIVE_ATOMIC_COUNTER_BUFFERS                        = 0x92D9,
    GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX                  = 0x92DA,
    GL_UNSIGNED_INT_ATOMIC_COUNTER                          = 0x92DB,
    GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT                      = 0x00000001,
    GL_ELEMENT_ARRAY_BARRIER_BIT                            = 0x00000002,
    GL_UNIFORM_BARRIER_BIT                                  = 0x00000004,
    GL_TEXTURE_FETCH_BARRIER_BIT                            = 0x00000008,
    GL_SHADER_IMAGE_ACCESS_BARRIER_BIT                      = 0x00000020,
    GL_COMMAND_BARRIER_BIT                                  = 0x00000040,
    GL_PIXEL_BUFFER_BARRIER_BIT                             = 0x00000080,
    GL_TEXTURE_UPDATE_BARRIER_BIT                           = 0x00000100,
    GL_BUFFER_UPDATE_BARRIER_BIT                            = 0x00000200,
    GL_FRAMEBUFFER_BARRIER_BIT                              = 0x00000400,
    GL_TRANSFORM_FEEDBACK_BARRIER_BIT                       = 0x00000800,
    GL_ATOMIC_COUNTER_BARRIER_BIT                           = 0x00001000,
    GL_ALL_BARRIER_BITS                                     = 0xFFFFFFFF,
    GL_MAX_IMAGE_UNITS                                      = 0x8F38,
    GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS        = 0x8F39,
    GL_IMAGE_BINDING_NAME                                   = 0x8F3A,
    GL_IMAGE_BINDING_LEVEL                                  = 0x8F3B,
    GL_IMAGE_BINDING_LAYERED                                = 0x8F3C,
    GL_IMAGE_BINDING_LAYER                                  = 0x8F3D,
    GL_IMAGE_BINDING_ACCESS                                 = 0x8F3E,
    GL_IMAGE_1D                                             = 0x904C,
    GL_IMAGE_2D                                             = 0x904D,
    GL_IMAGE_3D                                             = 0x904E,
    GL_IMAGE_2D_RECT                                        = 0x904F,
    GL_IMAGE_CUBE                                           = 0x9050,
    GL_IMAGE_BUFFER                                         = 0x9051,
    GL_IMAGE_1D_ARRAY                                       = 0x9052,
    GL_IMAGE_2D_ARRAY                                       = 0x9053,
    GL_IMAGE_CUBE_MAP_ARRAY                                 = 0x9054,
    GL_IMAGE_2D_MULTISAMPLE                                 = 0x9055,
    GL_IMAGE_2D_MULTISAMPLE_ARRAY                           = 0x9056,
    GL_INT_IMAGE_1D                                         = 0x9057,
    GL_INT_IMAGE_2D                                         = 0x9058,
    GL_INT_IMAGE_3D                                         = 0x9059,
    GL_INT_IMAGE_2D_RECT                                    = 0x905A,
    GL_INT_IMAGE_CUBE                                       = 0x905B,
    GL_INT_IMAGE_BUFFER                                     = 0x905C,
    GL_INT_IMAGE_1D_ARRAY                                   = 0x905D,
    GL_INT_IMAGE_2D_ARRAY                                   = 0x905E,
    GL_INT_IMAGE_CUBE_MAP_ARRAY                             = 0x905F,
    GL_INT_IMAGE_2D_MULTISAMPLE                             = 0x9060,
    GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY                       = 0x9061,
    GL_UNSIGNED_INT_IMAGE_1D                                = 0x9062,
    GL_UNSIGNED_INT_IMAGE_2D                                = 0x9063,
    GL_UNSIGNED_INT_IMAGE_3D                                = 0x9064,
    GL_UNSIGNED_INT_IMAGE_2D_RECT                           = 0x9065,
    GL_UNSIGNED_INT_IMAGE_CUBE                              = 0x9066,
    GL_UNSIGNED_INT_IMAGE_BUFFER                            = 0x9067,
    GL_UNSIGNED_INT_IMAGE_1D_ARRAY                          = 0x9068,
    GL_UNSIGNED_INT_IMAGE_2D_ARRAY                          = 0x9069,
    GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY                    = 0x906A,
    GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE                    = 0x906B,
    GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY              = 0x906C,
    GL_MAX_IMAGE_SAMPLES                                    = 0x906D,
    GL_IMAGE_BINDING_FORMAT                                 = 0x906E,
    GL_IMAGE_FORMAT_COMPATIBILITY_TYPE                      = 0x90C7,
    GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE                   = 0x90C8,
    GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS                  = 0x90C9,
    GL_MAX_VERTEX_IMAGE_UNIFORMS                            = 0x90CA,
    GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS                      = 0x90CB,
    GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS                   = 0x90CC,
    GL_MAX_GEOMETRY_IMAGE_UNIFORMS                          = 0x90CD,
    GL_MAX_FRAGMENT_IMAGE_UNIFORMS                          = 0x90CE,
    GL_MAX_COMBINED_IMAGE_UNIFORMS                          = 0x90CF,
    GL_COMPRESSED_RGBA_BPTC_UNORM                           = 0x8E8C,
    GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM                     = 0x8E8D,
    GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT                     = 0x8E8E,
    GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT                   = 0x8E8F,
    GL_TEXTURE_IMMUTABLE_FORMAT                             = 0x912F,
};
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstancedBaseInstance) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseInstance) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseVertexBaseInstance) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
extern void         (KHRONOS_APIENTRY* const& glGetInternalformativ) (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetActiveAtomicCounterBufferiv) (GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glBindImageTexture) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
extern void         (KHRONOS_APIENTRY* const& glMemoryBarrier) (GLbitfield barriers);
extern void         (KHRONOS_APIENTRY* const& glTexStorage1D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glTexStorage2D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTexStorage3D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern void         (KHRONOS_APIENTRY* const& glDrawTransformFeedbackInstanced) (GLenum mode, GLuint id, GLsizei instancecount);
extern void         (KHRONOS_APIENTRY* const& glDrawTransformFeedbackStreamInstanced) (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
#endif

#ifndef GL_VERSION_4_3
#define GL_VERSION_4_3 1
enum : GLenum
{
    GL_NUM_SHADING_LANGUAGE_VERSIONS                        = 0x82E9,
    GL_VERTEX_ATTRIB_ARRAY_LONG                             = 0x874E,
    GL_COMPRESSED_RGB8_ETC2                                 = 0x9274,
    GL_COMPRESSED_SRGB8_ETC2                                = 0x9275,
    GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2             = 0x9276,
    GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2            = 0x9277,
    GL_COMPRESSED_RGBA8_ETC2_EAC                            = 0x9278,
    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC                     = 0x9279,
    GL_COMPRESSED_R11_EAC                                   = 0x9270,
    GL_COMPRESSED_SIGNED_R11_EAC                            = 0x9271,
    GL_COMPRESSED_RG11_EAC                                  = 0x9272,
    GL_COMPRESSED_SIGNED_RG11_EAC                           = 0x9273,
    GL_PRIMITIVE_RESTART_FIXED_INDEX                        = 0x8D69,
    GL_ANY_SAMPLES_PASSED_CONSERVATIVE                      = 0x8D6A,
    GL_MAX_ELEMENT_INDEX                                    = 0x8D6B,
    GL_COMPUTE_SHADER                                       = 0x91B9,
    GL_MAX_COMPUTE_UNIFORM_BLOCKS                           = 0x91BB,
    GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS                      = 0x91BC,
    GL_MAX_COMPUTE_IMAGE_UNIFORMS                           = 0x91BD,
    GL_MAX_COMPUTE_SHARED_MEMORY_SIZE                       = 0x8262,
    GL_MAX_COMPUTE_UNIFORM_COMPONENTS                       = 0x8263,
    GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS                   = 0x8264,
    GL_MAX_COMPUTE_ATOMIC_COUNTERS                          = 0x8265,
    GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS              = 0x8266,
    GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS                   = 0x90EB,
    GL_MAX_COMPUTE_WORK_GROUP_COUNT                         = 0x91BE,
    GL_MAX_COMPUTE_WORK_GROUP_SIZE                          = 0x91BF,
    GL_COMPUTE_WORK_GROUP_SIZE                              = 0x8267,
    GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER           = 0x90EC,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER   = 0x90ED,
    GL_DISPATCH_INDIRECT_BUFFER                             = 0x90EE,
    GL_DISPATCH_INDIRECT_BUFFER_BINDING                     = 0x90EF,
    GL_COMPUTE_SHADER_BIT                                   = 0x00000020,
    GL_DEBUG_OUTPUT_SYNCHRONOUS                             = 0x8242,
    GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH                     = 0x8243,
    GL_DEBUG_CALLBACK_FUNCTION                              = 0x8244,
    GL_DEBUG_CALLBACK_USER_PARAM                            = 0x8245,
    GL_DEBUG_SOURCE_API                                     = 0x8246,
    GL_DEBUG_SOURCE_WINDOW_SYSTEM                           = 0x8247,
    GL_DEBUG_SOURCE_SHADER_COMPILER                         = 0x8248,
    GL_DEBUG_SOURCE_THIRD_PARTY                             = 0x8249,
    GL_DEBUG_SOURCE_APPLICATION                             = 0x824A,
    GL_DEBUG_SOURCE_OTHER                                   = 0x824B,
    GL_DEBUG_TYPE_ERROR                                     = 0x824C,
    GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR                       = 0x824D,
    GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR                        = 0x824E,
    GL_DEBUG_TYPE_PORTABILITY                               = 0x824F,
    GL_DEBUG_TYPE_PERFORMANCE                               = 0x8250,
    GL_DEBUG_TYPE_OTHER                                     = 0x8251,
    GL_MAX_DEBUG_MESSAGE_LENGTH                             = 0x9143,
    GL_MAX_DEBUG_LOGGED_MESSAGES                            = 0x9144,
    GL_DEBUG_LOGGED_MESSAGES                                = 0x9145,
    GL_DEBUG_SEVERITY_HIGH                                  = 0x9146,
    GL_DEBUG_SEVERITY_MEDIUM                                = 0x9147,
    GL_DEBUG_SEVERITY_LOW                                   = 0x9148,
    GL_DEBUG_TYPE_MARKER                                    = 0x8268,
    GL_DEBUG_TYPE_PUSH_GROUP                                = 0x8269,
    GL_DEBUG_TYPE_POP_GROUP                                 = 0x826A,
    GL_DEBUG_SEVERITY_NOTIFICATION                          = 0x826B,
    GL_MAX_DEBUG_GROUP_STACK_DEPTH                          = 0x826C,
    GL_DEBUG_GROUP_STACK_DEPTH                              = 0x826D,
    GL_BUFFER                                               = 0x82E0,
    GL_SHADER                                               = 0x82E1,
    GL_PROGRAM                                              = 0x82E2,
    GL_QUERY                                                = 0x82E3,
    GL_PROGRAM_PIPELINE                                     = 0x82E4,
    GL_SAMPLER                                              = 0x82E6,
    GL_MAX_LABEL_LENGTH                                     = 0x82E8,
    GL_DEBUG_OUTPUT                                         = 0x92E0,
    GL_CONTEXT_FLAG_DEBUG_BIT                               = 0x00000002,
    GL_MAX_UNIFORM_LOCATIONS                                = 0x826E,
    GL_FRAMEBUFFER_DEFAULT_WIDTH                            = 0x9310,
    GL_FRAMEBUFFER_DEFAULT_HEIGHT                           = 0x9311,
    GL_FRAMEBUFFER_DEFAULT_LAYERS                           = 0x9312,
    GL_FRAMEBUFFER_DEFAULT_SAMPLES                          = 0x9313,
    GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS           = 0x9314,
    GL_MAX_FRAMEBUFFER_WIDTH                                = 0x9315,
    GL_MAX_FRAMEBUFFER_HEIGHT                               = 0x9316,
    GL_MAX_FRAMEBUFFER_LAYERS                               = 0x9317,
    GL_MAX_FRAMEBUFFER_SAMPLES                              = 0x9318,
    GL_INTERNALFORMAT_SUPPORTED                             = 0x826F,
    GL_INTERNALFORMAT_PREFERRED                             = 0x8270,
    GL_INTERNALFORMAT_RED_SIZE                              = 0x8271,
    GL_INTERNALFORMAT_GREEN_SIZE                            = 0x8272,
    GL_INTERNALFORMAT_BLUE_SIZE                             = 0x8273,
    GL_INTERNALFORMAT_ALPHA_SIZE                            = 0x8274,
    GL_INTERNALFORMAT_DEPTH_SIZE                            = 0x8275,
    GL_INTERNALFORMAT_STENCIL_SIZE                          = 0x8276,
    GL_INTERNALFORMAT_SHARED_SIZE                           = 0x8277,
    GL_INTERNALFORMAT_RED_TYPE                              = 0x8278,
    GL_INTERNALFORMAT_GREEN_TYPE                            = 0x8279,
    GL_INTERNALFORMAT_BLUE_TYPE                             = 0x827A,
    GL_INTERNALFORMAT_ALPHA_TYPE                            = 0x827B,
    GL_INTERNALFORMAT_DEPTH_TYPE                            = 0x827C,
    GL_INTERNALFORMAT_STENCIL_TYPE                          = 0x827D,
    GL_MAX_WIDTH                                            = 0x827E,
    GL_MAX_HEIGHT                                           = 0x827F,
    GL_MAX_DEPTH                                            = 0x8280,
    GL_MAX_LAYERS                                           = 0x8281,
    GL_MAX_COMBINED_DIMENSIONS                              = 0x8282,
    GL_COLOR_COMPONENTS                                     = 0x8283,
    GL_DEPTH_COMPONENTS                                     = 0x8284,
    GL_STENCIL_COMPONENTS                                   = 0x8285,
    GL_COLOR_RENDERABLE                                     = 0x8286,
    GL_DEPTH_RENDERABLE                                     = 0x8287,
    GL_STENCIL_RENDERABLE                                   = 0x8288,
    GL_FRAMEBUFFER_RENDERABLE                               = 0x8289,
    GL_FRAMEBUFFER_RENDERABLE_LAYERED                       = 0x828A,
    GL_FRAMEBUFFER_BLEND                                    = 0x828B,
    GL_READ_PIXELS                                          = 0x828C,
    GL_READ_PIXELS_FORMAT                                   = 0x828D,
    GL_READ_PIXELS_TYPE                                     = 0x828E,
    GL_TEXTURE_IMAGE_FORMAT                                 = 0x828F,
    GL_TEXTURE_IMAGE_TYPE                                   = 0x8290,
    GL_GET_TEXTURE_IMAGE_FORMAT                             = 0x8291,
    GL_GET_TEXTURE_IMAGE_TYPE                               = 0x8292,
    GL_MIPMAP                                               = 0x8293,
    GL_MANUAL_GENERATE_MIPMAP                               = 0x8294,
    GL_AUTO_GENERATE_MIPMAP                                 = 0x8295,
    GL_COLOR_ENCODING                                       = 0x8296,
    GL_SRGB_READ                                            = 0x8297,
    GL_SRGB_WRITE                                           = 0x8298,
    GL_FILTER                                               = 0x829A,
    GL_VERTEX_TEXTURE                                       = 0x829B,
    GL_TESS_CONTROL_TEXTURE                                 = 0x829C,
    GL_TESS_EVALUATION_TEXTURE                              = 0x829D,
    GL_GEOMETRY_TEXTURE                                     = 0x829E,
    GL_FRAGMENT_TEXTURE                                     = 0x829F,
    GL_COMPUTE_TEXTURE                                      = 0x82A0,
    GL_TEXTURE_SHADOW                                       = 0x82A1,
    GL_TEXTURE_GATHER                                       = 0x82A2,
    GL_TEXTURE_GATHER_SHADOW                                = 0x82A3,
    GL_SHADER_IMAGE_LOAD                                    = 0x82A4,
    GL_SHADER_IMAGE_STORE                                   = 0x82A5,
    GL_SHADER_IMAGE_ATOMIC                                  = 0x82A6,
    GL_IMAGE_TEXEL_SIZE                                     = 0x82A7,
    GL_IMAGE_COMPATIBILITY_CLASS                            = 0x82A8,
    GL_IMAGE_PIXEL_FORMAT                                   = 0x82A9,
    GL_IMAGE_PIXEL_TYPE                                     = 0x82AA,
    GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST                  = 0x82AC,
    GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST                = 0x82AD,
    GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE                 = 0x82AE,
    GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE               = 0x82AF,
    GL_TEXTURE_COMPRESSED_BLOCK_WIDTH                       = 0x82B1,
    GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT                      = 0x82B2,
    GL_TEXTURE_COMPRESSED_BLOCK_SIZE                        = 0x82B3,
    GL_CLEAR_BUFFER                                         = 0x82B4,
    GL_TEXTURE_VIEW                                         = 0x82B5,
    GL_VIEW_COMPATIBILITY_CLASS                             = 0x82B6,
    GL_FULL_SUPPORT                                         = 0x82B7,
    GL_CAVEAT_SUPPORT                                       = 0x82B8,
    GL_IMAGE_CLASS_4_X_32                                   = 0x82B9,
    GL_IMAGE_CLASS_2_X_32                                   = 0x82BA,
    GL_IMAGE_CLASS_1_X_32                                   = 0x82BB,
    GL_IMAGE_CLASS_4_X_16                                   = 0x82BC,
    GL_IMAGE_CLASS_2_X_16                                   = 0x82BD,
    GL_IMAGE_CLASS_1_X_16                                   = 0x82BE,
    GL_IMAGE_CLASS_4_X_8                                    = 0x82BF,
    GL_IMAGE_CLASS_2_X_8                                    = 0x82C0,
    GL_IMAGE_CLASS_1_X_8                                    = 0x82C1,
    GL_IMAGE_CLASS_11_11_10                                 = 0x82C2,
    GL_IMAGE_CLASS_10_10_10_2                               = 0x82C3,
    GL_VIEW_CLASS_128_BITS                                  = 0x82C4,
    GL_VIEW_CLASS_96_BITS                                   = 0x82C5,
    GL_VIEW_CLASS_64_BITS                                   = 0x82C6,
    GL_VIEW_CLASS_48_BITS                                   = 0x82C7,
    GL_VIEW_CLASS_32_BITS                                   = 0x82C8,
    GL_VIEW_CLASS_24_BITS                                   = 0x82C9,
    GL_VIEW_CLASS_16_BITS                                   = 0x82CA,
    GL_VIEW_CLASS_8_BITS                                    = 0x82CB,
    GL_VIEW_CLASS_S3TC_DXT1_RGB                             = 0x82CC,
    GL_VIEW_CLASS_S3TC_DXT1_RGBA                            = 0x82CD,
    GL_VIEW_CLASS_S3TC_DXT3_RGBA                            = 0x82CE,
    GL_VIEW_CLASS_S3TC_DXT5_RGBA                            = 0x82CF,
    GL_VIEW_CLASS_RGTC1_RED                                 = 0x82D0,
    GL_VIEW_CLASS_RGTC2_RG                                  = 0x82D1,
    GL_VIEW_CLASS_BPTC_UNORM                                = 0x82D2,
    GL_VIEW_CLASS_BPTC_FLOAT                                = 0x82D3,
    GL_UNIFORM                                              = 0x92E1,
    GL_UNIFORM_BLOCK                                        = 0x92E2,
    GL_PROGRAM_INPUT                                        = 0x92E3,
    GL_PROGRAM_OUTPUT                                       = 0x92E4,
    GL_BUFFER_VARIABLE                                      = 0x92E5,
    GL_SHADER_STORAGE_BLOCK                                 = 0x92E6,
    GL_VERTEX_SUBROUTINE                                    = 0x92E8,
    GL_TESS_CONTROL_SUBROUTINE                              = 0x92E9,
    GL_TESS_EVALUATION_SUBROUTINE                           = 0x92EA,
    GL_GEOMETRY_SUBROUTINE                                  = 0x92EB,
    GL_FRAGMENT_SUBROUTINE                                  = 0x92EC,
    GL_COMPUTE_SUBROUTINE                                   = 0x92ED,
    GL_VERTEX_SUBROUTINE_UNIFORM                            = 0x92EE,
    GL_TESS_CONTROL_SUBROUTINE_UNIFORM                      = 0x92EF,
    GL_TESS_EVALUATION_SUBROUTINE_UNIFORM                   = 0x92F0,
    GL_GEOMETRY_SUBROUTINE_UNIFORM                          = 0x92F1,
    GL_FRAGMENT_SUBROUTINE_UNIFORM                          = 0x92F2,
    GL_COMPUTE_SUBROUTINE_UNIFORM                           = 0x92F3,
    GL_TRANSFORM_FEEDBACK_VARYING                           = 0x92F4,
    GL_ACTIVE_RESOURCES                                     = 0x92F5,
    GL_MAX_NAME_LENGTH                                      = 0x92F6,
    GL_MAX_NUM_ACTIVE_VARIABLES                             = 0x92F7,
    GL_MAX_NUM_COMPATIBLE_SUBROUTINES                       = 0x92F8,
    GL_NAME_LENGTH                                          = 0x92F9,
    GL_TYPE                                                 = 0x92FA,
    GL_ARRAY_SIZE                                           = 0x92FB,
    GL_OFFSET                                               = 0x92FC,
    GL_BLOCK_INDEX                                          = 0x92FD,
    GL_ARRAY_STRIDE                                         = 0x92FE,
    GL_MATRIX_STRIDE                                        = 0x92FF,
    GL_IS_ROW_MAJOR                                         = 0x9300,
    GL_ATOMIC_COUNTER_BUFFER_INDEX                          = 0x9301,
    GL_BUFFER_BINDING                                       = 0x9302,
    GL_BUFFER_DATA_SIZE                                     = 0x9303,
    GL_NUM_ACTIVE_VARIABLES                                 = 0x9304,
    GL_ACTIVE_VARIABLES                                     = 0x9305,
    GL_REFERENCED_BY_VERTEX_SHADER                          = 0x9306,
    GL_REFERENCED_BY_TESS_CONTROL_SHADER                    = 0x9307,
    GL_REFERENCED_BY_TESS_EVALUATION_SHADER                 = 0x9308,
    GL_REFERENCED_BY_GEOMETRY_SHADER                        = 0x9309,
    GL_REFERENCED_BY_FRAGMENT_SHADER                        = 0x930A,
    GL_REFERENCED_BY_COMPUTE_SHADER                         = 0x930B,
    GL_TOP_LEVEL_ARRAY_SIZE                                 = 0x930C,
    GL_TOP_LEVEL_ARRAY_STRIDE                               = 0x930D,
    GL_LOCATION                                             = 0x930E,
    GL_LOCATION_INDEX                                       = 0x930F,
    GL_IS_PER_PATCH                                         = 0x92E7,
    GL_SHADER_STORAGE_BUFFER                                = 0x90D2,
    GL_SHADER_STORAGE_BUFFER_BINDING                        = 0x90D3,
    GL_SHADER_STORAGE_BUFFER_START                          = 0x90D4,
    GL_SHADER_STORAGE_BUFFER_SIZE                           = 0x90D5,
    GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS                     = 0x90D6,
    GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS                   = 0x90D7,
    GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS               = 0x90D8,
    GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS            = 0x90D9,
    GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS                   = 0x90DA,
    GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS                    = 0x90DB,
    GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS                   = 0x90DC,
    GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS                   = 0x90DD,
    GL_MAX_SHADER_STORAGE_BLOCK_SIZE                        = 0x90DE,
    GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT               = 0x90DF,
    GL_SHADER_STORAGE_BARRIER_BIT                           = 0x00002000,
    GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES                 = 0x8F39,
    GL_DEPTH_STENCIL_TEXTURE_MODE                           = 0x90EA,
    GL_TEXTURE_BUFFER_OFFSET                                = 0x919D,
    GL_TEXTURE_BUFFER_SIZE                                  = 0x919E,
    GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT                      = 0x919F,
    GL_TEXTURE_VIEW_MIN_LEVEL                               = 0x82DB,
    GL_TEXTURE_VIEW_NUM_LEVELS                              = 0x82DC,
    GL_TEXTURE_VIEW_MIN_LAYER                               = 0x82DD,
    GL_TEXTURE_VIEW_NUM_LAYERS                              = 0x82DE,
    GL_TEXTURE_IMMUTABLE_LEVELS                             = 0x82DF,
    GL_VERTEX_ATTRIB_BINDING                                = 0x82D4,
    GL_VERTEX_ATTRIB_RELATIVE_OFFSET                        = 0x82D5,
    GL_VERTEX_BINDING_DIVISOR                               = 0x82D6,
    GL_VERTEX_BINDING_OFFSET                                = 0x82D7,
    GL_VERTEX_BINDING_STRIDE                                = 0x82D8,
    GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET                    = 0x82D9,
    GL_MAX_VERTEX_ATTRIB_BINDINGS                           = 0x82DA,
    GL_VERTEX_BINDING_BUFFER                                = 0x8F4F,
    GL_DISPLAY_LIST                                         = 0x82E7,
};
extern void         (KHRONOS_APIENTRY* const& glClearBufferData) (GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glClearBufferSubData) (GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glDispatchCompute) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
extern void         (KHRONOS_APIENTRY* const& glDispatchComputeIndirect) (GLintptr indirect);
extern void         (KHRONOS_APIENTRY* const& glCopyImageSubData) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
extern void         (KHRONOS_APIENTRY* const& glFramebufferParameteri) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetInternalformati64v) (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glInvalidateTexSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
extern void         (KHRONOS_APIENTRY* const& glInvalidateTexImage) (GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glInvalidateBufferSubData) (GLuint buffer, GLintptr offset, GLsizeiptr length);
extern void         (KHRONOS_APIENTRY* const& glInvalidateBufferData) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glInvalidateFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
extern void         (KHRONOS_APIENTRY* const& glInvalidateSubFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysIndirect) (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glGetProgramInterfaceiv) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
extern GLuint       (KHRONOS_APIENTRY* const& glGetProgramResourceIndex) (GLuint program, GLenum programInterface, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetProgramResourceName) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetProgramResourceiv) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params);
extern GLint        (KHRONOS_APIENTRY* const& glGetProgramResourceLocation) (GLuint program, GLenum programInterface, const GLchar *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetProgramResourceLocationIndex) (GLuint program, GLenum programInterface, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glShaderStorageBlockBinding) (GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
extern void         (KHRONOS_APIENTRY* const& glTexBufferRange) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glTexStorage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glTexStorage3DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glTextureView) (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
extern void         (KHRONOS_APIENTRY* const& glBindVertexBuffer) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribFormat) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribIFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribLFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribBinding) (GLuint attribindex, GLuint bindingindex);
extern void         (KHRONOS_APIENTRY* const& glVertexBindingDivisor) (GLuint bindingindex, GLuint divisor);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageControl) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageInsert) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageCallback) (GLDEBUGPROC callback, const void *userParam);
extern GLuint       (KHRONOS_APIENTRY* const& glGetDebugMessageLog) (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
extern void         (KHRONOS_APIENTRY* const& glPushDebugGroup) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
extern void         (KHRONOS_APIENTRY* const& glPopDebugGroup) ();
extern void         (KHRONOS_APIENTRY* const& glObjectLabel) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glGetObjectLabel) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glObjectPtrLabel) (const void *ptr, GLsizei length, const GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glGetObjectPtrLabel) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
#endif

#ifndef GL_VERSION_4_4
#define GL_VERSION_4_4 1
enum : GLenum
{
    GL_MAX_VERTEX_ATTRIB_STRIDE                             = 0x82E5,
    GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED              = 0x8221,
    GL_TEXTURE_BUFFER_BINDING                               = 0x8C2A,
    GL_MAP_PERSISTENT_BIT                                   = 0x0040,
    GL_MAP_COHERENT_BIT                                     = 0x0080,
    GL_DYNAMIC_STORAGE_BIT                                  = 0x0100,
    GL_CLIENT_STORAGE_BIT                                   = 0x0200,
    GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT                     = 0x00004000,
    GL_BUFFER_IMMUTABLE_STORAGE                             = 0x821F,
    GL_BUFFER_STORAGE_FLAGS                                 = 0x8220,
    GL_CLEAR_TEXTURE                                        = 0x9365,
    GL_LOCATION_COMPONENT                                   = 0x934A,
    GL_TRANSFORM_FEEDBACK_BUFFER_INDEX                      = 0x934B,
    GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE                     = 0x934C,
    GL_QUERY_BUFFER                                         = 0x9192,
    GL_QUERY_BUFFER_BARRIER_BIT                             = 0x00008000,
    GL_QUERY_BUFFER_BINDING                                 = 0x9193,
    GL_QUERY_RESULT_NO_WAIT                                 = 0x9194,
    GL_MIRROR_CLAMP_TO_EDGE                                 = 0x8743,
};
extern void         (KHRONOS_APIENTRY* const& glBufferStorage) (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
extern void         (KHRONOS_APIENTRY* const& glClearTexImage) (GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glClearTexSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glBindBuffersBase) (GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
extern void         (KHRONOS_APIENTRY* const& glBindBuffersRange) (GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
extern void         (KHRONOS_APIENTRY* const& glBindTextures) (GLuint first, GLsizei count, const GLuint *textures);
extern void         (KHRONOS_APIENTRY* const& glBindSamplers) (GLuint first, GLsizei count, const GLuint *samplers);
extern void         (KHRONOS_APIENTRY* const& glBindImageTextures) (GLuint first, GLsizei count, const GLuint *textures);
extern void         (KHRONOS_APIENTRY* const& glBindVertexBuffers) (GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
#endif

#ifndef GL_VERSION_4_5
#define GL_VERSION_4_5 1
enum : GLenum
{
    GL_CONTEXT_LOST                                         = 0x0507,
    GL_NEGATIVE_ONE_TO_ONE                                  = 0x935E,
    GL_ZERO_TO_ONE                                          = 0x935F,
    GL_CLIP_ORIGIN                                          = 0x935C,
    GL_CLIP_DEPTH_MODE                                      = 0x935D,
    GL_QUERY_WAIT_INVERTED                                  = 0x8E17,
    GL_QUERY_NO_WAIT_INVERTED                               = 0x8E18,
    GL_QUERY_BY_REGION_WAIT_INVERTED                        = 0x8E19,
    GL_QUERY_BY_REGION_NO_WAIT_INVERTED                     = 0x8E1A,
    GL_MAX_CULL_DISTANCES                                   = 0x82F9,
    GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES                 = 0x82FA,
    GL_TEXTURE_TARGET                                       = 0x1006,
    GL_QUERY_TARGET                                         = 0x82EA,
    GL_GUILTY_CONTEXT_RESET                                 = 0x8253,
    GL_INNOCENT_CONTEXT_RESET                               = 0x8254,
    GL_UNKNOWN_CONTEXT_RESET                                = 0x8255,
    GL_RESET_NOTIFICATION_STRATEGY                          = 0x8256,
    GL_LOSE_CONTEXT_ON_RESET                                = 0x8252,
    GL_NO_RESET_NOTIFICATION                                = 0x8261,
    GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT                       = 0x00000004,
    GL_COLOR_TABLE                                          = 0x80D0,
    GL_POST_CONVOLUTION_COLOR_TABLE                         = 0x80D1,
    GL_POST_COLOR_MATRIX_COLOR_TABLE                        = 0x80D2,
    GL_PROXY_COLOR_TABLE                                    = 0x80D3,
    GL_PROXY_POST_CONVOLUTION_COLOR_TABLE                   = 0x80D4,
    GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE                  = 0x80D5,
    GL_CONVOLUTION_1D                                       = 0x8010,
    GL_CONVOLUTION_2D                                       = 0x8011,
    GL_SEPARABLE_2D                                         = 0x8012,
    GL_HISTOGRAM                                            = 0x8024,
    GL_PROXY_HISTOGRAM                                      = 0x8025,
    GL_MINMAX                                               = 0x802E,
    GL_CONTEXT_RELEASE_BEHAVIOR                             = 0x82FB,
    GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH                       = 0x82FC,
};
extern void         (KHRONOS_APIENTRY* const& glClipControl) (GLenum origin, GLenum depth);
extern void         (KHRONOS_APIENTRY* const& glCreateTransformFeedbacks) (GLsizei n, GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackBufferBase) (GLuint xfb, GLuint index, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackBufferRange) (GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glGetTransformFeedbackiv) (GLuint xfb, GLenum pname, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetTransformFeedbacki_v) (GLuint xfb, GLenum pname, GLuint index, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetTransformFeedbacki64_v) (GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
extern void         (KHRONOS_APIENTRY* const& glCreateBuffers) (GLsizei n, GLuint *buffers);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferStorage) (GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferData) (GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferSubData) (GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCopyNamedBufferSubData) (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glClearNamedBufferData) (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glClearNamedBufferSubData) (GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
extern void *       (KHRONOS_APIENTRY* const& glMapNamedBuffer) (GLuint buffer, GLenum access);
extern void *       (KHRONOS_APIENTRY* const& glMapNamedBufferRange) (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern GLboolean    (KHRONOS_APIENTRY* const& glUnmapNamedBuffer) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glFlushMappedNamedBufferRange) (GLuint buffer, GLintptr offset, GLsizeiptr length);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferParameteriv) (GLuint buffer, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferParameteri64v) (GLuint buffer, GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferPointerv) (GLuint buffer, GLenum pname, void **params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferSubData) (GLuint buffer, GLintptr offset, GLsizeiptr size, void *data);
extern void         (KHRONOS_APIENTRY* const& glCreateFramebuffers) (GLsizei n, GLuint *framebuffers);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferRenderbuffer) (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferParameteri) (GLuint framebuffer, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTexture) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTextureLayer) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferDrawBuffer) (GLuint framebuffer, GLenum buf);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferDrawBuffers) (GLuint framebuffer, GLsizei n, const GLenum *bufs);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferReadBuffer) (GLuint framebuffer, GLenum src);
extern void         (KHRONOS_APIENTRY* const& glInvalidateNamedFramebufferData) (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
extern void         (KHRONOS_APIENTRY* const& glInvalidateNamedFramebufferSubData) (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glClearNamedFramebufferiv) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glClearNamedFramebufferuiv) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glClearNamedFramebufferfv) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glClearNamedFramebufferfi) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
extern void         (KHRONOS_APIENTRY* const& glBlitNamedFramebuffer) (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern GLenum       (KHRONOS_APIENTRY* const& glCheckNamedFramebufferStatus) (GLuint framebuffer, GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetNamedFramebufferParameteriv) (GLuint framebuffer, GLenum pname, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetNamedFramebufferAttachmentParameteriv) (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCreateRenderbuffers) (GLsizei n, GLuint *renderbuffers);
extern void         (KHRONOS_APIENTRY* const& glNamedRenderbufferStorage) (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glNamedRenderbufferStorageMultisample) (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetNamedRenderbufferParameteriv) (GLuint renderbuffer, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCreateTextures) (GLenum target, GLsizei n, GLuint *textures);
extern void         (KHRONOS_APIENTRY* const& glTextureBuffer) (GLuint texture, GLenum internalformat, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTextureBufferRange) (GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage1D) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage2D) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage3D) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage2DMultisample) (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage3DMultisample) (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glTextureSubImage1D) (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTextureSubImage2D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTextureSubImage3D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureSubImage1D) (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureSubImage2D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureSubImage3D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureSubImage1D) (GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureSubImage2D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureSubImage3D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterf) (GLuint texture, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterfv) (GLuint texture, GLenum pname, const GLfloat *param);
extern void         (KHRONOS_APIENTRY* const& glTextureParameteri) (GLuint texture, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterIiv) (GLuint texture, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterIuiv) (GLuint texture, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glTextureParameteriv) (GLuint texture, GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGenerateTextureMipmap) (GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glBindTextureUnit) (GLuint unit, GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glGetTextureImage) (GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetCompressedTextureImage) (GLuint texture, GLint level, GLsizei bufSize, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetTextureLevelParameterfv) (GLuint texture, GLint level, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureLevelParameteriv) (GLuint texture, GLint level, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameterfv) (GLuint texture, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameterIiv) (GLuint texture, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameterIuiv) (GLuint texture, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameteriv) (GLuint texture, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCreateVertexArrays) (GLsizei n, GLuint *arrays);
extern void         (KHRONOS_APIENTRY* const& glDisableVertexArrayAttrib) (GLuint vaobj, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glEnableVertexArrayAttrib) (GLuint vaobj, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayElementBuffer) (GLuint vaobj, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexBuffer) (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexBuffers) (GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayAttribBinding) (GLuint vaobj, GLuint attribindex, GLuint bindingindex);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayAttribFormat) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayAttribIFormat) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayAttribLFormat) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayBindingDivisor) (GLuint vaobj, GLuint bindingindex, GLuint divisor);
extern void         (KHRONOS_APIENTRY* const& glGetVertexArrayiv) (GLuint vaobj, GLenum pname, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetVertexArrayIndexediv) (GLuint vaobj, GLuint index, GLenum pname, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetVertexArrayIndexed64iv) (GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
extern void         (KHRONOS_APIENTRY* const& glCreateSamplers) (GLsizei n, GLuint *samplers);
extern void         (KHRONOS_APIENTRY* const& glCreateProgramPipelines) (GLsizei n, GLuint *pipelines);
extern void         (KHRONOS_APIENTRY* const& glCreateQueries) (GLenum target, GLsizei n, GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glGetQueryBufferObjecti64v) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glGetQueryBufferObjectiv) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glGetQueryBufferObjectui64v) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glGetQueryBufferObjectuiv) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glMemoryBarrierByRegion) (GLbitfield barriers);
extern void         (KHRONOS_APIENTRY* const& glGetTextureSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetCompressedTextureSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
extern GLenum       (KHRONOS_APIENTRY* const& glGetGraphicsResetStatus) ();
extern void         (KHRONOS_APIENTRY* const& glGetnCompressedTexImage) (GLenum target, GLint lod, GLsizei bufSize, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetnTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformdv) (GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformfv) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformiv) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformuiv) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glReadnPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
extern void         (KHRONOS_APIENTRY* const& glGetnMapdv) (GLenum target, GLenum query, GLsizei bufSize, GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glGetnMapfv) (GLenum target, GLenum query, GLsizei bufSize, GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glGetnMapiv) (GLenum target, GLenum query, GLsizei bufSize, GLint *v);
extern void         (KHRONOS_APIENTRY* const& glGetnPixelMapfv) (GLenum map, GLsizei bufSize, GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glGetnPixelMapuiv) (GLenum map, GLsizei bufSize, GLuint *values);
extern void         (KHRONOS_APIENTRY* const& glGetnPixelMapusv) (GLenum map, GLsizei bufSize, GLushort *values);
extern void         (KHRONOS_APIENTRY* const& glGetnPolygonStipple) (GLsizei bufSize, GLubyte *pattern);
extern void         (KHRONOS_APIENTRY* const& glGetnColorTable) (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table);
extern void         (KHRONOS_APIENTRY* const& glGetnConvolutionFilter) (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image);
extern void         (KHRONOS_APIENTRY* const& glGetnSeparableFilter) (GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span);
extern void         (KHRONOS_APIENTRY* const& glGetnHistogram) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
extern void         (KHRONOS_APIENTRY* const& glGetnMinmax) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
extern void         (KHRONOS_APIENTRY* const& glTextureBarrier) ();
#endif

#ifndef GL_VERSION_4_6
#define GL_VERSION_4_6 1
enum : GLenum
{
    GL_SHADER_BINARY_FORMAT_SPIR_V                          = 0x9551,
    GL_SPIR_V_BINARY                                        = 0x9552,
    GL_PARAMETER_BUFFER                                     = 0x80EE,
    GL_PARAMETER_BUFFER_BINDING                             = 0x80EF,
    GL_CONTEXT_FLAG_NO_ERROR_BIT                            = 0x00000008,
    GL_VERTICES_SUBMITTED                                   = 0x82EE,
    GL_PRIMITIVES_SUBMITTED                                 = 0x82EF,
    GL_VERTEX_SHADER_INVOCATIONS                            = 0x82F0,
    GL_TESS_CONTROL_SHADER_PATCHES                          = 0x82F1,
    GL_TESS_EVALUATION_SHADER_INVOCATIONS                   = 0x82F2,
    GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED                   = 0x82F3,
    GL_FRAGMENT_SHADER_INVOCATIONS                          = 0x82F4,
    GL_COMPUTE_SHADER_INVOCATIONS                           = 0x82F5,
    GL_CLIPPING_INPUT_PRIMITIVES                            = 0x82F6,
    GL_CLIPPING_OUTPUT_PRIMITIVES                           = 0x82F7,
    GL_POLYGON_OFFSET_CLAMP                                 = 0x8E1B,
    GL_SPIR_V_EXTENSIONS                                    = 0x9553,
    GL_NUM_SPIR_V_EXTENSIONS                                = 0x9554,
    GL_TEXTURE_MAX_ANISOTROPY                               = 0x84FE,
    GL_MAX_TEXTURE_MAX_ANISOTROPY                           = 0x84FF,
    GL_TRANSFORM_FEEDBACK_OVERFLOW                          = 0x82EC,
    GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW                   = 0x82ED,
};
extern void         (KHRONOS_APIENTRY* const& glSpecializeShader) (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysIndirectCount) (GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsIndirectCount) (GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glPolygonOffsetClamp) (GLfloat factor, GLfloat units, GLfloat clamp);
#endif

#ifndef GL_3DFX_multisample
#define GL_3DFX_multisample 1
enum : GLenum
{
    GL_MULTISAMPLE_3DFX                                     = 0x86B2,
    GL_SAMPLE_BUFFERS_3DFX                                  = 0x86B3,
    GL_SAMPLES_3DFX                                         = 0x86B4,
    GL_MULTISAMPLE_BIT_3DFX                                 = 0x20000000,
};
#endif

#ifndef GL_3DFX_tbuffer
#define GL_3DFX_tbuffer 1
extern void         (KHRONOS_APIENTRY* const& glTbufferMask3DFX) (GLuint mask);
#endif

#ifndef GL_3DFX_texture_compression_FXT1
#define GL_3DFX_texture_compression_FXT1 1
enum : GLenum
{
    GL_COMPRESSED_RGB_FXT1_3DFX                             = 0x86B0,
    GL_COMPRESSED_RGBA_FXT1_3DFX                            = 0x86B1,
};
#endif

#ifndef GL_AMD_blend_minmax_factor
#define GL_AMD_blend_minmax_factor 1
enum : GLenum
{
    GL_FACTOR_MIN_AMD                                       = 0x901C,
    GL_FACTOR_MAX_AMD                                       = 0x901D,
};
#endif

#ifndef GL_AMD_conservative_depth
#define GL_AMD_conservative_depth 1
#endif

#ifndef GL_AMD_debug_output
#define GL_AMD_debug_output 1
enum : GLenum
{
    GL_MAX_DEBUG_MESSAGE_LENGTH_AMD                         = 0x9143,
    GL_MAX_DEBUG_LOGGED_MESSAGES_AMD                        = 0x9144,
    GL_DEBUG_LOGGED_MESSAGES_AMD                            = 0x9145,
    GL_DEBUG_SEVERITY_HIGH_AMD                              = 0x9146,
    GL_DEBUG_SEVERITY_MEDIUM_AMD                            = 0x9147,
    GL_DEBUG_SEVERITY_LOW_AMD                               = 0x9148,
    GL_DEBUG_CATEGORY_API_ERROR_AMD                         = 0x9149,
    GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD                     = 0x914A,
    GL_DEBUG_CATEGORY_DEPRECATION_AMD                       = 0x914B,
    GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD                = 0x914C,
    GL_DEBUG_CATEGORY_PERFORMANCE_AMD                       = 0x914D,
    GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD                   = 0x914E,
    GL_DEBUG_CATEGORY_APPLICATION_AMD                       = 0x914F,
    GL_DEBUG_CATEGORY_OTHER_AMD                             = 0x9150,
};
extern void         (KHRONOS_APIENTRY* const& glDebugMessageEnableAMD) (GLenum category, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageInsertAMD) (GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar *buf);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageCallbackAMD) (GLDEBUGPROCAMD callback, void *userParam);
extern GLuint       (KHRONOS_APIENTRY* const& glGetDebugMessageLogAMD) (GLuint count, GLsizei bufSize, GLenum *categories, GLuint *severities, GLuint *ids, GLsizei *lengths, GLchar *message);
#endif

#ifndef GL_AMD_depth_clamp_separate
#define GL_AMD_depth_clamp_separate 1
enum : GLenum
{
    GL_DEPTH_CLAMP_NEAR_AMD                                 = 0x901E,
    GL_DEPTH_CLAMP_FAR_AMD                                  = 0x901F,
};
#endif

#ifndef GL_AMD_draw_buffers_blend
#define GL_AMD_draw_buffers_blend 1
extern void         (KHRONOS_APIENTRY* const& glBlendFuncIndexedAMD) (GLuint buf, GLenum src, GLenum dst);
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparateIndexedAMD) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationIndexedAMD) (GLuint buf, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparateIndexedAMD) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
#endif

#ifndef GL_AMD_framebuffer_multisample_advanced
#define GL_AMD_framebuffer_multisample_advanced 1
enum : GLenum
{
    GL_RENDERBUFFER_STORAGE_SAMPLES_AMD                     = 0x91B2,
    GL_MAX_COLOR_FRAMEBUFFER_SAMPLES_AMD                    = 0x91B3,
    GL_MAX_COLOR_FRAMEBUFFER_STORAGE_SAMPLES_AMD            = 0x91B4,
    GL_MAX_DEPTH_STENCIL_FRAMEBUFFER_SAMPLES_AMD            = 0x91B5,
    GL_NUM_SUPPORTED_MULTISAMPLE_MODES_AMD                  = 0x91B6,
    GL_SUPPORTED_MULTISAMPLE_MODES_AMD                      = 0x91B7,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleAdvancedAMD) (GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glNamedRenderbufferStorageMultisampleAdvancedAMD) (GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height);
#endif

#ifndef GL_AMD_framebuffer_sample_positions
#define GL_AMD_framebuffer_sample_positions 1
enum : GLenum
{
    GL_SUBSAMPLE_DISTANCE_AMD                               = 0x883F,
    GL_PIXELS_PER_SAMPLE_PATTERN_X_AMD                      = 0x91AE,
    GL_PIXELS_PER_SAMPLE_PATTERN_Y_AMD                      = 0x91AF,
    GL_ALL_PIXELS_AMD                                       = 0xFFFFFFFF,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferSamplePositionsfvAMD) (GLenum target, GLuint numsamples, GLuint pixelindex, const GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferSamplePositionsfvAMD) (GLuint framebuffer, GLuint numsamples, GLuint pixelindex, const GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferParameterfvAMD) (GLenum target, GLenum pname, GLuint numsamples, GLuint pixelindex, GLsizei size, GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glGetNamedFramebufferParameterfvAMD) (GLuint framebuffer, GLenum pname, GLuint numsamples, GLuint pixelindex, GLsizei size, GLfloat *values);
#endif

#ifndef GL_AMD_gcn_shader
#define GL_AMD_gcn_shader 1
#endif

#ifndef GL_AMD_gpu_shader_half_float
#define GL_AMD_gpu_shader_half_float 1
enum : GLenum
{
    GL_FLOAT16_NV                                           = 0x8FF8,
    GL_FLOAT16_VEC2_NV                                      = 0x8FF9,
    GL_FLOAT16_VEC3_NV                                      = 0x8FFA,
    GL_FLOAT16_VEC4_NV                                      = 0x8FFB,
    GL_FLOAT16_MAT2_AMD                                     = 0x91C5,
    GL_FLOAT16_MAT3_AMD                                     = 0x91C6,
    GL_FLOAT16_MAT4_AMD                                     = 0x91C7,
    GL_FLOAT16_MAT2x3_AMD                                   = 0x91C8,
    GL_FLOAT16_MAT2x4_AMD                                   = 0x91C9,
    GL_FLOAT16_MAT3x2_AMD                                   = 0x91CA,
    GL_FLOAT16_MAT3x4_AMD                                   = 0x91CB,
    GL_FLOAT16_MAT4x2_AMD                                   = 0x91CC,
    GL_FLOAT16_MAT4x3_AMD                                   = 0x91CD,
};
#endif

#ifndef GL_AMD_gpu_shader_int16
#define GL_AMD_gpu_shader_int16 1
#endif

#ifndef GL_AMD_gpu_shader_int64
#define GL_AMD_gpu_shader_int64 1
enum : GLenum
{
    GL_INT64_NV                                             = 0x140E,
    GL_UNSIGNED_INT64_NV                                    = 0x140F,
    GL_INT8_NV                                              = 0x8FE0,
    GL_INT8_VEC2_NV                                         = 0x8FE1,
    GL_INT8_VEC3_NV                                         = 0x8FE2,
    GL_INT8_VEC4_NV                                         = 0x8FE3,
    GL_INT16_NV                                             = 0x8FE4,
    GL_INT16_VEC2_NV                                        = 0x8FE5,
    GL_INT16_VEC3_NV                                        = 0x8FE6,
    GL_INT16_VEC4_NV                                        = 0x8FE7,
    GL_INT64_VEC2_NV                                        = 0x8FE9,
    GL_INT64_VEC3_NV                                        = 0x8FEA,
    GL_INT64_VEC4_NV                                        = 0x8FEB,
    GL_UNSIGNED_INT8_NV                                     = 0x8FEC,
    GL_UNSIGNED_INT8_VEC2_NV                                = 0x8FED,
    GL_UNSIGNED_INT8_VEC3_NV                                = 0x8FEE,
    GL_UNSIGNED_INT8_VEC4_NV                                = 0x8FEF,
    GL_UNSIGNED_INT16_NV                                    = 0x8FF0,
    GL_UNSIGNED_INT16_VEC2_NV                               = 0x8FF1,
    GL_UNSIGNED_INT16_VEC3_NV                               = 0x8FF2,
    GL_UNSIGNED_INT16_VEC4_NV                               = 0x8FF3,
    GL_UNSIGNED_INT64_VEC2_NV                               = 0x8FF5,
    GL_UNSIGNED_INT64_VEC3_NV                               = 0x8FF6,
    GL_UNSIGNED_INT64_VEC4_NV                               = 0x8FF7,
};
extern void         (KHRONOS_APIENTRY* const& glUniform1i64NV) (GLint location, GLint64EXT x);
extern void         (KHRONOS_APIENTRY* const& glUniform2i64NV) (GLint location, GLint64EXT x, GLint64EXT y);
extern void         (KHRONOS_APIENTRY* const& glUniform3i64NV) (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z);
extern void         (KHRONOS_APIENTRY* const& glUniform4i64NV) (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
extern void         (KHRONOS_APIENTRY* const& glUniform1i64vNV) (GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2i64vNV) (GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3i64vNV) (GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4i64vNV) (GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glUniform1ui64NV) (GLint location, GLuint64EXT x);
extern void         (KHRONOS_APIENTRY* const& glUniform2ui64NV) (GLint location, GLuint64EXT x, GLuint64EXT y);
extern void         (KHRONOS_APIENTRY* const& glUniform3ui64NV) (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
extern void         (KHRONOS_APIENTRY* const& glUniform4ui64NV) (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
extern void         (KHRONOS_APIENTRY* const& glUniform1ui64vNV) (GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2ui64vNV) (GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3ui64vNV) (GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4ui64vNV) (GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glGetUniformi64vNV) (GLuint program, GLint location, GLint64EXT *params);
extern void         (KHRONOS_APIENTRY* const& glGetUniformui64vNV) (GLuint program, GLint location, GLuint64EXT *params);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1i64NV) (GLuint program, GLint location, GLint64EXT x);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2i64NV) (GLuint program, GLint location, GLint64EXT x, GLint64EXT y);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3i64NV) (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4i64NV) (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1i64vNV) (GLuint program, GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2i64vNV) (GLuint program, GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3i64vNV) (GLuint program, GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4i64vNV) (GLuint program, GLint location, GLsizei count, const GLint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ui64NV) (GLuint program, GLint location, GLuint64EXT x);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ui64NV) (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ui64NV) (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ui64NV) (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ui64vNV) (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ui64vNV) (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ui64vNV) (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ui64vNV) (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value);
#endif

#ifndef GL_AMD_interleaved_elements
#define GL_AMD_interleaved_elements 1
enum : GLenum
{
    GL_VERTEX_ELEMENT_SWIZZLE_AMD                           = 0x91A4,
    GL_VERTEX_ID_SWIZZLE_AMD                                = 0x91A5,
};
extern void         (KHRONOS_APIENTRY* const& glVertexAttribParameteriAMD) (GLuint index, GLenum pname, GLint param);
#endif

#ifndef GL_AMD_multi_draw_indirect
#define GL_AMD_multi_draw_indirect 1
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysIndirectAMD) (GLenum mode, const void *indirect, GLsizei primcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsIndirectAMD) (GLenum mode, GLenum type, const void *indirect, GLsizei primcount, GLsizei stride);
#endif

#ifndef GL_AMD_name_gen_delete
#define GL_AMD_name_gen_delete 1
enum : GLenum
{
    GL_DATA_BUFFER_AMD                                      = 0x9151,
    GL_PERFORMANCE_MONITOR_AMD                              = 0x9152,
    GL_QUERY_OBJECT_AMD                                     = 0x9153,
    GL_VERTEX_ARRAY_OBJECT_AMD                              = 0x9154,
    GL_SAMPLER_OBJECT_AMD                                   = 0x9155,
};
extern void         (KHRONOS_APIENTRY* const& glGenNamesAMD) (GLenum identifier, GLuint num, GLuint *names);
extern void         (KHRONOS_APIENTRY* const& glDeleteNamesAMD) (GLenum identifier, GLuint num, const GLuint *names);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsNameAMD) (GLenum identifier, GLuint name);
#endif

#ifndef GL_AMD_occlusion_query_event
#define GL_AMD_occlusion_query_event 1
enum : GLenum
{
    GL_OCCLUSION_QUERY_EVENT_MASK_AMD                       = 0x874F,
    GL_QUERY_DEPTH_PASS_EVENT_BIT_AMD                       = 0x00000001,
    GL_QUERY_DEPTH_FAIL_EVENT_BIT_AMD                       = 0x00000002,
    GL_QUERY_STENCIL_FAIL_EVENT_BIT_AMD                     = 0x00000004,
    GL_QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT_AMD                = 0x00000008,
    GL_QUERY_ALL_EVENT_BITS_AMD                             = 0xFFFFFFFF,
};
extern void         (KHRONOS_APIENTRY* const& glQueryObjectParameteruiAMD) (GLenum target, GLuint id, GLenum pname, GLuint param);
#endif

#ifndef GL_AMD_performance_monitor
#define GL_AMD_performance_monitor 1
enum : GLenum
{
    GL_COUNTER_TYPE_AMD                                     = 0x8BC0,
    GL_COUNTER_RANGE_AMD                                    = 0x8BC1,
    GL_UNSIGNED_INT64_AMD                                   = 0x8BC2,
    GL_PERCENTAGE_AMD                                       = 0x8BC3,
    GL_PERFMON_RESULT_AVAILABLE_AMD                         = 0x8BC4,
    GL_PERFMON_RESULT_SIZE_AMD                              = 0x8BC5,
    GL_PERFMON_RESULT_AMD                                   = 0x8BC6,
};
extern void         (KHRONOS_APIENTRY* const& glGetPerfMonitorGroupsAMD) (GLint *numGroups, GLsizei groupsSize, GLuint *groups);
extern void         (KHRONOS_APIENTRY* const& glGetPerfMonitorCountersAMD) (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters);
extern void         (KHRONOS_APIENTRY* const& glGetPerfMonitorGroupStringAMD) (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString);
extern void         (KHRONOS_APIENTRY* const& glGetPerfMonitorCounterStringAMD) (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString);
extern void         (KHRONOS_APIENTRY* const& glGetPerfMonitorCounterInfoAMD) (GLuint group, GLuint counter, GLenum pname, void *data);
extern void         (KHRONOS_APIENTRY* const& glGenPerfMonitorsAMD) (GLsizei n, GLuint *monitors);
extern void         (KHRONOS_APIENTRY* const& glDeletePerfMonitorsAMD) (GLsizei n, GLuint *monitors);
extern void         (KHRONOS_APIENTRY* const& glSelectPerfMonitorCountersAMD) (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *counterList);
extern void         (KHRONOS_APIENTRY* const& glBeginPerfMonitorAMD) (GLuint monitor);
extern void         (KHRONOS_APIENTRY* const& glEndPerfMonitorAMD) (GLuint monitor);
extern void         (KHRONOS_APIENTRY* const& glGetPerfMonitorCounterDataAMD) (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten);
#endif

#ifndef GL_AMD_pinned_memory
#define GL_AMD_pinned_memory 1
enum : GLenum
{
    GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD                   = 0x9160,
};
#endif

#ifndef GL_AMD_query_buffer_object
#define GL_AMD_query_buffer_object 1
enum : GLenum
{
    GL_QUERY_BUFFER_AMD                                     = 0x9192,
    GL_QUERY_BUFFER_BINDING_AMD                             = 0x9193,
    GL_QUERY_RESULT_NO_WAIT_AMD                             = 0x9194,
};
#endif

#ifndef GL_AMD_sample_positions
#define GL_AMD_sample_positions 1
extern void         (KHRONOS_APIENTRY* const& glSetMultisamplefvAMD) (GLenum pname, GLuint index, const GLfloat *val);
#endif

#ifndef GL_AMD_seamless_cubemap_per_texture
#define GL_AMD_seamless_cubemap_per_texture 1
#endif

#ifndef GL_AMD_shader_atomic_counter_ops
#define GL_AMD_shader_atomic_counter_ops 1
#endif

#ifndef GL_AMD_shader_ballot
#define GL_AMD_shader_ballot 1
#endif

#ifndef GL_AMD_shader_gpu_shader_half_float_fetch
#define GL_AMD_shader_gpu_shader_half_float_fetch 1
#endif

#ifndef GL_AMD_shader_image_load_store_lod
#define GL_AMD_shader_image_load_store_lod 1
#endif

#ifndef GL_AMD_shader_stencil_export
#define GL_AMD_shader_stencil_export 1
#endif

#ifndef GL_AMD_shader_trinary_minmax
#define GL_AMD_shader_trinary_minmax 1
#endif

#ifndef GL_AMD_shader_explicit_vertex_parameter
#define GL_AMD_shader_explicit_vertex_parameter 1
#endif

#ifndef GL_AMD_sparse_texture
#define GL_AMD_sparse_texture 1
enum : GLenum
{
    GL_VIRTUAL_PAGE_SIZE_X_AMD                              = 0x9195,
    GL_VIRTUAL_PAGE_SIZE_Y_AMD                              = 0x9196,
    GL_VIRTUAL_PAGE_SIZE_Z_AMD                              = 0x9197,
    GL_MAX_SPARSE_TEXTURE_SIZE_AMD                          = 0x9198,
    GL_MAX_SPARSE_3D_TEXTURE_SIZE_AMD                       = 0x9199,
    GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS                      = 0x919A,
    GL_MIN_SPARSE_LEVEL_AMD                                 = 0x919B,
    GL_MIN_LOD_WARNING_AMD                                  = 0x919C,
    GL_TEXTURE_STORAGE_SPARSE_BIT_AMD                       = 0x00000001,
};
extern void         (KHRONOS_APIENTRY* const& glTexStorageSparseAMD) (GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags);
extern void         (KHRONOS_APIENTRY* const& glTextureStorageSparseAMD) (GLuint texture, GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags);
#endif

#ifndef GL_AMD_stencil_operation_extended
#define GL_AMD_stencil_operation_extended 1
enum : GLenum
{
    GL_SET_AMD                                              = 0x874A,
    GL_REPLACE_VALUE_AMD                                    = 0x874B,
    GL_STENCIL_OP_VALUE_AMD                                 = 0x874C,
    GL_STENCIL_BACK_OP_VALUE_AMD                            = 0x874D,
};
extern void         (KHRONOS_APIENTRY* const& glStencilOpValueAMD) (GLenum face, GLuint value);
#endif

#ifndef GL_AMD_texture_gather_bias_lod
#define GL_AMD_texture_gather_bias_lod 1
#endif

#ifndef GL_AMD_texture_texture4
#define GL_AMD_texture_texture4 1
#endif

#ifndef GL_AMD_transform_feedback3_lines_triangles
#define GL_AMD_transform_feedback3_lines_triangles 1
#endif

#ifndef GL_AMD_transform_feedback4
#define GL_AMD_transform_feedback4 1
enum : GLenum
{
    GL_STREAM_RASTERIZATION_AMD                             = 0x91A0,
};
#endif

#ifndef GL_AMD_vertex_shader_layer
#define GL_AMD_vertex_shader_layer 1
#endif

#ifndef GL_AMD_vertex_shader_tessellator
#define GL_AMD_vertex_shader_tessellator 1
enum : GLenum
{
    GL_SAMPLER_BUFFER_AMD                                   = 0x9001,
    GL_INT_SAMPLER_BUFFER_AMD                               = 0x9002,
    GL_UNSIGNED_INT_SAMPLER_BUFFER_AMD                      = 0x9003,
    GL_TESSELLATION_MODE_AMD                                = 0x9004,
    GL_TESSELLATION_FACTOR_AMD                              = 0x9005,
    GL_DISCRETE_AMD                                         = 0x9006,
    GL_CONTINUOUS_AMD                                       = 0x9007,
};
extern void         (KHRONOS_APIENTRY* const& glTessellationFactorAMD) (GLfloat factor);
extern void         (KHRONOS_APIENTRY* const& glTessellationModeAMD) (GLenum mode);
#endif

#ifndef GL_AMD_vertex_shader_viewport_index
#define GL_AMD_vertex_shader_viewport_index 1
#endif

#ifndef GL_APPLE_aux_depth_stencil
#define GL_APPLE_aux_depth_stencil 1
enum : GLenum
{
    GL_AUX_DEPTH_STENCIL_APPLE                              = 0x8A14,
};
#endif

#ifndef GL_APPLE_client_storage
#define GL_APPLE_client_storage 1
enum : GLenum
{
    GL_UNPACK_CLIENT_STORAGE_APPLE                          = 0x85B2,
};
#endif

#ifndef GL_APPLE_element_array
#define GL_APPLE_element_array 1
enum : GLenum
{
    GL_ELEMENT_ARRAY_APPLE                                  = 0x8A0C,
    GL_ELEMENT_ARRAY_TYPE_APPLE                             = 0x8A0D,
    GL_ELEMENT_ARRAY_POINTER_APPLE                          = 0x8A0E,
};
extern void         (KHRONOS_APIENTRY* const& glElementPointerAPPLE) (GLenum type, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glDrawElementArrayAPPLE) (GLenum mode, GLint first, GLsizei count);
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElementArrayAPPLE) (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementArrayAPPLE) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawRangeElementArrayAPPLE) (GLenum mode, GLuint start, GLuint end, const GLint *first, const GLsizei *count, GLsizei primcount);
#endif

#ifndef GL_APPLE_fence
#define GL_APPLE_fence 1
enum : GLenum
{
    GL_DRAW_PIXELS_APPLE                                    = 0x8A0A,
    GL_FENCE_APPLE                                          = 0x8A0B,
};
extern void         (KHRONOS_APIENTRY* const& glGenFencesAPPLE) (GLsizei n, GLuint *fences);
extern void         (KHRONOS_APIENTRY* const& glDeleteFencesAPPLE) (GLsizei n, const GLuint *fences);
extern void         (KHRONOS_APIENTRY* const& glSetFenceAPPLE) (GLuint fence);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsFenceAPPLE) (GLuint fence);
extern GLboolean    (KHRONOS_APIENTRY* const& glTestFenceAPPLE) (GLuint fence);
extern void         (KHRONOS_APIENTRY* const& glFinishFenceAPPLE) (GLuint fence);
extern GLboolean    (KHRONOS_APIENTRY* const& glTestObjectAPPLE) (GLenum object, GLuint name);
extern void         (KHRONOS_APIENTRY* const& glFinishObjectAPPLE) (GLenum object, GLint name);
#endif

#ifndef GL_APPLE_float_pixels
#define GL_APPLE_float_pixels 1
enum : GLenum
{
    GL_HALF_APPLE                                           = 0x140B,
    GL_RGBA_FLOAT32_APPLE                                   = 0x8814,
    GL_RGB_FLOAT32_APPLE                                    = 0x8815,
    GL_ALPHA_FLOAT32_APPLE                                  = 0x8816,
    GL_INTENSITY_FLOAT32_APPLE                              = 0x8817,
    GL_LUMINANCE_FLOAT32_APPLE                              = 0x8818,
    GL_LUMINANCE_ALPHA_FLOAT32_APPLE                        = 0x8819,
    GL_RGBA_FLOAT16_APPLE                                   = 0x881A,
    GL_RGB_FLOAT16_APPLE                                    = 0x881B,
    GL_ALPHA_FLOAT16_APPLE                                  = 0x881C,
    GL_INTENSITY_FLOAT16_APPLE                              = 0x881D,
    GL_LUMINANCE_FLOAT16_APPLE                              = 0x881E,
    GL_LUMINANCE_ALPHA_FLOAT16_APPLE                        = 0x881F,
    GL_COLOR_FLOAT_APPLE                                    = 0x8A0F,
};
#endif

#ifndef GL_APPLE_flush_buffer_range
#define GL_APPLE_flush_buffer_range 1
enum : GLenum
{
    GL_BUFFER_SERIALIZED_MODIFY_APPLE                       = 0x8A12,
    GL_BUFFER_FLUSHING_UNMAP_APPLE                          = 0x8A13,
};
extern void         (KHRONOS_APIENTRY* const& glBufferParameteriAPPLE) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glFlushMappedBufferRangeAPPLE) (GLenum target, GLintptr offset, GLsizeiptr size);
#endif

#ifndef GL_APPLE_object_purgeable
#define GL_APPLE_object_purgeable 1
enum : GLenum
{
    GL_BUFFER_OBJECT_APPLE                                  = 0x85B3,
    GL_RELEASED_APPLE                                       = 0x8A19,
    GL_VOLATILE_APPLE                                       = 0x8A1A,
    GL_RETAINED_APPLE                                       = 0x8A1B,
    GL_UNDEFINED_APPLE                                      = 0x8A1C,
    GL_PURGEABLE_APPLE                                      = 0x8A1D,
};
extern GLenum       (KHRONOS_APIENTRY* const& glObjectPurgeableAPPLE) (GLenum objectType, GLuint name, GLenum option);
extern GLenum       (KHRONOS_APIENTRY* const& glObjectUnpurgeableAPPLE) (GLenum objectType, GLuint name, GLenum option);
extern void         (KHRONOS_APIENTRY* const& glGetObjectParameterivAPPLE) (GLenum objectType, GLuint name, GLenum pname, GLint *params);
#endif

#ifndef GL_APPLE_rgb_422
#define GL_APPLE_rgb_422 1
enum : GLenum
{
    GL_RGB_422_APPLE                                        = 0x8A1F,
    GL_UNSIGNED_SHORT_8_8_APPLE                             = 0x85BA,
    GL_UNSIGNED_SHORT_8_8_REV_APPLE                         = 0x85BB,
    GL_RGB_RAW_422_APPLE                                    = 0x8A51,
};
#endif

#ifndef GL_APPLE_row_bytes
#define GL_APPLE_row_bytes 1
enum : GLenum
{
    GL_PACK_ROW_BYTES_APPLE                                 = 0x8A15,
    GL_UNPACK_ROW_BYTES_APPLE                               = 0x8A16,
};
#endif

#ifndef GL_APPLE_specular_vector
#define GL_APPLE_specular_vector 1
enum : GLenum
{
    GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE                    = 0x85B0,
};
#endif

#ifndef GL_APPLE_texture_range
#define GL_APPLE_texture_range 1
enum : GLenum
{
    GL_TEXTURE_RANGE_LENGTH_APPLE                           = 0x85B7,
    GL_TEXTURE_RANGE_POINTER_APPLE                          = 0x85B8,
    GL_TEXTURE_STORAGE_HINT_APPLE                           = 0x85BC,
    GL_STORAGE_PRIVATE_APPLE                                = 0x85BD,
    GL_STORAGE_CACHED_APPLE                                 = 0x85BE,
    GL_STORAGE_SHARED_APPLE                                 = 0x85BF,
};
extern void         (KHRONOS_APIENTRY* const& glTextureRangeAPPLE) (GLenum target, GLsizei length, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterPointervAPPLE) (GLenum target, GLenum pname, void **params);
#endif

#ifndef GL_APPLE_transform_hint
#define GL_APPLE_transform_hint 1
enum : GLenum
{
    GL_TRANSFORM_HINT_APPLE                                 = 0x85B1,
};
#endif

#ifndef GL_APPLE_vertex_array_object
#define GL_APPLE_vertex_array_object 1
enum : GLenum
{
    GL_VERTEX_ARRAY_BINDING_APPLE                           = 0x85B5,
};
extern void         (KHRONOS_APIENTRY* const& glBindVertexArrayAPPLE) (GLuint array);
extern void         (KHRONOS_APIENTRY* const& glDeleteVertexArraysAPPLE) (GLsizei n, const GLuint *arrays);
extern void         (KHRONOS_APIENTRY* const& glGenVertexArraysAPPLE) (GLsizei n, GLuint *arrays);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsVertexArrayAPPLE) (GLuint array);
#endif

#ifndef GL_APPLE_vertex_array_range
#define GL_APPLE_vertex_array_range 1
enum : GLenum
{
    GL_VERTEX_ARRAY_RANGE_APPLE                             = 0x851D,
    GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE                      = 0x851E,
    GL_VERTEX_ARRAY_STORAGE_HINT_APPLE                      = 0x851F,
    GL_VERTEX_ARRAY_RANGE_POINTER_APPLE                     = 0x8521,
    GL_STORAGE_CLIENT_APPLE                                 = 0x85B4,
};
extern void         (KHRONOS_APIENTRY* const& glVertexArrayRangeAPPLE) (GLsizei length, void *pointer);
extern void         (KHRONOS_APIENTRY* const& glFlushVertexArrayRangeAPPLE) (GLsizei length, void *pointer);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayParameteriAPPLE) (GLenum pname, GLint param);
#endif

#ifndef GL_APPLE_vertex_program_evaluators
#define GL_APPLE_vertex_program_evaluators 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_MAP1_APPLE                             = 0x8A00,
    GL_VERTEX_ATTRIB_MAP2_APPLE                             = 0x8A01,
    GL_VERTEX_ATTRIB_MAP1_SIZE_APPLE                        = 0x8A02,
    GL_VERTEX_ATTRIB_MAP1_COEFF_APPLE                       = 0x8A03,
    GL_VERTEX_ATTRIB_MAP1_ORDER_APPLE                       = 0x8A04,
    GL_VERTEX_ATTRIB_MAP1_DOMAIN_APPLE                      = 0x8A05,
    GL_VERTEX_ATTRIB_MAP2_SIZE_APPLE                        = 0x8A06,
    GL_VERTEX_ATTRIB_MAP2_COEFF_APPLE                       = 0x8A07,
    GL_VERTEX_ATTRIB_MAP2_ORDER_APPLE                       = 0x8A08,
    GL_VERTEX_ATTRIB_MAP2_DOMAIN_APPLE                      = 0x8A09,
};
extern void         (KHRONOS_APIENTRY* const& glEnableVertexAttribAPPLE) (GLuint index, GLenum pname);
extern void         (KHRONOS_APIENTRY* const& glDisableVertexAttribAPPLE) (GLuint index, GLenum pname);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsVertexAttribEnabledAPPLE) (GLuint index, GLenum pname);
extern void         (KHRONOS_APIENTRY* const& glMapVertexAttrib1dAPPLE) (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
extern void         (KHRONOS_APIENTRY* const& glMapVertexAttrib1fAPPLE) (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
extern void         (KHRONOS_APIENTRY* const& glMapVertexAttrib2dAPPLE) (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
extern void         (KHRONOS_APIENTRY* const& glMapVertexAttrib2fAPPLE) (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
#endif

#ifndef GL_APPLE_ycbcr_422
#define GL_APPLE_ycbcr_422 1
enum : GLenum
{
    GL_YCBCR_422_APPLE                                      = 0x85B9,
};
#endif

#ifndef GL_ARB_ES2_compatibility
#define GL_ARB_ES2_compatibility 1
#endif

#ifndef GL_ARB_ES3_1_compatibility
#define GL_ARB_ES3_1_compatibility 1
#endif

#ifndef GL_ARB_ES3_2_compatibility
#define GL_ARB_ES3_2_compatibility 1
enum : GLenum
{
    GL_PRIMITIVE_BOUNDING_BOX_ARB                           = 0x92BE,
    GL_MULTISAMPLE_LINE_WIDTH_RANGE_ARB                     = 0x9381,
    GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY_ARB               = 0x9382,
};
extern void         (KHRONOS_APIENTRY* const& glPrimitiveBoundingBoxARB) (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);
#endif

#ifndef GL_ARB_ES3_compatibility
#define GL_ARB_ES3_compatibility 1
#endif

#ifndef GL_ARB_arrays_of_arrays
#define GL_ARB_arrays_of_arrays 1
#endif

#ifndef GL_ARB_base_instance
#define GL_ARB_base_instance 1
#endif

#ifndef GL_ARB_bindless_texture
#define GL_ARB_bindless_texture 1
enum : GLenum
{
    GL_UNSIGNED_INT64_ARB                                   = 0x140F,
};
extern GLuint64     (KHRONOS_APIENTRY* const& glGetTextureHandleARB) (GLuint texture);
extern GLuint64     (KHRONOS_APIENTRY* const& glGetTextureSamplerHandleARB) (GLuint texture, GLuint sampler);
extern void         (KHRONOS_APIENTRY* const& glMakeTextureHandleResidentARB) (GLuint64 handle);
extern void         (KHRONOS_APIENTRY* const& glMakeTextureHandleNonResidentARB) (GLuint64 handle);
extern GLuint64     (KHRONOS_APIENTRY* const& glGetImageHandleARB) (GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
extern void         (KHRONOS_APIENTRY* const& glMakeImageHandleResidentARB) (GLuint64 handle, GLenum access);
extern void         (KHRONOS_APIENTRY* const& glMakeImageHandleNonResidentARB) (GLuint64 handle);
extern void         (KHRONOS_APIENTRY* const& glUniformHandleui64ARB) (GLint location, GLuint64 value);
extern void         (KHRONOS_APIENTRY* const& glUniformHandleui64vARB) (GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformHandleui64ARB) (GLuint program, GLint location, GLuint64 value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformHandleui64vARB) (GLuint program, GLint location, GLsizei count, const GLuint64 *values);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTextureHandleResidentARB) (GLuint64 handle);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsImageHandleResidentARB) (GLuint64 handle);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1ui64ARB) (GLuint index, GLuint64EXT x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1ui64vARB) (GLuint index, const GLuint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribLui64vARB) (GLuint index, GLenum pname, GLuint64EXT *params);
#endif

#ifndef GL_ARB_blend_func_extended
#define GL_ARB_blend_func_extended 1
#endif

#ifndef GL_ARB_buffer_storage
#define GL_ARB_buffer_storage 1
#endif

#ifndef GL_ARB_cl_event
#define GL_ARB_cl_event 1
enum : GLenum
{
    GL_SYNC_CL_EVENT_ARB                                    = 0x8240,
    GL_SYNC_CL_EVENT_COMPLETE_ARB                           = 0x8241,
};
extern GLsync       (KHRONOS_APIENTRY* const& glCreateSyncFromCLeventARB) (struct _cl_context *context, struct _cl_event *event, GLbitfield flags);
#endif

#ifndef GL_ARB_clear_buffer_object
#define GL_ARB_clear_buffer_object 1
#endif

#ifndef GL_ARB_clear_texture
#define GL_ARB_clear_texture 1
#endif

#ifndef GL_ARB_clip_control
#define GL_ARB_clip_control 1
#endif

#ifndef GL_ARB_color_buffer_float
#define GL_ARB_color_buffer_float 1
enum : GLenum
{
    GL_RGBA_FLOAT_MODE_ARB                                  = 0x8820,
    GL_CLAMP_VERTEX_COLOR_ARB                               = 0x891A,
    GL_CLAMP_FRAGMENT_COLOR_ARB                             = 0x891B,
    GL_CLAMP_READ_COLOR_ARB                                 = 0x891C,
    GL_FIXED_ONLY_ARB                                       = 0x891D,
};
extern void         (KHRONOS_APIENTRY* const& glClampColorARB) (GLenum target, GLenum clamp);
#endif

#ifndef GL_ARB_compatibility
#define GL_ARB_compatibility 1
#endif

#ifndef GL_ARB_compressed_texture_pixel_storage
#define GL_ARB_compressed_texture_pixel_storage 1
#endif

#ifndef GL_ARB_compute_shader
#define GL_ARB_compute_shader 1
#endif

#ifndef GL_ARB_compute_variable_group_size
#define GL_ARB_compute_variable_group_size 1
enum : GLenum
{
    GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB           = 0x9344,
    GL_MAX_COMPUTE_FIXED_GROUP_INVOCATIONS_ARB              = 0x90EB,
    GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB                  = 0x9345,
    GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB                     = 0x91BF,
};
extern void         (KHRONOS_APIENTRY* const& glDispatchComputeGroupSizeARB) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z);
#endif

#ifndef GL_ARB_conditional_render_inverted
#define GL_ARB_conditional_render_inverted 1
#endif

#ifndef GL_ARB_conservative_depth
#define GL_ARB_conservative_depth 1
#endif

#ifndef GL_ARB_copy_buffer
#define GL_ARB_copy_buffer 1
#endif

#ifndef GL_ARB_copy_image
#define GL_ARB_copy_image 1
#endif

#ifndef GL_ARB_cull_distance
#define GL_ARB_cull_distance 1
#endif

#ifndef GL_ARB_debug_output
#define GL_ARB_debug_output 1
enum : GLenum
{
    GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB                         = 0x8242,
    GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB                 = 0x8243,
    GL_DEBUG_CALLBACK_FUNCTION_ARB                          = 0x8244,
    GL_DEBUG_CALLBACK_USER_PARAM_ARB                        = 0x8245,
    GL_DEBUG_SOURCE_API_ARB                                 = 0x8246,
    GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB                       = 0x8247,
    GL_DEBUG_SOURCE_SHADER_COMPILER_ARB                     = 0x8248,
    GL_DEBUG_SOURCE_THIRD_PARTY_ARB                         = 0x8249,
    GL_DEBUG_SOURCE_APPLICATION_ARB                         = 0x824A,
    GL_DEBUG_SOURCE_OTHER_ARB                               = 0x824B,
    GL_DEBUG_TYPE_ERROR_ARB                                 = 0x824C,
    GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB                   = 0x824D,
    GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB                    = 0x824E,
    GL_DEBUG_TYPE_PORTABILITY_ARB                           = 0x824F,
    GL_DEBUG_TYPE_PERFORMANCE_ARB                           = 0x8250,
    GL_DEBUG_TYPE_OTHER_ARB                                 = 0x8251,
    GL_MAX_DEBUG_MESSAGE_LENGTH_ARB                         = 0x9143,
    GL_MAX_DEBUG_LOGGED_MESSAGES_ARB                        = 0x9144,
    GL_DEBUG_LOGGED_MESSAGES_ARB                            = 0x9145,
    GL_DEBUG_SEVERITY_HIGH_ARB                              = 0x9146,
    GL_DEBUG_SEVERITY_MEDIUM_ARB                            = 0x9147,
    GL_DEBUG_SEVERITY_LOW_ARB                               = 0x9148,
};
extern void         (KHRONOS_APIENTRY* const& glDebugMessageControlARB) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageInsertARB) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageCallbackARB) (GLDEBUGPROCARB callback, const void *userParam);
extern GLuint       (KHRONOS_APIENTRY* const& glGetDebugMessageLogARB) (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
#endif

#ifndef GL_ARB_depth_buffer_float
#define GL_ARB_depth_buffer_float 1
#endif

#ifndef GL_ARB_depth_clamp
#define GL_ARB_depth_clamp 1
#endif

#ifndef GL_ARB_depth_texture
#define GL_ARB_depth_texture 1
enum : GLenum
{
    GL_DEPTH_COMPONENT16_ARB                                = 0x81A5,
    GL_DEPTH_COMPONENT24_ARB                                = 0x81A6,
    GL_DEPTH_COMPONENT32_ARB                                = 0x81A7,
    GL_TEXTURE_DEPTH_SIZE_ARB                               = 0x884A,
    GL_DEPTH_TEXTURE_MODE_ARB                               = 0x884B,
};
#endif

#ifndef GL_ARB_derivative_control
#define GL_ARB_derivative_control 1
#endif

#ifndef GL_ARB_direct_state_access
#define GL_ARB_direct_state_access 1
#endif

#ifndef GL_ARB_draw_buffers
#define GL_ARB_draw_buffers 1
enum : GLenum
{
    GL_MAX_DRAW_BUFFERS_ARB                                 = 0x8824,
    GL_DRAW_BUFFER0_ARB                                     = 0x8825,
    GL_DRAW_BUFFER1_ARB                                     = 0x8826,
    GL_DRAW_BUFFER2_ARB                                     = 0x8827,
    GL_DRAW_BUFFER3_ARB                                     = 0x8828,
    GL_DRAW_BUFFER4_ARB                                     = 0x8829,
    GL_DRAW_BUFFER5_ARB                                     = 0x882A,
    GL_DRAW_BUFFER6_ARB                                     = 0x882B,
    GL_DRAW_BUFFER7_ARB                                     = 0x882C,
    GL_DRAW_BUFFER8_ARB                                     = 0x882D,
    GL_DRAW_BUFFER9_ARB                                     = 0x882E,
    GL_DRAW_BUFFER10_ARB                                    = 0x882F,
    GL_DRAW_BUFFER11_ARB                                    = 0x8830,
    GL_DRAW_BUFFER12_ARB                                    = 0x8831,
    GL_DRAW_BUFFER13_ARB                                    = 0x8832,
    GL_DRAW_BUFFER14_ARB                                    = 0x8833,
    GL_DRAW_BUFFER15_ARB                                    = 0x8834,
};
extern void         (KHRONOS_APIENTRY* const& glDrawBuffersARB) (GLsizei n, const GLenum *bufs);
#endif

#ifndef GL_ARB_draw_buffers_blend
#define GL_ARB_draw_buffers_blend 1
extern void         (KHRONOS_APIENTRY* const& glBlendEquationiARB) (GLuint buf, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparateiARB) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
extern void         (KHRONOS_APIENTRY* const& glBlendFunciARB) (GLuint buf, GLenum src, GLenum dst);
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparateiARB) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
#endif

#ifndef GL_ARB_draw_elements_base_vertex
#define GL_ARB_draw_elements_base_vertex 1
#endif

#ifndef GL_ARB_draw_indirect
#define GL_ARB_draw_indirect 1
#endif

#ifndef GL_ARB_draw_instanced
#define GL_ARB_draw_instanced 1
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstancedARB) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedARB) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#endif

#ifndef GL_ARB_enhanced_layouts
#define GL_ARB_enhanced_layouts 1
#endif

#ifndef GL_ARB_explicit_attrib_location
#define GL_ARB_explicit_attrib_location 1
#endif

#ifndef GL_ARB_explicit_uniform_location
#define GL_ARB_explicit_uniform_location 1
#endif

#ifndef GL_ARB_fragment_coord_conventions
#define GL_ARB_fragment_coord_conventions 1
#endif

#ifndef GL_ARB_fragment_layer_viewport
#define GL_ARB_fragment_layer_viewport 1
#endif

#ifndef GL_ARB_fragment_program
#define GL_ARB_fragment_program 1
enum : GLenum
{
    GL_FRAGMENT_PROGRAM_ARB                                 = 0x8804,
    GL_PROGRAM_FORMAT_ASCII_ARB                             = 0x8875,
    GL_PROGRAM_LENGTH_ARB                                   = 0x8627,
    GL_PROGRAM_FORMAT_ARB                                   = 0x8876,
    GL_PROGRAM_BINDING_ARB                                  = 0x8677,
    GL_PROGRAM_INSTRUCTIONS_ARB                             = 0x88A0,
    GL_MAX_PROGRAM_INSTRUCTIONS_ARB                         = 0x88A1,
    GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB                      = 0x88A2,
    GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB                  = 0x88A3,
    GL_PROGRAM_TEMPORARIES_ARB                              = 0x88A4,
    GL_MAX_PROGRAM_TEMPORARIES_ARB                          = 0x88A5,
    GL_PROGRAM_NATIVE_TEMPORARIES_ARB                       = 0x88A6,
    GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB                   = 0x88A7,
    GL_PROGRAM_PARAMETERS_ARB                               = 0x88A8,
    GL_MAX_PROGRAM_PARAMETERS_ARB                           = 0x88A9,
    GL_PROGRAM_NATIVE_PARAMETERS_ARB                        = 0x88AA,
    GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB                    = 0x88AB,
    GL_PROGRAM_ATTRIBS_ARB                                  = 0x88AC,
    GL_MAX_PROGRAM_ATTRIBS_ARB                              = 0x88AD,
    GL_PROGRAM_NATIVE_ATTRIBS_ARB                           = 0x88AE,
    GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB                       = 0x88AF,
    GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB                     = 0x88B4,
    GL_MAX_PROGRAM_ENV_PARAMETERS_ARB                       = 0x88B5,
    GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB                      = 0x88B6,
    GL_PROGRAM_ALU_INSTRUCTIONS_ARB                         = 0x8805,
    GL_PROGRAM_TEX_INSTRUCTIONS_ARB                         = 0x8806,
    GL_PROGRAM_TEX_INDIRECTIONS_ARB                         = 0x8807,
    GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB                  = 0x8808,
    GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB                  = 0x8809,
    GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB                  = 0x880A,
    GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB                     = 0x880B,
    GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB                     = 0x880C,
    GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB                     = 0x880D,
    GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB              = 0x880E,
    GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB              = 0x880F,
    GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB              = 0x8810,
    GL_PROGRAM_STRING_ARB                                   = 0x8628,
    GL_PROGRAM_ERROR_POSITION_ARB                           = 0x864B,
    GL_CURRENT_MATRIX_ARB                                   = 0x8641,
    GL_TRANSPOSE_CURRENT_MATRIX_ARB                         = 0x88B7,
    GL_CURRENT_MATRIX_STACK_DEPTH_ARB                       = 0x8640,
    GL_MAX_PROGRAM_MATRICES_ARB                             = 0x862F,
    GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB                   = 0x862E,
    GL_MAX_TEXTURE_COORDS_ARB                               = 0x8871,
    GL_MAX_TEXTURE_IMAGE_UNITS_ARB                          = 0x8872,
    GL_PROGRAM_ERROR_STRING_ARB                             = 0x8874,
    GL_MATRIX0_ARB                                          = 0x88C0,
    GL_MATRIX1_ARB                                          = 0x88C1,
    GL_MATRIX2_ARB                                          = 0x88C2,
    GL_MATRIX3_ARB                                          = 0x88C3,
    GL_MATRIX4_ARB                                          = 0x88C4,
    GL_MATRIX5_ARB                                          = 0x88C5,
    GL_MATRIX6_ARB                                          = 0x88C6,
    GL_MATRIX7_ARB                                          = 0x88C7,
    GL_MATRIX8_ARB                                          = 0x88C8,
    GL_MATRIX9_ARB                                          = 0x88C9,
    GL_MATRIX10_ARB                                         = 0x88CA,
    GL_MATRIX11_ARB                                         = 0x88CB,
    GL_MATRIX12_ARB                                         = 0x88CC,
    GL_MATRIX13_ARB                                         = 0x88CD,
    GL_MATRIX14_ARB                                         = 0x88CE,
    GL_MATRIX15_ARB                                         = 0x88CF,
    GL_MATRIX16_ARB                                         = 0x88D0,
    GL_MATRIX17_ARB                                         = 0x88D1,
    GL_MATRIX18_ARB                                         = 0x88D2,
    GL_MATRIX19_ARB                                         = 0x88D3,
    GL_MATRIX20_ARB                                         = 0x88D4,
    GL_MATRIX21_ARB                                         = 0x88D5,
    GL_MATRIX22_ARB                                         = 0x88D6,
    GL_MATRIX23_ARB                                         = 0x88D7,
    GL_MATRIX24_ARB                                         = 0x88D8,
    GL_MATRIX25_ARB                                         = 0x88D9,
    GL_MATRIX26_ARB                                         = 0x88DA,
    GL_MATRIX27_ARB                                         = 0x88DB,
    GL_MATRIX28_ARB                                         = 0x88DC,
    GL_MATRIX29_ARB                                         = 0x88DD,
    GL_MATRIX30_ARB                                         = 0x88DE,
    GL_MATRIX31_ARB                                         = 0x88DF,
};
extern void         (KHRONOS_APIENTRY* const& glProgramStringARB) (GLenum target, GLenum format, GLsizei len, const void *string);
extern void         (KHRONOS_APIENTRY* const& glBindProgramARB) (GLenum target, GLuint program);
extern void         (KHRONOS_APIENTRY* const& glDeleteProgramsARB) (GLsizei n, const GLuint *programs);
extern void         (KHRONOS_APIENTRY* const& glGenProgramsARB) (GLsizei n, GLuint *programs);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameter4dARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameter4dvARB) (GLenum target, GLuint index, const GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameter4fARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameter4fvARB) (GLenum target, GLuint index, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameter4dARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameter4dvARB) (GLenum target, GLuint index, const GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameter4fARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameter4fvARB) (GLenum target, GLuint index, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramEnvParameterdvARB) (GLenum target, GLuint index, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramEnvParameterfvARB) (GLenum target, GLuint index, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramLocalParameterdvARB) (GLenum target, GLuint index, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramLocalParameterfvARB) (GLenum target, GLuint index, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramivARB) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramStringARB) (GLenum target, GLenum pname, void *string);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsProgramARB) (GLuint program);
#endif

#ifndef GL_ARB_fragment_program_shadow
#define GL_ARB_fragment_program_shadow 1
#endif

#ifndef GL_ARB_fragment_shader
#define GL_ARB_fragment_shader 1
enum : GLenum
{
    GL_FRAGMENT_SHADER_ARB                                  = 0x8B30,
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB                  = 0x8B49,
    GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB                  = 0x8B8B,
};
#endif

#ifndef GL_ARB_fragment_shader_interlock
#define GL_ARB_fragment_shader_interlock 1
#endif

#ifndef GL_ARB_framebuffer_no_attachments
#define GL_ARB_framebuffer_no_attachments 1
#endif

#ifndef GL_ARB_framebuffer_object
#define GL_ARB_framebuffer_object 1
#endif

#ifndef GL_ARB_framebuffer_sRGB
#define GL_ARB_framebuffer_sRGB 1
#endif

#ifndef GL_ARB_geometry_shader4
#define GL_ARB_geometry_shader4 1
enum : GLenum
{
    GL_LINES_ADJACENCY_ARB                                  = 0x000A,
    GL_LINE_STRIP_ADJACENCY_ARB                             = 0x000B,
    GL_TRIANGLES_ADJACENCY_ARB                              = 0x000C,
    GL_TRIANGLE_STRIP_ADJACENCY_ARB                         = 0x000D,
    GL_PROGRAM_POINT_SIZE_ARB                               = 0x8642,
    GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB                 = 0x8C29,
    GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB                   = 0x8DA7,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB             = 0x8DA8,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB               = 0x8DA9,
    GL_GEOMETRY_SHADER_ARB                                  = 0x8DD9,
    GL_GEOMETRY_VERTICES_OUT_ARB                            = 0x8DDA,
    GL_GEOMETRY_INPUT_TYPE_ARB                              = 0x8DDB,
    GL_GEOMETRY_OUTPUT_TYPE_ARB                             = 0x8DDC,
    GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB                  = 0x8DDD,
    GL_MAX_VERTEX_VARYING_COMPONENTS_ARB                    = 0x8DDE,
    GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB                  = 0x8DDF,
    GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB                     = 0x8DE0,
    GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB             = 0x8DE1,
};
extern void         (KHRONOS_APIENTRY* const& glProgramParameteriARB) (GLuint program, GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureARB) (GLenum target, GLenum attachment, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureLayerARB) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureFaceARB) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#ifndef GL_ARB_get_program_binary
#define GL_ARB_get_program_binary 1
#endif

#ifndef GL_ARB_get_texture_sub_image
#define GL_ARB_get_texture_sub_image 1
#endif

#ifndef GL_ARB_gl_spirv
#define GL_ARB_gl_spirv 1
enum : GLenum
{
    GL_SHADER_BINARY_FORMAT_SPIR_V_ARB                      = 0x9551,
    GL_SPIR_V_BINARY_ARB                                    = 0x9552,
};
extern void         (KHRONOS_APIENTRY* const& glSpecializeShaderARB) (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue);
#endif

#ifndef GL_ARB_gpu_shader5
#define GL_ARB_gpu_shader5 1
#endif

#ifndef GL_ARB_gpu_shader_fp64
#define GL_ARB_gpu_shader_fp64 1
#endif

#ifndef GL_ARB_gpu_shader_int64
#define GL_ARB_gpu_shader_int64 1
enum : GLenum
{
    GL_INT64_ARB                                            = 0x140E,
    GL_INT64_VEC2_ARB                                       = 0x8FE9,
    GL_INT64_VEC3_ARB                                       = 0x8FEA,
    GL_INT64_VEC4_ARB                                       = 0x8FEB,
    GL_UNSIGNED_INT64_VEC2_ARB                              = 0x8FF5,
    GL_UNSIGNED_INT64_VEC3_ARB                              = 0x8FF6,
    GL_UNSIGNED_INT64_VEC4_ARB                              = 0x8FF7,
};
extern void         (KHRONOS_APIENTRY* const& glUniform1i64ARB) (GLint location, GLint64 x);
extern void         (KHRONOS_APIENTRY* const& glUniform2i64ARB) (GLint location, GLint64 x, GLint64 y);
extern void         (KHRONOS_APIENTRY* const& glUniform3i64ARB) (GLint location, GLint64 x, GLint64 y, GLint64 z);
extern void         (KHRONOS_APIENTRY* const& glUniform4i64ARB) (GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w);
extern void         (KHRONOS_APIENTRY* const& glUniform1i64vARB) (GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2i64vARB) (GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3i64vARB) (GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4i64vARB) (GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glUniform1ui64ARB) (GLint location, GLuint64 x);
extern void         (KHRONOS_APIENTRY* const& glUniform2ui64ARB) (GLint location, GLuint64 x, GLuint64 y);
extern void         (KHRONOS_APIENTRY* const& glUniform3ui64ARB) (GLint location, GLuint64 x, GLuint64 y, GLuint64 z);
extern void         (KHRONOS_APIENTRY* const& glUniform4ui64ARB) (GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w);
extern void         (KHRONOS_APIENTRY* const& glUniform1ui64vARB) (GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2ui64vARB) (GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3ui64vARB) (GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4ui64vARB) (GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glGetUniformi64vARB) (GLuint program, GLint location, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetUniformui64vARB) (GLuint program, GLint location, GLuint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformi64vARB) (GLuint program, GLint location, GLsizei bufSize, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformui64vARB) (GLuint program, GLint location, GLsizei bufSize, GLuint64 *params);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1i64ARB) (GLuint program, GLint location, GLint64 x);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2i64ARB) (GLuint program, GLint location, GLint64 x, GLint64 y);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3i64ARB) (GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4i64ARB) (GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1i64vARB) (GLuint program, GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2i64vARB) (GLuint program, GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3i64vARB) (GLuint program, GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4i64vARB) (GLuint program, GLint location, GLsizei count, const GLint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ui64ARB) (GLuint program, GLint location, GLuint64 x);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ui64ARB) (GLuint program, GLint location, GLuint64 x, GLuint64 y);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ui64ARB) (GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ui64ARB) (GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ui64vARB) (GLuint program, GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ui64vARB) (GLuint program, GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ui64vARB) (GLuint program, GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ui64vARB) (GLuint program, GLint location, GLsizei count, const GLuint64 *value);
#endif

#ifndef GL_ARB_half_float_pixel
#define GL_ARB_half_float_pixel 1
enum : GLenum
{
    GL_HALF_FLOAT_ARB                                       = 0x140B,
};
#endif

#ifndef GL_ARB_half_float_vertex
#define GL_ARB_half_float_vertex 1
#endif

#ifndef GL_ARB_imaging
#define GL_ARB_imaging 1
enum : GLenum
{
    GL_CONVOLUTION_BORDER_MODE                              = 0x8013,
    GL_CONVOLUTION_FILTER_SCALE                             = 0x8014,
    GL_CONVOLUTION_FILTER_BIAS                              = 0x8015,
    GL_REDUCE                                               = 0x8016,
    GL_CONVOLUTION_FORMAT                                   = 0x8017,
    GL_CONVOLUTION_WIDTH                                    = 0x8018,
    GL_CONVOLUTION_HEIGHT                                   = 0x8019,
    GL_MAX_CONVOLUTION_WIDTH                                = 0x801A,
    GL_MAX_CONVOLUTION_HEIGHT                               = 0x801B,
    GL_POST_CONVOLUTION_RED_SCALE                           = 0x801C,
    GL_POST_CONVOLUTION_GREEN_SCALE                         = 0x801D,
    GL_POST_CONVOLUTION_BLUE_SCALE                          = 0x801E,
    GL_POST_CONVOLUTION_ALPHA_SCALE                         = 0x801F,
    GL_POST_CONVOLUTION_RED_BIAS                            = 0x8020,
    GL_POST_CONVOLUTION_GREEN_BIAS                          = 0x8021,
    GL_POST_CONVOLUTION_BLUE_BIAS                           = 0x8022,
    GL_POST_CONVOLUTION_ALPHA_BIAS                          = 0x8023,
    GL_HISTOGRAM_WIDTH                                      = 0x8026,
    GL_HISTOGRAM_FORMAT                                     = 0x8027,
    GL_HISTOGRAM_RED_SIZE                                   = 0x8028,
    GL_HISTOGRAM_GREEN_SIZE                                 = 0x8029,
    GL_HISTOGRAM_BLUE_SIZE                                  = 0x802A,
    GL_HISTOGRAM_ALPHA_SIZE                                 = 0x802B,
    GL_HISTOGRAM_LUMINANCE_SIZE                             = 0x802C,
    GL_HISTOGRAM_SINK                                       = 0x802D,
    GL_MINMAX_FORMAT                                        = 0x802F,
    GL_MINMAX_SINK                                          = 0x8030,
    GL_TABLE_TOO_LARGE                                      = 0x8031,
    GL_COLOR_MATRIX                                         = 0x80B1,
    GL_COLOR_MATRIX_STACK_DEPTH                             = 0x80B2,
    GL_MAX_COLOR_MATRIX_STACK_DEPTH                         = 0x80B3,
    GL_POST_COLOR_MATRIX_RED_SCALE                          = 0x80B4,
    GL_POST_COLOR_MATRIX_GREEN_SCALE                        = 0x80B5,
    GL_POST_COLOR_MATRIX_BLUE_SCALE                         = 0x80B6,
    GL_POST_COLOR_MATRIX_ALPHA_SCALE                        = 0x80B7,
    GL_POST_COLOR_MATRIX_RED_BIAS                           = 0x80B8,
    GL_POST_COLOR_MATRIX_GREEN_BIAS                         = 0x80B9,
    GL_POST_COLOR_MATRIX_BLUE_BIAS                          = 0x80BA,
    GL_POST_COLOR_MATRIX_ALPHA_BIAS                         = 0x80BB,
    GL_COLOR_TABLE_SCALE                                    = 0x80D6,
    GL_COLOR_TABLE_BIAS                                     = 0x80D7,
    GL_COLOR_TABLE_FORMAT                                   = 0x80D8,
    GL_COLOR_TABLE_WIDTH                                    = 0x80D9,
    GL_COLOR_TABLE_RED_SIZE                                 = 0x80DA,
    GL_COLOR_TABLE_GREEN_SIZE                               = 0x80DB,
    GL_COLOR_TABLE_BLUE_SIZE                                = 0x80DC,
    GL_COLOR_TABLE_ALPHA_SIZE                               = 0x80DD,
    GL_COLOR_TABLE_LUMINANCE_SIZE                           = 0x80DE,
    GL_COLOR_TABLE_INTENSITY_SIZE                           = 0x80DF,
    GL_CONSTANT_BORDER                                      = 0x8151,
    GL_REPLICATE_BORDER                                     = 0x8153,
    GL_CONVOLUTION_BORDER_COLOR                             = 0x8154,
};
extern void         (KHRONOS_APIENTRY* const& glColorTable) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *table);
extern void         (KHRONOS_APIENTRY* const& glColorTableParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glColorTableParameteriv) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCopyColorTable) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glGetColorTable) (GLenum target, GLenum format, GLenum type, void *table);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableParameterfv) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glColorSubTable) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCopyColorSubTable) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glConvolutionFilter1D) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *image);
extern void         (KHRONOS_APIENTRY* const& glConvolutionFilter2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *image);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameterf) (GLenum target, GLenum pname, GLfloat params);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameteri) (GLenum target, GLenum pname, GLint params);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameteriv) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCopyConvolutionFilter1D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glCopyConvolutionFilter2D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetConvolutionFilter) (GLenum target, GLenum format, GLenum type, void *image);
extern void         (KHRONOS_APIENTRY* const& glGetConvolutionParameterfv) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetConvolutionParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSeparableFilter) (GLenum target, GLenum format, GLenum type, void *row, void *column, void *span);
extern void         (KHRONOS_APIENTRY* const& glSeparableFilter2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *row, const void *column);
extern void         (KHRONOS_APIENTRY* const& glGetHistogram) (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values);
extern void         (KHRONOS_APIENTRY* const& glGetHistogramParameterfv) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetHistogramParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMinmax) (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values);
extern void         (KHRONOS_APIENTRY* const& glGetMinmaxParameterfv) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMinmaxParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glHistogram) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
extern void         (KHRONOS_APIENTRY* const& glMinmax) (GLenum target, GLenum internalformat, GLboolean sink);
extern void         (KHRONOS_APIENTRY* const& glResetHistogram) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glResetMinmax) (GLenum target);
#endif

#ifndef GL_ARB_indirect_parameters
#define GL_ARB_indirect_parameters 1
enum : GLenum
{
    GL_PARAMETER_BUFFER_ARB                                 = 0x80EE,
    GL_PARAMETER_BUFFER_BINDING_ARB                         = 0x80EF,
};
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysIndirectCountARB) (GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsIndirectCountARB) (GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
#endif

#ifndef GL_ARB_instanced_arrays
#define GL_ARB_instanced_arrays 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB                      = 0x88FE,
};
extern void         (KHRONOS_APIENTRY* const& glVertexAttribDivisorARB) (GLuint index, GLuint divisor);
#endif

#ifndef GL_ARB_internalformat_query
#define GL_ARB_internalformat_query 1
#endif

#ifndef GL_ARB_internalformat_query2
#define GL_ARB_internalformat_query2 1
enum : GLenum
{
    GL_SRGB_DECODE_ARB                                      = 0x8299,
    GL_VIEW_CLASS_EAC_R11                                   = 0x9383,
    GL_VIEW_CLASS_EAC_RG11                                  = 0x9384,
    GL_VIEW_CLASS_ETC2_RGB                                  = 0x9385,
    GL_VIEW_CLASS_ETC2_RGBA                                 = 0x9386,
    GL_VIEW_CLASS_ETC2_EAC_RGBA                             = 0x9387,
    GL_VIEW_CLASS_ASTC_4x4_RGBA                             = 0x9388,
    GL_VIEW_CLASS_ASTC_5x4_RGBA                             = 0x9389,
    GL_VIEW_CLASS_ASTC_5x5_RGBA                             = 0x938A,
    GL_VIEW_CLASS_ASTC_6x5_RGBA                             = 0x938B,
    GL_VIEW_CLASS_ASTC_6x6_RGBA                             = 0x938C,
    GL_VIEW_CLASS_ASTC_8x5_RGBA                             = 0x938D,
    GL_VIEW_CLASS_ASTC_8x6_RGBA                             = 0x938E,
    GL_VIEW_CLASS_ASTC_8x8_RGBA                             = 0x938F,
    GL_VIEW_CLASS_ASTC_10x5_RGBA                            = 0x9390,
    GL_VIEW_CLASS_ASTC_10x6_RGBA                            = 0x9391,
    GL_VIEW_CLASS_ASTC_10x8_RGBA                            = 0x9392,
    GL_VIEW_CLASS_ASTC_10x10_RGBA                           = 0x9393,
    GL_VIEW_CLASS_ASTC_12x10_RGBA                           = 0x9394,
    GL_VIEW_CLASS_ASTC_12x12_RGBA                           = 0x9395,
};
#endif

#ifndef GL_ARB_invalidate_subdata
#define GL_ARB_invalidate_subdata 1
#endif

#ifndef GL_ARB_map_buffer_alignment
#define GL_ARB_map_buffer_alignment 1
#endif

#ifndef GL_ARB_map_buffer_range
#define GL_ARB_map_buffer_range 1
#endif

#ifndef GL_ARB_matrix_palette
#define GL_ARB_matrix_palette 1
enum : GLenum
{
    GL_MATRIX_PALETTE_ARB                                   = 0x8840,
    GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB                   = 0x8841,
    GL_MAX_PALETTE_MATRICES_ARB                             = 0x8842,
    GL_CURRENT_PALETTE_MATRIX_ARB                           = 0x8843,
    GL_MATRIX_INDEX_ARRAY_ARB                               = 0x8844,
    GL_CURRENT_MATRIX_INDEX_ARB                             = 0x8845,
    GL_MATRIX_INDEX_ARRAY_SIZE_ARB                          = 0x8846,
    GL_MATRIX_INDEX_ARRAY_TYPE_ARB                          = 0x8847,
    GL_MATRIX_INDEX_ARRAY_STRIDE_ARB                        = 0x8848,
    GL_MATRIX_INDEX_ARRAY_POINTER_ARB                       = 0x8849,
};
extern void         (KHRONOS_APIENTRY* const& glCurrentPaletteMatrixARB) (GLint index);
extern void         (KHRONOS_APIENTRY* const& glMatrixIndexubvARB) (GLint size, const GLubyte *indices);
extern void         (KHRONOS_APIENTRY* const& glMatrixIndexusvARB) (GLint size, const GLushort *indices);
extern void         (KHRONOS_APIENTRY* const& glMatrixIndexuivARB) (GLint size, const GLuint *indices);
extern void         (KHRONOS_APIENTRY* const& glMatrixIndexPointerARB) (GLint size, GLenum type, GLsizei stride, const void *pointer);
#endif

#ifndef GL_ARB_multi_bind
#define GL_ARB_multi_bind 1
#endif

#ifndef GL_ARB_multi_draw_indirect
#define GL_ARB_multi_draw_indirect 1
#endif

#ifndef GL_ARB_multisample
#define GL_ARB_multisample 1
enum : GLenum
{
    GL_MULTISAMPLE_ARB                                      = 0x809D,
    GL_SAMPLE_ALPHA_TO_COVERAGE_ARB                         = 0x809E,
    GL_SAMPLE_ALPHA_TO_ONE_ARB                              = 0x809F,
    GL_SAMPLE_COVERAGE_ARB                                  = 0x80A0,
    GL_SAMPLE_BUFFERS_ARB                                   = 0x80A8,
    GL_SAMPLES_ARB                                          = 0x80A9,
    GL_SAMPLE_COVERAGE_VALUE_ARB                            = 0x80AA,
    GL_SAMPLE_COVERAGE_INVERT_ARB                           = 0x80AB,
    GL_MULTISAMPLE_BIT_ARB                                  = 0x20000000,
};
extern void         (KHRONOS_APIENTRY* const& glSampleCoverageARB) (GLfloat value, GLboolean invert);
#endif

#ifndef GL_ARB_multitexture
#define GL_ARB_multitexture 1
enum : GLenum
{
    GL_TEXTURE0_ARB                                         = 0x84C0,
    GL_TEXTURE1_ARB                                         = 0x84C1,
    GL_TEXTURE2_ARB                                         = 0x84C2,
    GL_TEXTURE3_ARB                                         = 0x84C3,
    GL_TEXTURE4_ARB                                         = 0x84C4,
    GL_TEXTURE5_ARB                                         = 0x84C5,
    GL_TEXTURE6_ARB                                         = 0x84C6,
    GL_TEXTURE7_ARB                                         = 0x84C7,
    GL_TEXTURE8_ARB                                         = 0x84C8,
    GL_TEXTURE9_ARB                                         = 0x84C9,
    GL_TEXTURE10_ARB                                        = 0x84CA,
    GL_TEXTURE11_ARB                                        = 0x84CB,
    GL_TEXTURE12_ARB                                        = 0x84CC,
    GL_TEXTURE13_ARB                                        = 0x84CD,
    GL_TEXTURE14_ARB                                        = 0x84CE,
    GL_TEXTURE15_ARB                                        = 0x84CF,
    GL_TEXTURE16_ARB                                        = 0x84D0,
    GL_TEXTURE17_ARB                                        = 0x84D1,
    GL_TEXTURE18_ARB                                        = 0x84D2,
    GL_TEXTURE19_ARB                                        = 0x84D3,
    GL_TEXTURE20_ARB                                        = 0x84D4,
    GL_TEXTURE21_ARB                                        = 0x84D5,
    GL_TEXTURE22_ARB                                        = 0x84D6,
    GL_TEXTURE23_ARB                                        = 0x84D7,
    GL_TEXTURE24_ARB                                        = 0x84D8,
    GL_TEXTURE25_ARB                                        = 0x84D9,
    GL_TEXTURE26_ARB                                        = 0x84DA,
    GL_TEXTURE27_ARB                                        = 0x84DB,
    GL_TEXTURE28_ARB                                        = 0x84DC,
    GL_TEXTURE29_ARB                                        = 0x84DD,
    GL_TEXTURE30_ARB                                        = 0x84DE,
    GL_TEXTURE31_ARB                                        = 0x84DF,
    GL_ACTIVE_TEXTURE_ARB                                   = 0x84E0,
    GL_CLIENT_ACTIVE_TEXTURE_ARB                            = 0x84E1,
    GL_MAX_TEXTURE_UNITS_ARB                                = 0x84E2,
};
extern void         (KHRONOS_APIENTRY* const& glActiveTextureARB) (GLenum texture);
extern void         (KHRONOS_APIENTRY* const& glClientActiveTextureARB) (GLenum texture);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1dARB) (GLenum target, GLdouble s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1dvARB) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1fARB) (GLenum target, GLfloat s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1fvARB) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1iARB) (GLenum target, GLint s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1ivARB) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1sARB) (GLenum target, GLshort s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1svARB) (GLenum target, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2dARB) (GLenum target, GLdouble s, GLdouble t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2dvARB) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2fvARB) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2iARB) (GLenum target, GLint s, GLint t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2ivARB) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2sARB) (GLenum target, GLshort s, GLshort t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2svARB) (GLenum target, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3dARB) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3dvARB) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3fARB) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3fvARB) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3iARB) (GLenum target, GLint s, GLint t, GLint r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3ivARB) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3sARB) (GLenum target, GLshort s, GLshort t, GLshort r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3svARB) (GLenum target, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4dARB) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4dvARB) (GLenum target, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4fARB) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4fvARB) (GLenum target, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4iARB) (GLenum target, GLint s, GLint t, GLint r, GLint q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4ivARB) (GLenum target, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4sARB) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4svARB) (GLenum target, const GLshort *v);
#endif

#ifndef GL_ARB_occlusion_query
#define GL_ARB_occlusion_query 1
enum : GLenum
{
    GL_QUERY_COUNTER_BITS_ARB                               = 0x8864,
    GL_CURRENT_QUERY_ARB                                    = 0x8865,
    GL_QUERY_RESULT_ARB                                     = 0x8866,
    GL_QUERY_RESULT_AVAILABLE_ARB                           = 0x8867,
    GL_SAMPLES_PASSED_ARB                                   = 0x8914,
};
extern void         (KHRONOS_APIENTRY* const& glGenQueriesARB) (GLsizei n, GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glDeleteQueriesARB) (GLsizei n, const GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsQueryARB) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glBeginQueryARB) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glEndQueryARB) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetQueryivARB) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectivARB) (GLuint id, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectuivARB) (GLuint id, GLenum pname, GLuint *params);
#endif

#ifndef GL_ARB_occlusion_query2
#define GL_ARB_occlusion_query2 1
#endif

#ifndef GL_ARB_parallel_shader_compile
#define GL_ARB_parallel_shader_compile 1
enum : GLenum
{
    GL_MAX_SHADER_COMPILER_THREADS_ARB                      = 0x91B0,
    GL_COMPLETION_STATUS_ARB                                = 0x91B1,
};
extern void         (KHRONOS_APIENTRY* const& glMaxShaderCompilerThreadsARB) (GLuint count);
#endif

#ifndef GL_ARB_pipeline_statistics_query
#define GL_ARB_pipeline_statistics_query 1
enum : GLenum
{
    GL_VERTICES_SUBMITTED_ARB                               = 0x82EE,
    GL_PRIMITIVES_SUBMITTED_ARB                             = 0x82EF,
    GL_VERTEX_SHADER_INVOCATIONS_ARB                        = 0x82F0,
    GL_TESS_CONTROL_SHADER_PATCHES_ARB                      = 0x82F1,
    GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB               = 0x82F2,
    GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB               = 0x82F3,
    GL_FRAGMENT_SHADER_INVOCATIONS_ARB                      = 0x82F4,
    GL_COMPUTE_SHADER_INVOCATIONS_ARB                       = 0x82F5,
    GL_CLIPPING_INPUT_PRIMITIVES_ARB                        = 0x82F6,
    GL_CLIPPING_OUTPUT_PRIMITIVES_ARB                       = 0x82F7,
};
#endif

#ifndef GL_ARB_pixel_buffer_object
#define GL_ARB_pixel_buffer_object 1
enum : GLenum
{
    GL_PIXEL_PACK_BUFFER_ARB                                = 0x88EB,
    GL_PIXEL_UNPACK_BUFFER_ARB                              = 0x88EC,
    GL_PIXEL_PACK_BUFFER_BINDING_ARB                        = 0x88ED,
    GL_PIXEL_UNPACK_BUFFER_BINDING_ARB                      = 0x88EF,
};
#endif

#ifndef GL_ARB_point_parameters
#define GL_ARB_point_parameters 1
enum : GLenum
{
    GL_POINT_SIZE_MIN_ARB                                   = 0x8126,
    GL_POINT_SIZE_MAX_ARB                                   = 0x8127,
    GL_POINT_FADE_THRESHOLD_SIZE_ARB                        = 0x8128,
    GL_POINT_DISTANCE_ATTENUATION_ARB                       = 0x8129,
};
extern void         (KHRONOS_APIENTRY* const& glPointParameterfARB) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPointParameterfvARB) (GLenum pname, const GLfloat *params);
#endif

#ifndef GL_ARB_point_sprite
#define GL_ARB_point_sprite 1
enum : GLenum
{
    GL_POINT_SPRITE_ARB                                     = 0x8861,
    GL_COORD_REPLACE_ARB                                    = 0x8862,
};
#endif

#ifndef GL_ARB_polygon_offset_clamp
#define GL_ARB_polygon_offset_clamp 1
#endif

#ifndef GL_ARB_post_depth_coverage
#define GL_ARB_post_depth_coverage 1
#endif

#ifndef GL_ARB_program_interface_query
#define GL_ARB_program_interface_query 1
#endif

#ifndef GL_ARB_provoking_vertex
#define GL_ARB_provoking_vertex 1
#endif

#ifndef GL_ARB_query_buffer_object
#define GL_ARB_query_buffer_object 1
#endif

#ifndef GL_ARB_robust_buffer_access_behavior
#define GL_ARB_robust_buffer_access_behavior 1
#endif

#ifndef GL_ARB_robustness
#define GL_ARB_robustness 1
enum : GLenum
{
    GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB                   = 0x00000004,
    GL_LOSE_CONTEXT_ON_RESET_ARB                            = 0x8252,
    GL_GUILTY_CONTEXT_RESET_ARB                             = 0x8253,
    GL_INNOCENT_CONTEXT_RESET_ARB                           = 0x8254,
    GL_UNKNOWN_CONTEXT_RESET_ARB                            = 0x8255,
    GL_RESET_NOTIFICATION_STRATEGY_ARB                      = 0x8256,
    GL_NO_RESET_NOTIFICATION_ARB                            = 0x8261,
};
extern GLenum       (KHRONOS_APIENTRY* const& glGetGraphicsResetStatusARB) ();
extern void         (KHRONOS_APIENTRY* const& glGetnTexImageARB) (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *img);
extern void         (KHRONOS_APIENTRY* const& glReadnPixelsARB) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
extern void         (KHRONOS_APIENTRY* const& glGetnCompressedTexImageARB) (GLenum target, GLint lod, GLsizei bufSize, void *img);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformfvARB) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformivARB) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformuivARB) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformdvARB) (GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetnMapdvARB) (GLenum target, GLenum query, GLsizei bufSize, GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glGetnMapfvARB) (GLenum target, GLenum query, GLsizei bufSize, GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glGetnMapivARB) (GLenum target, GLenum query, GLsizei bufSize, GLint *v);
extern void         (KHRONOS_APIENTRY* const& glGetnPixelMapfvARB) (GLenum map, GLsizei bufSize, GLfloat *values);
extern void         (KHRONOS_APIENTRY* const& glGetnPixelMapuivARB) (GLenum map, GLsizei bufSize, GLuint *values);
extern void         (KHRONOS_APIENTRY* const& glGetnPixelMapusvARB) (GLenum map, GLsizei bufSize, GLushort *values);
extern void         (KHRONOS_APIENTRY* const& glGetnPolygonStippleARB) (GLsizei bufSize, GLubyte *pattern);
extern void         (KHRONOS_APIENTRY* const& glGetnColorTableARB) (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table);
extern void         (KHRONOS_APIENTRY* const& glGetnConvolutionFilterARB) (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image);
extern void         (KHRONOS_APIENTRY* const& glGetnSeparableFilterARB) (GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span);
extern void         (KHRONOS_APIENTRY* const& glGetnHistogramARB) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
extern void         (KHRONOS_APIENTRY* const& glGetnMinmaxARB) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
#endif

#ifndef GL_ARB_robustness_isolation
#define GL_ARB_robustness_isolation 1
#endif

#ifndef GL_ARB_sample_locations
#define GL_ARB_sample_locations 1
enum : GLenum
{
    GL_SAMPLE_LOCATION_SUBPIXEL_BITS_ARB                    = 0x933D,
    GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_ARB                 = 0x933E,
    GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_ARB                = 0x933F,
    GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_ARB          = 0x9340,
    GL_SAMPLE_LOCATION_ARB                                  = 0x8E50,
    GL_PROGRAMMABLE_SAMPLE_LOCATION_ARB                     = 0x9341,
    GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_ARB        = 0x9342,
    GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_ARB           = 0x9343,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferSampleLocationsfvARB) (GLenum target, GLuint start, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferSampleLocationsfvARB) (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glEvaluateDepthValuesARB) ();
#endif

#ifndef GL_ARB_sample_shading
#define GL_ARB_sample_shading 1
enum : GLenum
{
    GL_SAMPLE_SHADING_ARB                                   = 0x8C36,
    GL_MIN_SAMPLE_SHADING_VALUE_ARB                         = 0x8C37,
};
extern void         (KHRONOS_APIENTRY* const& glMinSampleShadingARB) (GLfloat value);
#endif

#ifndef GL_ARB_sampler_objects
#define GL_ARB_sampler_objects 1
#endif

#ifndef GL_ARB_seamless_cube_map
#define GL_ARB_seamless_cube_map 1
#endif

#ifndef GL_ARB_seamless_cubemap_per_texture
#define GL_ARB_seamless_cubemap_per_texture 1
#endif

#ifndef GL_ARB_separate_shader_objects
#define GL_ARB_separate_shader_objects 1
#endif

#ifndef GL_ARB_shader_atomic_counter_ops
#define GL_ARB_shader_atomic_counter_ops 1
#endif

#ifndef GL_ARB_shader_atomic_counters
#define GL_ARB_shader_atomic_counters 1
#endif

#ifndef GL_ARB_shader_ballot
#define GL_ARB_shader_ballot 1
#endif

#ifndef GL_ARB_shader_bit_encoding
#define GL_ARB_shader_bit_encoding 1
#endif

#ifndef GL_ARB_shader_clock
#define GL_ARB_shader_clock 1
#endif

#ifndef GL_ARB_shader_draw_parameters
#define GL_ARB_shader_draw_parameters 1
#endif

#ifndef GL_ARB_shader_group_vote
#define GL_ARB_shader_group_vote 1
#endif

#ifndef GL_ARB_shader_image_load_store
#define GL_ARB_shader_image_load_store 1
#endif

#ifndef GL_ARB_shader_image_size
#define GL_ARB_shader_image_size 1
#endif

#ifndef GL_ARB_shader_objects
#define GL_ARB_shader_objects 1
enum : GLenum
{
    GL_PROGRAM_OBJECT_ARB                                   = 0x8B40,
    GL_SHADER_OBJECT_ARB                                    = 0x8B48,
    GL_OBJECT_TYPE_ARB                                      = 0x8B4E,
    GL_OBJECT_SUBTYPE_ARB                                   = 0x8B4F,
    GL_FLOAT_VEC2_ARB                                       = 0x8B50,
    GL_FLOAT_VEC3_ARB                                       = 0x8B51,
    GL_FLOAT_VEC4_ARB                                       = 0x8B52,
    GL_INT_VEC2_ARB                                         = 0x8B53,
    GL_INT_VEC3_ARB                                         = 0x8B54,
    GL_INT_VEC4_ARB                                         = 0x8B55,
    GL_BOOL_ARB                                             = 0x8B56,
    GL_BOOL_VEC2_ARB                                        = 0x8B57,
    GL_BOOL_VEC3_ARB                                        = 0x8B58,
    GL_BOOL_VEC4_ARB                                        = 0x8B59,
    GL_FLOAT_MAT2_ARB                                       = 0x8B5A,
    GL_FLOAT_MAT3_ARB                                       = 0x8B5B,
    GL_FLOAT_MAT4_ARB                                       = 0x8B5C,
    GL_SAMPLER_1D_ARB                                       = 0x8B5D,
    GL_SAMPLER_2D_ARB                                       = 0x8B5E,
    GL_SAMPLER_3D_ARB                                       = 0x8B5F,
    GL_SAMPLER_CUBE_ARB                                     = 0x8B60,
    GL_SAMPLER_1D_SHADOW_ARB                                = 0x8B61,
    GL_SAMPLER_2D_SHADOW_ARB                                = 0x8B62,
    GL_SAMPLER_2D_RECT_ARB                                  = 0x8B63,
    GL_SAMPLER_2D_RECT_SHADOW_ARB                           = 0x8B64,
    GL_OBJECT_DELETE_STATUS_ARB                             = 0x8B80,
    GL_OBJECT_COMPILE_STATUS_ARB                            = 0x8B81,
    GL_OBJECT_LINK_STATUS_ARB                               = 0x8B82,
    GL_OBJECT_VALIDATE_STATUS_ARB                           = 0x8B83,
    GL_OBJECT_INFO_LOG_LENGTH_ARB                           = 0x8B84,
    GL_OBJECT_ATTACHED_OBJECTS_ARB                          = 0x8B85,
    GL_OBJECT_ACTIVE_UNIFORMS_ARB                           = 0x8B86,
    GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB                 = 0x8B87,
    GL_OBJECT_SHADER_SOURCE_LENGTH_ARB                      = 0x8B88,
};
extern void         (KHRONOS_APIENTRY* const& glDeleteObjectARB) (GLhandleARB obj);
extern GLhandleARB  (KHRONOS_APIENTRY* const& glGetHandleARB) (GLenum pname);
extern void         (KHRONOS_APIENTRY* const& glDetachObjectARB) (GLhandleARB containerObj, GLhandleARB attachedObj);
extern GLhandleARB  (KHRONOS_APIENTRY* const& glCreateShaderObjectARB) (GLenum shaderType);
extern void         (KHRONOS_APIENTRY* const& glShaderSourceARB) (GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
extern void         (KHRONOS_APIENTRY* const& glCompileShaderARB) (GLhandleARB shaderObj);
extern GLhandleARB  (KHRONOS_APIENTRY* const& glCreateProgramObjectARB) ();
extern void         (KHRONOS_APIENTRY* const& glAttachObjectARB) (GLhandleARB containerObj, GLhandleARB obj);
extern void         (KHRONOS_APIENTRY* const& glLinkProgramARB) (GLhandleARB programObj);
extern void         (KHRONOS_APIENTRY* const& glUseProgramObjectARB) (GLhandleARB programObj);
extern void         (KHRONOS_APIENTRY* const& glValidateProgramARB) (GLhandleARB programObj);
extern void         (KHRONOS_APIENTRY* const& glUniform1fARB) (GLint location, GLfloat v0);
extern void         (KHRONOS_APIENTRY* const& glUniform2fARB) (GLint location, GLfloat v0, GLfloat v1);
extern void         (KHRONOS_APIENTRY* const& glUniform3fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glUniform4fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void         (KHRONOS_APIENTRY* const& glUniform1iARB) (GLint location, GLint v0);
extern void         (KHRONOS_APIENTRY* const& glUniform2iARB) (GLint location, GLint v0, GLint v1);
extern void         (KHRONOS_APIENTRY* const& glUniform3iARB) (GLint location, GLint v0, GLint v1, GLint v2);
extern void         (KHRONOS_APIENTRY* const& glUniform4iARB) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void         (KHRONOS_APIENTRY* const& glUniform1fvARB) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2fvARB) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3fvARB) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4fvARB) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform1ivARB) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2ivARB) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3ivARB) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4ivARB) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glGetObjectParameterfvARB) (GLhandleARB obj, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetObjectParameterivARB) (GLhandleARB obj, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetInfoLogARB) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
extern void         (KHRONOS_APIENTRY* const& glGetAttachedObjectsARB) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
extern GLint        (KHRONOS_APIENTRY* const& glGetUniformLocationARB) (GLhandleARB programObj, const GLcharARB *name);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformARB) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
extern void         (KHRONOS_APIENTRY* const& glGetUniformfvARB) (GLhandleARB programObj, GLint location, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetUniformivARB) (GLhandleARB programObj, GLint location, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetShaderSourceARB) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
#endif

#ifndef GL_ARB_shader_precision
#define GL_ARB_shader_precision 1
#endif

#ifndef GL_ARB_shader_stencil_export
#define GL_ARB_shader_stencil_export 1
#endif

#ifndef GL_ARB_shader_storage_buffer_object
#define GL_ARB_shader_storage_buffer_object 1
#endif

#ifndef GL_ARB_shader_subroutine
#define GL_ARB_shader_subroutine 1
#endif

#ifndef GL_ARB_shader_texture_image_samples
#define GL_ARB_shader_texture_image_samples 1
#endif

#ifndef GL_ARB_shader_texture_lod
#define GL_ARB_shader_texture_lod 1
#endif

#ifndef GL_ARB_shader_viewport_layer_array
#define GL_ARB_shader_viewport_layer_array 1
#endif

#ifndef GL_ARB_shading_language_100
#define GL_ARB_shading_language_100 1
enum : GLenum
{
    GL_SHADING_LANGUAGE_VERSION_ARB                         = 0x8B8C,
};
#endif

#ifndef GL_ARB_shading_language_420pack
#define GL_ARB_shading_language_420pack 1
#endif

#ifndef GL_ARB_shading_language_include
#define GL_ARB_shading_language_include 1
enum : GLenum
{
    GL_SHADER_INCLUDE_ARB                                   = 0x8DAE,
    GL_NAMED_STRING_LENGTH_ARB                              = 0x8DE9,
    GL_NAMED_STRING_TYPE_ARB                                = 0x8DEA,
};
extern void         (KHRONOS_APIENTRY* const& glNamedStringARB) (GLenum type, GLint namelen, const GLchar *name, GLint stringlen, const GLchar *string);
extern void         (KHRONOS_APIENTRY* const& glDeleteNamedStringARB) (GLint namelen, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glCompileShaderIncludeARB) (GLuint shader, GLsizei count, const GLchar *const*path, const GLint *length);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsNamedStringARB) (GLint namelen, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetNamedStringARB) (GLint namelen, const GLchar *name, GLsizei bufSize, GLint *stringlen, GLchar *string);
extern void         (KHRONOS_APIENTRY* const& glGetNamedStringivARB) (GLint namelen, const GLchar *name, GLenum pname, GLint *params);
#endif

#ifndef GL_ARB_shading_language_packing
#define GL_ARB_shading_language_packing 1
#endif

#ifndef GL_ARB_shadow
#define GL_ARB_shadow 1
enum : GLenum
{
    GL_TEXTURE_COMPARE_MODE_ARB                             = 0x884C,
    GL_TEXTURE_COMPARE_FUNC_ARB                             = 0x884D,
    GL_COMPARE_R_TO_TEXTURE_ARB                             = 0x884E,
};
#endif

#ifndef GL_ARB_shadow_ambient
#define GL_ARB_shadow_ambient 1
enum : GLenum
{
    GL_TEXTURE_COMPARE_FAIL_VALUE_ARB                       = 0x80BF,
};
#endif

#ifndef GL_ARB_sparse_buffer
#define GL_ARB_sparse_buffer 1
enum : GLenum
{
    GL_SPARSE_STORAGE_BIT_ARB                               = 0x0400,
    GL_SPARSE_BUFFER_PAGE_SIZE_ARB                          = 0x82F8,
};
extern void         (KHRONOS_APIENTRY* const& glBufferPageCommitmentARB) (GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferPageCommitmentEXT) (GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferPageCommitmentARB) (GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit);
#endif

#ifndef GL_ARB_sparse_texture
#define GL_ARB_sparse_texture 1
enum : GLenum
{
    GL_TEXTURE_SPARSE_ARB                                   = 0x91A6,
    GL_VIRTUAL_PAGE_SIZE_INDEX_ARB                          = 0x91A7,
    GL_NUM_SPARSE_LEVELS_ARB                                = 0x91AA,
    GL_NUM_VIRTUAL_PAGE_SIZES_ARB                           = 0x91A8,
    GL_VIRTUAL_PAGE_SIZE_X_ARB                              = 0x9195,
    GL_VIRTUAL_PAGE_SIZE_Y_ARB                              = 0x9196,
    GL_VIRTUAL_PAGE_SIZE_Z_ARB                              = 0x9197,
    GL_MAX_SPARSE_TEXTURE_SIZE_ARB                          = 0x9198,
    GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB                       = 0x9199,
    GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB                  = 0x919A,
    GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB           = 0x91A9,
};
extern void         (KHRONOS_APIENTRY* const& glTexPageCommitmentARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
#endif

#ifndef GL_ARB_sparse_texture2
#define GL_ARB_sparse_texture2 1
#endif

#ifndef GL_ARB_sparse_texture_clamp
#define GL_ARB_sparse_texture_clamp 1
#endif

#ifndef GL_ARB_spirv_extensions
#define GL_ARB_spirv_extensions 1
#endif

#ifndef GL_ARB_stencil_texturing
#define GL_ARB_stencil_texturing 1
#endif

#ifndef GL_ARB_sync
#define GL_ARB_sync 1
#endif

#ifndef GL_ARB_tessellation_shader
#define GL_ARB_tessellation_shader 1
#endif

#ifndef GL_ARB_texture_barrier
#define GL_ARB_texture_barrier 1
#endif

#ifndef GL_ARB_texture_border_clamp
#define GL_ARB_texture_border_clamp 1
enum : GLenum
{
    GL_CLAMP_TO_BORDER_ARB                                  = 0x812D,
};
#endif

#ifndef GL_ARB_texture_buffer_object
#define GL_ARB_texture_buffer_object 1
enum : GLenum
{
    GL_TEXTURE_BUFFER_ARB                                   = 0x8C2A,
    GL_MAX_TEXTURE_BUFFER_SIZE_ARB                          = 0x8C2B,
    GL_TEXTURE_BINDING_BUFFER_ARB                           = 0x8C2C,
    GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB                = 0x8C2D,
    GL_TEXTURE_BUFFER_FORMAT_ARB                            = 0x8C2E,
};
extern void         (KHRONOS_APIENTRY* const& glTexBufferARB) (GLenum target, GLenum internalformat, GLuint buffer);
#endif

#ifndef GL_ARB_texture_buffer_object_rgb32
#define GL_ARB_texture_buffer_object_rgb32 1
#endif

#ifndef GL_ARB_texture_buffer_range
#define GL_ARB_texture_buffer_range 1
#endif

#ifndef GL_ARB_texture_compression
#define GL_ARB_texture_compression 1
enum : GLenum
{
    GL_COMPRESSED_ALPHA_ARB                                 = 0x84E9,
    GL_COMPRESSED_LUMINANCE_ARB                             = 0x84EA,
    GL_COMPRESSED_LUMINANCE_ALPHA_ARB                       = 0x84EB,
    GL_COMPRESSED_INTENSITY_ARB                             = 0x84EC,
    GL_COMPRESSED_RGB_ARB                                   = 0x84ED,
    GL_COMPRESSED_RGBA_ARB                                  = 0x84EE,
    GL_TEXTURE_COMPRESSION_HINT_ARB                         = 0x84EF,
    GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB                    = 0x86A0,
    GL_TEXTURE_COMPRESSED_ARB                               = 0x86A1,
    GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB                   = 0x86A2,
    GL_COMPRESSED_TEXTURE_FORMATS_ARB                       = 0x86A3,
};
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage3DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage2DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage1DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage3DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage2DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage1DARB) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glGetCompressedTexImageARB) (GLenum target, GLint level, void *img);
#endif

#ifndef GL_ARB_texture_compression_bptc
#define GL_ARB_texture_compression_bptc 1
enum : GLenum
{
    GL_COMPRESSED_RGBA_BPTC_UNORM_ARB                       = 0x8E8C,
    GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB                 = 0x8E8D,
    GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB                 = 0x8E8E,
    GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB               = 0x8E8F,
};
#endif

#ifndef GL_ARB_texture_compression_rgtc
#define GL_ARB_texture_compression_rgtc 1
#endif

#ifndef GL_ARB_texture_cube_map
#define GL_ARB_texture_cube_map 1
enum : GLenum
{
    GL_NORMAL_MAP_ARB                                       = 0x8511,
    GL_REFLECTION_MAP_ARB                                   = 0x8512,
    GL_TEXTURE_CUBE_MAP_ARB                                 = 0x8513,
    GL_TEXTURE_BINDING_CUBE_MAP_ARB                         = 0x8514,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB                      = 0x8515,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB                      = 0x8516,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB                      = 0x8517,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB                      = 0x8518,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB                      = 0x8519,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB                      = 0x851A,
    GL_PROXY_TEXTURE_CUBE_MAP_ARB                           = 0x851B,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB                        = 0x851C,
};
#endif

#ifndef GL_ARB_texture_cube_map_array
#define GL_ARB_texture_cube_map_array 1
enum : GLenum
{
    GL_TEXTURE_CUBE_MAP_ARRAY_ARB                           = 0x9009,
    GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB                   = 0x900A,
    GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB                     = 0x900B,
    GL_SAMPLER_CUBE_MAP_ARRAY_ARB                           = 0x900C,
    GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB                    = 0x900D,
    GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB                       = 0x900E,
    GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB              = 0x900F,
};
#endif

#ifndef GL_ARB_texture_env_add
#define GL_ARB_texture_env_add 1
#endif

#ifndef GL_ARB_texture_env_combine
#define GL_ARB_texture_env_combine 1
enum : GLenum
{
    GL_COMBINE_ARB                                          = 0x8570,
    GL_COMBINE_RGB_ARB                                      = 0x8571,
    GL_COMBINE_ALPHA_ARB                                    = 0x8572,
    GL_SOURCE0_RGB_ARB                                      = 0x8580,
    GL_SOURCE1_RGB_ARB                                      = 0x8581,
    GL_SOURCE2_RGB_ARB                                      = 0x8582,
    GL_SOURCE0_ALPHA_ARB                                    = 0x8588,
    GL_SOURCE1_ALPHA_ARB                                    = 0x8589,
    GL_SOURCE2_ALPHA_ARB                                    = 0x858A,
    GL_OPERAND0_RGB_ARB                                     = 0x8590,
    GL_OPERAND1_RGB_ARB                                     = 0x8591,
    GL_OPERAND2_RGB_ARB                                     = 0x8592,
    GL_OPERAND0_ALPHA_ARB                                   = 0x8598,
    GL_OPERAND1_ALPHA_ARB                                   = 0x8599,
    GL_OPERAND2_ALPHA_ARB                                   = 0x859A,
    GL_RGB_SCALE_ARB                                        = 0x8573,
    GL_ADD_SIGNED_ARB                                       = 0x8574,
    GL_INTERPOLATE_ARB                                      = 0x8575,
    GL_SUBTRACT_ARB                                         = 0x84E7,
    GL_CONSTANT_ARB                                         = 0x8576,
    GL_PRIMARY_COLOR_ARB                                    = 0x8577,
    GL_PREVIOUS_ARB                                         = 0x8578,
};
#endif

#ifndef GL_ARB_texture_env_crossbar
#define GL_ARB_texture_env_crossbar 1
#endif

#ifndef GL_ARB_texture_env_dot3
#define GL_ARB_texture_env_dot3 1
enum : GLenum
{
    GL_DOT3_RGB_ARB                                         = 0x86AE,
    GL_DOT3_RGBA_ARB                                        = 0x86AF,
};
#endif

#ifndef GL_ARB_texture_filter_anisotropic
#define GL_ARB_texture_filter_anisotropic 1
#endif

#ifndef GL_ARB_texture_filter_minmax
#define GL_ARB_texture_filter_minmax 1
enum : GLenum
{
    GL_TEXTURE_REDUCTION_MODE_ARB                           = 0x9366,
    GL_WEIGHTED_AVERAGE_ARB                                 = 0x9367,
};
#endif

#ifndef GL_ARB_texture_float
#define GL_ARB_texture_float 1
enum : GLenum
{
    GL_TEXTURE_RED_TYPE_ARB                                 = 0x8C10,
    GL_TEXTURE_GREEN_TYPE_ARB                               = 0x8C11,
    GL_TEXTURE_BLUE_TYPE_ARB                                = 0x8C12,
    GL_TEXTURE_ALPHA_TYPE_ARB                               = 0x8C13,
    GL_TEXTURE_LUMINANCE_TYPE_ARB                           = 0x8C14,
    GL_TEXTURE_INTENSITY_TYPE_ARB                           = 0x8C15,
    GL_TEXTURE_DEPTH_TYPE_ARB                               = 0x8C16,
    GL_UNSIGNED_NORMALIZED_ARB                              = 0x8C17,
    GL_RGBA32F_ARB                                          = 0x8814,
    GL_RGB32F_ARB                                           = 0x8815,
    GL_ALPHA32F_ARB                                         = 0x8816,
    GL_INTENSITY32F_ARB                                     = 0x8817,
    GL_LUMINANCE32F_ARB                                     = 0x8818,
    GL_LUMINANCE_ALPHA32F_ARB                               = 0x8819,
    GL_RGBA16F_ARB                                          = 0x881A,
    GL_RGB16F_ARB                                           = 0x881B,
    GL_ALPHA16F_ARB                                         = 0x881C,
    GL_INTENSITY16F_ARB                                     = 0x881D,
    GL_LUMINANCE16F_ARB                                     = 0x881E,
    GL_LUMINANCE_ALPHA16F_ARB                               = 0x881F,
};
#endif

#ifndef GL_ARB_texture_gather
#define GL_ARB_texture_gather 1
enum : GLenum
{
    GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB                = 0x8E5E,
    GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB                = 0x8E5F,
    GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB            = 0x8F9F,
};
#endif

#ifndef GL_ARB_texture_mirror_clamp_to_edge
#define GL_ARB_texture_mirror_clamp_to_edge 1
#endif

#ifndef GL_ARB_texture_mirrored_repeat
#define GL_ARB_texture_mirrored_repeat 1
enum : GLenum
{
    GL_MIRRORED_REPEAT_ARB                                  = 0x8370,
};
#endif

#ifndef GL_ARB_texture_multisample
#define GL_ARB_texture_multisample 1
#endif

#ifndef GL_ARB_texture_non_power_of_two
#define GL_ARB_texture_non_power_of_two 1
#endif

#ifndef GL_ARB_texture_query_levels
#define GL_ARB_texture_query_levels 1
#endif

#ifndef GL_ARB_texture_query_lod
#define GL_ARB_texture_query_lod 1
#endif

#ifndef GL_ARB_texture_rectangle
#define GL_ARB_texture_rectangle 1
enum : GLenum
{
    GL_TEXTURE_RECTANGLE_ARB                                = 0x84F5,
    GL_TEXTURE_BINDING_RECTANGLE_ARB                        = 0x84F6,
    GL_PROXY_TEXTURE_RECTANGLE_ARB                          = 0x84F7,
    GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB                       = 0x84F8,
};
#endif

#ifndef GL_ARB_texture_rg
#define GL_ARB_texture_rg 1
#endif

#ifndef GL_ARB_texture_rgb10_a2ui
#define GL_ARB_texture_rgb10_a2ui 1
#endif

#ifndef GL_ARB_texture_stencil8
#define GL_ARB_texture_stencil8 1
#endif

#ifndef GL_ARB_texture_storage
#define GL_ARB_texture_storage 1
#endif

#ifndef GL_ARB_texture_storage_multisample
#define GL_ARB_texture_storage_multisample 1
#endif

#ifndef GL_ARB_texture_swizzle
#define GL_ARB_texture_swizzle 1
#endif

#ifndef GL_ARB_texture_view
#define GL_ARB_texture_view 1
#endif

#ifndef GL_ARB_timer_query
#define GL_ARB_timer_query 1
#endif

#ifndef GL_ARB_transform_feedback2
#define GL_ARB_transform_feedback2 1
#endif

#ifndef GL_ARB_transform_feedback3
#define GL_ARB_transform_feedback3 1
#endif

#ifndef GL_ARB_transform_feedback_instanced
#define GL_ARB_transform_feedback_instanced 1
#endif

#ifndef GL_ARB_transform_feedback_overflow_query
#define GL_ARB_transform_feedback_overflow_query 1
enum : GLenum
{
    GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB                      = 0x82EC,
    GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB               = 0x82ED,
};
#endif

#ifndef GL_ARB_transpose_matrix
#define GL_ARB_transpose_matrix 1
enum : GLenum
{
    GL_TRANSPOSE_MODELVIEW_MATRIX_ARB                       = 0x84E3,
    GL_TRANSPOSE_PROJECTION_MATRIX_ARB                      = 0x84E4,
    GL_TRANSPOSE_TEXTURE_MATRIX_ARB                         = 0x84E5,
    GL_TRANSPOSE_COLOR_MATRIX_ARB                           = 0x84E6,
};
extern void         (KHRONOS_APIENTRY* const& glLoadTransposeMatrixfARB) (const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glLoadTransposeMatrixdARB) (const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMultTransposeMatrixfARB) (const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMultTransposeMatrixdARB) (const GLdouble *m);
#endif

#ifndef GL_ARB_uniform_buffer_object
#define GL_ARB_uniform_buffer_object 1
#endif

#ifndef GL_ARB_vertex_array_bgra
#define GL_ARB_vertex_array_bgra 1
#endif

#ifndef GL_ARB_vertex_array_object
#define GL_ARB_vertex_array_object 1
#endif

#ifndef GL_ARB_vertex_attrib_64bit
#define GL_ARB_vertex_attrib_64bit 1
#endif

#ifndef GL_ARB_vertex_attrib_binding
#define GL_ARB_vertex_attrib_binding 1
#endif

#ifndef GL_ARB_vertex_blend
#define GL_ARB_vertex_blend 1
enum : GLenum
{
    GL_MAX_VERTEX_UNITS_ARB                                 = 0x86A4,
    GL_ACTIVE_VERTEX_UNITS_ARB                              = 0x86A5,
    GL_WEIGHT_SUM_UNITY_ARB                                 = 0x86A6,
    GL_VERTEX_BLEND_ARB                                     = 0x86A7,
    GL_CURRENT_WEIGHT_ARB                                   = 0x86A8,
    GL_WEIGHT_ARRAY_TYPE_ARB                                = 0x86A9,
    GL_WEIGHT_ARRAY_STRIDE_ARB                              = 0x86AA,
    GL_WEIGHT_ARRAY_SIZE_ARB                                = 0x86AB,
    GL_WEIGHT_ARRAY_POINTER_ARB                             = 0x86AC,
    GL_WEIGHT_ARRAY_ARB                                     = 0x86AD,
    GL_MODELVIEW0_ARB                                       = 0x1700,
    GL_MODELVIEW1_ARB                                       = 0x850A,
    GL_MODELVIEW2_ARB                                       = 0x8722,
    GL_MODELVIEW3_ARB                                       = 0x8723,
    GL_MODELVIEW4_ARB                                       = 0x8724,
    GL_MODELVIEW5_ARB                                       = 0x8725,
    GL_MODELVIEW6_ARB                                       = 0x8726,
    GL_MODELVIEW7_ARB                                       = 0x8727,
    GL_MODELVIEW8_ARB                                       = 0x8728,
    GL_MODELVIEW9_ARB                                       = 0x8729,
    GL_MODELVIEW10_ARB                                      = 0x872A,
    GL_MODELVIEW11_ARB                                      = 0x872B,
    GL_MODELVIEW12_ARB                                      = 0x872C,
    GL_MODELVIEW13_ARB                                      = 0x872D,
    GL_MODELVIEW14_ARB                                      = 0x872E,
    GL_MODELVIEW15_ARB                                      = 0x872F,
    GL_MODELVIEW16_ARB                                      = 0x8730,
    GL_MODELVIEW17_ARB                                      = 0x8731,
    GL_MODELVIEW18_ARB                                      = 0x8732,
    GL_MODELVIEW19_ARB                                      = 0x8733,
    GL_MODELVIEW20_ARB                                      = 0x8734,
    GL_MODELVIEW21_ARB                                      = 0x8735,
    GL_MODELVIEW22_ARB                                      = 0x8736,
    GL_MODELVIEW23_ARB                                      = 0x8737,
    GL_MODELVIEW24_ARB                                      = 0x8738,
    GL_MODELVIEW25_ARB                                      = 0x8739,
    GL_MODELVIEW26_ARB                                      = 0x873A,
    GL_MODELVIEW27_ARB                                      = 0x873B,
    GL_MODELVIEW28_ARB                                      = 0x873C,
    GL_MODELVIEW29_ARB                                      = 0x873D,
    GL_MODELVIEW30_ARB                                      = 0x873E,
    GL_MODELVIEW31_ARB                                      = 0x873F,
};
extern void         (KHRONOS_APIENTRY* const& glWeightbvARB) (GLint size, const GLbyte *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightsvARB) (GLint size, const GLshort *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightivARB) (GLint size, const GLint *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightfvARB) (GLint size, const GLfloat *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightdvARB) (GLint size, const GLdouble *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightubvARB) (GLint size, const GLubyte *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightusvARB) (GLint size, const GLushort *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightuivARB) (GLint size, const GLuint *weights);
extern void         (KHRONOS_APIENTRY* const& glWeightPointerARB) (GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glVertexBlendARB) (GLint count);
#endif

#ifndef GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_buffer_object 1
enum : GLenum
{
    GL_BUFFER_SIZE_ARB                                      = 0x8764,
    GL_BUFFER_USAGE_ARB                                     = 0x8765,
    GL_ARRAY_BUFFER_ARB                                     = 0x8892,
    GL_ELEMENT_ARRAY_BUFFER_ARB                             = 0x8893,
    GL_ARRAY_BUFFER_BINDING_ARB                             = 0x8894,
    GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB                     = 0x8895,
    GL_VERTEX_ARRAY_BUFFER_BINDING_ARB                      = 0x8896,
    GL_NORMAL_ARRAY_BUFFER_BINDING_ARB                      = 0x8897,
    GL_COLOR_ARRAY_BUFFER_BINDING_ARB                       = 0x8898,
    GL_INDEX_ARRAY_BUFFER_BINDING_ARB                       = 0x8899,
    GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB               = 0x889A,
    GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB                   = 0x889B,
    GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB             = 0x889C,
    GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB              = 0x889D,
    GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB                      = 0x889E,
    GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB               = 0x889F,
    GL_READ_ONLY_ARB                                        = 0x88B8,
    GL_WRITE_ONLY_ARB                                       = 0x88B9,
    GL_READ_WRITE_ARB                                       = 0x88BA,
    GL_BUFFER_ACCESS_ARB                                    = 0x88BB,
    GL_BUFFER_MAPPED_ARB                                    = 0x88BC,
    GL_BUFFER_MAP_POINTER_ARB                               = 0x88BD,
    GL_STREAM_DRAW_ARB                                      = 0x88E0,
    GL_STREAM_READ_ARB                                      = 0x88E1,
    GL_STREAM_COPY_ARB                                      = 0x88E2,
    GL_STATIC_DRAW_ARB                                      = 0x88E4,
    GL_STATIC_READ_ARB                                      = 0x88E5,
    GL_STATIC_COPY_ARB                                      = 0x88E6,
    GL_DYNAMIC_DRAW_ARB                                     = 0x88E8,
    GL_DYNAMIC_READ_ARB                                     = 0x88E9,
    GL_DYNAMIC_COPY_ARB                                     = 0x88EA,
};
extern void         (KHRONOS_APIENTRY* const& glBindBufferARB) (GLenum target, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glDeleteBuffersARB) (GLsizei n, const GLuint *buffers);
extern void         (KHRONOS_APIENTRY* const& glGenBuffersARB) (GLsizei n, GLuint *buffers);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsBufferARB) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glBufferDataARB) (GLenum target, GLsizeiptrARB size, const void *data, GLenum usage);
extern void         (KHRONOS_APIENTRY* const& glBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const void *data);
extern void         (KHRONOS_APIENTRY* const& glGetBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, void *data);
extern void *       (KHRONOS_APIENTRY* const& glMapBufferARB) (GLenum target, GLenum access);
extern GLboolean    (KHRONOS_APIENTRY* const& glUnmapBufferARB) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetBufferParameterivARB) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetBufferPointervARB) (GLenum target, GLenum pname, void **params);
#endif

#ifndef GL_ARB_vertex_program
#define GL_ARB_vertex_program 1
enum : GLenum
{
    GL_COLOR_SUM_ARB                                        = 0x8458,
    GL_VERTEX_PROGRAM_ARB                                   = 0x8620,
    GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB                      = 0x8622,
    GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB                         = 0x8623,
    GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB                       = 0x8624,
    GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB                         = 0x8625,
    GL_CURRENT_VERTEX_ATTRIB_ARB                            = 0x8626,
    GL_VERTEX_PROGRAM_POINT_SIZE_ARB                        = 0x8642,
    GL_VERTEX_PROGRAM_TWO_SIDE_ARB                          = 0x8643,
    GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB                      = 0x8645,
    GL_MAX_VERTEX_ATTRIBS_ARB                               = 0x8869,
    GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB                   = 0x886A,
    GL_PROGRAM_ADDRESS_REGISTERS_ARB                        = 0x88B0,
    GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB                    = 0x88B1,
    GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB                 = 0x88B2,
    GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB             = 0x88B3,
};
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1dARB) (GLuint index, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1dvARB) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1fARB) (GLuint index, GLfloat x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1fvARB) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1sARB) (GLuint index, GLshort x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1svARB) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2dARB) (GLuint index, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2dvARB) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2fARB) (GLuint index, GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2fvARB) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2sARB) (GLuint index, GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2svARB) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3dARB) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3dvARB) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3fARB) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3fvARB) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3sARB) (GLuint index, GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3svARB) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4NbvARB) (GLuint index, const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4NivARB) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4NsvARB) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4NubARB) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4NubvARB) (GLuint index, const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4NuivARB) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4NusvARB) (GLuint index, const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4bvARB) (GLuint index, const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4dARB) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4dvARB) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4fARB) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4fvARB) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4ivARB) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4sARB) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4svARB) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4ubvARB) (GLuint index, const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4uivARB) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4usvARB) (GLuint index, const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribPointerARB) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glEnableVertexAttribArrayARB) (GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableVertexAttribArrayARB) (GLuint index);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribdvARB) (GLuint index, GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribfvARB) (GLuint index, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribivARB) (GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribPointervARB) (GLuint index, GLenum pname, void **pointer);
#endif

#ifndef GL_ARB_vertex_shader
#define GL_ARB_vertex_shader 1
enum : GLenum
{
    GL_VERTEX_SHADER_ARB                                    = 0x8B31,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB                    = 0x8B4A,
    GL_MAX_VARYING_FLOATS_ARB                               = 0x8B4B,
    GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB                   = 0x8B4C,
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB                 = 0x8B4D,
    GL_OBJECT_ACTIVE_ATTRIBUTES_ARB                         = 0x8B89,
    GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB               = 0x8B8A,
};
extern void         (KHRONOS_APIENTRY* const& glBindAttribLocationARB) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
extern void         (KHRONOS_APIENTRY* const& glGetActiveAttribARB) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetAttribLocationARB) (GLhandleARB programObj, const GLcharARB *name);
#endif

#ifndef GL_ARB_vertex_type_10f_11f_11f_rev
#define GL_ARB_vertex_type_10f_11f_11f_rev 1
#endif

#ifndef GL_ARB_vertex_type_2_10_10_10_rev
#define GL_ARB_vertex_type_2_10_10_10_rev 1
#endif

#ifndef GL_ARB_viewport_array
#define GL_ARB_viewport_array 1
extern void         (KHRONOS_APIENTRY* const& glDepthRangeArraydvNV) (GLuint first, GLsizei count, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glDepthRangeIndexeddNV) (GLuint index, GLdouble n, GLdouble f);
#endif

#ifndef GL_ARB_window_pos
#define GL_ARB_window_pos 1
extern void         (KHRONOS_APIENTRY* const& glWindowPos2dARB) (GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2dvARB) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2fARB) (GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2fvARB) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2iARB) (GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2ivARB) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2sARB) (GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2svARB) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3dARB) (GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3dvARB) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3fARB) (GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3fvARB) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3iARB) (GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3ivARB) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3sARB) (GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3svARB) (const GLshort *v);
#endif

#ifndef GL_ATI_draw_buffers
#define GL_ATI_draw_buffers 1
enum : GLenum
{
    GL_MAX_DRAW_BUFFERS_ATI                                 = 0x8824,
    GL_DRAW_BUFFER0_ATI                                     = 0x8825,
    GL_DRAW_BUFFER1_ATI                                     = 0x8826,
    GL_DRAW_BUFFER2_ATI                                     = 0x8827,
    GL_DRAW_BUFFER3_ATI                                     = 0x8828,
    GL_DRAW_BUFFER4_ATI                                     = 0x8829,
    GL_DRAW_BUFFER5_ATI                                     = 0x882A,
    GL_DRAW_BUFFER6_ATI                                     = 0x882B,
    GL_DRAW_BUFFER7_ATI                                     = 0x882C,
    GL_DRAW_BUFFER8_ATI                                     = 0x882D,
    GL_DRAW_BUFFER9_ATI                                     = 0x882E,
    GL_DRAW_BUFFER10_ATI                                    = 0x882F,
    GL_DRAW_BUFFER11_ATI                                    = 0x8830,
    GL_DRAW_BUFFER12_ATI                                    = 0x8831,
    GL_DRAW_BUFFER13_ATI                                    = 0x8832,
    GL_DRAW_BUFFER14_ATI                                    = 0x8833,
    GL_DRAW_BUFFER15_ATI                                    = 0x8834,
};
extern void         (KHRONOS_APIENTRY* const& glDrawBuffersATI) (GLsizei n, const GLenum *bufs);
#endif

#ifndef GL_ATI_element_array
#define GL_ATI_element_array 1
enum : GLenum
{
    GL_ELEMENT_ARRAY_ATI                                    = 0x8768,
    GL_ELEMENT_ARRAY_TYPE_ATI                               = 0x8769,
    GL_ELEMENT_ARRAY_POINTER_ATI                            = 0x876A,
};
extern void         (KHRONOS_APIENTRY* const& glElementPointerATI) (GLenum type, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glDrawElementArrayATI) (GLenum mode, GLsizei count);
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElementArrayATI) (GLenum mode, GLuint start, GLuint end, GLsizei count);
#endif

#ifndef GL_ATI_envmap_bumpmap
#define GL_ATI_envmap_bumpmap 1
enum : GLenum
{
    GL_BUMP_ROT_MATRIX_ATI                                  = 0x8775,
    GL_BUMP_ROT_MATRIX_SIZE_ATI                             = 0x8776,
    GL_BUMP_NUM_TEX_UNITS_ATI                               = 0x8777,
    GL_BUMP_TEX_UNITS_ATI                                   = 0x8778,
    GL_DUDV_ATI                                             = 0x8779,
    GL_DU8DV8_ATI                                           = 0x877A,
    GL_BUMP_ENVMAP_ATI                                      = 0x877B,
    GL_BUMP_TARGET_ATI                                      = 0x877C,
};
extern void         (KHRONOS_APIENTRY* const& glTexBumpParameterivATI) (GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glTexBumpParameterfvATI) (GLenum pname, const GLfloat *param);
extern void         (KHRONOS_APIENTRY* const& glGetTexBumpParameterivATI) (GLenum pname, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetTexBumpParameterfvATI) (GLenum pname, GLfloat *param);
#endif

#ifndef GL_ATI_fragment_shader
#define GL_ATI_fragment_shader 1
enum : GLenum
{
    GL_FRAGMENT_SHADER_ATI                                  = 0x8920,
    GL_REG_0_ATI                                            = 0x8921,
    GL_REG_1_ATI                                            = 0x8922,
    GL_REG_2_ATI                                            = 0x8923,
    GL_REG_3_ATI                                            = 0x8924,
    GL_REG_4_ATI                                            = 0x8925,
    GL_REG_5_ATI                                            = 0x8926,
    GL_REG_6_ATI                                            = 0x8927,
    GL_REG_7_ATI                                            = 0x8928,
    GL_REG_8_ATI                                            = 0x8929,
    GL_REG_9_ATI                                            = 0x892A,
    GL_REG_10_ATI                                           = 0x892B,
    GL_REG_11_ATI                                           = 0x892C,
    GL_REG_12_ATI                                           = 0x892D,
    GL_REG_13_ATI                                           = 0x892E,
    GL_REG_14_ATI                                           = 0x892F,
    GL_REG_15_ATI                                           = 0x8930,
    GL_REG_16_ATI                                           = 0x8931,
    GL_REG_17_ATI                                           = 0x8932,
    GL_REG_18_ATI                                           = 0x8933,
    GL_REG_19_ATI                                           = 0x8934,
    GL_REG_20_ATI                                           = 0x8935,
    GL_REG_21_ATI                                           = 0x8936,
    GL_REG_22_ATI                                           = 0x8937,
    GL_REG_23_ATI                                           = 0x8938,
    GL_REG_24_ATI                                           = 0x8939,
    GL_REG_25_ATI                                           = 0x893A,
    GL_REG_26_ATI                                           = 0x893B,
    GL_REG_27_ATI                                           = 0x893C,
    GL_REG_28_ATI                                           = 0x893D,
    GL_REG_29_ATI                                           = 0x893E,
    GL_REG_30_ATI                                           = 0x893F,
    GL_REG_31_ATI                                           = 0x8940,
    GL_CON_0_ATI                                            = 0x8941,
    GL_CON_1_ATI                                            = 0x8942,
    GL_CON_2_ATI                                            = 0x8943,
    GL_CON_3_ATI                                            = 0x8944,
    GL_CON_4_ATI                                            = 0x8945,
    GL_CON_5_ATI                                            = 0x8946,
    GL_CON_6_ATI                                            = 0x8947,
    GL_CON_7_ATI                                            = 0x8948,
    GL_CON_8_ATI                                            = 0x8949,
    GL_CON_9_ATI                                            = 0x894A,
    GL_CON_10_ATI                                           = 0x894B,
    GL_CON_11_ATI                                           = 0x894C,
    GL_CON_12_ATI                                           = 0x894D,
    GL_CON_13_ATI                                           = 0x894E,
    GL_CON_14_ATI                                           = 0x894F,
    GL_CON_15_ATI                                           = 0x8950,
    GL_CON_16_ATI                                           = 0x8951,
    GL_CON_17_ATI                                           = 0x8952,
    GL_CON_18_ATI                                           = 0x8953,
    GL_CON_19_ATI                                           = 0x8954,
    GL_CON_20_ATI                                           = 0x8955,
    GL_CON_21_ATI                                           = 0x8956,
    GL_CON_22_ATI                                           = 0x8957,
    GL_CON_23_ATI                                           = 0x8958,
    GL_CON_24_ATI                                           = 0x8959,
    GL_CON_25_ATI                                           = 0x895A,
    GL_CON_26_ATI                                           = 0x895B,
    GL_CON_27_ATI                                           = 0x895C,
    GL_CON_28_ATI                                           = 0x895D,
    GL_CON_29_ATI                                           = 0x895E,
    GL_CON_30_ATI                                           = 0x895F,
    GL_CON_31_ATI                                           = 0x8960,
    GL_MOV_ATI                                              = 0x8961,
    GL_ADD_ATI                                              = 0x8963,
    GL_MUL_ATI                                              = 0x8964,
    GL_SUB_ATI                                              = 0x8965,
    GL_DOT3_ATI                                             = 0x8966,
    GL_DOT4_ATI                                             = 0x8967,
    GL_MAD_ATI                                              = 0x8968,
    GL_LERP_ATI                                             = 0x8969,
    GL_CND_ATI                                              = 0x896A,
    GL_CND0_ATI                                             = 0x896B,
    GL_DOT2_ADD_ATI                                         = 0x896C,
    GL_SECONDARY_INTERPOLATOR_ATI                           = 0x896D,
    GL_NUM_FRAGMENT_REGISTERS_ATI                           = 0x896E,
    GL_NUM_FRAGMENT_CONSTANTS_ATI                           = 0x896F,
    GL_NUM_PASSES_ATI                                       = 0x8970,
    GL_NUM_INSTRUCTIONS_PER_PASS_ATI                        = 0x8971,
    GL_NUM_INSTRUCTIONS_TOTAL_ATI                           = 0x8972,
    GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI                = 0x8973,
    GL_NUM_LOOPBACK_COMPONENTS_ATI                          = 0x8974,
    GL_COLOR_ALPHA_PAIRING_ATI                              = 0x8975,
    GL_SWIZZLE_STR_ATI                                      = 0x8976,
    GL_SWIZZLE_STQ_ATI                                      = 0x8977,
    GL_SWIZZLE_STR_DR_ATI                                   = 0x8978,
    GL_SWIZZLE_STQ_DQ_ATI                                   = 0x8979,
    GL_SWIZZLE_STRQ_ATI                                     = 0x897A,
    GL_SWIZZLE_STRQ_DQ_ATI                                  = 0x897B,
    GL_RED_BIT_ATI                                          = 0x00000001,
    GL_GREEN_BIT_ATI                                        = 0x00000002,
    GL_BLUE_BIT_ATI                                         = 0x00000004,
    GL_2X_BIT_ATI                                           = 0x00000001,
    GL_4X_BIT_ATI                                           = 0x00000002,
    GL_8X_BIT_ATI                                           = 0x00000004,
    GL_HALF_BIT_ATI                                         = 0x00000008,
    GL_QUARTER_BIT_ATI                                      = 0x00000010,
    GL_EIGHTH_BIT_ATI                                       = 0x00000020,
    GL_SATURATE_BIT_ATI                                     = 0x00000040,
    GL_COMP_BIT_ATI                                         = 0x00000002,
    GL_NEGATE_BIT_ATI                                       = 0x00000004,
    GL_BIAS_BIT_ATI                                         = 0x00000008,
};
extern GLuint       (KHRONOS_APIENTRY* const& glGenFragmentShadersATI) (GLuint range);
extern void         (KHRONOS_APIENTRY* const& glBindFragmentShaderATI) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDeleteFragmentShaderATI) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glBeginFragmentShaderATI) ();
extern void         (KHRONOS_APIENTRY* const& glEndFragmentShaderATI) ();
extern void         (KHRONOS_APIENTRY* const& glPassTexCoordATI) (GLuint dst, GLuint coord, GLenum swizzle);
extern void         (KHRONOS_APIENTRY* const& glSampleMapATI) (GLuint dst, GLuint interp, GLenum swizzle);
extern void         (KHRONOS_APIENTRY* const& glColorFragmentOp1ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
extern void         (KHRONOS_APIENTRY* const& glColorFragmentOp2ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
extern void         (KHRONOS_APIENTRY* const& glColorFragmentOp3ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
extern void         (KHRONOS_APIENTRY* const& glAlphaFragmentOp1ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
extern void         (KHRONOS_APIENTRY* const& glAlphaFragmentOp2ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
extern void         (KHRONOS_APIENTRY* const& glAlphaFragmentOp3ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
extern void         (KHRONOS_APIENTRY* const& glSetFragmentShaderConstantATI) (GLuint dst, const GLfloat *value);
#endif

#ifndef GL_ATI_map_object_buffer
#define GL_ATI_map_object_buffer 1
extern void *       (KHRONOS_APIENTRY* const& glMapObjectBufferATI) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glUnmapObjectBufferATI) (GLuint buffer);
#endif

#ifndef GL_ATI_meminfo
#define GL_ATI_meminfo 1
enum : GLenum
{
    GL_VBO_FREE_MEMORY_ATI                                  = 0x87FB,
    GL_TEXTURE_FREE_MEMORY_ATI                              = 0x87FC,
    GL_RENDERBUFFER_FREE_MEMORY_ATI                         = 0x87FD,
};
#endif

#ifndef GL_ATI_pixel_format_float
#define GL_ATI_pixel_format_float 1
enum : GLenum
{
    GL_RGBA_FLOAT_MODE_ATI                                  = 0x8820,
    GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI                      = 0x8835,
};
#endif

#ifndef GL_ATI_pn_triangles
#define GL_ATI_pn_triangles 1
enum : GLenum
{
    GL_PN_TRIANGLES_ATI                                     = 0x87F0,
    GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI               = 0x87F1,
    GL_PN_TRIANGLES_POINT_MODE_ATI                          = 0x87F2,
    GL_PN_TRIANGLES_NORMAL_MODE_ATI                         = 0x87F3,
    GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI                   = 0x87F4,
    GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI                   = 0x87F5,
    GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI                    = 0x87F6,
    GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI                  = 0x87F7,
    GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI               = 0x87F8,
};
extern void         (KHRONOS_APIENTRY* const& glPNTrianglesiATI) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glPNTrianglesfATI) (GLenum pname, GLfloat param);
#endif

#ifndef GL_ATI_separate_stencil
#define GL_ATI_separate_stencil 1
enum : GLenum
{
    GL_STENCIL_BACK_FUNC_ATI                                = 0x8800,
    GL_STENCIL_BACK_FAIL_ATI                                = 0x8801,
    GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI                     = 0x8802,
    GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI                     = 0x8803,
};
extern void         (KHRONOS_APIENTRY* const& glStencilOpSeparateATI) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
extern void         (KHRONOS_APIENTRY* const& glStencilFuncSeparateATI) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#ifndef GL_ATI_text_fragment_shader
#define GL_ATI_text_fragment_shader 1
enum : GLenum
{
    GL_TEXT_FRAGMENT_SHADER_ATI                             = 0x8200,
};
#endif

#ifndef GL_ATI_texture_env_combine3
#define GL_ATI_texture_env_combine3 1
enum : GLenum
{
    GL_MODULATE_ADD_ATI                                     = 0x8744,
    GL_MODULATE_SIGNED_ADD_ATI                              = 0x8745,
    GL_MODULATE_SUBTRACT_ATI                                = 0x8746,
};
#endif

#ifndef GL_ATI_texture_float
#define GL_ATI_texture_float 1
enum : GLenum
{
    GL_RGBA_FLOAT32_ATI                                     = 0x8814,
    GL_RGB_FLOAT32_ATI                                      = 0x8815,
    GL_ALPHA_FLOAT32_ATI                                    = 0x8816,
    GL_INTENSITY_FLOAT32_ATI                                = 0x8817,
    GL_LUMINANCE_FLOAT32_ATI                                = 0x8818,
    GL_LUMINANCE_ALPHA_FLOAT32_ATI                          = 0x8819,
    GL_RGBA_FLOAT16_ATI                                     = 0x881A,
    GL_RGB_FLOAT16_ATI                                      = 0x881B,
    GL_ALPHA_FLOAT16_ATI                                    = 0x881C,
    GL_INTENSITY_FLOAT16_ATI                                = 0x881D,
    GL_LUMINANCE_FLOAT16_ATI                                = 0x881E,
    GL_LUMINANCE_ALPHA_FLOAT16_ATI                          = 0x881F,
};
#endif

#ifndef GL_ATI_texture_mirror_once
#define GL_ATI_texture_mirror_once 1
enum : GLenum
{
    GL_MIRROR_CLAMP_ATI                                     = 0x8742,
    GL_MIRROR_CLAMP_TO_EDGE_ATI                             = 0x8743,
};
#endif

#ifndef GL_ATI_vertex_array_object
#define GL_ATI_vertex_array_object 1
enum : GLenum
{
    GL_STATIC_ATI                                           = 0x8760,
    GL_DYNAMIC_ATI                                          = 0x8761,
    GL_PRESERVE_ATI                                         = 0x8762,
    GL_DISCARD_ATI                                          = 0x8763,
    GL_OBJECT_BUFFER_SIZE_ATI                               = 0x8764,
    GL_OBJECT_BUFFER_USAGE_ATI                              = 0x8765,
    GL_ARRAY_OBJECT_BUFFER_ATI                              = 0x8766,
    GL_ARRAY_OBJECT_OFFSET_ATI                              = 0x8767,
};
extern GLuint       (KHRONOS_APIENTRY* const& glNewObjectBufferATI) (GLsizei size, const void *pointer, GLenum usage);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsObjectBufferATI) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glUpdateObjectBufferATI) (GLuint buffer, GLuint offset, GLsizei size, const void *pointer, GLenum preserve);
extern void         (KHRONOS_APIENTRY* const& glGetObjectBufferfvATI) (GLuint buffer, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetObjectBufferivATI) (GLuint buffer, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glFreeObjectBufferATI) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glArrayObjectATI) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
extern void         (KHRONOS_APIENTRY* const& glGetArrayObjectfvATI) (GLenum array, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetArrayObjectivATI) (GLenum array, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glVariantArrayObjectATI) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
extern void         (KHRONOS_APIENTRY* const& glGetVariantArrayObjectfvATI) (GLuint id, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetVariantArrayObjectivATI) (GLuint id, GLenum pname, GLint *params);
#endif

#ifndef GL_ATI_vertex_attrib_array_object
#define GL_ATI_vertex_attrib_array_object 1
extern void         (KHRONOS_APIENTRY* const& glVertexAttribArrayObjectATI) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribArrayObjectfvATI) (GLuint index, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribArrayObjectivATI) (GLuint index, GLenum pname, GLint *params);
#endif

#ifndef GL_ATI_vertex_streams
#define GL_ATI_vertex_streams 1
enum : GLenum
{
    GL_MAX_VERTEX_STREAMS_ATI                               = 0x876B,
    GL_VERTEX_STREAM0_ATI                                   = 0x876C,
    GL_VERTEX_STREAM1_ATI                                   = 0x876D,
    GL_VERTEX_STREAM2_ATI                                   = 0x876E,
    GL_VERTEX_STREAM3_ATI                                   = 0x876F,
    GL_VERTEX_STREAM4_ATI                                   = 0x8770,
    GL_VERTEX_STREAM5_ATI                                   = 0x8771,
    GL_VERTEX_STREAM6_ATI                                   = 0x8772,
    GL_VERTEX_STREAM7_ATI                                   = 0x8773,
    GL_VERTEX_SOURCE_ATI                                    = 0x8774,
};
extern void         (KHRONOS_APIENTRY* const& glVertexStream1sATI) (GLenum stream, GLshort x);
extern void         (KHRONOS_APIENTRY* const& glVertexStream1svATI) (GLenum stream, const GLshort *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream1iATI) (GLenum stream, GLint x);
extern void         (KHRONOS_APIENTRY* const& glVertexStream1ivATI) (GLenum stream, const GLint *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream1fATI) (GLenum stream, GLfloat x);
extern void         (KHRONOS_APIENTRY* const& glVertexStream1fvATI) (GLenum stream, const GLfloat *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream1dATI) (GLenum stream, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glVertexStream1dvATI) (GLenum stream, const GLdouble *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2sATI) (GLenum stream, GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2svATI) (GLenum stream, const GLshort *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2iATI) (GLenum stream, GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2ivATI) (GLenum stream, const GLint *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2fATI) (GLenum stream, GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2fvATI) (GLenum stream, const GLfloat *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2dATI) (GLenum stream, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glVertexStream2dvATI) (GLenum stream, const GLdouble *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3sATI) (GLenum stream, GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3svATI) (GLenum stream, const GLshort *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3iATI) (GLenum stream, GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3ivATI) (GLenum stream, const GLint *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3fATI) (GLenum stream, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3fvATI) (GLenum stream, const GLfloat *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3dATI) (GLenum stream, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glVertexStream3dvATI) (GLenum stream, const GLdouble *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4sATI) (GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4svATI) (GLenum stream, const GLshort *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4iATI) (GLenum stream, GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4ivATI) (GLenum stream, const GLint *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4fATI) (GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4fvATI) (GLenum stream, const GLfloat *coords);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4dATI) (GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glVertexStream4dvATI) (GLenum stream, const GLdouble *coords);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3bATI) (GLenum stream, GLbyte nx, GLbyte ny, GLbyte nz);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3bvATI) (GLenum stream, const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3sATI) (GLenum stream, GLshort nx, GLshort ny, GLshort nz);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3svATI) (GLenum stream, const GLshort *coords);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3iATI) (GLenum stream, GLint nx, GLint ny, GLint nz);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3ivATI) (GLenum stream, const GLint *coords);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3fATI) (GLenum stream, GLfloat nx, GLfloat ny, GLfloat nz);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3fvATI) (GLenum stream, const GLfloat *coords);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3dATI) (GLenum stream, GLdouble nx, GLdouble ny, GLdouble nz);
extern void         (KHRONOS_APIENTRY* const& glNormalStream3dvATI) (GLenum stream, const GLdouble *coords);
extern void         (KHRONOS_APIENTRY* const& glClientActiveVertexStreamATI) (GLenum stream);
extern void         (KHRONOS_APIENTRY* const& glVertexBlendEnviATI) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glVertexBlendEnvfATI) (GLenum pname, GLfloat param);
#endif

#ifndef GL_EXT_422_pixels
#define GL_EXT_422_pixels 1
enum : GLenum
{
    GL_422_EXT                                              = 0x80CC,
    GL_422_REV_EXT                                          = 0x80CD,
    GL_422_AVERAGE_EXT                                      = 0x80CE,
    GL_422_REV_AVERAGE_EXT                                  = 0x80CF,
};
#endif

#ifndef GL_EXT_EGL_image_storage
#define GL_EXT_EGL_image_storage 1
extern void         (KHRONOS_APIENTRY* const& glEGLImageTargetTexStorageEXT) (GLenum target, GLeglImageOES image, const GLint* attrib_list);
extern void         (KHRONOS_APIENTRY* const& glEGLImageTargetTextureStorageEXT) (GLuint texture, GLeglImageOES image, const GLint* attrib_list);
#endif

#ifndef GL_EXT_EGL_sync
#define GL_EXT_EGL_sync 1
#endif

#ifndef GL_EXT_abgr
#define GL_EXT_abgr 1
enum : GLenum
{
    GL_ABGR_EXT                                             = 0x8000,
};
#endif

#ifndef GL_EXT_bgra
#define GL_EXT_bgra 1
enum : GLenum
{
    GL_BGR_EXT                                              = 0x80E0,
    GL_BGRA_EXT                                             = 0x80E1,
};
#endif

#ifndef GL_EXT_bindable_uniform
#define GL_EXT_bindable_uniform 1
enum : GLenum
{
    GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT                     = 0x8DE2,
    GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT                   = 0x8DE3,
    GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT                   = 0x8DE4,
    GL_MAX_BINDABLE_UNIFORM_SIZE_EXT                        = 0x8DED,
    GL_UNIFORM_BUFFER_EXT                                   = 0x8DEE,
    GL_UNIFORM_BUFFER_BINDING_EXT                           = 0x8DEF,
};
extern void         (KHRONOS_APIENTRY* const& glUniformBufferEXT) (GLuint program, GLint location, GLuint buffer);
extern GLint        (KHRONOS_APIENTRY* const& glGetUniformBufferSizeEXT) (GLuint program, GLint location);
extern GLintptr     (KHRONOS_APIENTRY* const& glGetUniformOffsetEXT) (GLuint program, GLint location);
#endif

#ifndef GL_EXT_blend_color
#define GL_EXT_blend_color 1
enum : GLenum
{
    GL_CONSTANT_COLOR_EXT                                   = 0x8001,
    GL_ONE_MINUS_CONSTANT_COLOR_EXT                         = 0x8002,
    GL_CONSTANT_ALPHA_EXT                                   = 0x8003,
    GL_ONE_MINUS_CONSTANT_ALPHA_EXT                         = 0x8004,
    GL_BLEND_COLOR_EXT                                      = 0x8005,
};
extern void         (KHRONOS_APIENTRY* const& glBlendColorEXT) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
#endif

#ifndef GL_EXT_blend_equation_separate
#define GL_EXT_blend_equation_separate 1
enum : GLenum
{
    GL_BLEND_EQUATION_RGB_EXT                               = 0x8009,
    GL_BLEND_EQUATION_ALPHA_EXT                             = 0x883D,
};
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparateEXT) (GLenum modeRGB, GLenum modeAlpha);
#endif

#ifndef GL_EXT_blend_func_separate
#define GL_EXT_blend_func_separate 1
enum : GLenum
{
    GL_BLEND_DST_RGB_EXT                                    = 0x80C8,
    GL_BLEND_SRC_RGB_EXT                                    = 0x80C9,
    GL_BLEND_DST_ALPHA_EXT                                  = 0x80CA,
    GL_BLEND_SRC_ALPHA_EXT                                  = 0x80CB,
};
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparateEXT) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
#endif

#ifndef GL_EXT_blend_logic_op
#define GL_EXT_blend_logic_op 1
#endif

#ifndef GL_EXT_blend_minmax
#define GL_EXT_blend_minmax 1
enum : GLenum
{
    GL_MIN_EXT                                              = 0x8007,
    GL_MAX_EXT                                              = 0x8008,
    GL_FUNC_ADD_EXT                                         = 0x8006,
    GL_BLEND_EQUATION_EXT                                   = 0x8009,
};
extern void         (KHRONOS_APIENTRY* const& glBlendEquationEXT) (GLenum mode);
#endif

#ifndef GL_EXT_blend_subtract
#define GL_EXT_blend_subtract 1
enum : GLenum
{
    GL_FUNC_SUBTRACT_EXT                                    = 0x800A,
    GL_FUNC_REVERSE_SUBTRACT_EXT                            = 0x800B,
};
#endif

#ifndef GL_EXT_clip_volume_hint
#define GL_EXT_clip_volume_hint 1
enum : GLenum
{
    GL_CLIP_VOLUME_CLIPPING_HINT_EXT                        = 0x80F0,
};
#endif

#ifndef GL_EXT_cmyka
#define GL_EXT_cmyka 1
enum : GLenum
{
    GL_CMYK_EXT                                             = 0x800C,
    GL_CMYKA_EXT                                            = 0x800D,
    GL_PACK_CMYK_HINT_EXT                                   = 0x800E,
    GL_UNPACK_CMYK_HINT_EXT                                 = 0x800F,
};
#endif

#ifndef GL_EXT_color_subtable
#define GL_EXT_color_subtable 1
extern void         (KHRONOS_APIENTRY* const& glColorSubTableEXT) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCopyColorSubTableEXT) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
#endif

#ifndef GL_EXT_compiled_vertex_array
#define GL_EXT_compiled_vertex_array 1
enum : GLenum
{
    GL_ARRAY_ELEMENT_LOCK_FIRST_EXT                         = 0x81A8,
    GL_ARRAY_ELEMENT_LOCK_COUNT_EXT                         = 0x81A9,
};
extern void         (KHRONOS_APIENTRY* const& glLockArraysEXT) (GLint first, GLsizei count);
extern void         (KHRONOS_APIENTRY* const& glUnlockArraysEXT) ();
#endif

#ifndef GL_EXT_convolution
#define GL_EXT_convolution 1
enum : GLenum
{
    GL_CONVOLUTION_1D_EXT                                   = 0x8010,
    GL_CONVOLUTION_2D_EXT                                   = 0x8011,
    GL_SEPARABLE_2D_EXT                                     = 0x8012,
    GL_CONVOLUTION_BORDER_MODE_EXT                          = 0x8013,
    GL_CONVOLUTION_FILTER_SCALE_EXT                         = 0x8014,
    GL_CONVOLUTION_FILTER_BIAS_EXT                          = 0x8015,
    GL_REDUCE_EXT                                           = 0x8016,
    GL_CONVOLUTION_FORMAT_EXT                               = 0x8017,
    GL_CONVOLUTION_WIDTH_EXT                                = 0x8018,
    GL_CONVOLUTION_HEIGHT_EXT                               = 0x8019,
    GL_MAX_CONVOLUTION_WIDTH_EXT                            = 0x801A,
    GL_MAX_CONVOLUTION_HEIGHT_EXT                           = 0x801B,
    GL_POST_CONVOLUTION_RED_SCALE_EXT                       = 0x801C,
    GL_POST_CONVOLUTION_GREEN_SCALE_EXT                     = 0x801D,
    GL_POST_CONVOLUTION_BLUE_SCALE_EXT                      = 0x801E,
    GL_POST_CONVOLUTION_ALPHA_SCALE_EXT                     = 0x801F,
    GL_POST_CONVOLUTION_RED_BIAS_EXT                        = 0x8020,
    GL_POST_CONVOLUTION_GREEN_BIAS_EXT                      = 0x8021,
    GL_POST_CONVOLUTION_BLUE_BIAS_EXT                       = 0x8022,
    GL_POST_CONVOLUTION_ALPHA_BIAS_EXT                      = 0x8023,
};
extern void         (KHRONOS_APIENTRY* const& glConvolutionFilter1DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *image);
extern void         (KHRONOS_APIENTRY* const& glConvolutionFilter2DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *image);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameterfEXT) (GLenum target, GLenum pname, GLfloat params);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameterfvEXT) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameteriEXT) (GLenum target, GLenum pname, GLint params);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameterivEXT) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCopyConvolutionFilter1DEXT) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glCopyConvolutionFilter2DEXT) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetConvolutionFilterEXT) (GLenum target, GLenum format, GLenum type, void *image);
extern void         (KHRONOS_APIENTRY* const& glGetConvolutionParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetConvolutionParameterivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSeparableFilterEXT) (GLenum target, GLenum format, GLenum type, void *row, void *column, void *span);
extern void         (KHRONOS_APIENTRY* const& glSeparableFilter2DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *row, const void *column);
#endif

#ifndef GL_EXT_coordinate_frame
#define GL_EXT_coordinate_frame 1
enum : GLenum
{
    GL_TANGENT_ARRAY_EXT                                    = 0x8439,
    GL_BINORMAL_ARRAY_EXT                                   = 0x843A,
    GL_CURRENT_TANGENT_EXT                                  = 0x843B,
    GL_CURRENT_BINORMAL_EXT                                 = 0x843C,
    GL_TANGENT_ARRAY_TYPE_EXT                               = 0x843E,
    GL_TANGENT_ARRAY_STRIDE_EXT                             = 0x843F,
    GL_BINORMAL_ARRAY_TYPE_EXT                              = 0x8440,
    GL_BINORMAL_ARRAY_STRIDE_EXT                            = 0x8441,
    GL_TANGENT_ARRAY_POINTER_EXT                            = 0x8442,
    GL_BINORMAL_ARRAY_POINTER_EXT                           = 0x8443,
    GL_MAP1_TANGENT_EXT                                     = 0x8444,
    GL_MAP2_TANGENT_EXT                                     = 0x8445,
    GL_MAP1_BINORMAL_EXT                                    = 0x8446,
    GL_MAP2_BINORMAL_EXT                                    = 0x8447,
};
extern void         (KHRONOS_APIENTRY* const& glTangent3bEXT) (GLbyte tx, GLbyte ty, GLbyte tz);
extern void         (KHRONOS_APIENTRY* const& glTangent3bvEXT) (const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glTangent3dEXT) (GLdouble tx, GLdouble ty, GLdouble tz);
extern void         (KHRONOS_APIENTRY* const& glTangent3dvEXT) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glTangent3fEXT) (GLfloat tx, GLfloat ty, GLfloat tz);
extern void         (KHRONOS_APIENTRY* const& glTangent3fvEXT) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTangent3iEXT) (GLint tx, GLint ty, GLint tz);
extern void         (KHRONOS_APIENTRY* const& glTangent3ivEXT) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glTangent3sEXT) (GLshort tx, GLshort ty, GLshort tz);
extern void         (KHRONOS_APIENTRY* const& glTangent3svEXT) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glBinormal3bEXT) (GLbyte bx, GLbyte by, GLbyte bz);
extern void         (KHRONOS_APIENTRY* const& glBinormal3bvEXT) (const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glBinormal3dEXT) (GLdouble bx, GLdouble by, GLdouble bz);
extern void         (KHRONOS_APIENTRY* const& glBinormal3dvEXT) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glBinormal3fEXT) (GLfloat bx, GLfloat by, GLfloat bz);
extern void         (KHRONOS_APIENTRY* const& glBinormal3fvEXT) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glBinormal3iEXT) (GLint bx, GLint by, GLint bz);
extern void         (KHRONOS_APIENTRY* const& glBinormal3ivEXT) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glBinormal3sEXT) (GLshort bx, GLshort by, GLshort bz);
extern void         (KHRONOS_APIENTRY* const& glBinormal3svEXT) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glTangentPointerEXT) (GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glBinormalPointerEXT) (GLenum type, GLsizei stride, const void *pointer);
#endif

#ifndef GL_EXT_copy_texture
#define GL_EXT_copy_texture 1
extern void         (KHRONOS_APIENTRY* const& glCopyTexImage1DEXT) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyTexImage2DEXT) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage1DEXT) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage2DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage3DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#endif

#ifndef GL_EXT_cull_vertex
#define GL_EXT_cull_vertex 1
enum : GLenum
{
    GL_CULL_VERTEX_EXT                                      = 0x81AA,
    GL_CULL_VERTEX_EYE_POSITION_EXT                         = 0x81AB,
    GL_CULL_VERTEX_OBJECT_POSITION_EXT                      = 0x81AC,
};
extern void         (KHRONOS_APIENTRY* const& glCullParameterdvEXT) (GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glCullParameterfvEXT) (GLenum pname, GLfloat *params);
#endif

#ifndef GL_EXT_debug_label
#define GL_EXT_debug_label 1
enum : GLenum
{
    GL_PROGRAM_PIPELINE_OBJECT_EXT                          = 0x8A4F,
    GL_PROGRAM_OBJECT_EXT                                   = 0x8B40,
    GL_SHADER_OBJECT_EXT                                    = 0x8B48,
    GL_BUFFER_OBJECT_EXT                                    = 0x9151,
    GL_QUERY_OBJECT_EXT                                     = 0x9153,
    GL_VERTEX_ARRAY_OBJECT_EXT                              = 0x9154,
};
extern void         (KHRONOS_APIENTRY* const& glLabelObjectEXT) (GLenum type, GLuint object, GLsizei length, const GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glGetObjectLabelEXT) (GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label);
#endif

#ifndef GL_EXT_debug_marker
#define GL_EXT_debug_marker 1
extern void         (KHRONOS_APIENTRY* const& glInsertEventMarkerEXT) (GLsizei length, const GLchar *marker);
extern void         (KHRONOS_APIENTRY* const& glPushGroupMarkerEXT) (GLsizei length, const GLchar *marker);
extern void         (KHRONOS_APIENTRY* const& glPopGroupMarkerEXT) ();
#endif

#ifndef GL_EXT_depth_bounds_test
#define GL_EXT_depth_bounds_test 1
enum : GLenum
{
    GL_DEPTH_BOUNDS_TEST_EXT                                = 0x8890,
    GL_DEPTH_BOUNDS_EXT                                     = 0x8891,
};
extern void         (KHRONOS_APIENTRY* const& glDepthBoundsEXT) (GLclampd zmin, GLclampd zmax);
#endif

#ifndef GL_EXT_direct_state_access
#define GL_EXT_direct_state_access 1
enum : GLenum
{
    GL_PROGRAM_MATRIX_EXT                                   = 0x8E2D,
    GL_TRANSPOSE_PROGRAM_MATRIX_EXT                         = 0x8E2E,
    GL_PROGRAM_MATRIX_STACK_DEPTH_EXT                       = 0x8E2F,
};
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadfEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoaddEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultfEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultdEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadIdentityEXT) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glMatrixRotatefEXT) (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glMatrixRotatedEXT) (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glMatrixScalefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glMatrixScaledEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glMatrixTranslatefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glMatrixTranslatedEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glMatrixFrustumEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void         (KHRONOS_APIENTRY* const& glMatrixOrthoEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void         (KHRONOS_APIENTRY* const& glMatrixPopEXT) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glMatrixPushEXT) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glClientAttribDefaultEXT) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glPushClientAttribDefaultEXT) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterfEXT) (GLuint texture, GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterfvEXT) (GLuint texture, GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glTextureParameteriEXT) (GLuint texture, GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterivEXT) (GLuint texture, GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTextureImage1DEXT) (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTextureImage2DEXT) (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTextureSubImage1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTextureSubImage2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureImage1DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureImage2DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureSubImage1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureSubImage2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetTextureImageEXT) (GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameterfvEXT) (GLuint texture, GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameterivEXT) (GLuint texture, GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureLevelParameterfvEXT) (GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureLevelParameterivEXT) (GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTextureImage3DEXT) (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTextureSubImage3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCopyTextureSubImage3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glBindMultiTextureEXT) (GLenum texunit, GLenum target, GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoordPointerEXT) (GLenum texunit, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glMultiTexEnvfEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glMultiTexEnvfvEXT) (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexEnviEXT) (GLenum texunit, GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glMultiTexEnvivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexGendEXT) (GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
extern void         (KHRONOS_APIENTRY* const& glMultiTexGendvEXT) (GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexGenfEXT) (GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glMultiTexGenfvEXT) (GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexGeniEXT) (GLenum texunit, GLenum coord, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glMultiTexGenivEXT) (GLenum texunit, GLenum coord, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexEnvfvEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexEnvivEXT) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexGendvEXT) (GLenum texunit, GLenum coord, GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexGenfvEXT) (GLenum texunit, GLenum coord, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexGenivEXT) (GLenum texunit, GLenum coord, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexParameteriEXT) (GLenum texunit, GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glMultiTexParameterivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexParameterfEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glMultiTexParameterfvEXT) (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glMultiTexImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glMultiTexSubImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glMultiTexSubImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCopyMultiTexImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyMultiTexImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyMultiTexSubImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glCopyMultiTexSubImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexImageEXT) (GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexParameterfvEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexParameterivEXT) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexLevelParameterfvEXT) (GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexLevelParameterivEXT) (GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glMultiTexSubImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCopyMultiTexSubImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glEnableClientStateIndexedEXT) (GLenum array, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableClientStateIndexedEXT) (GLenum array, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glGetFloatIndexedvEXT) (GLenum target, GLuint index, GLfloat *data);
extern void         (KHRONOS_APIENTRY* const& glGetDoubleIndexedvEXT) (GLenum target, GLuint index, GLdouble *data);
extern void         (KHRONOS_APIENTRY* const& glGetPointerIndexedvEXT) (GLenum target, GLuint index, void **data);
extern void         (KHRONOS_APIENTRY* const& glEnableIndexedEXT) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableIndexedEXT) (GLenum target, GLuint index);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnabledIndexedEXT) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glGetIntegerIndexedvEXT) (GLenum target, GLuint index, GLint *data);
extern void         (KHRONOS_APIENTRY* const& glGetBooleanIndexedvEXT) (GLenum target, GLuint index, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureImage3DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureImage2DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureImage1DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureSubImage3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureSubImage2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedTextureSubImage1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glGetCompressedTextureImageEXT) (GLuint texture, GLenum target, GLint lod, void *img);
extern void         (KHRONOS_APIENTRY* const& glCompressedMultiTexImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedMultiTexImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedMultiTexImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedMultiTexSubImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedMultiTexSubImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glCompressedMultiTexSubImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *bits);
extern void         (KHRONOS_APIENTRY* const& glGetCompressedMultiTexImageEXT) (GLenum texunit, GLenum target, GLint lod, void *img);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadTransposefEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadTransposedEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultTransposefEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultTransposedEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferDataEXT) (GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferSubDataEXT) (GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
extern void *       (KHRONOS_APIENTRY* const& glMapNamedBufferEXT) (GLuint buffer, GLenum access);
extern GLboolean    (KHRONOS_APIENTRY* const& glUnmapNamedBufferEXT) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferParameterivEXT) (GLuint buffer, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferPointervEXT) (GLuint buffer, GLenum pname, void **params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferSubDataEXT) (GLuint buffer, GLintptr offset, GLsizeiptr size, void *data);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1fEXT) (GLuint program, GLint location, GLfloat v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1iEXT) (GLuint program, GLint location, GLint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2iEXT) (GLuint program, GLint location, GLint v0, GLint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3iEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4iEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glTextureBufferEXT) (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glMultiTexBufferEXT) (GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterIivEXT) (GLuint texture, GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTextureParameterIuivEXT) (GLuint texture, GLenum target, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameterIivEXT) (GLuint texture, GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTextureParameterIuivEXT) (GLuint texture, GLenum target, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexParameterIivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glMultiTexParameterIuivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexParameterIivEXT) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMultiTexParameterIuivEXT) (GLenum texunit, GLenum target, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1uiEXT) (GLuint program, GLint location, GLuint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameters4fvEXT) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameterI4iEXT) (GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameterI4ivEXT) (GLuint program, GLenum target, GLuint index, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParametersI4ivEXT) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameterI4uiEXT) (GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameterI4uivEXT) (GLuint program, GLenum target, GLuint index, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParametersI4uivEXT) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedProgramLocalParameterIivEXT) (GLuint program, GLenum target, GLuint index, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedProgramLocalParameterIuivEXT) (GLuint program, GLenum target, GLuint index, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glEnableClientStateiEXT) (GLenum array, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableClientStateiEXT) (GLenum array, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glGetFloati_vEXT) (GLenum pname, GLuint index, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetDoublei_vEXT) (GLenum pname, GLuint index, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetPointeri_vEXT) (GLenum pname, GLuint index, void **params);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramStringEXT) (GLuint program, GLenum target, GLenum format, GLsizei len, const void *string);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameter4dEXT) (GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameter4dvEXT) (GLuint program, GLenum target, GLuint index, const GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameter4fEXT) (GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glNamedProgramLocalParameter4fvEXT) (GLuint program, GLenum target, GLuint index, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedProgramLocalParameterdvEXT) (GLuint program, GLenum target, GLuint index, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedProgramLocalParameterfvEXT) (GLuint program, GLenum target, GLuint index, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedProgramivEXT) (GLuint program, GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedProgramStringEXT) (GLuint program, GLenum target, GLenum pname, void *string);
extern void         (KHRONOS_APIENTRY* const& glNamedRenderbufferStorageEXT) (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetNamedRenderbufferParameterivEXT) (GLuint renderbuffer, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glNamedRenderbufferStorageMultisampleEXT) (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glNamedRenderbufferStorageMultisampleCoverageEXT) (GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
extern GLenum       (KHRONOS_APIENTRY* const& glCheckNamedFramebufferStatusEXT) (GLuint framebuffer, GLenum target);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTexture1DEXT) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTexture2DEXT) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTexture3DEXT) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferRenderbufferEXT) (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glGetNamedFramebufferAttachmentParameterivEXT) (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGenerateTextureMipmapEXT) (GLuint texture, GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGenerateMultiTexMipmapEXT) (GLenum texunit, GLenum target);
extern void         (KHRONOS_APIENTRY* const& glFramebufferDrawBufferEXT) (GLuint framebuffer, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glFramebufferDrawBuffersEXT) (GLuint framebuffer, GLsizei n, const GLenum *bufs);
extern void         (KHRONOS_APIENTRY* const& glFramebufferReadBufferEXT) (GLuint framebuffer, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferParameterivEXT) (GLuint framebuffer, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glNamedCopyBufferSubDataEXT) (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTextureEXT) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTextureLayerEXT) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferTextureFaceEXT) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face);
extern void         (KHRONOS_APIENTRY* const& glTextureRenderbufferEXT) (GLuint texture, GLenum target, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glMultiTexRenderbufferEXT) (GLenum texunit, GLenum target, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexOffsetEXT) (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayColorOffsetEXT) (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayEdgeFlagOffsetEXT) (GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayIndexOffsetEXT) (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayNormalOffsetEXT) (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayTexCoordOffsetEXT) (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayMultiTexCoordOffsetEXT) (GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayFogCoordOffsetEXT) (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArraySecondaryColorOffsetEXT) (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribOffsetEXT) (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribIOffsetEXT) (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glEnableVertexArrayEXT) (GLuint vaobj, GLenum array);
extern void         (KHRONOS_APIENTRY* const& glDisableVertexArrayEXT) (GLuint vaobj, GLenum array);
extern void         (KHRONOS_APIENTRY* const& glEnableVertexArrayAttribEXT) (GLuint vaobj, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableVertexArrayAttribEXT) (GLuint vaobj, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glGetVertexArrayIntegervEXT) (GLuint vaobj, GLenum pname, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetVertexArrayPointervEXT) (GLuint vaobj, GLenum pname, void **param);
extern void         (KHRONOS_APIENTRY* const& glGetVertexArrayIntegeri_vEXT) (GLuint vaobj, GLuint index, GLenum pname, GLint *param);
extern void         (KHRONOS_APIENTRY* const& glGetVertexArrayPointeri_vEXT) (GLuint vaobj, GLuint index, GLenum pname, void **param);
extern void *       (KHRONOS_APIENTRY* const& glMapNamedBufferRangeEXT) (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern void         (KHRONOS_APIENTRY* const& glFlushMappedNamedBufferRangeEXT) (GLuint buffer, GLintptr offset, GLsizeiptr length);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferStorageEXT) (GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
extern void         (KHRONOS_APIENTRY* const& glClearNamedBufferDataEXT) (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glClearNamedBufferSubDataEXT) (GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferParameteriEXT) (GLuint framebuffer, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glGetNamedFramebufferParameterivEXT) (GLuint framebuffer, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1dEXT) (GLuint program, GLint location, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2dEXT) (GLuint program, GLint location, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3dEXT) (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4dEXT) (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1dvEXT) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2dvEXT) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3dvEXT) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4dvEXT) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x3dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x4dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x2dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x4dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x2dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x3dvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
extern void         (KHRONOS_APIENTRY* const& glTextureBufferRangeEXT) (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage1DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage2DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage3DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage2DMultisampleEXT) (GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage3DMultisampleEXT) (GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayBindVertexBufferEXT) (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribFormatEXT) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribIFormatEXT) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribLFormatEXT) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribBindingEXT) (GLuint vaobj, GLuint attribindex, GLuint bindingindex);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexBindingDivisorEXT) (GLuint vaobj, GLuint bindingindex, GLuint divisor);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribLOffsetEXT) (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glTexturePageCommitmentEXT) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
extern void         (KHRONOS_APIENTRY* const& glVertexArrayVertexAttribDivisorEXT) (GLuint vaobj, GLuint index, GLuint divisor);
#endif

#ifndef GL_EXT_draw_buffers2
#define GL_EXT_draw_buffers2 1
extern void         (KHRONOS_APIENTRY* const& glColorMaskIndexedEXT) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
#endif

#ifndef GL_EXT_draw_instanced
#define GL_EXT_draw_instanced 1
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstancedEXT) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedEXT) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#endif

#ifndef GL_EXT_draw_range_elements
#define GL_EXT_draw_range_elements 1
enum : GLenum
{
    GL_MAX_ELEMENTS_VERTICES_EXT                            = 0x80E8,
    GL_MAX_ELEMENTS_INDICES_EXT                             = 0x80E9,
};
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElementsEXT) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
#endif

#ifndef GL_EXT_external_buffer
#define GL_EXT_external_buffer 1
extern void         (KHRONOS_APIENTRY* const& glBufferStorageExternalEXT) (GLenum target, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferStorageExternalEXT) (GLuint buffer, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags);
#endif

#ifndef GL_EXT_fog_coord
#define GL_EXT_fog_coord 1
enum : GLenum
{
    GL_FOG_COORDINATE_SOURCE_EXT                            = 0x8450,
    GL_FOG_COORDINATE_EXT                                   = 0x8451,
    GL_FRAGMENT_DEPTH_EXT                                   = 0x8452,
    GL_CURRENT_FOG_COORDINATE_EXT                           = 0x8453,
    GL_FOG_COORDINATE_ARRAY_TYPE_EXT                        = 0x8454,
    GL_FOG_COORDINATE_ARRAY_STRIDE_EXT                      = 0x8455,
    GL_FOG_COORDINATE_ARRAY_POINTER_EXT                     = 0x8456,
    GL_FOG_COORDINATE_ARRAY_EXT                             = 0x8457,
};
extern void         (KHRONOS_APIENTRY* const& glFogCoordfEXT) (GLfloat coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoordfvEXT) (const GLfloat *coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoorddEXT) (GLdouble coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoorddvEXT) (const GLdouble *coord);
extern void         (KHRONOS_APIENTRY* const& glFogCoordPointerEXT) (GLenum type, GLsizei stride, const void *pointer);
#endif

#ifndef GL_EXT_framebuffer_blit
#define GL_EXT_framebuffer_blit 1
enum : GLenum
{
    GL_READ_FRAMEBUFFER_EXT                                 = 0x8CA8,
    GL_DRAW_FRAMEBUFFER_EXT                                 = 0x8CA9,
    GL_DRAW_FRAMEBUFFER_BINDING_EXT                         = 0x8CA6,
    GL_READ_FRAMEBUFFER_BINDING_EXT                         = 0x8CAA,
};
extern void         (KHRONOS_APIENTRY* const& glBlitFramebufferEXT) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#endif

#ifndef GL_EXT_framebuffer_multisample
#define GL_EXT_framebuffer_multisample 1
enum : GLenum
{
    GL_RENDERBUFFER_SAMPLES_EXT                             = 0x8CAB,
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT               = 0x8D56,
    GL_MAX_SAMPLES_EXT                                      = 0x8D57,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleEXT) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif

#ifndef GL_EXT_framebuffer_multisample_blit_scaled
#define GL_EXT_framebuffer_multisample_blit_scaled 1
enum : GLenum
{
    GL_SCALED_RESOLVE_FASTEST_EXT                           = 0x90BA,
    GL_SCALED_RESOLVE_NICEST_EXT                            = 0x90BB,
};
#endif

#ifndef GL_EXT_framebuffer_object
#define GL_EXT_framebuffer_object 1
enum : GLenum
{
    GL_INVALID_FRAMEBUFFER_OPERATION_EXT                    = 0x0506,
    GL_MAX_RENDERBUFFER_SIZE_EXT                            = 0x84E8,
    GL_FRAMEBUFFER_BINDING_EXT                              = 0x8CA6,
    GL_RENDERBUFFER_BINDING_EXT                             = 0x8CA7,
    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT               = 0x8CD0,
    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT               = 0x8CD1,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT             = 0x8CD2,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT     = 0x8CD3,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT        = 0x8CD4,
    GL_FRAMEBUFFER_COMPLETE_EXT                             = 0x8CD5,
    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT                = 0x8CD6,
    GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT        = 0x8CD7,
    GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT                = 0x8CD9,
    GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                   = 0x8CDA,
    GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT               = 0x8CDB,
    GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT               = 0x8CDC,
    GL_FRAMEBUFFER_UNSUPPORTED_EXT                          = 0x8CDD,
    GL_MAX_COLOR_ATTACHMENTS_EXT                            = 0x8CDF,
    GL_COLOR_ATTACHMENT0_EXT                                = 0x8CE0,
    GL_COLOR_ATTACHMENT1_EXT                                = 0x8CE1,
    GL_COLOR_ATTACHMENT2_EXT                                = 0x8CE2,
    GL_COLOR_ATTACHMENT3_EXT                                = 0x8CE3,
    GL_COLOR_ATTACHMENT4_EXT                                = 0x8CE4,
    GL_COLOR_ATTACHMENT5_EXT                                = 0x8CE5,
    GL_COLOR_ATTACHMENT6_EXT                                = 0x8CE6,
    GL_COLOR_ATTACHMENT7_EXT                                = 0x8CE7,
    GL_COLOR_ATTACHMENT8_EXT                                = 0x8CE8,
    GL_COLOR_ATTACHMENT9_EXT                                = 0x8CE9,
    GL_COLOR_ATTACHMENT10_EXT                               = 0x8CEA,
    GL_COLOR_ATTACHMENT11_EXT                               = 0x8CEB,
    GL_COLOR_ATTACHMENT12_EXT                               = 0x8CEC,
    GL_COLOR_ATTACHMENT13_EXT                               = 0x8CED,
    GL_COLOR_ATTACHMENT14_EXT                               = 0x8CEE,
    GL_COLOR_ATTACHMENT15_EXT                               = 0x8CEF,
    GL_DEPTH_ATTACHMENT_EXT                                 = 0x8D00,
    GL_STENCIL_ATTACHMENT_EXT                               = 0x8D20,
    GL_FRAMEBUFFER_EXT                                      = 0x8D40,
    GL_RENDERBUFFER_EXT                                     = 0x8D41,
    GL_RENDERBUFFER_WIDTH_EXT                               = 0x8D42,
    GL_RENDERBUFFER_HEIGHT_EXT                              = 0x8D43,
    GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                     = 0x8D44,
    GL_STENCIL_INDEX1_EXT                                   = 0x8D46,
    GL_STENCIL_INDEX4_EXT                                   = 0x8D47,
    GL_STENCIL_INDEX8_EXT                                   = 0x8D48,
    GL_STENCIL_INDEX16_EXT                                  = 0x8D49,
    GL_RENDERBUFFER_RED_SIZE_EXT                            = 0x8D50,
    GL_RENDERBUFFER_GREEN_SIZE_EXT                          = 0x8D51,
    GL_RENDERBUFFER_BLUE_SIZE_EXT                           = 0x8D52,
    GL_RENDERBUFFER_ALPHA_SIZE_EXT                          = 0x8D53,
    GL_RENDERBUFFER_DEPTH_SIZE_EXT                          = 0x8D54,
    GL_RENDERBUFFER_STENCIL_SIZE_EXT                        = 0x8D55,
};
extern GLboolean    (KHRONOS_APIENTRY* const& glIsRenderbufferEXT) (GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glBindRenderbufferEXT) (GLenum target, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glDeleteRenderbuffersEXT) (GLsizei n, const GLuint *renderbuffers);
extern void         (KHRONOS_APIENTRY* const& glGenRenderbuffersEXT) (GLsizei n, GLuint *renderbuffers);
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glGetRenderbufferParameterivEXT) (GLenum target, GLenum pname, GLint *params);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsFramebufferEXT) (GLuint framebuffer);
extern void         (KHRONOS_APIENTRY* const& glBindFramebufferEXT) (GLenum target, GLuint framebuffer);
extern void         (KHRONOS_APIENTRY* const& glDeleteFramebuffersEXT) (GLsizei n, const GLuint *framebuffers);
extern void         (KHRONOS_APIENTRY* const& glGenFramebuffersEXT) (GLsizei n, GLuint *framebuffers);
extern GLenum       (KHRONOS_APIENTRY* const& glCheckFramebufferStatusEXT) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture1DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture2DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture3DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
extern void         (KHRONOS_APIENTRY* const& glFramebufferRenderbufferEXT) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferAttachmentParameterivEXT) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGenerateMipmapEXT) (GLenum target);
#endif

#ifndef GL_EXT_framebuffer_sRGB
#define GL_EXT_framebuffer_sRGB 1
enum : GLenum
{
    GL_FRAMEBUFFER_SRGB_EXT                                 = 0x8DB9,
    GL_FRAMEBUFFER_SRGB_CAPABLE_EXT                         = 0x8DBA,
};
#endif

#ifndef GL_EXT_geometry_shader4
#define GL_EXT_geometry_shader4 1
enum : GLenum
{
    GL_GEOMETRY_SHADER_EXT                                  = 0x8DD9,
    GL_GEOMETRY_VERTICES_OUT_EXT                            = 0x8DDA,
    GL_GEOMETRY_INPUT_TYPE_EXT                              = 0x8DDB,
    GL_GEOMETRY_OUTPUT_TYPE_EXT                             = 0x8DDC,
    GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT                 = 0x8C29,
    GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT                  = 0x8DDD,
    GL_MAX_VERTEX_VARYING_COMPONENTS_EXT                    = 0x8DDE,
    GL_MAX_VARYING_COMPONENTS_EXT                           = 0x8B4B,
    GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT                  = 0x8DDF,
    GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT                     = 0x8DE0,
    GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT             = 0x8DE1,
    GL_LINES_ADJACENCY_EXT                                  = 0x000A,
    GL_LINE_STRIP_ADJACENCY_EXT                             = 0x000B,
    GL_TRIANGLES_ADJACENCY_EXT                              = 0x000C,
    GL_TRIANGLE_STRIP_ADJACENCY_EXT                         = 0x000D,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT             = 0x8DA8,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT               = 0x8DA9,
    GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT                   = 0x8DA7,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT             = 0x8CD4,
    GL_PROGRAM_POINT_SIZE_EXT                               = 0x8642,
};
extern void         (KHRONOS_APIENTRY* const& glProgramParameteriEXT) (GLuint program, GLenum pname, GLint value);
#endif

#ifndef GL_EXT_gpu_program_parameters
#define GL_EXT_gpu_program_parameters 1
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameters4fvEXT) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameters4fvEXT) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif

#ifndef GL_EXT_gpu_shader4
#define GL_EXT_gpu_shader4 1
enum : GLenum
{
    GL_SAMPLER_1D_ARRAY_EXT                                 = 0x8DC0,
    GL_SAMPLER_2D_ARRAY_EXT                                 = 0x8DC1,
    GL_SAMPLER_BUFFER_EXT                                   = 0x8DC2,
    GL_SAMPLER_1D_ARRAY_SHADOW_EXT                          = 0x8DC3,
    GL_SAMPLER_2D_ARRAY_SHADOW_EXT                          = 0x8DC4,
    GL_SAMPLER_CUBE_SHADOW_EXT                              = 0x8DC5,
    GL_UNSIGNED_INT_VEC2_EXT                                = 0x8DC6,
    GL_UNSIGNED_INT_VEC3_EXT                                = 0x8DC7,
    GL_UNSIGNED_INT_VEC4_EXT                                = 0x8DC8,
    GL_INT_SAMPLER_1D_EXT                                   = 0x8DC9,
    GL_INT_SAMPLER_2D_EXT                                   = 0x8DCA,
    GL_INT_SAMPLER_3D_EXT                                   = 0x8DCB,
    GL_INT_SAMPLER_CUBE_EXT                                 = 0x8DCC,
    GL_INT_SAMPLER_2D_RECT_EXT                              = 0x8DCD,
    GL_INT_SAMPLER_1D_ARRAY_EXT                             = 0x8DCE,
    GL_INT_SAMPLER_2D_ARRAY_EXT                             = 0x8DCF,
    GL_INT_SAMPLER_BUFFER_EXT                               = 0x8DD0,
    GL_UNSIGNED_INT_SAMPLER_1D_EXT                          = 0x8DD1,
    GL_UNSIGNED_INT_SAMPLER_2D_EXT                          = 0x8DD2,
    GL_UNSIGNED_INT_SAMPLER_3D_EXT                          = 0x8DD3,
    GL_UNSIGNED_INT_SAMPLER_CUBE_EXT                        = 0x8DD4,
    GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT                     = 0x8DD5,
    GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT                    = 0x8DD6,
    GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT                    = 0x8DD7,
    GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT                      = 0x8DD8,
    GL_MIN_PROGRAM_TEXEL_OFFSET_EXT                         = 0x8904,
    GL_MAX_PROGRAM_TEXEL_OFFSET_EXT                         = 0x8905,
    GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT                      = 0x88FD,
};
extern void         (KHRONOS_APIENTRY* const& glGetUniformuivEXT) (GLuint program, GLint location, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glBindFragDataLocationEXT) (GLuint program, GLuint color, const GLchar *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetFragDataLocationEXT) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glUniform1uiEXT) (GLint location, GLuint v0);
extern void         (KHRONOS_APIENTRY* const& glUniform2uiEXT) (GLint location, GLuint v0, GLuint v1);
extern void         (KHRONOS_APIENTRY* const& glUniform3uiEXT) (GLint location, GLuint v0, GLuint v1, GLuint v2);
extern void         (KHRONOS_APIENTRY* const& glUniform4uiEXT) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern void         (KHRONOS_APIENTRY* const& glUniform1uivEXT) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2uivEXT) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3uivEXT) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4uivEXT) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1iEXT) (GLuint index, GLint x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2iEXT) (GLuint index, GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3iEXT) (GLuint index, GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4iEXT) (GLuint index, GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1uiEXT) (GLuint index, GLuint x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2uiEXT) (GLuint index, GLuint x, GLuint y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3uiEXT) (GLuint index, GLuint x, GLuint y, GLuint z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4uiEXT) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1ivEXT) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2ivEXT) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3ivEXT) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4ivEXT) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI1uivEXT) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI2uivEXT) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI3uivEXT) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4uivEXT) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4bvEXT) (GLuint index, const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4svEXT) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4ubvEXT) (GLuint index, const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4usvEXT) (GLuint index, const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribIPointerEXT) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribIivEXT) (GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribIuivEXT) (GLuint index, GLenum pname, GLuint *params);
#endif

#ifndef GL_EXT_histogram
#define GL_EXT_histogram 1
enum : GLenum
{
    GL_HISTOGRAM_EXT                                        = 0x8024,
    GL_PROXY_HISTOGRAM_EXT                                  = 0x8025,
    GL_HISTOGRAM_WIDTH_EXT                                  = 0x8026,
    GL_HISTOGRAM_FORMAT_EXT                                 = 0x8027,
    GL_HISTOGRAM_RED_SIZE_EXT                               = 0x8028,
    GL_HISTOGRAM_GREEN_SIZE_EXT                             = 0x8029,
    GL_HISTOGRAM_BLUE_SIZE_EXT                              = 0x802A,
    GL_HISTOGRAM_ALPHA_SIZE_EXT                             = 0x802B,
    GL_HISTOGRAM_LUMINANCE_SIZE_EXT                         = 0x802C,
    GL_HISTOGRAM_SINK_EXT                                   = 0x802D,
    GL_MINMAX_EXT                                           = 0x802E,
    GL_MINMAX_FORMAT_EXT                                    = 0x802F,
    GL_MINMAX_SINK_EXT                                      = 0x8030,
    GL_TABLE_TOO_LARGE_EXT                                  = 0x8031,
};
extern void         (KHRONOS_APIENTRY* const& glGetHistogramEXT) (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values);
extern void         (KHRONOS_APIENTRY* const& glGetHistogramParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetHistogramParameterivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMinmaxEXT) (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values);
extern void         (KHRONOS_APIENTRY* const& glGetMinmaxParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMinmaxParameterivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glHistogramEXT) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
extern void         (KHRONOS_APIENTRY* const& glMinmaxEXT) (GLenum target, GLenum internalformat, GLboolean sink);
extern void         (KHRONOS_APIENTRY* const& glResetHistogramEXT) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glResetMinmaxEXT) (GLenum target);
#endif

#ifndef GL_EXT_index_array_formats
#define GL_EXT_index_array_formats 1
enum : GLenum
{
    GL_IUI_V2F_EXT                                          = 0x81AD,
    GL_IUI_V3F_EXT                                          = 0x81AE,
    GL_IUI_N3F_V2F_EXT                                      = 0x81AF,
    GL_IUI_N3F_V3F_EXT                                      = 0x81B0,
    GL_T2F_IUI_V2F_EXT                                      = 0x81B1,
    GL_T2F_IUI_V3F_EXT                                      = 0x81B2,
    GL_T2F_IUI_N3F_V2F_EXT                                  = 0x81B3,
    GL_T2F_IUI_N3F_V3F_EXT                                  = 0x81B4,
};
#endif

#ifndef GL_EXT_index_func
#define GL_EXT_index_func 1
enum : GLenum
{
    GL_INDEX_TEST_EXT                                       = 0x81B5,
    GL_INDEX_TEST_FUNC_EXT                                  = 0x81B6,
    GL_INDEX_TEST_REF_EXT                                   = 0x81B7,
};
extern void         (KHRONOS_APIENTRY* const& glIndexFuncEXT) (GLenum func, GLclampf ref);
#endif

#ifndef GL_EXT_index_material
#define GL_EXT_index_material 1
enum : GLenum
{
    GL_INDEX_MATERIAL_EXT                                   = 0x81B8,
    GL_INDEX_MATERIAL_PARAMETER_EXT                         = 0x81B9,
    GL_INDEX_MATERIAL_FACE_EXT                              = 0x81BA,
};
extern void         (KHRONOS_APIENTRY* const& glIndexMaterialEXT) (GLenum face, GLenum mode);
#endif

#ifndef GL_EXT_index_texture
#define GL_EXT_index_texture 1
#endif

#ifndef GL_EXT_light_texture
#define GL_EXT_light_texture 1
enum : GLenum
{
    GL_FRAGMENT_MATERIAL_EXT                                = 0x8349,
    GL_FRAGMENT_NORMAL_EXT                                  = 0x834A,
    GL_FRAGMENT_COLOR_EXT                                   = 0x834C,
    GL_ATTENUATION_EXT                                      = 0x834D,
    GL_SHADOW_ATTENUATION_EXT                               = 0x834E,
    GL_TEXTURE_APPLICATION_MODE_EXT                         = 0x834F,
    GL_TEXTURE_LIGHT_EXT                                    = 0x8350,
    GL_TEXTURE_MATERIAL_FACE_EXT                            = 0x8351,
    GL_TEXTURE_MATERIAL_PARAMETER_EXT                       = 0x8352,
};
extern void         (KHRONOS_APIENTRY* const& glApplyTextureEXT) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glTextureLightEXT) (GLenum pname);
extern void         (KHRONOS_APIENTRY* const& glTextureMaterialEXT) (GLenum face, GLenum mode);
#endif

#ifndef GL_EXT_memory_object
#define GL_EXT_memory_object 1
enum : GLenum
{
    GL_TEXTURE_TILING_EXT                                   = 0x9580,
    GL_DEDICATED_MEMORY_OBJECT_EXT                          = 0x9581,
    GL_PROTECTED_MEMORY_OBJECT_EXT                          = 0x959B,
    GL_NUM_TILING_TYPES_EXT                                 = 0x9582,
    GL_TILING_TYPES_EXT                                     = 0x9583,
    GL_OPTIMAL_TILING_EXT                                   = 0x9584,
    GL_LINEAR_TILING_EXT                                    = 0x9585,
    GL_NUM_DEVICE_UUIDS_EXT                                 = 0x9596,
    GL_DEVICE_UUID_EXT                                      = 0x9597,
    GL_DRIVER_UUID_EXT                                      = 0x9598,
    GL_UUID_SIZE_EXT                                        = 16,
};
extern void         (KHRONOS_APIENTRY* const& glGetUnsignedBytevEXT) (GLenum pname, GLubyte *data);
extern void         (KHRONOS_APIENTRY* const& glGetUnsignedBytei_vEXT) (GLenum target, GLuint index, GLubyte *data);
extern void         (KHRONOS_APIENTRY* const& glDeleteMemoryObjectsEXT) (GLsizei n, const GLuint *memoryObjects);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsMemoryObjectEXT) (GLuint memoryObject);
extern void         (KHRONOS_APIENTRY* const& glCreateMemoryObjectsEXT) (GLsizei n, GLuint *memoryObjects);
extern void         (KHRONOS_APIENTRY* const& glMemoryObjectParameterivEXT) (GLuint memoryObject, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMemoryObjectParameterivEXT) (GLuint memoryObject, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexStorageMem2DEXT) (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTexStorageMem2DMultisampleEXT) (GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTexStorageMem3DEXT) (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTexStorageMem3DMultisampleEXT) (GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glBufferStorageMemEXT) (GLenum target, GLsizeiptr size, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTextureStorageMem2DEXT) (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTextureStorageMem2DMultisampleEXT) (GLuint texture, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTextureStorageMem3DEXT) (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTextureStorageMem3DMultisampleEXT) (GLuint texture, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferStorageMemEXT) (GLuint buffer, GLsizeiptr size, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTexStorageMem1DEXT) (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTextureStorageMem1DEXT) (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLuint memory, GLuint64 offset);
#endif

#ifndef GL_EXT_memory_object_fd
#define GL_EXT_memory_object_fd 1
enum : GLenum
{
    GL_HANDLE_TYPE_OPAQUE_FD_EXT                            = 0x9586,
};
extern void         (KHRONOS_APIENTRY* const& glImportMemoryFdEXT) (GLuint memory, GLuint64 size, GLenum handleType, GLint fd);
#endif

#ifndef GL_EXT_memory_object_win32
#define GL_EXT_memory_object_win32 1
enum : GLenum
{
    GL_HANDLE_TYPE_OPAQUE_WIN32_EXT                         = 0x9587,
    GL_HANDLE_TYPE_OPAQUE_WIN32_KMT_EXT                     = 0x9588,
    GL_DEVICE_LUID_EXT                                      = 0x9599,
    GL_DEVICE_NODE_MASK_EXT                                 = 0x959A,
    GL_LUID_SIZE_EXT                                        = 8,
    GL_HANDLE_TYPE_D3D12_TILEPOOL_EXT                       = 0x9589,
    GL_HANDLE_TYPE_D3D12_RESOURCE_EXT                       = 0x958A,
    GL_HANDLE_TYPE_D3D11_IMAGE_EXT                          = 0x958B,
    GL_HANDLE_TYPE_D3D11_IMAGE_KMT_EXT                      = 0x958C,
};
extern void         (KHRONOS_APIENTRY* const& glImportMemoryWin32HandleEXT) (GLuint memory, GLuint64 size, GLenum handleType, void *handle);
extern void         (KHRONOS_APIENTRY* const& glImportMemoryWin32NameEXT) (GLuint memory, GLuint64 size, GLenum handleType, const void *name);
#endif

#ifndef GL_EXT_misc_attribute
#define GL_EXT_misc_attribute 1
#endif

#ifndef GL_EXT_multi_draw_arrays
#define GL_EXT_multi_draw_arrays 1
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysEXT) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsEXT) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount);
#endif

#ifndef GL_EXT_multisample
#define GL_EXT_multisample 1
enum : GLenum
{
    GL_MULTISAMPLE_EXT                                      = 0x809D,
    GL_SAMPLE_ALPHA_TO_MASK_EXT                             = 0x809E,
    GL_SAMPLE_ALPHA_TO_ONE_EXT                              = 0x809F,
    GL_SAMPLE_MASK_EXT                                      = 0x80A0,
    GL_1PASS_EXT                                            = 0x80A1,
    GL_2PASS_0_EXT                                          = 0x80A2,
    GL_2PASS_1_EXT                                          = 0x80A3,
    GL_4PASS_0_EXT                                          = 0x80A4,
    GL_4PASS_1_EXT                                          = 0x80A5,
    GL_4PASS_2_EXT                                          = 0x80A6,
    GL_4PASS_3_EXT                                          = 0x80A7,
    GL_SAMPLE_BUFFERS_EXT                                   = 0x80A8,
    GL_SAMPLES_EXT                                          = 0x80A9,
    GL_SAMPLE_MASK_VALUE_EXT                                = 0x80AA,
    GL_SAMPLE_MASK_INVERT_EXT                               = 0x80AB,
    GL_SAMPLE_PATTERN_EXT                                   = 0x80AC,
    GL_MULTISAMPLE_BIT_EXT                                  = 0x20000000,
};
extern void         (KHRONOS_APIENTRY* const& glSampleMaskEXT) (GLclampf value, GLboolean invert);
extern void         (KHRONOS_APIENTRY* const& glSamplePatternEXT) (GLenum pattern);
#endif

#ifndef GL_EXT_multiview_tessellation_geometry_shader
#define GL_EXT_multiview_tessellation_geometry_shader 1
#endif

#ifndef GL_EXT_multiview_texture_multisample
#define GL_EXT_multiview_texture_multisample 1
#endif

#ifndef GL_EXT_multiview_timer_query
#define GL_EXT_multiview_timer_query 1
#endif

#ifndef GL_EXT_packed_depth_stencil
#define GL_EXT_packed_depth_stencil 1
enum : GLenum
{
    GL_DEPTH_STENCIL_EXT                                    = 0x84F9,
    GL_UNSIGNED_INT_24_8_EXT                                = 0x84FA,
    GL_DEPTH24_STENCIL8_EXT                                 = 0x88F0,
    GL_TEXTURE_STENCIL_SIZE_EXT                             = 0x88F1,
};
#endif

#ifndef GL_EXT_packed_float
#define GL_EXT_packed_float 1
enum : GLenum
{
    GL_R11F_G11F_B10F_EXT                                   = 0x8C3A,
    GL_UNSIGNED_INT_10F_11F_11F_REV_EXT                     = 0x8C3B,
    GL_RGBA_SIGNED_COMPONENTS_EXT                           = 0x8C3C,
};
#endif

#ifndef GL_EXT_packed_pixels
#define GL_EXT_packed_pixels 1
enum : GLenum
{
    GL_UNSIGNED_BYTE_3_3_2_EXT                              = 0x8032,
    GL_UNSIGNED_SHORT_4_4_4_4_EXT                           = 0x8033,
    GL_UNSIGNED_SHORT_5_5_5_1_EXT                           = 0x8034,
    GL_UNSIGNED_INT_8_8_8_8_EXT                             = 0x8035,
    GL_UNSIGNED_INT_10_10_10_2_EXT                          = 0x8036,
};
#endif

#ifndef GL_EXT_paletted_texture
#define GL_EXT_paletted_texture 1
enum : GLenum
{
    GL_COLOR_INDEX1_EXT                                     = 0x80E2,
    GL_COLOR_INDEX2_EXT                                     = 0x80E3,
    GL_COLOR_INDEX4_EXT                                     = 0x80E4,
    GL_COLOR_INDEX8_EXT                                     = 0x80E5,
    GL_COLOR_INDEX12_EXT                                    = 0x80E6,
    GL_COLOR_INDEX16_EXT                                    = 0x80E7,
    GL_TEXTURE_INDEX_SIZE_EXT                               = 0x80ED,
};
extern void         (KHRONOS_APIENTRY* const& glColorTableEXT) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void *table);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableEXT) (GLenum target, GLenum format, GLenum type, void *data);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableParameterivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
#endif

#ifndef GL_EXT_pixel_buffer_object
#define GL_EXT_pixel_buffer_object 1
enum : GLenum
{
    GL_PIXEL_PACK_BUFFER_EXT                                = 0x88EB,
    GL_PIXEL_UNPACK_BUFFER_EXT                              = 0x88EC,
    GL_PIXEL_PACK_BUFFER_BINDING_EXT                        = 0x88ED,
    GL_PIXEL_UNPACK_BUFFER_BINDING_EXT                      = 0x88EF,
};
#endif

#ifndef GL_EXT_pixel_transform
#define GL_EXT_pixel_transform 1
enum : GLenum
{
    GL_PIXEL_TRANSFORM_2D_EXT                               = 0x8330,
    GL_PIXEL_MAG_FILTER_EXT                                 = 0x8331,
    GL_PIXEL_MIN_FILTER_EXT                                 = 0x8332,
    GL_PIXEL_CUBIC_WEIGHT_EXT                               = 0x8333,
    GL_CUBIC_EXT                                            = 0x8334,
    GL_AVERAGE_EXT                                          = 0x8335,
    GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT                   = 0x8336,
    GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT               = 0x8337,
    GL_PIXEL_TRANSFORM_2D_MATRIX_EXT                        = 0x8338,
};
extern void         (KHRONOS_APIENTRY* const& glPixelTransformParameteriEXT) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glPixelTransformParameterfEXT) (GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPixelTransformParameterivEXT) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glPixelTransformParameterfvEXT) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetPixelTransformParameterivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetPixelTransformParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
#endif

#ifndef GL_EXT_pixel_transform_color_table
#define GL_EXT_pixel_transform_color_table 1
#endif

#ifndef GL_EXT_point_parameters
#define GL_EXT_point_parameters 1
enum : GLenum
{
    GL_POINT_SIZE_MIN_EXT                                   = 0x8126,
    GL_POINT_SIZE_MAX_EXT                                   = 0x8127,
    GL_POINT_FADE_THRESHOLD_SIZE_EXT                        = 0x8128,
    GL_DISTANCE_ATTENUATION_EXT                             = 0x8129,
};
extern void         (KHRONOS_APIENTRY* const& glPointParameterfEXT) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPointParameterfvEXT) (GLenum pname, const GLfloat *params);
#endif

#ifndef GL_EXT_polygon_offset
#define GL_EXT_polygon_offset 1
enum : GLenum
{
    GL_POLYGON_OFFSET_EXT                                   = 0x8037,
    GL_POLYGON_OFFSET_FACTOR_EXT                            = 0x8038,
    GL_POLYGON_OFFSET_BIAS_EXT                              = 0x8039,
};
extern void         (KHRONOS_APIENTRY* const& glPolygonOffsetEXT) (GLfloat factor, GLfloat bias);
#endif

#ifndef GL_EXT_polygon_offset_clamp
#define GL_EXT_polygon_offset_clamp 1
enum : GLenum
{
    GL_POLYGON_OFFSET_CLAMP_EXT                             = 0x8E1B,
};
extern void         (KHRONOS_APIENTRY* const& glPolygonOffsetClampEXT) (GLfloat factor, GLfloat units, GLfloat clamp);
#endif

#ifndef GL_EXT_post_depth_coverage
#define GL_EXT_post_depth_coverage 1
#endif

#ifndef GL_EXT_provoking_vertex
#define GL_EXT_provoking_vertex 1
enum : GLenum
{
    GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT         = 0x8E4C,
    GL_FIRST_VERTEX_CONVENTION_EXT                          = 0x8E4D,
    GL_LAST_VERTEX_CONVENTION_EXT                           = 0x8E4E,
    GL_PROVOKING_VERTEX_EXT                                 = 0x8E4F,
};
extern void         (KHRONOS_APIENTRY* const& glProvokingVertexEXT) (GLenum mode);
#endif

#ifndef GL_EXT_raster_multisample
#define GL_EXT_raster_multisample 1
enum : GLenum
{
    GL_RASTER_MULTISAMPLE_EXT                               = 0x9327,
    GL_RASTER_SAMPLES_EXT                                   = 0x9328,
    GL_MAX_RASTER_SAMPLES_EXT                               = 0x9329,
    GL_RASTER_FIXED_SAMPLE_LOCATIONS_EXT                    = 0x932A,
    GL_MULTISAMPLE_RASTERIZATION_ALLOWED_EXT                = 0x932B,
    GL_EFFECTIVE_RASTER_SAMPLES_EXT                         = 0x932C,
};
extern void         (KHRONOS_APIENTRY* const& glRasterSamplesEXT) (GLuint samples, GLboolean fixedsamplelocations);
#endif

#ifndef GL_EXT_rescale_normal
#define GL_EXT_rescale_normal 1
enum : GLenum
{
    GL_RESCALE_NORMAL_EXT                                   = 0x803A,
};
#endif

#ifndef GL_EXT_semaphore
#define GL_EXT_semaphore 1
enum : GLenum
{
    GL_LAYOUT_GENERAL_EXT                                   = 0x958D,
    GL_LAYOUT_COLOR_ATTACHMENT_EXT                          = 0x958E,
    GL_LAYOUT_DEPTH_STENCIL_ATTACHMENT_EXT                  = 0x958F,
    GL_LAYOUT_DEPTH_STENCIL_READ_ONLY_EXT                   = 0x9590,
    GL_LAYOUT_SHADER_READ_ONLY_EXT                          = 0x9591,
    GL_LAYOUT_TRANSFER_SRC_EXT                              = 0x9592,
    GL_LAYOUT_TRANSFER_DST_EXT                              = 0x9593,
    GL_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_EXT        = 0x9530,
    GL_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_EXT        = 0x9531,
};
extern void         (KHRONOS_APIENTRY* const& glGenSemaphoresEXT) (GLsizei n, GLuint *semaphores);
extern void         (KHRONOS_APIENTRY* const& glDeleteSemaphoresEXT) (GLsizei n, const GLuint *semaphores);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsSemaphoreEXT) (GLuint semaphore);
extern void         (KHRONOS_APIENTRY* const& glSemaphoreParameterui64vEXT) (GLuint semaphore, GLenum pname, const GLuint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetSemaphoreParameterui64vEXT) (GLuint semaphore, GLenum pname, GLuint64 *params);
extern void         (KHRONOS_APIENTRY* const& glWaitSemaphoreEXT) (GLuint semaphore, GLuint numBufferBarriers, const GLuint *buffers, GLuint numTextureBarriers, const GLuint *textures, const GLenum *srcLayouts);
extern void         (KHRONOS_APIENTRY* const& glSignalSemaphoreEXT) (GLuint semaphore, GLuint numBufferBarriers, const GLuint *buffers, GLuint numTextureBarriers, const GLuint *textures, const GLenum *dstLayouts);
#endif

#ifndef GL_EXT_semaphore_fd
#define GL_EXT_semaphore_fd 1
extern void         (KHRONOS_APIENTRY* const& glImportSemaphoreFdEXT) (GLuint semaphore, GLenum handleType, GLint fd);
#endif

#ifndef GL_EXT_semaphore_win32
#define GL_EXT_semaphore_win32 1
enum : GLenum
{
    GL_HANDLE_TYPE_D3D12_FENCE_EXT                          = 0x9594,
    GL_D3D12_FENCE_VALUE_EXT                                = 0x9595,
};
extern void         (KHRONOS_APIENTRY* const& glImportSemaphoreWin32HandleEXT) (GLuint semaphore, GLenum handleType, void *handle);
extern void         (KHRONOS_APIENTRY* const& glImportSemaphoreWin32NameEXT) (GLuint semaphore, GLenum handleType, const void *name);
#endif

#ifndef GL_EXT_secondary_color
#define GL_EXT_secondary_color 1
enum : GLenum
{
    GL_COLOR_SUM_EXT                                        = 0x8458,
    GL_CURRENT_SECONDARY_COLOR_EXT                          = 0x8459,
    GL_SECONDARY_COLOR_ARRAY_SIZE_EXT                       = 0x845A,
    GL_SECONDARY_COLOR_ARRAY_TYPE_EXT                       = 0x845B,
    GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT                     = 0x845C,
    GL_SECONDARY_COLOR_ARRAY_POINTER_EXT                    = 0x845D,
    GL_SECONDARY_COLOR_ARRAY_EXT                            = 0x845E,
};
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3bEXT) (GLbyte red, GLbyte green, GLbyte blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3bvEXT) (const GLbyte *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3dEXT) (GLdouble red, GLdouble green, GLdouble blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3dvEXT) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3fEXT) (GLfloat red, GLfloat green, GLfloat blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3fvEXT) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3iEXT) (GLint red, GLint green, GLint blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3ivEXT) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3sEXT) (GLshort red, GLshort green, GLshort blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3svEXT) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3ubEXT) (GLubyte red, GLubyte green, GLubyte blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3ubvEXT) (const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3uiEXT) (GLuint red, GLuint green, GLuint blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3uivEXT) (const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3usEXT) (GLushort red, GLushort green, GLushort blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3usvEXT) (const GLushort *v);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColorPointerEXT) (GLint size, GLenum type, GLsizei stride, const void *pointer);
#endif

#ifndef GL_EXT_separate_shader_objects
#define GL_EXT_separate_shader_objects 1
enum : GLenum
{
    GL_ACTIVE_PROGRAM_EXT                                   = 0x8B8D,
    GL_VERTEX_SHADER_BIT_EXT                                = 0x00000001,
    GL_FRAGMENT_SHADER_BIT_EXT                              = 0x00000002,
    GL_ALL_SHADER_BITS_EXT                                  = 0xFFFFFFFF,
    GL_PROGRAM_SEPARABLE_EXT                                = 0x8258,
    GL_PROGRAM_PIPELINE_BINDING_EXT                         = 0x825A,
};
extern void         (KHRONOS_APIENTRY* const& glUseShaderProgramEXT) (GLenum type, GLuint program);
extern void         (KHRONOS_APIENTRY* const& glActiveProgramEXT) (GLuint program);
extern GLuint       (KHRONOS_APIENTRY* const& glCreateShaderProgramEXT) (GLenum type, const GLchar *string);
extern void         (KHRONOS_APIENTRY* const& glActiveShaderProgramEXT) (GLuint pipeline, GLuint program);
extern void         (KHRONOS_APIENTRY* const& glBindProgramPipelineEXT) (GLuint pipeline);
extern GLuint       (KHRONOS_APIENTRY* const& glCreateShaderProgramvEXT) (GLenum type, GLsizei count, const GLchar **strings);
extern void         (KHRONOS_APIENTRY* const& glDeleteProgramPipelinesEXT) (GLsizei n, const GLuint *pipelines);
extern void         (KHRONOS_APIENTRY* const& glGenProgramPipelinesEXT) (GLsizei n, GLuint *pipelines);
extern void         (KHRONOS_APIENTRY* const& glGetProgramPipelineInfoLogEXT) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void         (KHRONOS_APIENTRY* const& glGetProgramPipelineivEXT) (GLuint pipeline, GLenum pname, GLint *params);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsProgramPipelineEXT) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glUseProgramStagesEXT) (GLuint pipeline, GLbitfield stages, GLuint program);
extern void         (KHRONOS_APIENTRY* const& glValidateProgramPipelineEXT) (GLuint pipeline);
#endif

#ifndef GL_EXT_separate_specular_color
#define GL_EXT_separate_specular_color 1
enum : GLenum
{
    GL_LIGHT_MODEL_COLOR_CONTROL_EXT                        = 0x81F8,
    GL_SINGLE_COLOR_EXT                                     = 0x81F9,
    GL_SEPARATE_SPECULAR_COLOR_EXT                          = 0x81FA,
};
#endif

#ifndef GL_EXT_shader_framebuffer_fetch
#define GL_EXT_shader_framebuffer_fetch 1
enum : GLenum
{
    GL_FRAGMENT_SHADER_DISCARDS_SAMPLES_EXT                 = 0x8A52,
};
#endif

#ifndef GL_EXT_shader_framebuffer_fetch_non_coherent
#define GL_EXT_shader_framebuffer_fetch_non_coherent 1
extern void         (KHRONOS_APIENTRY* const& glFramebufferFetchBarrierEXT) ();
#endif

#ifndef GL_EXT_shader_image_load_formatted
#define GL_EXT_shader_image_load_formatted 1
#endif

#ifndef GL_EXT_shader_image_load_store
#define GL_EXT_shader_image_load_store 1
enum : GLenum
{
    GL_MAX_IMAGE_UNITS_EXT                                  = 0x8F38,
    GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT    = 0x8F39,
    GL_IMAGE_BINDING_NAME_EXT                               = 0x8F3A,
    GL_IMAGE_BINDING_LEVEL_EXT                              = 0x8F3B,
    GL_IMAGE_BINDING_LAYERED_EXT                            = 0x8F3C,
    GL_IMAGE_BINDING_LAYER_EXT                              = 0x8F3D,
    GL_IMAGE_BINDING_ACCESS_EXT                             = 0x8F3E,
    GL_IMAGE_1D_EXT                                         = 0x904C,
    GL_IMAGE_2D_EXT                                         = 0x904D,
    GL_IMAGE_3D_EXT                                         = 0x904E,
    GL_IMAGE_2D_RECT_EXT                                    = 0x904F,
    GL_IMAGE_CUBE_EXT                                       = 0x9050,
    GL_IMAGE_BUFFER_EXT                                     = 0x9051,
    GL_IMAGE_1D_ARRAY_EXT                                   = 0x9052,
    GL_IMAGE_2D_ARRAY_EXT                                   = 0x9053,
    GL_IMAGE_CUBE_MAP_ARRAY_EXT                             = 0x9054,
    GL_IMAGE_2D_MULTISAMPLE_EXT                             = 0x9055,
    GL_IMAGE_2D_MULTISAMPLE_ARRAY_EXT                       = 0x9056,
    GL_INT_IMAGE_1D_EXT                                     = 0x9057,
    GL_INT_IMAGE_2D_EXT                                     = 0x9058,
    GL_INT_IMAGE_3D_EXT                                     = 0x9059,
    GL_INT_IMAGE_2D_RECT_EXT                                = 0x905A,
    GL_INT_IMAGE_CUBE_EXT                                   = 0x905B,
    GL_INT_IMAGE_BUFFER_EXT                                 = 0x905C,
    GL_INT_IMAGE_1D_ARRAY_EXT                               = 0x905D,
    GL_INT_IMAGE_2D_ARRAY_EXT                               = 0x905E,
    GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT                         = 0x905F,
    GL_INT_IMAGE_2D_MULTISAMPLE_EXT                         = 0x9060,
    GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT                   = 0x9061,
    GL_UNSIGNED_INT_IMAGE_1D_EXT                            = 0x9062,
    GL_UNSIGNED_INT_IMAGE_2D_EXT                            = 0x9063,
    GL_UNSIGNED_INT_IMAGE_3D_EXT                            = 0x9064,
    GL_UNSIGNED_INT_IMAGE_2D_RECT_EXT                       = 0x9065,
    GL_UNSIGNED_INT_IMAGE_CUBE_EXT                          = 0x9066,
    GL_UNSIGNED_INT_IMAGE_BUFFER_EXT                        = 0x9067,
    GL_UNSIGNED_INT_IMAGE_1D_ARRAY_EXT                      = 0x9068,
    GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT                      = 0x9069,
    GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT                = 0x906A,
    GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT                = 0x906B,
    GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT          = 0x906C,
    GL_MAX_IMAGE_SAMPLES_EXT                                = 0x906D,
    GL_IMAGE_BINDING_FORMAT_EXT                             = 0x906E,
    GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT                  = 0x00000001,
    GL_ELEMENT_ARRAY_BARRIER_BIT_EXT                        = 0x00000002,
    GL_UNIFORM_BARRIER_BIT_EXT                              = 0x00000004,
    GL_TEXTURE_FETCH_BARRIER_BIT_EXT                        = 0x00000008,
    GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT                  = 0x00000020,
    GL_COMMAND_BARRIER_BIT_EXT                              = 0x00000040,
    GL_PIXEL_BUFFER_BARRIER_BIT_EXT                         = 0x00000080,
    GL_TEXTURE_UPDATE_BARRIER_BIT_EXT                       = 0x00000100,
    GL_BUFFER_UPDATE_BARRIER_BIT_EXT                        = 0x00000200,
    GL_FRAMEBUFFER_BARRIER_BIT_EXT                          = 0x00000400,
    GL_TRANSFORM_FEEDBACK_BARRIER_BIT_EXT                   = 0x00000800,
    GL_ATOMIC_COUNTER_BARRIER_BIT_EXT                       = 0x00001000,
    GL_ALL_BARRIER_BITS_EXT                                 = 0xFFFFFFFF,
};
extern void         (KHRONOS_APIENTRY* const& glBindImageTextureEXT) (GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format);
extern void         (KHRONOS_APIENTRY* const& glMemoryBarrierEXT) (GLbitfield barriers);
#endif

#ifndef GL_EXT_shader_integer_mix
#define GL_EXT_shader_integer_mix 1
#endif

#ifndef GL_EXT_shadow_funcs
#define GL_EXT_shadow_funcs 1
#endif

#ifndef GL_EXT_shared_texture_palette
#define GL_EXT_shared_texture_palette 1
enum : GLenum
{
    GL_SHARED_TEXTURE_PALETTE_EXT                           = 0x81FB,
};
#endif

#ifndef GL_EXT_sparse_texture2
#define GL_EXT_sparse_texture2 1
#endif

#ifndef GL_EXT_stencil_clear_tag
#define GL_EXT_stencil_clear_tag 1
enum : GLenum
{
    GL_STENCIL_TAG_BITS_EXT                                 = 0x88F2,
    GL_STENCIL_CLEAR_TAG_VALUE_EXT                          = 0x88F3,
};
extern void         (KHRONOS_APIENTRY* const& glStencilClearTagEXT) (GLsizei stencilTagBits, GLuint stencilClearTag);
#endif

#ifndef GL_EXT_stencil_two_side
#define GL_EXT_stencil_two_side 1
enum : GLenum
{
    GL_STENCIL_TEST_TWO_SIDE_EXT                            = 0x8910,
    GL_ACTIVE_STENCIL_FACE_EXT                              = 0x8911,
};
extern void         (KHRONOS_APIENTRY* const& glActiveStencilFaceEXT) (GLenum face);
#endif

#ifndef GL_EXT_stencil_wrap
#define GL_EXT_stencil_wrap 1
enum : GLenum
{
    GL_INCR_WRAP_EXT                                        = 0x8507,
    GL_DECR_WRAP_EXT                                        = 0x8508,
};
#endif

#ifndef GL_EXT_subtexture
#define GL_EXT_subtexture 1
extern void         (KHRONOS_APIENTRY* const& glTexSubImage1DEXT) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage2DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
#endif

#ifndef GL_EXT_texture
#define GL_EXT_texture 1
enum : GLenum
{
    GL_ALPHA4_EXT                                           = 0x803B,
    GL_ALPHA8_EXT                                           = 0x803C,
    GL_ALPHA12_EXT                                          = 0x803D,
    GL_ALPHA16_EXT                                          = 0x803E,
    GL_LUMINANCE4_EXT                                       = 0x803F,
    GL_LUMINANCE8_EXT                                       = 0x8040,
    GL_LUMINANCE12_EXT                                      = 0x8041,
    GL_LUMINANCE16_EXT                                      = 0x8042,
    GL_LUMINANCE4_ALPHA4_EXT                                = 0x8043,
    GL_LUMINANCE6_ALPHA2_EXT                                = 0x8044,
    GL_LUMINANCE8_ALPHA8_EXT                                = 0x8045,
    GL_LUMINANCE12_ALPHA4_EXT                               = 0x8046,
    GL_LUMINANCE12_ALPHA12_EXT                              = 0x8047,
    GL_LUMINANCE16_ALPHA16_EXT                              = 0x8048,
    GL_INTENSITY_EXT                                        = 0x8049,
    GL_INTENSITY4_EXT                                       = 0x804A,
    GL_INTENSITY8_EXT                                       = 0x804B,
    GL_INTENSITY12_EXT                                      = 0x804C,
    GL_INTENSITY16_EXT                                      = 0x804D,
    GL_RGB2_EXT                                             = 0x804E,
    GL_RGB4_EXT                                             = 0x804F,
    GL_RGB5_EXT                                             = 0x8050,
    GL_RGB8_EXT                                             = 0x8051,
    GL_RGB10_EXT                                            = 0x8052,
    GL_RGB12_EXT                                            = 0x8053,
    GL_RGB16_EXT                                            = 0x8054,
    GL_RGBA2_EXT                                            = 0x8055,
    GL_RGBA4_EXT                                            = 0x8056,
    GL_RGB5_A1_EXT                                          = 0x8057,
    GL_RGBA8_EXT                                            = 0x8058,
    GL_RGB10_A2_EXT                                         = 0x8059,
    GL_RGBA12_EXT                                           = 0x805A,
    GL_RGBA16_EXT                                           = 0x805B,
    GL_TEXTURE_RED_SIZE_EXT                                 = 0x805C,
    GL_TEXTURE_GREEN_SIZE_EXT                               = 0x805D,
    GL_TEXTURE_BLUE_SIZE_EXT                                = 0x805E,
    GL_TEXTURE_ALPHA_SIZE_EXT                               = 0x805F,
    GL_TEXTURE_LUMINANCE_SIZE_EXT                           = 0x8060,
    GL_TEXTURE_INTENSITY_SIZE_EXT                           = 0x8061,
    GL_REPLACE_EXT                                          = 0x8062,
    GL_PROXY_TEXTURE_1D_EXT                                 = 0x8063,
    GL_PROXY_TEXTURE_2D_EXT                                 = 0x8064,
    GL_TEXTURE_TOO_LARGE_EXT                                = 0x8065,
};
#endif

#ifndef GL_EXT_texture3D
#define GL_EXT_texture3D 1
enum : GLenum
{
    GL_PACK_SKIP_IMAGES_EXT                                 = 0x806B,
    GL_PACK_IMAGE_HEIGHT_EXT                                = 0x806C,
    GL_UNPACK_SKIP_IMAGES_EXT                               = 0x806D,
    GL_UNPACK_IMAGE_HEIGHT_EXT                              = 0x806E,
    GL_TEXTURE_3D_EXT                                       = 0x806F,
    GL_PROXY_TEXTURE_3D_EXT                                 = 0x8070,
    GL_TEXTURE_DEPTH_EXT                                    = 0x8071,
    GL_TEXTURE_WRAP_R_EXT                                   = 0x8072,
    GL_MAX_3D_TEXTURE_SIZE_EXT                              = 0x8073,
};
extern void         (KHRONOS_APIENTRY* const& glTexImage3DEXT) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage3DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
#endif

#ifndef GL_EXT_texture_array
#define GL_EXT_texture_array 1
enum : GLenum
{
    GL_TEXTURE_1D_ARRAY_EXT                                 = 0x8C18,
    GL_PROXY_TEXTURE_1D_ARRAY_EXT                           = 0x8C19,
    GL_TEXTURE_2D_ARRAY_EXT                                 = 0x8C1A,
    GL_PROXY_TEXTURE_2D_ARRAY_EXT                           = 0x8C1B,
    GL_TEXTURE_BINDING_1D_ARRAY_EXT                         = 0x8C1C,
    GL_TEXTURE_BINDING_2D_ARRAY_EXT                         = 0x8C1D,
    GL_MAX_ARRAY_TEXTURE_LAYERS_EXT                         = 0x88FF,
    GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT                     = 0x884E,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureLayerEXT) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
#endif

#ifndef GL_EXT_texture_buffer_object
#define GL_EXT_texture_buffer_object 1
enum : GLenum
{
    GL_TEXTURE_BUFFER_EXT                                   = 0x8C2A,
    GL_MAX_TEXTURE_BUFFER_SIZE_EXT                          = 0x8C2B,
    GL_TEXTURE_BINDING_BUFFER_EXT                           = 0x8C2C,
    GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT                = 0x8C2D,
    GL_TEXTURE_BUFFER_FORMAT_EXT                            = 0x8C2E,
};
extern void         (KHRONOS_APIENTRY* const& glTexBufferEXT) (GLenum target, GLenum internalformat, GLuint buffer);
#endif

#ifndef GL_EXT_texture_compression_latc
#define GL_EXT_texture_compression_latc 1
enum : GLenum
{
    GL_COMPRESSED_LUMINANCE_LATC1_EXT                       = 0x8C70,
    GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT                = 0x8C71,
    GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT                 = 0x8C72,
    GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT          = 0x8C73,
};
#endif

#ifndef GL_EXT_texture_compression_rgtc
#define GL_EXT_texture_compression_rgtc 1
enum : GLenum
{
    GL_COMPRESSED_RED_RGTC1_EXT                             = 0x8DBB,
    GL_COMPRESSED_SIGNED_RED_RGTC1_EXT                      = 0x8DBC,
    GL_COMPRESSED_RED_GREEN_RGTC2_EXT                       = 0x8DBD,
    GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT                = 0x8DBE,
};
#endif

#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc 1
enum : GLenum
{
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT                         = 0x83F0,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                        = 0x83F1,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                        = 0x83F2,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                        = 0x83F3,
};
#endif

#ifndef GL_EXT_texture_cube_map
#define GL_EXT_texture_cube_map 1
enum : GLenum
{
    GL_NORMAL_MAP_EXT                                       = 0x8511,
    GL_REFLECTION_MAP_EXT                                   = 0x8512,
    GL_TEXTURE_CUBE_MAP_EXT                                 = 0x8513,
    GL_TEXTURE_BINDING_CUBE_MAP_EXT                         = 0x8514,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT                      = 0x8515,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT                      = 0x8516,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT                      = 0x8517,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT                      = 0x8518,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT                      = 0x8519,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT                      = 0x851A,
    GL_PROXY_TEXTURE_CUBE_MAP_EXT                           = 0x851B,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT                        = 0x851C,
};
#endif

#ifndef GL_EXT_texture_env_add
#define GL_EXT_texture_env_add 1
#endif

#ifndef GL_EXT_texture_env_combine
#define GL_EXT_texture_env_combine 1
enum : GLenum
{
    GL_COMBINE_EXT                                          = 0x8570,
    GL_COMBINE_RGB_EXT                                      = 0x8571,
    GL_COMBINE_ALPHA_EXT                                    = 0x8572,
    GL_RGB_SCALE_EXT                                        = 0x8573,
    GL_ADD_SIGNED_EXT                                       = 0x8574,
    GL_INTERPOLATE_EXT                                      = 0x8575,
    GL_CONSTANT_EXT                                         = 0x8576,
    GL_PRIMARY_COLOR_EXT                                    = 0x8577,
    GL_PREVIOUS_EXT                                         = 0x8578,
    GL_SOURCE0_RGB_EXT                                      = 0x8580,
    GL_SOURCE1_RGB_EXT                                      = 0x8581,
    GL_SOURCE2_RGB_EXT                                      = 0x8582,
    GL_SOURCE0_ALPHA_EXT                                    = 0x8588,
    GL_SOURCE1_ALPHA_EXT                                    = 0x8589,
    GL_SOURCE2_ALPHA_EXT                                    = 0x858A,
    GL_OPERAND0_RGB_EXT                                     = 0x8590,
    GL_OPERAND1_RGB_EXT                                     = 0x8591,
    GL_OPERAND2_RGB_EXT                                     = 0x8592,
    GL_OPERAND0_ALPHA_EXT                                   = 0x8598,
    GL_OPERAND1_ALPHA_EXT                                   = 0x8599,
    GL_OPERAND2_ALPHA_EXT                                   = 0x859A,
};
#endif

#ifndef GL_EXT_texture_env_dot3
#define GL_EXT_texture_env_dot3 1
enum : GLenum
{
    GL_DOT3_RGB_EXT                                         = 0x8740,
    GL_DOT3_RGBA_EXT                                        = 0x8741,
};
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
enum : GLenum
{
    GL_TEXTURE_MAX_ANISOTROPY_EXT                           = 0x84FE,
    GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                       = 0x84FF,
};
#endif

#ifndef GL_EXT_texture_filter_minmax
#define GL_EXT_texture_filter_minmax 1
enum : GLenum
{
    GL_TEXTURE_REDUCTION_MODE_EXT                           = 0x9366,
    GL_WEIGHTED_AVERAGE_EXT                                 = 0x9367,
};
#endif

#ifndef GL_EXT_texture_integer
#define GL_EXT_texture_integer 1
enum : GLenum
{
    GL_RGBA32UI_EXT                                         = 0x8D70,
    GL_RGB32UI_EXT                                          = 0x8D71,
    GL_ALPHA32UI_EXT                                        = 0x8D72,
    GL_INTENSITY32UI_EXT                                    = 0x8D73,
    GL_LUMINANCE32UI_EXT                                    = 0x8D74,
    GL_LUMINANCE_ALPHA32UI_EXT                              = 0x8D75,
    GL_RGBA16UI_EXT                                         = 0x8D76,
    GL_RGB16UI_EXT                                          = 0x8D77,
    GL_ALPHA16UI_EXT                                        = 0x8D78,
    GL_INTENSITY16UI_EXT                                    = 0x8D79,
    GL_LUMINANCE16UI_EXT                                    = 0x8D7A,
    GL_LUMINANCE_ALPHA16UI_EXT                              = 0x8D7B,
    GL_RGBA8UI_EXT                                          = 0x8D7C,
    GL_RGB8UI_EXT                                           = 0x8D7D,
    GL_ALPHA8UI_EXT                                         = 0x8D7E,
    GL_INTENSITY8UI_EXT                                     = 0x8D7F,
    GL_LUMINANCE8UI_EXT                                     = 0x8D80,
    GL_LUMINANCE_ALPHA8UI_EXT                               = 0x8D81,
    GL_RGBA32I_EXT                                          = 0x8D82,
    GL_RGB32I_EXT                                           = 0x8D83,
    GL_ALPHA32I_EXT                                         = 0x8D84,
    GL_INTENSITY32I_EXT                                     = 0x8D85,
    GL_LUMINANCE32I_EXT                                     = 0x8D86,
    GL_LUMINANCE_ALPHA32I_EXT                               = 0x8D87,
    GL_RGBA16I_EXT                                          = 0x8D88,
    GL_RGB16I_EXT                                           = 0x8D89,
    GL_ALPHA16I_EXT                                         = 0x8D8A,
    GL_INTENSITY16I_EXT                                     = 0x8D8B,
    GL_LUMINANCE16I_EXT                                     = 0x8D8C,
    GL_LUMINANCE_ALPHA16I_EXT                               = 0x8D8D,
    GL_RGBA8I_EXT                                           = 0x8D8E,
    GL_RGB8I_EXT                                            = 0x8D8F,
    GL_ALPHA8I_EXT                                          = 0x8D90,
    GL_INTENSITY8I_EXT                                      = 0x8D91,
    GL_LUMINANCE8I_EXT                                      = 0x8D92,
    GL_LUMINANCE_ALPHA8I_EXT                                = 0x8D93,
    GL_RED_INTEGER_EXT                                      = 0x8D94,
    GL_GREEN_INTEGER_EXT                                    = 0x8D95,
    GL_BLUE_INTEGER_EXT                                     = 0x8D96,
    GL_ALPHA_INTEGER_EXT                                    = 0x8D97,
    GL_RGB_INTEGER_EXT                                      = 0x8D98,
    GL_RGBA_INTEGER_EXT                                     = 0x8D99,
    GL_BGR_INTEGER_EXT                                      = 0x8D9A,
    GL_BGRA_INTEGER_EXT                                     = 0x8D9B,
    GL_LUMINANCE_INTEGER_EXT                                = 0x8D9C,
    GL_LUMINANCE_ALPHA_INTEGER_EXT                          = 0x8D9D,
    GL_RGBA_INTEGER_MODE_EXT                                = 0x8D9E,
};
extern void         (KHRONOS_APIENTRY* const& glTexParameterIivEXT) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameterIuivEXT) (GLenum target, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIuivEXT) (GLenum target, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glClearColorIiEXT) (GLint red, GLint green, GLint blue, GLint alpha);
extern void         (KHRONOS_APIENTRY* const& glClearColorIuiEXT) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
#endif

#ifndef GL_EXT_texture_lod_bias
#define GL_EXT_texture_lod_bias 1
enum : GLenum
{
    GL_MAX_TEXTURE_LOD_BIAS_EXT                             = 0x84FD,
    GL_TEXTURE_FILTER_CONTROL_EXT                           = 0x8500,
    GL_TEXTURE_LOD_BIAS_EXT                                 = 0x8501,
};
#endif

#ifndef GL_EXT_texture_mirror_clamp
#define GL_EXT_texture_mirror_clamp 1
enum : GLenum
{
    GL_MIRROR_CLAMP_EXT                                     = 0x8742,
    GL_MIRROR_CLAMP_TO_EDGE_EXT                             = 0x8743,
    GL_MIRROR_CLAMP_TO_BORDER_EXT                           = 0x8912,
};
#endif

#ifndef GL_EXT_texture_object
#define GL_EXT_texture_object 1
enum : GLenum
{
    GL_TEXTURE_PRIORITY_EXT                                 = 0x8066,
    GL_TEXTURE_RESIDENT_EXT                                 = 0x8067,
    GL_TEXTURE_1D_BINDING_EXT                               = 0x8068,
    GL_TEXTURE_2D_BINDING_EXT                               = 0x8069,
    GL_TEXTURE_3D_BINDING_EXT                               = 0x806A,
};
extern GLboolean    (KHRONOS_APIENTRY* const& glAreTexturesResidentEXT) (GLsizei n, const GLuint *textures, GLboolean *residences);
extern void         (KHRONOS_APIENTRY* const& glBindTextureEXT) (GLenum target, GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glDeleteTexturesEXT) (GLsizei n, const GLuint *textures);
extern void         (KHRONOS_APIENTRY* const& glGenTexturesEXT) (GLsizei n, GLuint *textures);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTextureEXT) (GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glPrioritizeTexturesEXT) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
#endif

#ifndef GL_EXT_texture_perturb_normal
#define GL_EXT_texture_perturb_normal 1
enum : GLenum
{
    GL_PERTURB_EXT                                          = 0x85AE,
    GL_TEXTURE_NORMAL_EXT                                   = 0x85AF,
};
extern void         (KHRONOS_APIENTRY* const& glTextureNormalEXT) (GLenum mode);
#endif

#ifndef GL_EXT_texture_sRGB
#define GL_EXT_texture_sRGB 1
enum : GLenum
{
    GL_SRGB_EXT                                             = 0x8C40,
    GL_SRGB8_EXT                                            = 0x8C41,
    GL_SRGB_ALPHA_EXT                                       = 0x8C42,
    GL_SRGB8_ALPHA8_EXT                                     = 0x8C43,
    GL_SLUMINANCE_ALPHA_EXT                                 = 0x8C44,
    GL_SLUMINANCE8_ALPHA8_EXT                               = 0x8C45,
    GL_SLUMINANCE_EXT                                       = 0x8C46,
    GL_SLUMINANCE8_EXT                                      = 0x8C47,
    GL_COMPRESSED_SRGB_EXT                                  = 0x8C48,
    GL_COMPRESSED_SRGB_ALPHA_EXT                            = 0x8C49,
    GL_COMPRESSED_SLUMINANCE_EXT                            = 0x8C4A,
    GL_COMPRESSED_SLUMINANCE_ALPHA_EXT                      = 0x8C4B,
    GL_COMPRESSED_SRGB_S3TC_DXT1_EXT                        = 0x8C4C,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT                  = 0x8C4D,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT                  = 0x8C4E,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT                  = 0x8C4F,
};
#endif

#ifndef GL_EXT_texture_sRGB_R8
#define GL_EXT_texture_sRGB_R8 1
enum : GLenum
{
    GL_SR8_EXT                                              = 0x8FBD,
};
#endif

#ifndef GL_EXT_texture_sRGB_RG8
#define GL_EXT_texture_sRGB_RG8 1
enum : GLenum
{
    GL_SRG8_EXT                                             = 0x8FBE,
};
#endif

#ifndef GL_EXT_texture_sRGB_decode
#define GL_EXT_texture_sRGB_decode 1
enum : GLenum
{
    GL_TEXTURE_SRGB_DECODE_EXT                              = 0x8A48,
    GL_DECODE_EXT                                           = 0x8A49,
    GL_SKIP_DECODE_EXT                                      = 0x8A4A,
};
#endif

#ifndef GL_EXT_texture_shared_exponent
#define GL_EXT_texture_shared_exponent 1
enum : GLenum
{
    GL_RGB9_E5_EXT                                          = 0x8C3D,
    GL_UNSIGNED_INT_5_9_9_9_REV_EXT                         = 0x8C3E,
    GL_TEXTURE_SHARED_SIZE_EXT                              = 0x8C3F,
};
#endif

#ifndef GL_EXT_texture_snorm
#define GL_EXT_texture_snorm 1
enum : GLenum
{
    GL_ALPHA_SNORM                                          = 0x9010,
    GL_LUMINANCE_SNORM                                      = 0x9011,
    GL_LUMINANCE_ALPHA_SNORM                                = 0x9012,
    GL_INTENSITY_SNORM                                      = 0x9013,
    GL_ALPHA8_SNORM                                         = 0x9014,
    GL_LUMINANCE8_SNORM                                     = 0x9015,
    GL_LUMINANCE8_ALPHA8_SNORM                              = 0x9016,
    GL_INTENSITY8_SNORM                                     = 0x9017,
    GL_ALPHA16_SNORM                                        = 0x9018,
    GL_LUMINANCE16_SNORM                                    = 0x9019,
    GL_LUMINANCE16_ALPHA16_SNORM                            = 0x901A,
    GL_INTENSITY16_SNORM                                    = 0x901B,
    GL_RED_SNORM                                            = 0x8F90,
    GL_RG_SNORM                                             = 0x8F91,
    GL_RGB_SNORM                                            = 0x8F92,
    GL_RGBA_SNORM                                           = 0x8F93,
};
#endif

#ifndef GL_EXT_texture_swizzle
#define GL_EXT_texture_swizzle 1
enum : GLenum
{
    GL_TEXTURE_SWIZZLE_R_EXT                                = 0x8E42,
    GL_TEXTURE_SWIZZLE_G_EXT                                = 0x8E43,
    GL_TEXTURE_SWIZZLE_B_EXT                                = 0x8E44,
    GL_TEXTURE_SWIZZLE_A_EXT                                = 0x8E45,
    GL_TEXTURE_SWIZZLE_RGBA_EXT                             = 0x8E46,
};
#endif

#ifndef GL_NV_timeline_semaphore
#define GL_NV_timeline_semaphore 1
enum : GLenum
{
    GL_TIMELINE_SEMAPHORE_VALUE_NV                          = 0x9595,
    GL_SEMAPHORE_TYPE_NV                                    = 0x95B3,
    GL_SEMAPHORE_TYPE_BINARY_NV                             = 0x95B4,
    GL_SEMAPHORE_TYPE_TIMELINE_NV                           = 0x95B5,
    GL_MAX_TIMELINE_SEMAPHORE_VALUE_DIFFERENCE_NV           = 0x95B6,
};
extern void         (KHRONOS_APIENTRY* const& glCreateSemaphoresNV) (GLsizei n, GLuint *semaphores);
extern void         (KHRONOS_APIENTRY* const& glSemaphoreParameterivNV) (GLuint semaphore, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSemaphoreParameterivNV) (GLuint semaphore, GLenum pname, GLint *params);
#endif

#ifndef GL_EXT_timer_query
#define GL_EXT_timer_query 1
enum : GLenum
{
    GL_TIME_ELAPSED_EXT                                     = 0x88BF,
};
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjecti64vEXT) (GLuint id, GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectui64vEXT) (GLuint id, GLenum pname, GLuint64 *params);
#endif

#ifndef GL_EXT_transform_feedback
#define GL_EXT_transform_feedback 1
enum : GLenum
{
    GL_TRANSFORM_FEEDBACK_BUFFER_EXT                        = 0x8C8E,
    GL_TRANSFORM_FEEDBACK_BUFFER_START_EXT                  = 0x8C84,
    GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT                   = 0x8C85,
    GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT                = 0x8C8F,
    GL_INTERLEAVED_ATTRIBS_EXT                              = 0x8C8C,
    GL_SEPARATE_ATTRIBS_EXT                                 = 0x8C8D,
    GL_PRIMITIVES_GENERATED_EXT                             = 0x8C87,
    GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT            = 0x8C88,
    GL_RASTERIZER_DISCARD_EXT                               = 0x8C89,
    GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT    = 0x8C8A,
    GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT          = 0x8C8B,
    GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT       = 0x8C80,
    GL_TRANSFORM_FEEDBACK_VARYINGS_EXT                      = 0x8C83,
    GL_TRANSFORM_FEEDBACK_BUFFER_MODE_EXT                   = 0x8C7F,
    GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT            = 0x8C76,
};
extern void         (KHRONOS_APIENTRY* const& glBeginTransformFeedbackEXT) (GLenum primitiveMode);
extern void         (KHRONOS_APIENTRY* const& glEndTransformFeedbackEXT) ();
extern void         (KHRONOS_APIENTRY* const& glBindBufferRangeEXT) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glBindBufferOffsetEXT) (GLenum target, GLuint index, GLuint buffer, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glBindBufferBaseEXT) (GLenum target, GLuint index, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackVaryingsEXT) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
extern void         (KHRONOS_APIENTRY* const& glGetTransformFeedbackVaryingEXT) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
#endif

#ifndef GL_EXT_vertex_array
#define GL_EXT_vertex_array 1
enum : GLenum
{
    GL_VERTEX_ARRAY_EXT                                     = 0x8074,
    GL_NORMAL_ARRAY_EXT                                     = 0x8075,
    GL_COLOR_ARRAY_EXT                                      = 0x8076,
    GL_INDEX_ARRAY_EXT                                      = 0x8077,
    GL_TEXTURE_COORD_ARRAY_EXT                              = 0x8078,
    GL_EDGE_FLAG_ARRAY_EXT                                  = 0x8079,
    GL_VERTEX_ARRAY_SIZE_EXT                                = 0x807A,
    GL_VERTEX_ARRAY_TYPE_EXT                                = 0x807B,
    GL_VERTEX_ARRAY_STRIDE_EXT                              = 0x807C,
    GL_VERTEX_ARRAY_COUNT_EXT                               = 0x807D,
    GL_NORMAL_ARRAY_TYPE_EXT                                = 0x807E,
    GL_NORMAL_ARRAY_STRIDE_EXT                              = 0x807F,
    GL_NORMAL_ARRAY_COUNT_EXT                               = 0x8080,
    GL_COLOR_ARRAY_SIZE_EXT                                 = 0x8081,
    GL_COLOR_ARRAY_TYPE_EXT                                 = 0x8082,
    GL_COLOR_ARRAY_STRIDE_EXT                               = 0x8083,
    GL_COLOR_ARRAY_COUNT_EXT                                = 0x8084,
    GL_INDEX_ARRAY_TYPE_EXT                                 = 0x8085,
    GL_INDEX_ARRAY_STRIDE_EXT                               = 0x8086,
    GL_INDEX_ARRAY_COUNT_EXT                                = 0x8087,
    GL_TEXTURE_COORD_ARRAY_SIZE_EXT                         = 0x8088,
    GL_TEXTURE_COORD_ARRAY_TYPE_EXT                         = 0x8089,
    GL_TEXTURE_COORD_ARRAY_STRIDE_EXT                       = 0x808A,
    GL_TEXTURE_COORD_ARRAY_COUNT_EXT                        = 0x808B,
    GL_EDGE_FLAG_ARRAY_STRIDE_EXT                           = 0x808C,
    GL_EDGE_FLAG_ARRAY_COUNT_EXT                            = 0x808D,
    GL_VERTEX_ARRAY_POINTER_EXT                             = 0x808E,
    GL_NORMAL_ARRAY_POINTER_EXT                             = 0x808F,
    GL_COLOR_ARRAY_POINTER_EXT                              = 0x8090,
    GL_INDEX_ARRAY_POINTER_EXT                              = 0x8091,
    GL_TEXTURE_COORD_ARRAY_POINTER_EXT                      = 0x8092,
    GL_EDGE_FLAG_ARRAY_POINTER_EXT                          = 0x8093,
};
extern void         (KHRONOS_APIENTRY* const& glArrayElementEXT) (GLint i);
extern void         (KHRONOS_APIENTRY* const& glColorPointerEXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glDrawArraysEXT) (GLenum mode, GLint first, GLsizei count);
extern void         (KHRONOS_APIENTRY* const& glEdgeFlagPointerEXT) (GLsizei stride, GLsizei count, const GLboolean *pointer);
extern void         (KHRONOS_APIENTRY* const& glGetPointervEXT) (GLenum pname, void **params);
extern void         (KHRONOS_APIENTRY* const& glIndexPointerEXT) (GLenum type, GLsizei stride, GLsizei count, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glNormalPointerEXT) (GLenum type, GLsizei stride, GLsizei count, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glTexCoordPointerEXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glVertexPointerEXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer);
#endif

#ifndef GL_EXT_vertex_array_bgra
#define GL_EXT_vertex_array_bgra 1
#endif

#ifndef GL_EXT_vertex_attrib_64bit
#define GL_EXT_vertex_attrib_64bit 1
enum : GLenum
{
    GL_DOUBLE_VEC2_EXT                                      = 0x8FFC,
    GL_DOUBLE_VEC3_EXT                                      = 0x8FFD,
    GL_DOUBLE_VEC4_EXT                                      = 0x8FFE,
    GL_DOUBLE_MAT2_EXT                                      = 0x8F46,
    GL_DOUBLE_MAT3_EXT                                      = 0x8F47,
    GL_DOUBLE_MAT4_EXT                                      = 0x8F48,
    GL_DOUBLE_MAT2x3_EXT                                    = 0x8F49,
    GL_DOUBLE_MAT2x4_EXT                                    = 0x8F4A,
    GL_DOUBLE_MAT3x2_EXT                                    = 0x8F4B,
    GL_DOUBLE_MAT3x4_EXT                                    = 0x8F4C,
    GL_DOUBLE_MAT4x2_EXT                                    = 0x8F4D,
    GL_DOUBLE_MAT4x3_EXT                                    = 0x8F4E,
};
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1dEXT) (GLuint index, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2dEXT) (GLuint index, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3dEXT) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4dEXT) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1dvEXT) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2dvEXT) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3dvEXT) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4dvEXT) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribLPointerEXT) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribLdvEXT) (GLuint index, GLenum pname, GLdouble *params);
#endif

#ifndef GL_EXT_vertex_shader
#define GL_EXT_vertex_shader 1
enum : GLenum
{
    GL_VERTEX_SHADER_EXT                                    = 0x8780,
    GL_VERTEX_SHADER_BINDING_EXT                            = 0x8781,
    GL_OP_INDEX_EXT                                         = 0x8782,
    GL_OP_NEGATE_EXT                                        = 0x8783,
    GL_OP_DOT3_EXT                                          = 0x8784,
    GL_OP_DOT4_EXT                                          = 0x8785,
    GL_OP_MUL_EXT                                           = 0x8786,
    GL_OP_ADD_EXT                                           = 0x8787,
    GL_OP_MADD_EXT                                          = 0x8788,
    GL_OP_FRAC_EXT                                          = 0x8789,
    GL_OP_MAX_EXT                                           = 0x878A,
    GL_OP_MIN_EXT                                           = 0x878B,
    GL_OP_SET_GE_EXT                                        = 0x878C,
    GL_OP_SET_LT_EXT                                        = 0x878D,
    GL_OP_CLAMP_EXT                                         = 0x878E,
    GL_OP_FLOOR_EXT                                         = 0x878F,
    GL_OP_ROUND_EXT                                         = 0x8790,
    GL_OP_EXP_BASE_2_EXT                                    = 0x8791,
    GL_OP_LOG_BASE_2_EXT                                    = 0x8792,
    GL_OP_POWER_EXT                                         = 0x8793,
    GL_OP_RECIP_EXT                                         = 0x8794,
    GL_OP_RECIP_SQRT_EXT                                    = 0x8795,
    GL_OP_SUB_EXT                                           = 0x8796,
    GL_OP_CROSS_PRODUCT_EXT                                 = 0x8797,
    GL_OP_MULTIPLY_MATRIX_EXT                               = 0x8798,
    GL_OP_MOV_EXT                                           = 0x8799,
    GL_OUTPUT_VERTEX_EXT                                    = 0x879A,
    GL_OUTPUT_COLOR0_EXT                                    = 0x879B,
    GL_OUTPUT_COLOR1_EXT                                    = 0x879C,
    GL_OUTPUT_TEXTURE_COORD0_EXT                            = 0x879D,
    GL_OUTPUT_TEXTURE_COORD1_EXT                            = 0x879E,
    GL_OUTPUT_TEXTURE_COORD2_EXT                            = 0x879F,
    GL_OUTPUT_TEXTURE_COORD3_EXT                            = 0x87A0,
    GL_OUTPUT_TEXTURE_COORD4_EXT                            = 0x87A1,
    GL_OUTPUT_TEXTURE_COORD5_EXT                            = 0x87A2,
    GL_OUTPUT_TEXTURE_COORD6_EXT                            = 0x87A3,
    GL_OUTPUT_TEXTURE_COORD7_EXT                            = 0x87A4,
    GL_OUTPUT_TEXTURE_COORD8_EXT                            = 0x87A5,
    GL_OUTPUT_TEXTURE_COORD9_EXT                            = 0x87A6,
    GL_OUTPUT_TEXTURE_COORD10_EXT                           = 0x87A7,
    GL_OUTPUT_TEXTURE_COORD11_EXT                           = 0x87A8,
    GL_OUTPUT_TEXTURE_COORD12_EXT                           = 0x87A9,
    GL_OUTPUT_TEXTURE_COORD13_EXT                           = 0x87AA,
    GL_OUTPUT_TEXTURE_COORD14_EXT                           = 0x87AB,
    GL_OUTPUT_TEXTURE_COORD15_EXT                           = 0x87AC,
    GL_OUTPUT_TEXTURE_COORD16_EXT                           = 0x87AD,
    GL_OUTPUT_TEXTURE_COORD17_EXT                           = 0x87AE,
    GL_OUTPUT_TEXTURE_COORD18_EXT                           = 0x87AF,
    GL_OUTPUT_TEXTURE_COORD19_EXT                           = 0x87B0,
    GL_OUTPUT_TEXTURE_COORD20_EXT                           = 0x87B1,
    GL_OUTPUT_TEXTURE_COORD21_EXT                           = 0x87B2,
    GL_OUTPUT_TEXTURE_COORD22_EXT                           = 0x87B3,
    GL_OUTPUT_TEXTURE_COORD23_EXT                           = 0x87B4,
    GL_OUTPUT_TEXTURE_COORD24_EXT                           = 0x87B5,
    GL_OUTPUT_TEXTURE_COORD25_EXT                           = 0x87B6,
    GL_OUTPUT_TEXTURE_COORD26_EXT                           = 0x87B7,
    GL_OUTPUT_TEXTURE_COORD27_EXT                           = 0x87B8,
    GL_OUTPUT_TEXTURE_COORD28_EXT                           = 0x87B9,
    GL_OUTPUT_TEXTURE_COORD29_EXT                           = 0x87BA,
    GL_OUTPUT_TEXTURE_COORD30_EXT                           = 0x87BB,
    GL_OUTPUT_TEXTURE_COORD31_EXT                           = 0x87BC,
    GL_OUTPUT_FOG_EXT                                       = 0x87BD,
    GL_SCALAR_EXT                                           = 0x87BE,
    GL_VECTOR_EXT                                           = 0x87BF,
    GL_MATRIX_EXT                                           = 0x87C0,
    GL_VARIANT_EXT                                          = 0x87C1,
    GL_INVARIANT_EXT                                        = 0x87C2,
    GL_LOCAL_CONSTANT_EXT                                   = 0x87C3,
    GL_LOCAL_EXT                                            = 0x87C4,
    GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT                   = 0x87C5,
    GL_MAX_VERTEX_SHADER_VARIANTS_EXT                       = 0x87C6,
    GL_MAX_VERTEX_SHADER_INVARIANTS_EXT                     = 0x87C7,
    GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                = 0x87C8,
    GL_MAX_VERTEX_SHADER_LOCALS_EXT                         = 0x87C9,
    GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT         = 0x87CA,
    GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT             = 0x87CB,
    GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT      = 0x87CC,
    GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT           = 0x87CD,
    GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT               = 0x87CE,
    GL_VERTEX_SHADER_INSTRUCTIONS_EXT                       = 0x87CF,
    GL_VERTEX_SHADER_VARIANTS_EXT                           = 0x87D0,
    GL_VERTEX_SHADER_INVARIANTS_EXT                         = 0x87D1,
    GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                    = 0x87D2,
    GL_VERTEX_SHADER_LOCALS_EXT                             = 0x87D3,
    GL_VERTEX_SHADER_OPTIMIZED_EXT                          = 0x87D4,
    GL_X_EXT                                                = 0x87D5,
    GL_Y_EXT                                                = 0x87D6,
    GL_Z_EXT                                                = 0x87D7,
    GL_W_EXT                                                = 0x87D8,
    GL_NEGATIVE_X_EXT                                       = 0x87D9,
    GL_NEGATIVE_Y_EXT                                       = 0x87DA,
    GL_NEGATIVE_Z_EXT                                       = 0x87DB,
    GL_NEGATIVE_W_EXT                                       = 0x87DC,
    GL_ZERO_EXT                                             = 0x87DD,
    GL_ONE_EXT                                              = 0x87DE,
    GL_NEGATIVE_ONE_EXT                                     = 0x87DF,
    GL_NORMALIZED_RANGE_EXT                                 = 0x87E0,
    GL_FULL_RANGE_EXT                                       = 0x87E1,
    GL_CURRENT_VERTEX_EXT                                   = 0x87E2,
    GL_MVP_MATRIX_EXT                                       = 0x87E3,
    GL_VARIANT_VALUE_EXT                                    = 0x87E4,
    GL_VARIANT_DATATYPE_EXT                                 = 0x87E5,
    GL_VARIANT_ARRAY_STRIDE_EXT                             = 0x87E6,
    GL_VARIANT_ARRAY_TYPE_EXT                               = 0x87E7,
    GL_VARIANT_ARRAY_EXT                                    = 0x87E8,
    GL_VARIANT_ARRAY_POINTER_EXT                            = 0x87E9,
    GL_INVARIANT_VALUE_EXT                                  = 0x87EA,
    GL_INVARIANT_DATATYPE_EXT                               = 0x87EB,
    GL_LOCAL_CONSTANT_VALUE_EXT                             = 0x87EC,
    GL_LOCAL_CONSTANT_DATATYPE_EXT                          = 0x87ED,
};
extern void         (KHRONOS_APIENTRY* const& glBeginVertexShaderEXT) ();
extern void         (KHRONOS_APIENTRY* const& glEndVertexShaderEXT) ();
extern void         (KHRONOS_APIENTRY* const& glBindVertexShaderEXT) (GLuint id);
extern GLuint       (KHRONOS_APIENTRY* const& glGenVertexShadersEXT) (GLuint range);
extern void         (KHRONOS_APIENTRY* const& glDeleteVertexShaderEXT) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glShaderOp1EXT) (GLenum op, GLuint res, GLuint arg1);
extern void         (KHRONOS_APIENTRY* const& glShaderOp2EXT) (GLenum op, GLuint res, GLuint arg1, GLuint arg2);
extern void         (KHRONOS_APIENTRY* const& glShaderOp3EXT) (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3);
extern void         (KHRONOS_APIENTRY* const& glSwizzleEXT) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
extern void         (KHRONOS_APIENTRY* const& glWriteMaskEXT) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
extern void         (KHRONOS_APIENTRY* const& glInsertComponentEXT) (GLuint res, GLuint src, GLuint num);
extern void         (KHRONOS_APIENTRY* const& glExtractComponentEXT) (GLuint res, GLuint src, GLuint num);
extern GLuint       (KHRONOS_APIENTRY* const& glGenSymbolsEXT) (GLenum datatype, GLenum storagetype, GLenum range, GLuint components);
extern void         (KHRONOS_APIENTRY* const& glSetInvariantEXT) (GLuint id, GLenum type, const void *addr);
extern void         (KHRONOS_APIENTRY* const& glSetLocalConstantEXT) (GLuint id, GLenum type, const void *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantbvEXT) (GLuint id, const GLbyte *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantsvEXT) (GLuint id, const GLshort *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantivEXT) (GLuint id, const GLint *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantfvEXT) (GLuint id, const GLfloat *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantdvEXT) (GLuint id, const GLdouble *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantubvEXT) (GLuint id, const GLubyte *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantusvEXT) (GLuint id, const GLushort *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantuivEXT) (GLuint id, const GLuint *addr);
extern void         (KHRONOS_APIENTRY* const& glVariantPointerEXT) (GLuint id, GLenum type, GLuint stride, const void *addr);
extern void         (KHRONOS_APIENTRY* const& glEnableVariantClientStateEXT) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDisableVariantClientStateEXT) (GLuint id);
extern GLuint       (KHRONOS_APIENTRY* const& glBindLightParameterEXT) (GLenum light, GLenum value);
extern GLuint       (KHRONOS_APIENTRY* const& glBindMaterialParameterEXT) (GLenum face, GLenum value);
extern GLuint       (KHRONOS_APIENTRY* const& glBindTexGenParameterEXT) (GLenum unit, GLenum coord, GLenum value);
extern GLuint       (KHRONOS_APIENTRY* const& glBindTextureUnitParameterEXT) (GLenum unit, GLenum value);
extern GLuint       (KHRONOS_APIENTRY* const& glBindParameterEXT) (GLenum value);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsVariantEnabledEXT) (GLuint id, GLenum cap);
extern void         (KHRONOS_APIENTRY* const& glGetVariantBooleanvEXT) (GLuint id, GLenum value, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glGetVariantIntegervEXT) (GLuint id, GLenum value, GLint *data);
extern void         (KHRONOS_APIENTRY* const& glGetVariantFloatvEXT) (GLuint id, GLenum value, GLfloat *data);
extern void         (KHRONOS_APIENTRY* const& glGetVariantPointervEXT) (GLuint id, GLenum value, void **data);
extern void         (KHRONOS_APIENTRY* const& glGetInvariantBooleanvEXT) (GLuint id, GLenum value, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glGetInvariantIntegervEXT) (GLuint id, GLenum value, GLint *data);
extern void         (KHRONOS_APIENTRY* const& glGetInvariantFloatvEXT) (GLuint id, GLenum value, GLfloat *data);
extern void         (KHRONOS_APIENTRY* const& glGetLocalConstantBooleanvEXT) (GLuint id, GLenum value, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glGetLocalConstantIntegervEXT) (GLuint id, GLenum value, GLint *data);
extern void         (KHRONOS_APIENTRY* const& glGetLocalConstantFloatvEXT) (GLuint id, GLenum value, GLfloat *data);
#endif

#ifndef GL_EXT_vertex_weighting
#define GL_EXT_vertex_weighting 1
enum : GLenum
{
    GL_MODELVIEW0_STACK_DEPTH_EXT                           = 0x0BA3,
    GL_MODELVIEW1_STACK_DEPTH_EXT                           = 0x8502,
    GL_MODELVIEW0_MATRIX_EXT                                = 0x0BA6,
    GL_MODELVIEW1_MATRIX_EXT                                = 0x8506,
    GL_VERTEX_WEIGHTING_EXT                                 = 0x8509,
    GL_MODELVIEW0_EXT                                       = 0x1700,
    GL_MODELVIEW1_EXT                                       = 0x850A,
    GL_CURRENT_VERTEX_WEIGHT_EXT                            = 0x850B,
    GL_VERTEX_WEIGHT_ARRAY_EXT                              = 0x850C,
    GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT                         = 0x850D,
    GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT                         = 0x850E,
    GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT                       = 0x850F,
    GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT                      = 0x8510,
};
extern void         (KHRONOS_APIENTRY* const& glVertexWeightfEXT) (GLfloat weight);
extern void         (KHRONOS_APIENTRY* const& glVertexWeightfvEXT) (const GLfloat *weight);
extern void         (KHRONOS_APIENTRY* const& glVertexWeightPointerEXT) (GLint size, GLenum type, GLsizei stride, const void *pointer);
#endif

#ifndef GL_EXT_win32_keyed_mutex
#define GL_EXT_win32_keyed_mutex 1
extern GLboolean    (KHRONOS_APIENTRY* const& glAcquireKeyedMutexWin32EXT) (GLuint memory, GLuint64 key, GLuint timeout);
extern GLboolean    (KHRONOS_APIENTRY* const& glReleaseKeyedMutexWin32EXT) (GLuint memory, GLuint64 key);
#endif

#ifndef GL_EXT_window_rectangles
#define GL_EXT_window_rectangles 1
enum : GLenum
{
    GL_INCLUSIVE_EXT                                        = 0x8F10,
    GL_EXCLUSIVE_EXT                                        = 0x8F11,
    GL_WINDOW_RECTANGLE_EXT                                 = 0x8F12,
    GL_WINDOW_RECTANGLE_MODE_EXT                            = 0x8F13,
    GL_MAX_WINDOW_RECTANGLES_EXT                            = 0x8F14,
    GL_NUM_WINDOW_RECTANGLES_EXT                            = 0x8F15,
};
extern void         (KHRONOS_APIENTRY* const& glWindowRectanglesEXT) (GLenum mode, GLsizei count, const GLint *box);
#endif

#ifndef GL_EXT_x11_sync_object
#define GL_EXT_x11_sync_object 1
enum : GLenum
{
    GL_SYNC_X11_FENCE_EXT                                   = 0x90E1,
};
extern GLsync       (KHRONOS_APIENTRY* const& glImportSyncEXT) (GLenum external_sync_type, GLintptr external_sync, GLbitfield flags);
#endif

#ifndef GL_GREMEDY_frame_terminator
#define GL_GREMEDY_frame_terminator 1
extern void         (KHRONOS_APIENTRY* const& glFrameTerminatorGREMEDY) ();
#endif

#ifndef GL_GREMEDY_string_marker
#define GL_GREMEDY_string_marker 1
extern void         (KHRONOS_APIENTRY* const& glStringMarkerGREMEDY) (GLsizei len, const void *string);
#endif

#ifndef GL_HP_convolution_border_modes
#define GL_HP_convolution_border_modes 1
enum : GLenum
{
    GL_IGNORE_BORDER_HP                                     = 0x8150,
    GL_CONSTANT_BORDER_HP                                   = 0x8151,
    GL_REPLICATE_BORDER_HP                                  = 0x8153,
    GL_CONVOLUTION_BORDER_COLOR_HP                          = 0x8154,
};
#endif

#ifndef GL_HP_image_transform
#define GL_HP_image_transform 1
enum : GLenum
{
    GL_IMAGE_SCALE_X_HP                                     = 0x8155,
    GL_IMAGE_SCALE_Y_HP                                     = 0x8156,
    GL_IMAGE_TRANSLATE_X_HP                                 = 0x8157,
    GL_IMAGE_TRANSLATE_Y_HP                                 = 0x8158,
    GL_IMAGE_ROTATE_ANGLE_HP                                = 0x8159,
    GL_IMAGE_ROTATE_ORIGIN_X_HP                             = 0x815A,
    GL_IMAGE_ROTATE_ORIGIN_Y_HP                             = 0x815B,
    GL_IMAGE_MAG_FILTER_HP                                  = 0x815C,
    GL_IMAGE_MIN_FILTER_HP                                  = 0x815D,
    GL_IMAGE_CUBIC_WEIGHT_HP                                = 0x815E,
    GL_CUBIC_HP                                             = 0x815F,
    GL_AVERAGE_HP                                           = 0x8160,
    GL_IMAGE_TRANSFORM_2D_HP                                = 0x8161,
    GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP                  = 0x8162,
    GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP            = 0x8163,
};
extern void         (KHRONOS_APIENTRY* const& glImageTransformParameteriHP) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glImageTransformParameterfHP) (GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glImageTransformParameterivHP) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glImageTransformParameterfvHP) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetImageTransformParameterivHP) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetImageTransformParameterfvHP) (GLenum target, GLenum pname, GLfloat *params);
#endif

#ifndef GL_HP_occlusion_test
#define GL_HP_occlusion_test 1
enum : GLenum
{
    GL_OCCLUSION_TEST_HP                                    = 0x8165,
    GL_OCCLUSION_TEST_RESULT_HP                             = 0x8166,
};
#endif

#ifndef GL_HP_texture_lighting
#define GL_HP_texture_lighting 1
enum : GLenum
{
    GL_TEXTURE_LIGHTING_MODE_HP                             = 0x8167,
    GL_TEXTURE_POST_SPECULAR_HP                             = 0x8168,
    GL_TEXTURE_PRE_SPECULAR_HP                              = 0x8169,
};
#endif

#ifndef GL_IBM_cull_vertex
#define GL_IBM_cull_vertex 1
enum : GLenum
{
    GL_CULL_VERTEX_IBM                                      = 103050,
};
#endif

#ifndef GL_IBM_multimode_draw_arrays
#define GL_IBM_multimode_draw_arrays 1
extern void         (KHRONOS_APIENTRY* const& glMultiModeDrawArraysIBM) (const GLenum *mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride);
extern void         (KHRONOS_APIENTRY* const& glMultiModeDrawElementsIBM) (const GLenum *mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount, GLint modestride);
#endif

#ifndef GL_IBM_rasterpos_clip
#define GL_IBM_rasterpos_clip 1
enum : GLenum
{
    GL_RASTER_POSITION_UNCLIPPED_IBM                        = 0x19262,
};
#endif

#ifndef GL_IBM_static_data
#define GL_IBM_static_data 1
enum : GLenum
{
    GL_ALL_STATIC_DATA_IBM                                  = 103060,
    GL_STATIC_VERTEX_ARRAY_IBM                              = 103061,
};
extern void         (KHRONOS_APIENTRY* const& glFlushStaticDataIBM) (GLenum target);
#endif

#ifndef GL_IBM_texture_mirrored_repeat
#define GL_IBM_texture_mirrored_repeat 1
enum : GLenum
{
    GL_MIRRORED_REPEAT_IBM                                  = 0x8370,
};
#endif

#ifndef GL_IBM_vertex_array_lists
#define GL_IBM_vertex_array_lists 1
enum : GLenum
{
    GL_VERTEX_ARRAY_LIST_IBM                                = 103070,
    GL_NORMAL_ARRAY_LIST_IBM                                = 103071,
    GL_COLOR_ARRAY_LIST_IBM                                 = 103072,
    GL_INDEX_ARRAY_LIST_IBM                                 = 103073,
    GL_TEXTURE_COORD_ARRAY_LIST_IBM                         = 103074,
    GL_EDGE_FLAG_ARRAY_LIST_IBM                             = 103075,
    GL_FOG_COORDINATE_ARRAY_LIST_IBM                        = 103076,
    GL_SECONDARY_COLOR_ARRAY_LIST_IBM                       = 103077,
    GL_VERTEX_ARRAY_LIST_STRIDE_IBM                         = 103080,
    GL_NORMAL_ARRAY_LIST_STRIDE_IBM                         = 103081,
    GL_COLOR_ARRAY_LIST_STRIDE_IBM                          = 103082,
    GL_INDEX_ARRAY_LIST_STRIDE_IBM                          = 103083,
    GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM                  = 103084,
    GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM                      = 103085,
    GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM                 = 103086,
    GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM                = 103087,
};
extern void         (KHRONOS_APIENTRY* const& glColorPointerListIBM) (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColorPointerListIBM) (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride);
extern void         (KHRONOS_APIENTRY* const& glEdgeFlagPointerListIBM) (GLint stride, const GLboolean **pointer, GLint ptrstride);
extern void         (KHRONOS_APIENTRY* const& glFogCoordPointerListIBM) (GLenum type, GLint stride, const void **pointer, GLint ptrstride);
extern void         (KHRONOS_APIENTRY* const& glIndexPointerListIBM) (GLenum type, GLint stride, const void **pointer, GLint ptrstride);
extern void         (KHRONOS_APIENTRY* const& glNormalPointerListIBM) (GLenum type, GLint stride, const void **pointer, GLint ptrstride);
extern void         (KHRONOS_APIENTRY* const& glTexCoordPointerListIBM) (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride);
extern void         (KHRONOS_APIENTRY* const& glVertexPointerListIBM) (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride);
#endif

#ifndef GL_INGR_blend_func_separate
#define GL_INGR_blend_func_separate 1
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparateINGR) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
#endif

#ifndef GL_INGR_color_clamp
#define GL_INGR_color_clamp 1
enum : GLenum
{
    GL_RED_MIN_CLAMP_INGR                                   = 0x8560,
    GL_GREEN_MIN_CLAMP_INGR                                 = 0x8561,
    GL_BLUE_MIN_CLAMP_INGR                                  = 0x8562,
    GL_ALPHA_MIN_CLAMP_INGR                                 = 0x8563,
    GL_RED_MAX_CLAMP_INGR                                   = 0x8564,
    GL_GREEN_MAX_CLAMP_INGR                                 = 0x8565,
    GL_BLUE_MAX_CLAMP_INGR                                  = 0x8566,
    GL_ALPHA_MAX_CLAMP_INGR                                 = 0x8567,
};
#endif

#ifndef GL_INGR_interlace_read
#define GL_INGR_interlace_read 1
enum : GLenum
{
    GL_INTERLACE_READ_INGR                                  = 0x8568,
};
#endif

#ifndef GL_INTEL_conservative_rasterization
#define GL_INTEL_conservative_rasterization 1
enum : GLenum
{
    GL_CONSERVATIVE_RASTERIZATION_INTEL                     = 0x83FE,
};
#endif

#ifndef GL_INTEL_fragment_shader_ordering
#define GL_INTEL_fragment_shader_ordering 1
#endif

#ifndef GL_INTEL_framebuffer_CMAA
#define GL_INTEL_framebuffer_CMAA 1
extern void         (KHRONOS_APIENTRY* const& glApplyFramebufferAttachmentCMAAINTEL) ();
#endif

#ifndef GL_INTEL_map_texture
#define GL_INTEL_map_texture 1
enum : GLenum
{
    GL_TEXTURE_MEMORY_LAYOUT_INTEL                          = 0x83FF,
    GL_LAYOUT_DEFAULT_INTEL                                 = 0,
    GL_LAYOUT_LINEAR_INTEL                                  = 1,
    GL_LAYOUT_LINEAR_CPU_CACHED_INTEL                       = 2,
};
extern void         (KHRONOS_APIENTRY* const& glSyncTextureINTEL) (GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glUnmapTexture2DINTEL) (GLuint texture, GLint level);
extern void *       (KHRONOS_APIENTRY* const& glMapTexture2DINTEL) (GLuint texture, GLint level, GLbitfield access, GLint *stride, GLenum *layout);
#endif

#ifndef GL_INTEL_blackhole_render
#define GL_INTEL_blackhole_render 1
enum : GLenum
{
    GL_BLACKHOLE_RENDER_INTEL                               = 0x83FC,
};
#endif

#ifndef GL_INTEL_parallel_arrays
#define GL_INTEL_parallel_arrays 1
enum : GLenum
{
    GL_PARALLEL_ARRAYS_INTEL                                = 0x83F4,
    GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL                 = 0x83F5,
    GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL                 = 0x83F6,
    GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL                  = 0x83F7,
    GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL          = 0x83F8,
};
extern void         (KHRONOS_APIENTRY* const& glVertexPointervINTEL) (GLint size, GLenum type, const void **pointer);
extern void         (KHRONOS_APIENTRY* const& glNormalPointervINTEL) (GLenum type, const void **pointer);
extern void         (KHRONOS_APIENTRY* const& glColorPointervINTEL) (GLint size, GLenum type, const void **pointer);
extern void         (KHRONOS_APIENTRY* const& glTexCoordPointervINTEL) (GLint size, GLenum type, const void **pointer);
#endif

#ifndef GL_INTEL_performance_query
#define GL_INTEL_performance_query 1
enum : GLenum
{
    GL_PERFQUERY_SINGLE_CONTEXT_INTEL                       = 0x00000000,
    GL_PERFQUERY_GLOBAL_CONTEXT_INTEL                       = 0x00000001,
    GL_PERFQUERY_WAIT_INTEL                                 = 0x83FB,
    GL_PERFQUERY_FLUSH_INTEL                                = 0x83FA,
    GL_PERFQUERY_DONOT_FLUSH_INTEL                          = 0x83F9,
    GL_PERFQUERY_COUNTER_EVENT_INTEL                        = 0x94F0,
    GL_PERFQUERY_COUNTER_DURATION_NORM_INTEL                = 0x94F1,
    GL_PERFQUERY_COUNTER_DURATION_RAW_INTEL                 = 0x94F2,
    GL_PERFQUERY_COUNTER_THROUGHPUT_INTEL                   = 0x94F3,
    GL_PERFQUERY_COUNTER_RAW_INTEL                          = 0x94F4,
    GL_PERFQUERY_COUNTER_TIMESTAMP_INTEL                    = 0x94F5,
    GL_PERFQUERY_COUNTER_DATA_UINT32_INTEL                  = 0x94F8,
    GL_PERFQUERY_COUNTER_DATA_UINT64_INTEL                  = 0x94F9,
    GL_PERFQUERY_COUNTER_DATA_FLOAT_INTEL                   = 0x94FA,
    GL_PERFQUERY_COUNTER_DATA_DOUBLE_INTEL                  = 0x94FB,
    GL_PERFQUERY_COUNTER_DATA_BOOL32_INTEL                  = 0x94FC,
    GL_PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL                = 0x94FD,
    GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL              = 0x94FE,
    GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL              = 0x94FF,
    GL_PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL                = 0x9500,
};
extern void         (KHRONOS_APIENTRY* const& glBeginPerfQueryINTEL) (GLuint queryHandle);
extern void         (KHRONOS_APIENTRY* const& glCreatePerfQueryINTEL) (GLuint queryId, GLuint *queryHandle);
extern void         (KHRONOS_APIENTRY* const& glDeletePerfQueryINTEL) (GLuint queryHandle);
extern void         (KHRONOS_APIENTRY* const& glEndPerfQueryINTEL) (GLuint queryHandle);
extern void         (KHRONOS_APIENTRY* const& glGetFirstPerfQueryIdINTEL) (GLuint *queryId);
extern void         (KHRONOS_APIENTRY* const& glGetNextPerfQueryIdINTEL) (GLuint queryId, GLuint *nextQueryId);
extern void         (KHRONOS_APIENTRY* const& glGetPerfCounterInfoINTEL) (GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar *counterName, GLuint counterDescLength, GLchar *counterDesc, GLuint *counterOffset, GLuint *counterDataSize, GLuint *counterTypeEnum, GLuint *counterDataTypeEnum, GLuint64 *rawCounterMaxValue);
extern void         (KHRONOS_APIENTRY* const& glGetPerfQueryDataINTEL) (GLuint queryHandle, GLuint flags, GLsizei dataSize, void *data, GLuint *bytesWritten);
extern void         (KHRONOS_APIENTRY* const& glGetPerfQueryIdByNameINTEL) (GLchar *queryName, GLuint *queryId);
extern void         (KHRONOS_APIENTRY* const& glGetPerfQueryInfoINTEL) (GLuint queryId, GLuint queryNameLength, GLchar *queryName, GLuint *dataSize, GLuint *noCounters, GLuint *noInstances, GLuint *capsMask);
#endif

#ifndef GL_KHR_blend_equation_advanced
#define GL_KHR_blend_equation_advanced 1
enum : GLenum
{
    GL_MULTIPLY_KHR                                         = 0x9294,
    GL_SCREEN_KHR                                           = 0x9295,
    GL_OVERLAY_KHR                                          = 0x9296,
    GL_DARKEN_KHR                                           = 0x9297,
    GL_LIGHTEN_KHR                                          = 0x9298,
    GL_COLORDODGE_KHR                                       = 0x9299,
    GL_COLORBURN_KHR                                        = 0x929A,
    GL_HARDLIGHT_KHR                                        = 0x929B,
    GL_SOFTLIGHT_KHR                                        = 0x929C,
    GL_DIFFERENCE_KHR                                       = 0x929E,
    GL_EXCLUSION_KHR                                        = 0x92A0,
    GL_HSL_HUE_KHR                                          = 0x92AD,
    GL_HSL_SATURATION_KHR                                   = 0x92AE,
    GL_HSL_COLOR_KHR                                        = 0x92AF,
    GL_HSL_LUMINOSITY_KHR                                   = 0x92B0,
};
extern void         (KHRONOS_APIENTRY* const& glBlendBarrierKHR) ();
#endif

#ifndef GL_KHR_blend_equation_advanced_coherent
#define GL_KHR_blend_equation_advanced_coherent 1
enum : GLenum
{
    GL_BLEND_ADVANCED_COHERENT_KHR                          = 0x9285,
};
#endif

#ifndef GL_KHR_context_flush_control
#define GL_KHR_context_flush_control 1
enum : GLenum
{
    GL_CONTEXT_RELEASE_BEHAVIOR_KHR                         = 0x82FB,
    GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR                   = 0x82FC,
};
#endif

#ifndef GL_KHR_debug
#define GL_KHR_debug 1
enum : GLenum
{
    GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR                         = 0x8242,
    GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR                 = 0x8243,
    GL_DEBUG_CALLBACK_FUNCTION_KHR                          = 0x8244,
    GL_DEBUG_CALLBACK_USER_PARAM_KHR                        = 0x8245,
    GL_DEBUG_SOURCE_API_KHR                                 = 0x8246,
    GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR                       = 0x8247,
    GL_DEBUG_SOURCE_SHADER_COMPILER_KHR                     = 0x8248,
    GL_DEBUG_SOURCE_THIRD_PARTY_KHR                         = 0x8249,
    GL_DEBUG_SOURCE_APPLICATION_KHR                         = 0x824A,
    GL_DEBUG_SOURCE_OTHER_KHR                               = 0x824B,
    GL_DEBUG_TYPE_ERROR_KHR                                 = 0x824C,
    GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR                   = 0x824D,
    GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR                    = 0x824E,
    GL_DEBUG_TYPE_PORTABILITY_KHR                           = 0x824F,
    GL_DEBUG_TYPE_PERFORMANCE_KHR                           = 0x8250,
    GL_DEBUG_TYPE_OTHER_KHR                                 = 0x8251,
    GL_DEBUG_TYPE_MARKER_KHR                                = 0x8268,
    GL_DEBUG_TYPE_PUSH_GROUP_KHR                            = 0x8269,
    GL_DEBUG_TYPE_POP_GROUP_KHR                             = 0x826A,
    GL_DEBUG_SEVERITY_NOTIFICATION_KHR                      = 0x826B,
    GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR                      = 0x826C,
    GL_DEBUG_GROUP_STACK_DEPTH_KHR                          = 0x826D,
    GL_BUFFER_KHR                                           = 0x82E0,
    GL_SHADER_KHR                                           = 0x82E1,
    GL_PROGRAM_KHR                                          = 0x82E2,
    GL_VERTEX_ARRAY_KHR                                     = 0x8074,
    GL_QUERY_KHR                                            = 0x82E3,
    GL_PROGRAM_PIPELINE_KHR                                 = 0x82E4,
    GL_SAMPLER_KHR                                          = 0x82E6,
    GL_MAX_LABEL_LENGTH_KHR                                 = 0x82E8,
    GL_MAX_DEBUG_MESSAGE_LENGTH_KHR                         = 0x9143,
    GL_MAX_DEBUG_LOGGED_MESSAGES_KHR                        = 0x9144,
    GL_DEBUG_LOGGED_MESSAGES_KHR                            = 0x9145,
    GL_DEBUG_SEVERITY_HIGH_KHR                              = 0x9146,
    GL_DEBUG_SEVERITY_MEDIUM_KHR                            = 0x9147,
    GL_DEBUG_SEVERITY_LOW_KHR                               = 0x9148,
    GL_DEBUG_OUTPUT_KHR                                     = 0x92E0,
    GL_CONTEXT_FLAG_DEBUG_BIT_KHR                           = 0x00000002,
    GL_STACK_OVERFLOW_KHR                                   = 0x0503,
    GL_STACK_UNDERFLOW_KHR                                  = 0x0504,
};
extern void         (KHRONOS_APIENTRY* const& glDebugMessageControlKHR) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageInsertKHR) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
extern void         (KHRONOS_APIENTRY* const& glDebugMessageCallbackKHR) (GLDEBUGPROCKHR callback, const void *userParam);
extern GLuint       (KHRONOS_APIENTRY* const& glGetDebugMessageLogKHR) (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
extern void         (KHRONOS_APIENTRY* const& glPushDebugGroupKHR) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
extern void         (KHRONOS_APIENTRY* const& glPopDebugGroupKHR) ();
extern void         (KHRONOS_APIENTRY* const& glObjectLabelKHR) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glGetObjectLabelKHR) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glObjectPtrLabelKHR) (const void *ptr, GLsizei length, const GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glGetObjectPtrLabelKHR) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
extern void         (KHRONOS_APIENTRY* const& glGetPointervKHR) (GLenum pname, void **params);
#endif

#ifndef GL_KHR_no_error
#define GL_KHR_no_error 1
enum : GLenum
{
    GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR                        = 0x00000008,
};
#endif

#ifndef GL_KHR_robust_buffer_access_behavior
#define GL_KHR_robust_buffer_access_behavior 1
#endif

#ifndef GL_KHR_robustness
#define GL_KHR_robustness 1
enum : GLenum
{
    GL_CONTEXT_ROBUST_ACCESS                                = 0x90F3,
    GL_CONTEXT_ROBUST_ACCESS_KHR                            = 0x90F3,
    GL_LOSE_CONTEXT_ON_RESET_KHR                            = 0x8252,
    GL_GUILTY_CONTEXT_RESET_KHR                             = 0x8253,
    GL_INNOCENT_CONTEXT_RESET_KHR                           = 0x8254,
    GL_UNKNOWN_CONTEXT_RESET_KHR                            = 0x8255,
    GL_RESET_NOTIFICATION_STRATEGY_KHR                      = 0x8256,
    GL_NO_RESET_NOTIFICATION_KHR                            = 0x8261,
    GL_CONTEXT_LOST_KHR                                     = 0x0507,
};
extern GLenum       (KHRONOS_APIENTRY* const& glGetGraphicsResetStatusKHR) ();
extern void         (KHRONOS_APIENTRY* const& glReadnPixelsKHR) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformfvKHR) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformivKHR) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformuivKHR) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
#endif

#ifndef GL_KHR_shader_subgroup
#define GL_KHR_shader_subgroup 1
enum : GLenum
{
    GL_SUBGROUP_SIZE_KHR                                    = 0x9532,
    GL_SUBGROUP_SUPPORTED_STAGES_KHR                        = 0x9533,
    GL_SUBGROUP_SUPPORTED_FEATURES_KHR                      = 0x9534,
    GL_SUBGROUP_QUAD_ALL_STAGES_KHR                         = 0x9535,
    GL_SUBGROUP_FEATURE_BASIC_BIT_KHR                       = 0x00000001,
    GL_SUBGROUP_FEATURE_VOTE_BIT_KHR                        = 0x00000002,
    GL_SUBGROUP_FEATURE_ARITHMETIC_BIT_KHR                  = 0x00000004,
    GL_SUBGROUP_FEATURE_BALLOT_BIT_KHR                      = 0x00000008,
    GL_SUBGROUP_FEATURE_SHUFFLE_BIT_KHR                     = 0x00000010,
    GL_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT_KHR            = 0x00000020,
    GL_SUBGROUP_FEATURE_CLUSTERED_BIT_KHR                   = 0x00000040,
    GL_SUBGROUP_FEATURE_QUAD_BIT_KHR                        = 0x00000080,
};
#endif

#ifndef GL_KHR_texture_compression_astc_hdr
#define GL_KHR_texture_compression_astc_hdr 1
enum : GLenum
{
    GL_COMPRESSED_RGBA_ASTC_4x4_KHR                         = 0x93B0,
    GL_COMPRESSED_RGBA_ASTC_5x4_KHR                         = 0x93B1,
    GL_COMPRESSED_RGBA_ASTC_5x5_KHR                         = 0x93B2,
    GL_COMPRESSED_RGBA_ASTC_6x5_KHR                         = 0x93B3,
    GL_COMPRESSED_RGBA_ASTC_6x6_KHR                         = 0x93B4,
    GL_COMPRESSED_RGBA_ASTC_8x5_KHR                         = 0x93B5,
    GL_COMPRESSED_RGBA_ASTC_8x6_KHR                         = 0x93B6,
    GL_COMPRESSED_RGBA_ASTC_8x8_KHR                         = 0x93B7,
    GL_COMPRESSED_RGBA_ASTC_10x5_KHR                        = 0x93B8,
    GL_COMPRESSED_RGBA_ASTC_10x6_KHR                        = 0x93B9,
    GL_COMPRESSED_RGBA_ASTC_10x8_KHR                        = 0x93BA,
    GL_COMPRESSED_RGBA_ASTC_10x10_KHR                       = 0x93BB,
    GL_COMPRESSED_RGBA_ASTC_12x10_KHR                       = 0x93BC,
    GL_COMPRESSED_RGBA_ASTC_12x12_KHR                       = 0x93BD,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR                 = 0x93D0,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR                 = 0x93D1,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR                 = 0x93D2,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR                 = 0x93D3,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR                 = 0x93D4,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR                 = 0x93D5,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR                 = 0x93D6,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR                 = 0x93D7,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR                = 0x93D8,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR                = 0x93D9,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR                = 0x93DA,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR               = 0x93DB,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR               = 0x93DC,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR               = 0x93DD,
};
#endif

#ifndef GL_KHR_texture_compression_astc_ldr
#define GL_KHR_texture_compression_astc_ldr 1
#endif

#ifndef GL_KHR_texture_compression_astc_sliced_3d
#define GL_KHR_texture_compression_astc_sliced_3d 1
#endif

#ifndef GL_KHR_parallel_shader_compile
#define GL_KHR_parallel_shader_compile 1
enum : GLenum
{
    GL_MAX_SHADER_COMPILER_THREADS_KHR                      = 0x91B0,
    GL_COMPLETION_STATUS_KHR                                = 0x91B1,
};
extern void         (KHRONOS_APIENTRY* const& glMaxShaderCompilerThreadsKHR) (GLuint count);
#endif

#ifndef GL_MESAX_texture_stack
#define GL_MESAX_texture_stack 1
enum : GLenum
{
    GL_TEXTURE_1D_STACK_MESAX                               = 0x8759,
    GL_TEXTURE_2D_STACK_MESAX                               = 0x875A,
    GL_PROXY_TEXTURE_1D_STACK_MESAX                         = 0x875B,
    GL_PROXY_TEXTURE_2D_STACK_MESAX                         = 0x875C,
    GL_TEXTURE_1D_STACK_BINDING_MESAX                       = 0x875D,
    GL_TEXTURE_2D_STACK_BINDING_MESAX                       = 0x875E,
};
#endif

#ifndef GL_MESA_framebuffer_flip_x
#define GL_MESA_framebuffer_flip_x 1
enum : GLenum
{
    GL_FRAMEBUFFER_FLIP_X_MESA                              = 0x8BBC,
};
#endif

#ifndef GL_MESA_framebuffer_flip_y
#define GL_MESA_framebuffer_flip_y 1
enum : GLenum
{
    GL_FRAMEBUFFER_FLIP_Y_MESA                              = 0x8BBB,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferParameteriMESA) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferParameterivMESA) (GLenum target, GLenum pname, GLint *params);
#endif

#ifndef GL_MESA_framebuffer_swap_xy
#define GL_MESA_framebuffer_swap_xy 1
enum : GLenum
{
    GL_FRAMEBUFFER_SWAP_XY_MESA                             = 0x8BBD,
};
#endif

#ifndef GL_MESA_pack_invert
#define GL_MESA_pack_invert 1
enum : GLenum
{
    GL_PACK_INVERT_MESA                                     = 0x8758,
};
#endif

#ifndef GL_MESA_program_binary_formats
#define GL_MESA_program_binary_formats 1
enum : GLenum
{
    GL_PROGRAM_BINARY_FORMAT_MESA                           = 0x875F,
};
#endif

#ifndef GL_MESA_resize_buffers
#define GL_MESA_resize_buffers 1
extern void         (KHRONOS_APIENTRY* const& glResizeBuffersMESA) ();
#endif

#ifndef GL_MESA_shader_integer_functions
#define GL_MESA_shader_integer_functions 1
#endif

#ifndef GL_MESA_tile_raster_order
#define GL_MESA_tile_raster_order 1
enum : GLenum
{
    GL_TILE_RASTER_ORDER_FIXED_MESA                         = 0x8BB8,
    GL_TILE_RASTER_ORDER_INCREASING_X_MESA                  = 0x8BB9,
    GL_TILE_RASTER_ORDER_INCREASING_Y_MESA                  = 0x8BBA,
};
#endif

#ifndef GL_MESA_window_pos
#define GL_MESA_window_pos 1
extern void         (KHRONOS_APIENTRY* const& glWindowPos2dMESA) (GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2dvMESA) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2fMESA) (GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2fvMESA) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2iMESA) (GLint x, GLint y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2ivMESA) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2sMESA) (GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glWindowPos2svMESA) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3dMESA) (GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3dvMESA) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3fMESA) (GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3fvMESA) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3iMESA) (GLint x, GLint y, GLint z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3ivMESA) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3sMESA) (GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glWindowPos3svMESA) (const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4dMESA) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4dvMESA) (const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4fMESA) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4fvMESA) (const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4iMESA) (GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4ivMESA) (const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4sMESA) (GLshort x, GLshort y, GLshort z, GLshort w);
extern void         (KHRONOS_APIENTRY* const& glWindowPos4svMESA) (const GLshort *v);
#endif

#ifndef GL_MESA_ycbcr_texture
#define GL_MESA_ycbcr_texture 1
enum : GLenum
{
    GL_UNSIGNED_SHORT_8_8_MESA                              = 0x85BA,
    GL_UNSIGNED_SHORT_8_8_REV_MESA                          = 0x85BB,
    GL_YCBCR_MESA                                           = 0x8757,
};
#endif

#ifndef GL_NVX_blend_equation_advanced_multi_draw_buffers
#define GL_NVX_blend_equation_advanced_multi_draw_buffers 1
#endif

#ifndef GL_NVX_conditional_render
#define GL_NVX_conditional_render 1
extern void         (KHRONOS_APIENTRY* const& glBeginConditionalRenderNVX) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glEndConditionalRenderNVX) ();
#endif

#ifndef GL_NVX_gpu_memory_info
#define GL_NVX_gpu_memory_info 1
enum : GLenum
{
    GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX                 = 0x9047,
    GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX           = 0x9048,
    GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX         = 0x9049,
    GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX                   = 0x904A,
    GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX                   = 0x904B,
};
#endif

#ifndef GL_NVX_linked_gpu_multicast
#define GL_NVX_linked_gpu_multicast 1
enum : GLenum
{
    GL_LGPU_SEPARATE_STORAGE_BIT_NVX                        = 0x0800,
    GL_MAX_LGPU_GPUS_NVX                                    = 0x92BA,
};
extern void         (KHRONOS_APIENTRY* const& glLGPUNamedBufferSubDataNVX) (GLbitfield gpuMask, GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
extern void         (KHRONOS_APIENTRY* const& glLGPUCopyImageSubDataNVX) (GLuint sourceGpu, GLbitfield destinationGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srxY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth);
extern void         (KHRONOS_APIENTRY* const& glLGPUInterlockNVX) ();
#endif

#ifndef GL_NV_alpha_to_coverage_dither_control
#define GL_NV_alpha_to_coverage_dither_control 1
enum : GLenum
{
    GL_ALPHA_TO_COVERAGE_DITHER_DEFAULT_NV                  = 0x934D,
    GL_ALPHA_TO_COVERAGE_DITHER_ENABLE_NV                   = 0x934E,
    GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV                  = 0x934F,
    GL_ALPHA_TO_COVERAGE_DITHER_MODE_NV                     = 0x92BF,
};
extern void         (KHRONOS_APIENTRY* const& glAlphaToCoverageDitherControlNV) (GLenum mode);
#endif

#ifndef GL_NV_bindless_multi_draw_indirect
#define GL_NV_bindless_multi_draw_indirect 1
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysIndirectBindlessNV) (GLenum mode, const void *indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsIndirectBindlessNV) (GLenum mode, GLenum type, const void *indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount);
#endif

#ifndef GL_NV_bindless_multi_draw_indirect_count
#define GL_NV_bindless_multi_draw_indirect_count 1
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysIndirectBindlessCountNV) (GLenum mode, const void *indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsIndirectBindlessCountNV) (GLenum mode, GLenum type, const void *indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount);
#endif

#ifndef GL_NV_bindless_texture
#define GL_NV_bindless_texture 1
extern GLuint64     (KHRONOS_APIENTRY* const& glGetTextureHandleNV) (GLuint texture);
extern GLuint64     (KHRONOS_APIENTRY* const& glGetTextureSamplerHandleNV) (GLuint texture, GLuint sampler);
extern void         (KHRONOS_APIENTRY* const& glMakeTextureHandleResidentNV) (GLuint64 handle);
extern void         (KHRONOS_APIENTRY* const& glMakeTextureHandleNonResidentNV) (GLuint64 handle);
extern GLuint64     (KHRONOS_APIENTRY* const& glGetImageHandleNV) (GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
extern void         (KHRONOS_APIENTRY* const& glMakeImageHandleResidentNV) (GLuint64 handle, GLenum access);
extern void         (KHRONOS_APIENTRY* const& glMakeImageHandleNonResidentNV) (GLuint64 handle);
extern void         (KHRONOS_APIENTRY* const& glUniformHandleui64NV) (GLint location, GLuint64 value);
extern void         (KHRONOS_APIENTRY* const& glUniformHandleui64vNV) (GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformHandleui64NV) (GLuint program, GLint location, GLuint64 value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformHandleui64vNV) (GLuint program, GLint location, GLsizei count, const GLuint64 *values);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTextureHandleResidentNV) (GLuint64 handle);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsImageHandleResidentNV) (GLuint64 handle);
#endif

#ifndef GL_NV_blend_equation_advanced
#define GL_NV_blend_equation_advanced 1
enum : GLenum
{
    GL_BLEND_OVERLAP_NV                                     = 0x9281,
    GL_BLEND_PREMULTIPLIED_SRC_NV                           = 0x9280,
    GL_BLUE_NV                                              = 0x1905,
    GL_COLORBURN_NV                                         = 0x929A,
    GL_COLORDODGE_NV                                        = 0x9299,
    GL_CONJOINT_NV                                          = 0x9284,
    GL_CONTRAST_NV                                          = 0x92A1,
    GL_DARKEN_NV                                            = 0x9297,
    GL_DIFFERENCE_NV                                        = 0x929E,
    GL_DISJOINT_NV                                          = 0x9283,
    GL_DST_ATOP_NV                                          = 0x928F,
    GL_DST_IN_NV                                            = 0x928B,
    GL_DST_NV                                               = 0x9287,
    GL_DST_OUT_NV                                           = 0x928D,
    GL_DST_OVER_NV                                          = 0x9289,
    GL_EXCLUSION_NV                                         = 0x92A0,
    GL_GREEN_NV                                             = 0x1904,
    GL_HARDLIGHT_NV                                         = 0x929B,
    GL_HARDMIX_NV                                           = 0x92A9,
    GL_HSL_COLOR_NV                                         = 0x92AF,
    GL_HSL_HUE_NV                                           = 0x92AD,
    GL_HSL_LUMINOSITY_NV                                    = 0x92B0,
    GL_HSL_SATURATION_NV                                    = 0x92AE,
    GL_INVERT_OVG_NV                                        = 0x92B4,
    GL_INVERT_RGB_NV                                        = 0x92A3,
    GL_LIGHTEN_NV                                           = 0x9298,
    GL_LINEARBURN_NV                                        = 0x92A5,
    GL_LINEARDODGE_NV                                       = 0x92A4,
    GL_LINEARLIGHT_NV                                       = 0x92A7,
    GL_MINUS_CLAMPED_NV                                     = 0x92B3,
    GL_MINUS_NV                                             = 0x929F,
    GL_MULTIPLY_NV                                          = 0x9294,
    GL_OVERLAY_NV                                           = 0x9296,
    GL_PINLIGHT_NV                                          = 0x92A8,
    GL_PLUS_CLAMPED_ALPHA_NV                                = 0x92B2,
    GL_PLUS_CLAMPED_NV                                      = 0x92B1,
    GL_PLUS_DARKER_NV                                       = 0x9292,
    GL_PLUS_NV                                              = 0x9291,
    GL_RED_NV                                               = 0x1903,
    GL_SCREEN_NV                                            = 0x9295,
    GL_SOFTLIGHT_NV                                         = 0x929C,
    GL_SRC_ATOP_NV                                          = 0x928E,
    GL_SRC_IN_NV                                            = 0x928A,
    GL_SRC_NV                                               = 0x9286,
    GL_SRC_OUT_NV                                           = 0x928C,
    GL_SRC_OVER_NV                                          = 0x9288,
    GL_UNCORRELATED_NV                                      = 0x9282,
    GL_VIVIDLIGHT_NV                                        = 0x92A6,
    GL_XOR_NV                                               = 0x1506,
};
extern void         (KHRONOS_APIENTRY* const& glBlendParameteriNV) (GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glBlendBarrierNV) ();
#endif

#ifndef GL_NV_blend_equation_advanced_coherent
#define GL_NV_blend_equation_advanced_coherent 1
enum : GLenum
{
    GL_BLEND_ADVANCED_COHERENT_NV                           = 0x9285,
};
#endif

#ifndef GL_NV_blend_minmax_factor
#define GL_NV_blend_minmax_factor 1
#endif

#ifndef GL_NV_blend_square
#define GL_NV_blend_square 1
#endif

#ifndef GL_NV_clip_space_w_scaling
#define GL_NV_clip_space_w_scaling 1
enum : GLenum
{
    GL_VIEWPORT_POSITION_W_SCALE_NV                         = 0x937C,
    GL_VIEWPORT_POSITION_W_SCALE_X_COEFF_NV                 = 0x937D,
    GL_VIEWPORT_POSITION_W_SCALE_Y_COEFF_NV                 = 0x937E,
};
extern void         (KHRONOS_APIENTRY* const& glViewportPositionWScaleNV) (GLuint index, GLfloat xcoeff, GLfloat ycoeff);
#endif

#ifndef GL_NV_command_list
#define GL_NV_command_list 1
enum : GLenum
{
    GL_TERMINATE_SEQUENCE_COMMAND_NV                        = 0x0000,
    GL_NOP_COMMAND_NV                                       = 0x0001,
    GL_DRAW_ELEMENTS_COMMAND_NV                             = 0x0002,
    GL_DRAW_ARRAYS_COMMAND_NV                               = 0x0003,
    GL_DRAW_ELEMENTS_STRIP_COMMAND_NV                       = 0x0004,
    GL_DRAW_ARRAYS_STRIP_COMMAND_NV                         = 0x0005,
    GL_DRAW_ELEMENTS_INSTANCED_COMMAND_NV                   = 0x0006,
    GL_DRAW_ARRAYS_INSTANCED_COMMAND_NV                     = 0x0007,
    GL_ELEMENT_ADDRESS_COMMAND_NV                           = 0x0008,
    GL_ATTRIBUTE_ADDRESS_COMMAND_NV                         = 0x0009,
    GL_UNIFORM_ADDRESS_COMMAND_NV                           = 0x000A,
    GL_BLEND_COLOR_COMMAND_NV                               = 0x000B,
    GL_STENCIL_REF_COMMAND_NV                               = 0x000C,
    GL_LINE_WIDTH_COMMAND_NV                                = 0x000D,
    GL_POLYGON_OFFSET_COMMAND_NV                            = 0x000E,
    GL_ALPHA_REF_COMMAND_NV                                 = 0x000F,
    GL_VIEWPORT_COMMAND_NV                                  = 0x0010,
    GL_SCISSOR_COMMAND_NV                                   = 0x0011,
    GL_FRONT_FACE_COMMAND_NV                                = 0x0012,
};
extern void         (KHRONOS_APIENTRY* const& glCreateStatesNV) (GLsizei n, GLuint *states);
extern void         (KHRONOS_APIENTRY* const& glDeleteStatesNV) (GLsizei n, const GLuint *states);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsStateNV) (GLuint state);
extern void         (KHRONOS_APIENTRY* const& glStateCaptureNV) (GLuint state, GLenum mode);
extern GLuint       (KHRONOS_APIENTRY* const& glGetCommandHeaderNV) (GLenum tokenID, GLuint size);
extern GLushort     (KHRONOS_APIENTRY* const& glGetStageIndexNV) (GLenum shadertype);
extern void         (KHRONOS_APIENTRY* const& glDrawCommandsNV) (GLenum primitiveMode, GLuint buffer, const GLintptr *indirects, const GLsizei *sizes, GLuint count);
extern void         (KHRONOS_APIENTRY* const& glDrawCommandsAddressNV) (GLenum primitiveMode, const GLuint64 *indirects, const GLsizei *sizes, GLuint count);
extern void         (KHRONOS_APIENTRY* const& glDrawCommandsStatesNV) (GLuint buffer, const GLintptr *indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count);
extern void         (KHRONOS_APIENTRY* const& glDrawCommandsStatesAddressNV) (const GLuint64 *indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count);
extern void         (KHRONOS_APIENTRY* const& glCreateCommandListsNV) (GLsizei n, GLuint *lists);
extern void         (KHRONOS_APIENTRY* const& glDeleteCommandListsNV) (GLsizei n, const GLuint *lists);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsCommandListNV) (GLuint list);
extern void         (KHRONOS_APIENTRY* const& glListDrawCommandsStatesClientNV) (GLuint list, GLuint segment, const void **indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count);
extern void         (KHRONOS_APIENTRY* const& glCommandListSegmentsNV) (GLuint list, GLuint segments);
extern void         (KHRONOS_APIENTRY* const& glCompileCommandListNV) (GLuint list);
extern void         (KHRONOS_APIENTRY* const& glCallCommandListNV) (GLuint list);
#endif

#ifndef GL_NV_compute_program5
#define GL_NV_compute_program5 1
enum : GLenum
{
    GL_COMPUTE_PROGRAM_NV                                   = 0x90FB,
    GL_COMPUTE_PROGRAM_PARAMETER_BUFFER_NV                  = 0x90FC,
};
#endif

#ifndef GL_NV_compute_shader_derivatives
#define GL_NV_compute_shader_derivatives 1
#endif

#ifndef GL_NV_conditional_render
#define GL_NV_conditional_render 1
enum : GLenum
{
    GL_QUERY_WAIT_NV                                        = 0x8E13,
    GL_QUERY_NO_WAIT_NV                                     = 0x8E14,
    GL_QUERY_BY_REGION_WAIT_NV                              = 0x8E15,
    GL_QUERY_BY_REGION_NO_WAIT_NV                           = 0x8E16,
};
extern void         (KHRONOS_APIENTRY* const& glBeginConditionalRenderNV) (GLuint id, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glEndConditionalRenderNV) ();
#endif

#ifndef GL_NV_conservative_raster
#define GL_NV_conservative_raster 1
enum : GLenum
{
    GL_CONSERVATIVE_RASTERIZATION_NV                        = 0x9346,
    GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV                    = 0x9347,
    GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV                    = 0x9348,
    GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV                  = 0x9349,
};
extern void         (KHRONOS_APIENTRY* const& glSubpixelPrecisionBiasNV) (GLuint xbits, GLuint ybits);
#endif

#ifndef GL_NV_conservative_raster_dilate
#define GL_NV_conservative_raster_dilate 1
enum : GLenum
{
    GL_CONSERVATIVE_RASTER_DILATE_NV                        = 0x9379,
    GL_CONSERVATIVE_RASTER_DILATE_RANGE_NV                  = 0x937A,
    GL_CONSERVATIVE_RASTER_DILATE_GRANULARITY_NV            = 0x937B,
};
extern void         (KHRONOS_APIENTRY* const& glConservativeRasterParameterfNV) (GLenum pname, GLfloat value);
#endif

#ifndef GL_NV_conservative_raster_pre_snap
#define GL_NV_conservative_raster_pre_snap 1
enum : GLenum
{
    GL_CONSERVATIVE_RASTER_MODE_PRE_SNAP_NV                 = 0x9550,
};
#endif

#ifndef GL_NV_conservative_raster_pre_snap_triangles
#define GL_NV_conservative_raster_pre_snap_triangles 1
enum : GLenum
{
    GL_CONSERVATIVE_RASTER_MODE_NV                          = 0x954D,
    GL_CONSERVATIVE_RASTER_MODE_POST_SNAP_NV                = 0x954E,
    GL_CONSERVATIVE_RASTER_MODE_PRE_SNAP_TRIANGLES_NV       = 0x954F,
};
extern void         (KHRONOS_APIENTRY* const& glConservativeRasterParameteriNV) (GLenum pname, GLint param);
#endif

#ifndef GL_NV_conservative_raster_underestimation
#define GL_NV_conservative_raster_underestimation 1
#endif

#ifndef GL_NV_copy_depth_to_color
#define GL_NV_copy_depth_to_color 1
enum : GLenum
{
    GL_DEPTH_STENCIL_TO_RGBA_NV                             = 0x886E,
    GL_DEPTH_STENCIL_TO_BGRA_NV                             = 0x886F,
};
#endif

#ifndef GL_NV_copy_image
#define GL_NV_copy_image 1
extern void         (KHRONOS_APIENTRY* const& glCopyImageSubDataNV) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth);
#endif

#ifndef GL_NV_deep_texture3D
#define GL_NV_deep_texture3D 1
enum : GLenum
{
    GL_MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT_NV                  = 0x90D0,
    GL_MAX_DEEP_3D_TEXTURE_DEPTH_NV                         = 0x90D1,
};
#endif

#ifndef GL_NV_depth_buffer_float
#define GL_NV_depth_buffer_float 1
enum : GLenum
{
    GL_DEPTH_COMPONENT32F_NV                                = 0x8DAB,
    GL_DEPTH32F_STENCIL8_NV                                 = 0x8DAC,
    GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV                    = 0x8DAD,
    GL_DEPTH_BUFFER_FLOAT_MODE_NV                           = 0x8DAF,
};
extern void         (KHRONOS_APIENTRY* const& glDepthRangedNV) (GLdouble zNear, GLdouble zFar);
extern void         (KHRONOS_APIENTRY* const& glClearDepthdNV) (GLdouble depth);
extern void         (KHRONOS_APIENTRY* const& glDepthBoundsdNV) (GLdouble zmin, GLdouble zmax);
#endif

#ifndef GL_NV_depth_clamp
#define GL_NV_depth_clamp 1
enum : GLenum
{
    GL_DEPTH_CLAMP_NV                                       = 0x864F,
};
#endif

#ifndef GL_NV_draw_texture
#define GL_NV_draw_texture 1
extern void         (KHRONOS_APIENTRY* const& glDrawTextureNV) (GLuint texture, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1);
#endif

#ifndef GL_NV_draw_vulkan_image
#define GL_NV_draw_vulkan_image 1
extern void         (KHRONOS_APIENTRY* const& glDrawVkImageNV) (GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1);
extern GLVULKANPROCNV (KHRONOS_APIENTRY* const& glGetVkProcAddrNV) (const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glWaitVkSemaphoreNV) (GLuint64 vkSemaphore);
extern void         (KHRONOS_APIENTRY* const& glSignalVkSemaphoreNV) (GLuint64 vkSemaphore);
extern void         (KHRONOS_APIENTRY* const& glSignalVkFenceNV) (GLuint64 vkFence);
#endif

#ifndef GL_NV_evaluators
#define GL_NV_evaluators 1
enum : GLenum
{
    GL_EVAL_2D_NV                                           = 0x86C0,
    GL_EVAL_TRIANGULAR_2D_NV                                = 0x86C1,
    GL_MAP_TESSELLATION_NV                                  = 0x86C2,
    GL_MAP_ATTRIB_U_ORDER_NV                                = 0x86C3,
    GL_MAP_ATTRIB_V_ORDER_NV                                = 0x86C4,
    GL_EVAL_FRACTIONAL_TESSELLATION_NV                      = 0x86C5,
    GL_EVAL_VERTEX_ATTRIB0_NV                               = 0x86C6,
    GL_EVAL_VERTEX_ATTRIB1_NV                               = 0x86C7,
    GL_EVAL_VERTEX_ATTRIB2_NV                               = 0x86C8,
    GL_EVAL_VERTEX_ATTRIB3_NV                               = 0x86C9,
    GL_EVAL_VERTEX_ATTRIB4_NV                               = 0x86CA,
    GL_EVAL_VERTEX_ATTRIB5_NV                               = 0x86CB,
    GL_EVAL_VERTEX_ATTRIB6_NV                               = 0x86CC,
    GL_EVAL_VERTEX_ATTRIB7_NV                               = 0x86CD,
    GL_EVAL_VERTEX_ATTRIB8_NV                               = 0x86CE,
    GL_EVAL_VERTEX_ATTRIB9_NV                               = 0x86CF,
    GL_EVAL_VERTEX_ATTRIB10_NV                              = 0x86D0,
    GL_EVAL_VERTEX_ATTRIB11_NV                              = 0x86D1,
    GL_EVAL_VERTEX_ATTRIB12_NV                              = 0x86D2,
    GL_EVAL_VERTEX_ATTRIB13_NV                              = 0x86D3,
    GL_EVAL_VERTEX_ATTRIB14_NV                              = 0x86D4,
    GL_EVAL_VERTEX_ATTRIB15_NV                              = 0x86D5,
    GL_MAX_MAP_TESSELLATION_NV                              = 0x86D6,
    GL_MAX_RATIONAL_EVAL_ORDER_NV                           = 0x86D7,
};
extern void         (KHRONOS_APIENTRY* const& glMapControlPointsNV) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const void *points);
extern void         (KHRONOS_APIENTRY* const& glMapParameterivNV) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glMapParameterfvNV) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMapControlPointsNV) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, void *points);
extern void         (KHRONOS_APIENTRY* const& glGetMapParameterivNV) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMapParameterfvNV) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetMapAttribParameterivNV) (GLenum target, GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetMapAttribParameterfvNV) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glEvalMapsNV) (GLenum target, GLenum mode);
#endif

#ifndef GL_NV_explicit_multisample
#define GL_NV_explicit_multisample 1
enum : GLenum
{
    GL_SAMPLE_POSITION_NV                                   = 0x8E50,
    GL_SAMPLE_MASK_NV                                       = 0x8E51,
    GL_SAMPLE_MASK_VALUE_NV                                 = 0x8E52,
    GL_TEXTURE_BINDING_RENDERBUFFER_NV                      = 0x8E53,
    GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV           = 0x8E54,
    GL_TEXTURE_RENDERBUFFER_NV                              = 0x8E55,
    GL_SAMPLER_RENDERBUFFER_NV                              = 0x8E56,
    GL_INT_SAMPLER_RENDERBUFFER_NV                          = 0x8E57,
    GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV                 = 0x8E58,
    GL_MAX_SAMPLE_MASK_WORDS_NV                             = 0x8E59,
};
extern void         (KHRONOS_APIENTRY* const& glGetMultisamplefvNV) (GLenum pname, GLuint index, GLfloat *val);
extern void         (KHRONOS_APIENTRY* const& glSampleMaskIndexedNV) (GLuint index, GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glTexRenderbufferNV) (GLenum target, GLuint renderbuffer);
#endif

#ifndef GL_NV_fence
#define GL_NV_fence 1
enum : GLenum
{
    GL_ALL_COMPLETED_NV                                     = 0x84F2,
    GL_FENCE_STATUS_NV                                      = 0x84F3,
    GL_FENCE_CONDITION_NV                                   = 0x84F4,
};
extern void         (KHRONOS_APIENTRY* const& glDeleteFencesNV) (GLsizei n, const GLuint *fences);
extern void         (KHRONOS_APIENTRY* const& glGenFencesNV) (GLsizei n, GLuint *fences);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsFenceNV) (GLuint fence);
extern GLboolean    (KHRONOS_APIENTRY* const& glTestFenceNV) (GLuint fence);
extern void         (KHRONOS_APIENTRY* const& glGetFenceivNV) (GLuint fence, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glFinishFenceNV) (GLuint fence);
extern void         (KHRONOS_APIENTRY* const& glSetFenceNV) (GLuint fence, GLenum condition);
#endif

#ifndef GL_NV_fill_rectangle
#define GL_NV_fill_rectangle 1
enum : GLenum
{
    GL_FILL_RECTANGLE_NV                                    = 0x933C,
};
#endif

#ifndef GL_NV_float_buffer
#define GL_NV_float_buffer 1
enum : GLenum
{
    GL_FLOAT_R_NV                                           = 0x8880,
    GL_FLOAT_RG_NV                                          = 0x8881,
    GL_FLOAT_RGB_NV                                         = 0x8882,
    GL_FLOAT_RGBA_NV                                        = 0x8883,
    GL_FLOAT_R16_NV                                         = 0x8884,
    GL_FLOAT_R32_NV                                         = 0x8885,
    GL_FLOAT_RG16_NV                                        = 0x8886,
    GL_FLOAT_RG32_NV                                        = 0x8887,
    GL_FLOAT_RGB16_NV                                       = 0x8888,
    GL_FLOAT_RGB32_NV                                       = 0x8889,
    GL_FLOAT_RGBA16_NV                                      = 0x888A,
    GL_FLOAT_RGBA32_NV                                      = 0x888B,
    GL_TEXTURE_FLOAT_COMPONENTS_NV                          = 0x888C,
    GL_FLOAT_CLEAR_COLOR_VALUE_NV                           = 0x888D,
    GL_FLOAT_RGBA_MODE_NV                                   = 0x888E,
};
#endif

#ifndef GL_NV_fog_distance
#define GL_NV_fog_distance 1
enum : GLenum
{
    GL_FOG_DISTANCE_MODE_NV                                 = 0x855A,
    GL_EYE_RADIAL_NV                                        = 0x855B,
    GL_EYE_PLANE_ABSOLUTE_NV                                = 0x855C,
};
#endif

#ifndef GL_NV_fragment_coverage_to_color
#define GL_NV_fragment_coverage_to_color 1
enum : GLenum
{
    GL_FRAGMENT_COVERAGE_TO_COLOR_NV                        = 0x92DD,
    GL_FRAGMENT_COVERAGE_COLOR_NV                           = 0x92DE,
};
extern void         (KHRONOS_APIENTRY* const& glFragmentCoverageColorNV) (GLuint color);
#endif

#ifndef GL_NV_fragment_program
#define GL_NV_fragment_program 1
enum : GLenum
{
    GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV             = 0x8868,
    GL_FRAGMENT_PROGRAM_NV                                  = 0x8870,
    GL_MAX_TEXTURE_COORDS_NV                                = 0x8871,
    GL_MAX_TEXTURE_IMAGE_UNITS_NV                           = 0x8872,
    GL_FRAGMENT_PROGRAM_BINDING_NV                          = 0x8873,
    GL_PROGRAM_ERROR_STRING_NV                              = 0x8874,
};
extern void         (KHRONOS_APIENTRY* const& glProgramNamedParameter4fNV) (GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glProgramNamedParameter4fvNV) (GLuint id, GLsizei len, const GLubyte *name, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glProgramNamedParameter4dNV) (GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glProgramNamedParameter4dvNV) (GLuint id, GLsizei len, const GLubyte *name, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glGetProgramNamedParameterfvNV) (GLuint id, GLsizei len, const GLubyte *name, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramNamedParameterdvNV) (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params);
#endif

#ifndef GL_NV_fragment_program2
#define GL_NV_fragment_program2 1
enum : GLenum
{
    GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV                     = 0x88F4,
    GL_MAX_PROGRAM_CALL_DEPTH_NV                            = 0x88F5,
    GL_MAX_PROGRAM_IF_DEPTH_NV                              = 0x88F6,
    GL_MAX_PROGRAM_LOOP_DEPTH_NV                            = 0x88F7,
    GL_MAX_PROGRAM_LOOP_COUNT_NV                            = 0x88F8,
};
#endif

#ifndef GL_NV_fragment_program4
#define GL_NV_fragment_program4 1
#endif

#ifndef GL_NV_fragment_program_option
#define GL_NV_fragment_program_option 1
#endif

#ifndef GL_NV_fragment_shader_barycentric
#define GL_NV_fragment_shader_barycentric 1
#endif

#ifndef GL_NV_fragment_shader_interlock
#define GL_NV_fragment_shader_interlock 1
#endif

#ifndef GL_NV_framebuffer_mixed_samples
#define GL_NV_framebuffer_mixed_samples 1
enum : GLenum
{
    GL_COVERAGE_MODULATION_TABLE_NV                         = 0x9331,
    GL_COLOR_SAMPLES_NV                                     = 0x8E20,
    GL_DEPTH_SAMPLES_NV                                     = 0x932D,
    GL_STENCIL_SAMPLES_NV                                   = 0x932E,
    GL_MIXED_DEPTH_SAMPLES_SUPPORTED_NV                     = 0x932F,
    GL_MIXED_STENCIL_SAMPLES_SUPPORTED_NV                   = 0x9330,
    GL_COVERAGE_MODULATION_NV                               = 0x9332,
    GL_COVERAGE_MODULATION_TABLE_SIZE_NV                    = 0x9333,
};
extern void         (KHRONOS_APIENTRY* const& glCoverageModulationTableNV) (GLsizei n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glGetCoverageModulationTableNV) (GLsizei bufSize, GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glCoverageModulationNV) (GLenum components);
#endif

#ifndef GL_NV_framebuffer_multisample_coverage
#define GL_NV_framebuffer_multisample_coverage 1
enum : GLenum
{
    GL_RENDERBUFFER_COVERAGE_SAMPLES_NV                     = 0x8CAB,
    GL_RENDERBUFFER_COLOR_SAMPLES_NV                        = 0x8E10,
    GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV                    = 0x8E11,
    GL_MULTISAMPLE_COVERAGE_MODES_NV                        = 0x8E12,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleCoverageNV) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
#endif

#ifndef GL_NV_geometry_program4
#define GL_NV_geometry_program4 1
enum : GLenum
{
    GL_GEOMETRY_PROGRAM_NV                                  = 0x8C26,
    GL_MAX_PROGRAM_OUTPUT_VERTICES_NV                       = 0x8C27,
    GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV               = 0x8C28,
};
extern void         (KHRONOS_APIENTRY* const& glProgramVertexLimitNV) (GLenum target, GLint limit);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureEXT) (GLenum target, GLenum attachment, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureFaceEXT) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
#endif

#ifndef GL_NV_geometry_shader4
#define GL_NV_geometry_shader4 1
#endif

#ifndef GL_NV_geometry_shader_passthrough
#define GL_NV_geometry_shader_passthrough 1
#endif

#ifndef GL_NV_gpu_program4
#define GL_NV_gpu_program4 1
enum : GLenum
{
    GL_MIN_PROGRAM_TEXEL_OFFSET_NV                          = 0x8904,
    GL_MAX_PROGRAM_TEXEL_OFFSET_NV                          = 0x8905,
    GL_PROGRAM_ATTRIB_COMPONENTS_NV                         = 0x8906,
    GL_PROGRAM_RESULT_COMPONENTS_NV                         = 0x8907,
    GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV                     = 0x8908,
    GL_MAX_PROGRAM_RESULT_COMPONENTS_NV                     = 0x8909,
    GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV                       = 0x8DA5,
    GL_MAX_PROGRAM_GENERIC_RESULTS_NV                       = 0x8DA6,
};
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameterI4iNV) (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameterI4ivNV) (GLenum target, GLuint index, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParametersI4ivNV) (GLenum target, GLuint index, GLsizei count, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameterI4uiNV) (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParameterI4uivNV) (GLenum target, GLuint index, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramLocalParametersI4uivNV) (GLenum target, GLuint index, GLsizei count, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameterI4iNV) (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameterI4ivNV) (GLenum target, GLuint index, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParametersI4ivNV) (GLenum target, GLuint index, GLsizei count, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameterI4uiNV) (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParameterI4uivNV) (GLenum target, GLuint index, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramEnvParametersI4uivNV) (GLenum target, GLuint index, GLsizei count, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramLocalParameterIivNV) (GLenum target, GLuint index, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramLocalParameterIuivNV) (GLenum target, GLuint index, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramEnvParameterIivNV) (GLenum target, GLuint index, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramEnvParameterIuivNV) (GLenum target, GLuint index, GLuint *params);
#endif

#ifndef GL_NV_gpu_program5
#define GL_NV_gpu_program5 1
enum : GLenum
{
    GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV                  = 0x8E5A,
    GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV                 = 0x8E5B,
    GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV                 = 0x8E5C,
    GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV        = 0x8E5D,
    GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV                 = 0x8E5E,
    GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV                 = 0x8E5F,
    GL_MAX_PROGRAM_SUBROUTINE_PARAMETERS_NV                 = 0x8F44,
    GL_MAX_PROGRAM_SUBROUTINE_NUM_NV                        = 0x8F45,
};
extern void         (KHRONOS_APIENTRY* const& glProgramSubroutineParametersuivNV) (GLenum target, GLsizei count, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramSubroutineParameteruivNV) (GLenum target, GLuint index, GLuint *param);
#endif

#ifndef GL_NV_gpu_program5_mem_extended
#define GL_NV_gpu_program5_mem_extended 1
#endif

#ifndef GL_NV_gpu_shader5
#define GL_NV_gpu_shader5 1
#endif

#ifndef GL_NV_half_float
#define GL_NV_half_float 1
enum : GLenum
{
    GL_HALF_FLOAT_NV                                        = 0x140B,
};
extern void         (KHRONOS_APIENTRY* const& glVertex2hNV) (GLhalfNV x, GLhalfNV y);
extern void         (KHRONOS_APIENTRY* const& glVertex2hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertex3hNV) (GLhalfNV x, GLhalfNV y, GLhalfNV z);
extern void         (KHRONOS_APIENTRY* const& glVertex3hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertex4hNV) (GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w);
extern void         (KHRONOS_APIENTRY* const& glVertex4hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glNormal3hNV) (GLhalfNV nx, GLhalfNV ny, GLhalfNV nz);
extern void         (KHRONOS_APIENTRY* const& glNormal3hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glColor3hNV) (GLhalfNV red, GLhalfNV green, GLhalfNV blue);
extern void         (KHRONOS_APIENTRY* const& glColor3hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glColor4hNV) (GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha);
extern void         (KHRONOS_APIENTRY* const& glColor4hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1hNV) (GLhalfNV s);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2hNV) (GLhalfNV s, GLhalfNV t);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3hNV) (GLhalfNV s, GLhalfNV t, GLhalfNV r);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4hNV) (GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1hNV) (GLenum target, GLhalfNV s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1hvNV) (GLenum target, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2hNV) (GLenum target, GLhalfNV s, GLhalfNV t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2hvNV) (GLenum target, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3hNV) (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3hvNV) (GLenum target, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4hNV) (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4hvNV) (GLenum target, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glFogCoordhNV) (GLhalfNV fog);
extern void         (KHRONOS_APIENTRY* const& glFogCoordhvNV) (const GLhalfNV *fog);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3hNV) (GLhalfNV red, GLhalfNV green, GLhalfNV blue);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColor3hvNV) (const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexWeighthNV) (GLhalfNV weight);
extern void         (KHRONOS_APIENTRY* const& glVertexWeighthvNV) (const GLhalfNV *weight);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1hNV) (GLuint index, GLhalfNV x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1hvNV) (GLuint index, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2hNV) (GLuint index, GLhalfNV x, GLhalfNV y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2hvNV) (GLuint index, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3hNV) (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3hvNV) (GLuint index, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4hNV) (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4hvNV) (GLuint index, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs1hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs2hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs3hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs4hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
#endif

#ifndef GL_NV_internalformat_sample_query
#define GL_NV_internalformat_sample_query 1
enum : GLenum
{
    GL_MULTISAMPLES_NV                                      = 0x9371,
    GL_SUPERSAMPLE_SCALE_X_NV                               = 0x9372,
    GL_SUPERSAMPLE_SCALE_Y_NV                               = 0x9373,
    GL_CONFORMANT_NV                                        = 0x9374,
};
extern void         (KHRONOS_APIENTRY* const& glGetInternalformatSampleivNV) (GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint *params);
#endif

#ifndef GL_NV_light_max_exponent
#define GL_NV_light_max_exponent 1
enum : GLenum
{
    GL_MAX_SHININESS_NV                                     = 0x8504,
    GL_MAX_SPOT_EXPONENT_NV                                 = 0x8505,
};
#endif

#ifndef GL_NV_gpu_multicast
#define GL_NV_gpu_multicast 1
enum : GLenum
{
    GL_PER_GPU_STORAGE_BIT_NV                               = 0x0800,
    GL_MULTICAST_GPUS_NV                                    = 0x92BA,
    GL_RENDER_GPU_MASK_NV                                   = 0x9558,
    GL_PER_GPU_STORAGE_NV                                   = 0x9548,
    GL_MULTICAST_PROGRAMMABLE_SAMPLE_LOCATION_NV            = 0x9549,
};
extern void         (KHRONOS_APIENTRY* const& glRenderGpuMaskNV) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glMulticastBufferSubDataNV) (GLbitfield gpuMask, GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
extern void         (KHRONOS_APIENTRY* const& glMulticastCopyBufferSubDataNV) (GLuint readGpu, GLbitfield writeGpuMask, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glMulticastCopyImageSubDataNV) (GLuint srcGpu, GLbitfield dstGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
extern void         (KHRONOS_APIENTRY* const& glMulticastBlitFramebufferNV) (GLuint srcGpu, GLuint dstGpu, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern void         (KHRONOS_APIENTRY* const& glMulticastFramebufferSampleLocationsfvNV) (GLuint gpu, GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMulticastBarrierNV) ();
extern void         (KHRONOS_APIENTRY* const& glMulticastWaitSyncNV) (GLuint signalGpu, GLbitfield waitGpuMask);
extern void         (KHRONOS_APIENTRY* const& glMulticastGetQueryObjectivNV) (GLuint gpu, GLuint id, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glMulticastGetQueryObjectuivNV) (GLuint gpu, GLuint id, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glMulticastGetQueryObjecti64vNV) (GLuint gpu, GLuint id, GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glMulticastGetQueryObjectui64vNV) (GLuint gpu, GLuint id, GLenum pname, GLuint64 *params);
#endif

#ifndef GL_NVX_gpu_multicast2
#define GL_NVX_gpu_multicast2 1
enum : GLenum
{
    GL_UPLOAD_GPU_MASK_NVX                                  = 0x954A,
};
extern void         (KHRONOS_APIENTRY* const& glUploadGpuMaskNVX) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glMulticastViewportArrayvNVX) (GLuint gpu, GLuint first, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glMulticastViewportPositionWScaleNVX) (GLuint gpu, GLuint index, GLfloat xcoeff, GLfloat ycoeff);
extern void         (KHRONOS_APIENTRY* const& glMulticastScissorArrayvNVX) (GLuint gpu, GLuint first, GLsizei count, const GLint *v);
extern GLuint       (KHRONOS_APIENTRY* const& glAsyncCopyBufferSubDataNVX) (GLsizei waitSemaphoreCount, const GLuint *waitSemaphoreArray, const GLuint64 *fenceValueArray, GLuint readGpu, GLbitfield writeGpuMask, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size, GLsizei signalSemaphoreCount, const GLuint *signalSemaphoreArray, const GLuint64 *signalValueArray);
extern GLuint       (KHRONOS_APIENTRY* const& glAsyncCopyImageSubDataNVX) (GLsizei waitSemaphoreCount, const GLuint *waitSemaphoreArray, const GLuint64 *waitValueArray, GLuint srcGpu, GLbitfield dstGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth, GLsizei signalSemaphoreCount, const GLuint *signalSemaphoreArray, const GLuint64 *signalValueArray);
#endif

#ifndef GL_NVX_progress_fence
#define GL_NVX_progress_fence 1
extern GLuint       (KHRONOS_APIENTRY* const& glCreateProgressFenceNVX) ();
extern void         (KHRONOS_APIENTRY* const& glSignalSemaphoreui64NVX) (GLuint signalGpu, GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray);
extern void         (KHRONOS_APIENTRY* const& glWaitSemaphoreui64NVX) (GLuint waitGpu, GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray);
extern void         (KHRONOS_APIENTRY* const& glClientWaitSemaphoreui64NVX) (GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray);
#endif

#ifndef GL_NV_memory_attachment
#define GL_NV_memory_attachment 1
enum : GLenum
{
    GL_ATTACHED_MEMORY_OBJECT_NV                            = 0x95A4,
    GL_ATTACHED_MEMORY_OFFSET_NV                            = 0x95A5,
    GL_MEMORY_ATTACHABLE_ALIGNMENT_NV                       = 0x95A6,
    GL_MEMORY_ATTACHABLE_SIZE_NV                            = 0x95A7,
    GL_MEMORY_ATTACHABLE_NV                                 = 0x95A8,
    GL_DETACHED_MEMORY_INCARNATION_NV                       = 0x95A9,
    GL_DETACHED_TEXTURES_NV                                 = 0x95AA,
    GL_DETACHED_BUFFERS_NV                                  = 0x95AB,
    GL_MAX_DETACHED_TEXTURES_NV                             = 0x95AC,
    GL_MAX_DETACHED_BUFFERS_NV                              = 0x95AD,
};
extern void         (KHRONOS_APIENTRY* const& glGetMemoryObjectDetachedResourcesuivNV) (GLuint memory, GLenum pname, GLint first, GLsizei count, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glResetMemoryObjectParameterNV) (GLuint memory, GLenum pname);
extern void         (KHRONOS_APIENTRY* const& glTexAttachMemoryNV) (GLenum target, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glBufferAttachMemoryNV) (GLenum target, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glTextureAttachMemoryNV) (GLuint texture, GLuint memory, GLuint64 offset);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferAttachMemoryNV) (GLuint buffer, GLuint memory, GLuint64 offset);
#endif

#ifndef GL_NV_memory_object_sparse
#define GL_NV_memory_object_sparse 1
extern void         (KHRONOS_APIENTRY* const& glBufferPageCommitmentMemNV) (GLenum target, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit);
extern void         (KHRONOS_APIENTRY* const& glTexPageCommitmentMemNV) (GLenum target, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferPageCommitmentMemNV) (GLuint buffer, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit);
extern void         (KHRONOS_APIENTRY* const& glTexturePageCommitmentMemNV) (GLuint texture, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit);
#endif

#ifndef GL_NV_mesh_shader
#define GL_NV_mesh_shader 1
enum : GLenum
{
    GL_MESH_SHADER_NV                                       = 0x9559,
    GL_TASK_SHADER_NV                                       = 0x955A,
    GL_MAX_MESH_UNIFORM_BLOCKS_NV                           = 0x8E60,
    GL_MAX_MESH_TEXTURE_IMAGE_UNITS_NV                      = 0x8E61,
    GL_MAX_MESH_IMAGE_UNIFORMS_NV                           = 0x8E62,
    GL_MAX_MESH_UNIFORM_COMPONENTS_NV                       = 0x8E63,
    GL_MAX_MESH_ATOMIC_COUNTER_BUFFERS_NV                   = 0x8E64,
    GL_MAX_MESH_ATOMIC_COUNTERS_NV                          = 0x8E65,
    GL_MAX_MESH_SHADER_STORAGE_BLOCKS_NV                    = 0x8E66,
    GL_MAX_COMBINED_MESH_UNIFORM_COMPONENTS_NV              = 0x8E67,
    GL_MAX_TASK_UNIFORM_BLOCKS_NV                           = 0x8E68,
    GL_MAX_TASK_TEXTURE_IMAGE_UNITS_NV                      = 0x8E69,
    GL_MAX_TASK_IMAGE_UNIFORMS_NV                           = 0x8E6A,
    GL_MAX_TASK_UNIFORM_COMPONENTS_NV                       = 0x8E6B,
    GL_MAX_TASK_ATOMIC_COUNTER_BUFFERS_NV                   = 0x8E6C,
    GL_MAX_TASK_ATOMIC_COUNTERS_NV                          = 0x8E6D,
    GL_MAX_TASK_SHADER_STORAGE_BLOCKS_NV                    = 0x8E6E,
    GL_MAX_COMBINED_TASK_UNIFORM_COMPONENTS_NV              = 0x8E6F,
    GL_MAX_MESH_WORK_GROUP_INVOCATIONS_NV                   = 0x95A2,
    GL_MAX_TASK_WORK_GROUP_INVOCATIONS_NV                   = 0x95A3,
    GL_MAX_MESH_TOTAL_MEMORY_SIZE_NV                        = 0x9536,
    GL_MAX_TASK_TOTAL_MEMORY_SIZE_NV                        = 0x9537,
    GL_MAX_MESH_OUTPUT_VERTICES_NV                          = 0x9538,
    GL_MAX_MESH_OUTPUT_PRIMITIVES_NV                        = 0x9539,
    GL_MAX_TASK_OUTPUT_COUNT_NV                             = 0x953A,
    GL_MAX_DRAW_MESH_TASKS_COUNT_NV                         = 0x953D,
    GL_MAX_MESH_VIEWS_NV                                    = 0x9557,
    GL_MESH_OUTPUT_PER_VERTEX_GRANULARITY_NV                = 0x92DF,
    GL_MESH_OUTPUT_PER_PRIMITIVE_GRANULARITY_NV             = 0x9543,
    GL_MAX_MESH_WORK_GROUP_SIZE_NV                          = 0x953B,
    GL_MAX_TASK_WORK_GROUP_SIZE_NV                          = 0x953C,
    GL_MESH_WORK_GROUP_SIZE_NV                              = 0x953E,
    GL_TASK_WORK_GROUP_SIZE_NV                              = 0x953F,
    GL_MESH_VERTICES_OUT_NV                                 = 0x9579,
    GL_MESH_PRIMITIVES_OUT_NV                               = 0x957A,
    GL_MESH_OUTPUT_TYPE_NV                                  = 0x957B,
    GL_UNIFORM_BLOCK_REFERENCED_BY_MESH_SHADER_NV           = 0x959C,
    GL_UNIFORM_BLOCK_REFERENCED_BY_TASK_SHADER_NV           = 0x959D,
    GL_REFERENCED_BY_MESH_SHADER_NV                         = 0x95A0,
    GL_REFERENCED_BY_TASK_SHADER_NV                         = 0x95A1,
    GL_MESH_SHADER_BIT_NV                                   = 0x00000040,
    GL_TASK_SHADER_BIT_NV                                   = 0x00000080,
    GL_MESH_SUBROUTINE_NV                                   = 0x957C,
    GL_TASK_SUBROUTINE_NV                                   = 0x957D,
    GL_MESH_SUBROUTINE_UNIFORM_NV                           = 0x957E,
    GL_TASK_SUBROUTINE_UNIFORM_NV                           = 0x957F,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_MESH_SHADER_NV   = 0x959E,
    GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TASK_SHADER_NV   = 0x959F,
};
extern void         (KHRONOS_APIENTRY* const& glDrawMeshTasksNV) (GLuint first, GLuint count);
extern void         (KHRONOS_APIENTRY* const& glDrawMeshTasksIndirectNV) (GLintptr indirect);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawMeshTasksIndirectNV) (GLintptr indirect, GLsizei drawcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawMeshTasksIndirectCountNV) (GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
#endif

#ifndef GL_NV_multisample_coverage
#define GL_NV_multisample_coverage 1
#endif

#ifndef GL_NV_multisample_filter_hint
#define GL_NV_multisample_filter_hint 1
enum : GLenum
{
    GL_MULTISAMPLE_FILTER_HINT_NV                           = 0x8534,
};
#endif

#ifndef GL_NV_occlusion_query
#define GL_NV_occlusion_query 1
enum : GLenum
{
    GL_PIXEL_COUNTER_BITS_NV                                = 0x8864,
    GL_CURRENT_OCCLUSION_QUERY_ID_NV                        = 0x8865,
    GL_PIXEL_COUNT_NV                                       = 0x8866,
    GL_PIXEL_COUNT_AVAILABLE_NV                             = 0x8867,
};
extern void         (KHRONOS_APIENTRY* const& glGenOcclusionQueriesNV) (GLsizei n, GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glDeleteOcclusionQueriesNV) (GLsizei n, const GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsOcclusionQueryNV) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glBeginOcclusionQueryNV) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glEndOcclusionQueryNV) ();
extern void         (KHRONOS_APIENTRY* const& glGetOcclusionQueryivNV) (GLuint id, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetOcclusionQueryuivNV) (GLuint id, GLenum pname, GLuint *params);
#endif

#ifndef GL_NV_packed_depth_stencil
#define GL_NV_packed_depth_stencil 1
enum : GLenum
{
    GL_DEPTH_STENCIL_NV                                     = 0x84F9,
    GL_UNSIGNED_INT_24_8_NV                                 = 0x84FA,
};
#endif

#ifndef GL_NV_parameter_buffer_object
#define GL_NV_parameter_buffer_object 1
enum : GLenum
{
    GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV             = 0x8DA0,
    GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV                 = 0x8DA1,
    GL_VERTEX_PROGRAM_PARAMETER_BUFFER_NV                   = 0x8DA2,
    GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV                 = 0x8DA3,
    GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV                 = 0x8DA4,
};
extern void         (KHRONOS_APIENTRY* const& glProgramBufferParametersfvNV) (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glProgramBufferParametersIivNV) (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramBufferParametersIuivNV) (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLuint *params);
#endif

#ifndef GL_NV_parameter_buffer_object2
#define GL_NV_parameter_buffer_object2 1
#endif

#ifndef GL_NV_path_rendering
#define GL_NV_path_rendering 1
enum : GLenum
{
    GL_PATH_FORMAT_SVG_NV                                   = 0x9070,
    GL_PATH_FORMAT_PS_NV                                    = 0x9071,
    GL_STANDARD_FONT_NAME_NV                                = 0x9072,
    GL_SYSTEM_FONT_NAME_NV                                  = 0x9073,
    GL_FILE_NAME_NV                                         = 0x9074,
    GL_PATH_STROKE_WIDTH_NV                                 = 0x9075,
    GL_PATH_END_CAPS_NV                                     = 0x9076,
    GL_PATH_INITIAL_END_CAP_NV                              = 0x9077,
    GL_PATH_TERMINAL_END_CAP_NV                             = 0x9078,
    GL_PATH_JOIN_STYLE_NV                                   = 0x9079,
    GL_PATH_MITER_LIMIT_NV                                  = 0x907A,
    GL_PATH_DASH_CAPS_NV                                    = 0x907B,
    GL_PATH_INITIAL_DASH_CAP_NV                             = 0x907C,
    GL_PATH_TERMINAL_DASH_CAP_NV                            = 0x907D,
    GL_PATH_DASH_OFFSET_NV                                  = 0x907E,
    GL_PATH_CLIENT_LENGTH_NV                                = 0x907F,
    GL_PATH_FILL_MODE_NV                                    = 0x9080,
    GL_PATH_FILL_MASK_NV                                    = 0x9081,
    GL_PATH_FILL_COVER_MODE_NV                              = 0x9082,
    GL_PATH_STROKE_COVER_MODE_NV                            = 0x9083,
    GL_PATH_STROKE_MASK_NV                                  = 0x9084,
    GL_COUNT_UP_NV                                          = 0x9088,
    GL_COUNT_DOWN_NV                                        = 0x9089,
    GL_PATH_OBJECT_BOUNDING_BOX_NV                          = 0x908A,
    GL_CONVEX_HULL_NV                                       = 0x908B,
    GL_BOUNDING_BOX_NV                                      = 0x908D,
    GL_TRANSLATE_X_NV                                       = 0x908E,
    GL_TRANSLATE_Y_NV                                       = 0x908F,
    GL_TRANSLATE_2D_NV                                      = 0x9090,
    GL_TRANSLATE_3D_NV                                      = 0x9091,
    GL_AFFINE_2D_NV                                         = 0x9092,
    GL_AFFINE_3D_NV                                         = 0x9094,
    GL_TRANSPOSE_AFFINE_2D_NV                               = 0x9096,
    GL_TRANSPOSE_AFFINE_3D_NV                               = 0x9098,
    GL_UTF8_NV                                              = 0x909A,
    GL_UTF16_NV                                             = 0x909B,
    GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV                    = 0x909C,
    GL_PATH_COMMAND_COUNT_NV                                = 0x909D,
    GL_PATH_COORD_COUNT_NV                                  = 0x909E,
    GL_PATH_DASH_ARRAY_COUNT_NV                             = 0x909F,
    GL_PATH_COMPUTED_LENGTH_NV                              = 0x90A0,
    GL_PATH_FILL_BOUNDING_BOX_NV                            = 0x90A1,
    GL_PATH_STROKE_BOUNDING_BOX_NV                          = 0x90A2,
    GL_SQUARE_NV                                            = 0x90A3,
    GL_ROUND_NV                                             = 0x90A4,
    GL_TRIANGULAR_NV                                        = 0x90A5,
    GL_BEVEL_NV                                             = 0x90A6,
    GL_MITER_REVERT_NV                                      = 0x90A7,
    GL_MITER_TRUNCATE_NV                                    = 0x90A8,
    GL_SKIP_MISSING_GLYPH_NV                                = 0x90A9,
    GL_USE_MISSING_GLYPH_NV                                 = 0x90AA,
    GL_PATH_ERROR_POSITION_NV                               = 0x90AB,
    GL_ACCUM_ADJACENT_PAIRS_NV                              = 0x90AD,
    GL_ADJACENT_PAIRS_NV                                    = 0x90AE,
    GL_FIRST_TO_REST_NV                                     = 0x90AF,
    GL_PATH_GEN_MODE_NV                                     = 0x90B0,
    GL_PATH_GEN_COEFF_NV                                    = 0x90B1,
    GL_PATH_GEN_COMPONENTS_NV                               = 0x90B3,
    GL_PATH_STENCIL_FUNC_NV                                 = 0x90B7,
    GL_PATH_STENCIL_REF_NV                                  = 0x90B8,
    GL_PATH_STENCIL_VALUE_MASK_NV                           = 0x90B9,
    GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV                  = 0x90BD,
    GL_PATH_STENCIL_DEPTH_OFFSET_UNITS_NV                   = 0x90BE,
    GL_PATH_COVER_DEPTH_FUNC_NV                             = 0x90BF,
    GL_PATH_DASH_OFFSET_RESET_NV                            = 0x90B4,
    GL_MOVE_TO_RESETS_NV                                    = 0x90B5,
    GL_MOVE_TO_CONTINUES_NV                                 = 0x90B6,
    GL_CLOSE_PATH_NV                                        = 0x00,
    GL_MOVE_TO_NV                                           = 0x02,
    GL_RELATIVE_MOVE_TO_NV                                  = 0x03,
    GL_LINE_TO_NV                                           = 0x04,
    GL_RELATIVE_LINE_TO_NV                                  = 0x05,
    GL_HORIZONTAL_LINE_TO_NV                                = 0x06,
    GL_RELATIVE_HORIZONTAL_LINE_TO_NV                       = 0x07,
    GL_VERTICAL_LINE_TO_NV                                  = 0x08,
    GL_RELATIVE_VERTICAL_LINE_TO_NV                         = 0x09,
    GL_QUADRATIC_CURVE_TO_NV                                = 0x0A,
    GL_RELATIVE_QUADRATIC_CURVE_TO_NV                       = 0x0B,
    GL_CUBIC_CURVE_TO_NV                                    = 0x0C,
    GL_RELATIVE_CUBIC_CURVE_TO_NV                           = 0x0D,
    GL_SMOOTH_QUADRATIC_CURVE_TO_NV                         = 0x0E,
    GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV                = 0x0F,
    GL_SMOOTH_CUBIC_CURVE_TO_NV                             = 0x10,
    GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV                    = 0x11,
    GL_SMALL_CCW_ARC_TO_NV                                  = 0x12,
    GL_RELATIVE_SMALL_CCW_ARC_TO_NV                         = 0x13,
    GL_SMALL_CW_ARC_TO_NV                                   = 0x14,
    GL_RELATIVE_SMALL_CW_ARC_TO_NV                          = 0x15,
    GL_LARGE_CCW_ARC_TO_NV                                  = 0x16,
    GL_RELATIVE_LARGE_CCW_ARC_TO_NV                         = 0x17,
    GL_LARGE_CW_ARC_TO_NV                                   = 0x18,
    GL_RELATIVE_LARGE_CW_ARC_TO_NV                          = 0x19,
    GL_RESTART_PATH_NV                                      = 0xF0,
    GL_DUP_FIRST_CUBIC_CURVE_TO_NV                          = 0xF2,
    GL_DUP_LAST_CUBIC_CURVE_TO_NV                           = 0xF4,
    GL_RECT_NV                                              = 0xF6,
    GL_CIRCULAR_CCW_ARC_TO_NV                               = 0xF8,
    GL_CIRCULAR_CW_ARC_TO_NV                                = 0xFA,
    GL_CIRCULAR_TANGENT_ARC_TO_NV                           = 0xFC,
    GL_ARC_TO_NV                                            = 0xFE,
    GL_RELATIVE_ARC_TO_NV                                   = 0xFF,
    GL_BOLD_BIT_NV                                          = 0x01,
    GL_ITALIC_BIT_NV                                        = 0x02,
    GL_GLYPH_WIDTH_BIT_NV                                   = 0x01,
    GL_GLYPH_HEIGHT_BIT_NV                                  = 0x02,
    GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV                    = 0x04,
    GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV                    = 0x08,
    GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV              = 0x10,
    GL_GLYPH_VERTICAL_BEARING_X_BIT_NV                      = 0x20,
    GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV                      = 0x40,
    GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV                = 0x80,
    GL_GLYPH_HAS_KERNING_BIT_NV                             = 0x100,
    GL_FONT_X_MIN_BOUNDS_BIT_NV                             = 0x00010000,
    GL_FONT_Y_MIN_BOUNDS_BIT_NV                             = 0x00020000,
    GL_FONT_X_MAX_BOUNDS_BIT_NV                             = 0x00040000,
    GL_FONT_Y_MAX_BOUNDS_BIT_NV                             = 0x00080000,
    GL_FONT_UNITS_PER_EM_BIT_NV                             = 0x00100000,
    GL_FONT_ASCENDER_BIT_NV                                 = 0x00200000,
    GL_FONT_DESCENDER_BIT_NV                                = 0x00400000,
    GL_FONT_HEIGHT_BIT_NV                                   = 0x00800000,
    GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV                        = 0x01000000,
    GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV                       = 0x02000000,
    GL_FONT_UNDERLINE_POSITION_BIT_NV                       = 0x04000000,
    GL_FONT_UNDERLINE_THICKNESS_BIT_NV                      = 0x08000000,
    GL_FONT_HAS_KERNING_BIT_NV                              = 0x10000000,
    GL_ROUNDED_RECT_NV                                      = 0xE8,
    GL_RELATIVE_ROUNDED_RECT_NV                             = 0xE9,
    GL_ROUNDED_RECT2_NV                                     = 0xEA,
    GL_RELATIVE_ROUNDED_RECT2_NV                            = 0xEB,
    GL_ROUNDED_RECT4_NV                                     = 0xEC,
    GL_RELATIVE_ROUNDED_RECT4_NV                            = 0xED,
    GL_ROUNDED_RECT8_NV                                     = 0xEE,
    GL_RELATIVE_ROUNDED_RECT8_NV                            = 0xEF,
    GL_RELATIVE_RECT_NV                                     = 0xF7,
    GL_FONT_GLYPHS_AVAILABLE_NV                             = 0x9368,
    GL_FONT_TARGET_UNAVAILABLE_NV                           = 0x9369,
    GL_FONT_UNAVAILABLE_NV                                  = 0x936A,
    GL_FONT_UNINTELLIGIBLE_NV                               = 0x936B,
    GL_CONIC_CURVE_TO_NV                                    = 0x1A,
    GL_RELATIVE_CONIC_CURVE_TO_NV                           = 0x1B,
    GL_FONT_NUM_GLYPH_INDICES_BIT_NV                        = 0x20000000,
    GL_STANDARD_FONT_FORMAT_NV                              = 0x936C,
    GL_2_BYTES_NV                                           = 0x1407,
    GL_3_BYTES_NV                                           = 0x1408,
    GL_4_BYTES_NV                                           = 0x1409,
    GL_EYE_LINEAR_NV                                        = 0x2400,
    GL_OBJECT_LINEAR_NV                                     = 0x2401,
    GL_CONSTANT_NV                                          = 0x8576,
    GL_PATH_FOG_GEN_MODE_NV                                 = 0x90AC,
    GL_PRIMARY_COLOR_NV                                     = 0x852C,
    GL_SECONDARY_COLOR_NV                                   = 0x852D,
    GL_PATH_GEN_COLOR_FORMAT_NV                             = 0x90B2,
    GL_PATH_PROJECTION_NV                                   = 0x1701,
    GL_PATH_MODELVIEW_NV                                    = 0x1700,
    GL_PATH_MODELVIEW_STACK_DEPTH_NV                        = 0x0BA3,
    GL_PATH_MODELVIEW_MATRIX_NV                             = 0x0BA6,
    GL_PATH_MAX_MODELVIEW_STACK_DEPTH_NV                    = 0x0D36,
    GL_PATH_TRANSPOSE_MODELVIEW_MATRIX_NV                   = 0x84E3,
    GL_PATH_PROJECTION_STACK_DEPTH_NV                       = 0x0BA4,
    GL_PATH_PROJECTION_MATRIX_NV                            = 0x0BA7,
    GL_PATH_MAX_PROJECTION_STACK_DEPTH_NV                   = 0x0D38,
    GL_PATH_TRANSPOSE_PROJECTION_MATRIX_NV                  = 0x84E4,
    GL_FRAGMENT_INPUT_NV                                    = 0x936D,
};
extern GLuint       (KHRONOS_APIENTRY* const& glGenPathsNV) (GLsizei range);
extern void         (KHRONOS_APIENTRY* const& glDeletePathsNV) (GLuint path, GLsizei range);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsPathNV) (GLuint path);
extern void         (KHRONOS_APIENTRY* const& glPathCommandsNV) (GLuint path, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const void *coords);
extern void         (KHRONOS_APIENTRY* const& glPathCoordsNV) (GLuint path, GLsizei numCoords, GLenum coordType, const void *coords);
extern void         (KHRONOS_APIENTRY* const& glPathSubCommandsNV) (GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const void *coords);
extern void         (KHRONOS_APIENTRY* const& glPathSubCoordsNV) (GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void *coords);
extern void         (KHRONOS_APIENTRY* const& glPathStringNV) (GLuint path, GLenum format, GLsizei length, const void *pathString);
extern void         (KHRONOS_APIENTRY* const& glPathGlyphsNV) (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void *charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
extern void         (KHRONOS_APIENTRY* const& glPathGlyphRangeNV) (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
extern void         (KHRONOS_APIENTRY* const& glWeightPathsNV) (GLuint resultPath, GLsizei numPaths, const GLuint *paths, const GLfloat *weights);
extern void         (KHRONOS_APIENTRY* const& glCopyPathNV) (GLuint resultPath, GLuint srcPath);
extern void         (KHRONOS_APIENTRY* const& glInterpolatePathsNV) (GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight);
extern void         (KHRONOS_APIENTRY* const& glTransformPathNV) (GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat *transformValues);
extern void         (KHRONOS_APIENTRY* const& glPathParameterivNV) (GLuint path, GLenum pname, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glPathParameteriNV) (GLuint path, GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glPathParameterfvNV) (GLuint path, GLenum pname, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glPathParameterfNV) (GLuint path, GLenum pname, GLfloat value);
extern void         (KHRONOS_APIENTRY* const& glPathDashArrayNV) (GLuint path, GLsizei dashCount, const GLfloat *dashArray);
extern void         (KHRONOS_APIENTRY* const& glPathStencilFuncNV) (GLenum func, GLint ref, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glPathStencilDepthOffsetNV) (GLfloat factor, GLfloat units);
extern void         (KHRONOS_APIENTRY* const& glStencilFillPathNV) (GLuint path, GLenum fillMode, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilStrokePathNV) (GLuint path, GLint reference, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilFillPathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat *transformValues);
extern void         (KHRONOS_APIENTRY* const& glStencilStrokePathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat *transformValues);
extern void         (KHRONOS_APIENTRY* const& glPathCoverDepthFuncNV) (GLenum func);
extern void         (KHRONOS_APIENTRY* const& glCoverFillPathNV) (GLuint path, GLenum coverMode);
extern void         (KHRONOS_APIENTRY* const& glCoverStrokePathNV) (GLuint path, GLenum coverMode);
extern void         (KHRONOS_APIENTRY* const& glCoverFillPathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
extern void         (KHRONOS_APIENTRY* const& glCoverStrokePathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
extern void         (KHRONOS_APIENTRY* const& glGetPathParameterivNV) (GLuint path, GLenum pname, GLint *value);
extern void         (KHRONOS_APIENTRY* const& glGetPathParameterfvNV) (GLuint path, GLenum pname, GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glGetPathCommandsNV) (GLuint path, GLubyte *commands);
extern void         (KHRONOS_APIENTRY* const& glGetPathCoordsNV) (GLuint path, GLfloat *coords);
extern void         (KHRONOS_APIENTRY* const& glGetPathDashArrayNV) (GLuint path, GLfloat *dashArray);
extern void         (KHRONOS_APIENTRY* const& glGetPathMetricsNV) (GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLsizei stride, GLfloat *metrics);
extern void         (KHRONOS_APIENTRY* const& glGetPathMetricRangeNV) (GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat *metrics);
extern void         (KHRONOS_APIENTRY* const& glGetPathSpacingNV) (GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat *returnedSpacing);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsPointInFillPathNV) (GLuint path, GLuint mask, GLfloat x, GLfloat y);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsPointInStrokePathNV) (GLuint path, GLfloat x, GLfloat y);
extern GLfloat      (KHRONOS_APIENTRY* const& glGetPathLengthNV) (GLuint path, GLsizei startSegment, GLsizei numSegments);
extern GLboolean    (KHRONOS_APIENTRY* const& glPointAlongPathNV) (GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat *x, GLfloat *y, GLfloat *tangentX, GLfloat *tangentY);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoad3x2fNV) (GLenum matrixMode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoad3x3fNV) (GLenum matrixMode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadTranspose3x3fNV) (GLenum matrixMode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMult3x2fNV) (GLenum matrixMode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMult3x3fNV) (GLenum matrixMode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultTranspose3x3fNV) (GLenum matrixMode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glStencilThenCoverFillPathNV) (GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
extern void         (KHRONOS_APIENTRY* const& glStencilThenCoverStrokePathNV) (GLuint path, GLint reference, GLuint mask, GLenum coverMode);
extern void         (KHRONOS_APIENTRY* const& glStencilThenCoverFillPathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
extern void         (KHRONOS_APIENTRY* const& glStencilThenCoverStrokePathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
extern GLenum       (KHRONOS_APIENTRY* const& glPathGlyphIndexRangeNV) (GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint *baseAndCount);
extern GLenum       (KHRONOS_APIENTRY* const& glPathGlyphIndexArrayNV) (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
extern GLenum       (KHRONOS_APIENTRY* const& glPathMemoryGlyphIndexArrayNV) (GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void *fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
extern void         (KHRONOS_APIENTRY* const& glProgramPathFragmentInputGenNV) (GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat *coeffs);
extern void         (KHRONOS_APIENTRY* const& glGetProgramResourcefvNV) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glPathColorGenNV) (GLenum color, GLenum genMode, GLenum colorFormat, const GLfloat *coeffs);
extern void         (KHRONOS_APIENTRY* const& glPathTexGenNV) (GLenum texCoordSet, GLenum genMode, GLint components, const GLfloat *coeffs);
extern void         (KHRONOS_APIENTRY* const& glPathFogGenNV) (GLenum genMode);
extern void         (KHRONOS_APIENTRY* const& glGetPathColorGenivNV) (GLenum color, GLenum pname, GLint *value);
extern void         (KHRONOS_APIENTRY* const& glGetPathColorGenfvNV) (GLenum color, GLenum pname, GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glGetPathTexGenivNV) (GLenum texCoordSet, GLenum pname, GLint *value);
extern void         (KHRONOS_APIENTRY* const& glGetPathTexGenfvNV) (GLenum texCoordSet, GLenum pname, GLfloat *value);
#endif

#ifndef GL_NV_path_rendering_shared_edge
#define GL_NV_path_rendering_shared_edge 1
enum : GLenum
{
    GL_SHARED_EDGE_NV                                       = 0xC0,
};
#endif

#ifndef GL_NV_pixel_data_range
#define GL_NV_pixel_data_range 1
enum : GLenum
{
    GL_WRITE_PIXEL_DATA_RANGE_NV                            = 0x8878,
    GL_READ_PIXEL_DATA_RANGE_NV                             = 0x8879,
    GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV                     = 0x887A,
    GL_READ_PIXEL_DATA_RANGE_LENGTH_NV                      = 0x887B,
    GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV                    = 0x887C,
    GL_READ_PIXEL_DATA_RANGE_POINTER_NV                     = 0x887D,
};
extern void         (KHRONOS_APIENTRY* const& glPixelDataRangeNV) (GLenum target, GLsizei length, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glFlushPixelDataRangeNV) (GLenum target);
#endif

#ifndef GL_NV_point_sprite
#define GL_NV_point_sprite 1
enum : GLenum
{
    GL_POINT_SPRITE_NV                                      = 0x8861,
    GL_COORD_REPLACE_NV                                     = 0x8862,
    GL_POINT_SPRITE_R_MODE_NV                               = 0x8863,
};
extern void         (KHRONOS_APIENTRY* const& glPointParameteriNV) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glPointParameterivNV) (GLenum pname, const GLint *params);
#endif

#ifndef GL_NV_present_video
#define GL_NV_present_video 1
enum : GLenum
{
    GL_FRAME_NV                                             = 0x8E26,
    GL_FIELDS_NV                                            = 0x8E27,
    GL_CURRENT_TIME_NV                                      = 0x8E28,
    GL_NUM_FILL_STREAMS_NV                                  = 0x8E29,
    GL_PRESENT_TIME_NV                                      = 0x8E2A,
    GL_PRESENT_DURATION_NV                                  = 0x8E2B,
};
extern void         (KHRONOS_APIENTRY* const& glPresentFrameKeyedNV) (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1);
extern void         (KHRONOS_APIENTRY* const& glPresentFrameDualFillNV) (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3);
extern void         (KHRONOS_APIENTRY* const& glGetVideoivNV) (GLuint video_slot, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVideouivNV) (GLuint video_slot, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVideoi64vNV) (GLuint video_slot, GLenum pname, GLint64EXT *params);
extern void         (KHRONOS_APIENTRY* const& glGetVideoui64vNV) (GLuint video_slot, GLenum pname, GLuint64EXT *params);
#endif

#ifndef GL_NV_primitive_restart
#define GL_NV_primitive_restart 1
enum : GLenum
{
    GL_PRIMITIVE_RESTART_NV                                 = 0x8558,
    GL_PRIMITIVE_RESTART_INDEX_NV                           = 0x8559,
};
extern void         (KHRONOS_APIENTRY* const& glPrimitiveRestartNV) ();
extern void         (KHRONOS_APIENTRY* const& glPrimitiveRestartIndexNV) (GLuint index);
#endif

#ifndef GL_NV_primitive_shading_rate
#define GL_NV_primitive_shading_rate 1
enum : GLenum
{
    GL_SHADING_RATE_IMAGE_PER_PRIMITIVE_NV                  = 0x95B1,
    GL_SHADING_RATE_IMAGE_PALETTE_COUNT_NV                  = 0x95B2,
};
#endif

#ifndef GL_NV_query_resource
#define GL_NV_query_resource 1
enum : GLenum
{
    GL_QUERY_RESOURCE_TYPE_VIDMEM_ALLOC_NV                  = 0x9540,
    GL_QUERY_RESOURCE_MEMTYPE_VIDMEM_NV                     = 0x9542,
    GL_QUERY_RESOURCE_SYS_RESERVED_NV                       = 0x9544,
    GL_QUERY_RESOURCE_TEXTURE_NV                            = 0x9545,
    GL_QUERY_RESOURCE_RENDERBUFFER_NV                       = 0x9546,
    GL_QUERY_RESOURCE_BUFFEROBJECT_NV                       = 0x9547,
};
extern GLint        (KHRONOS_APIENTRY* const& glQueryResourceNV) (GLenum queryType, GLint tagId, GLuint count, GLint *buffer);
#endif

#ifndef GL_NV_query_resource_tag
#define GL_NV_query_resource_tag 1
extern void         (KHRONOS_APIENTRY* const& glGenQueryResourceTagNV) (GLsizei n, GLint *tagIds);
extern void         (KHRONOS_APIENTRY* const& glDeleteQueryResourceTagNV) (GLsizei n, const GLint *tagIds);
extern void         (KHRONOS_APIENTRY* const& glQueryResourceTagNV) (GLint tagId, const GLchar *tagString);
#endif

#ifndef GL_NV_register_combiners
#define GL_NV_register_combiners 1
enum : GLenum
{
    GL_REGISTER_COMBINERS_NV                                = 0x8522,
    GL_VARIABLE_A_NV                                        = 0x8523,
    GL_VARIABLE_B_NV                                        = 0x8524,
    GL_VARIABLE_C_NV                                        = 0x8525,
    GL_VARIABLE_D_NV                                        = 0x8526,
    GL_VARIABLE_E_NV                                        = 0x8527,
    GL_VARIABLE_F_NV                                        = 0x8528,
    GL_VARIABLE_G_NV                                        = 0x8529,
    GL_CONSTANT_COLOR0_NV                                   = 0x852A,
    GL_CONSTANT_COLOR1_NV                                   = 0x852B,
    GL_SPARE0_NV                                            = 0x852E,
    GL_SPARE1_NV                                            = 0x852F,
    GL_DISCARD_NV                                           = 0x8530,
    GL_E_TIMES_F_NV                                         = 0x8531,
    GL_SPARE0_PLUS_SECONDARY_COLOR_NV                       = 0x8532,
    GL_UNSIGNED_IDENTITY_NV                                 = 0x8536,
    GL_UNSIGNED_INVERT_NV                                   = 0x8537,
    GL_EXPAND_NORMAL_NV                                     = 0x8538,
    GL_EXPAND_NEGATE_NV                                     = 0x8539,
    GL_HALF_BIAS_NORMAL_NV                                  = 0x853A,
    GL_HALF_BIAS_NEGATE_NV                                  = 0x853B,
    GL_SIGNED_IDENTITY_NV                                   = 0x853C,
    GL_SIGNED_NEGATE_NV                                     = 0x853D,
    GL_SCALE_BY_TWO_NV                                      = 0x853E,
    GL_SCALE_BY_FOUR_NV                                     = 0x853F,
    GL_SCALE_BY_ONE_HALF_NV                                 = 0x8540,
    GL_BIAS_BY_NEGATIVE_ONE_HALF_NV                         = 0x8541,
    GL_COMBINER_INPUT_NV                                    = 0x8542,
    GL_COMBINER_MAPPING_NV                                  = 0x8543,
    GL_COMBINER_COMPONENT_USAGE_NV                          = 0x8544,
    GL_COMBINER_AB_DOT_PRODUCT_NV                           = 0x8545,
    GL_COMBINER_CD_DOT_PRODUCT_NV                           = 0x8546,
    GL_COMBINER_MUX_SUM_NV                                  = 0x8547,
    GL_COMBINER_SCALE_NV                                    = 0x8548,
    GL_COMBINER_BIAS_NV                                     = 0x8549,
    GL_COMBINER_AB_OUTPUT_NV                                = 0x854A,
    GL_COMBINER_CD_OUTPUT_NV                                = 0x854B,
    GL_COMBINER_SUM_OUTPUT_NV                               = 0x854C,
    GL_MAX_GENERAL_COMBINERS_NV                             = 0x854D,
    GL_NUM_GENERAL_COMBINERS_NV                             = 0x854E,
    GL_COLOR_SUM_CLAMP_NV                                   = 0x854F,
    GL_COMBINER0_NV                                         = 0x8550,
    GL_COMBINER1_NV                                         = 0x8551,
    GL_COMBINER2_NV                                         = 0x8552,
    GL_COMBINER3_NV                                         = 0x8553,
    GL_COMBINER4_NV                                         = 0x8554,
    GL_COMBINER5_NV                                         = 0x8555,
    GL_COMBINER6_NV                                         = 0x8556,
    GL_COMBINER7_NV                                         = 0x8557,
};
extern void         (KHRONOS_APIENTRY* const& glCombinerParameterfvNV) (GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glCombinerParameterfNV) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glCombinerParameterivNV) (GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCombinerParameteriNV) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glCombinerInputNV) (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
extern void         (KHRONOS_APIENTRY* const& glCombinerOutputNV) (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
extern void         (KHRONOS_APIENTRY* const& glFinalCombinerInputNV) (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
extern void         (KHRONOS_APIENTRY* const& glGetCombinerInputParameterfvNV) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetCombinerInputParameterivNV) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetCombinerOutputParameterfvNV) (GLenum stage, GLenum portion, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetCombinerOutputParameterivNV) (GLenum stage, GLenum portion, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetFinalCombinerInputParameterfvNV) (GLenum variable, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetFinalCombinerInputParameterivNV) (GLenum variable, GLenum pname, GLint *params);
#endif

#ifndef GL_NV_register_combiners2
#define GL_NV_register_combiners2 1
enum : GLenum
{
    GL_PER_STAGE_CONSTANTS_NV                               = 0x8535,
};
extern void         (KHRONOS_APIENTRY* const& glCombinerStageParameterfvNV) (GLenum stage, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetCombinerStageParameterfvNV) (GLenum stage, GLenum pname, GLfloat *params);
#endif

#ifndef GL_NV_representative_fragment_test
#define GL_NV_representative_fragment_test 1
enum : GLenum
{
    GL_REPRESENTATIVE_FRAGMENT_TEST_NV                      = 0x937F,
};
#endif

#ifndef GL_NV_robustness_video_memory_purge
#define GL_NV_robustness_video_memory_purge 1
enum : GLenum
{
    GL_PURGED_CONTEXT_RESET_NV                              = 0x92BB,
};
#endif

#ifndef GL_NV_sample_locations
#define GL_NV_sample_locations 1
enum : GLenum
{
    GL_SAMPLE_LOCATION_SUBPIXEL_BITS_NV                     = 0x933D,
    GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_NV                  = 0x933E,
    GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_NV                 = 0x933F,
    GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_NV           = 0x9340,
    GL_SAMPLE_LOCATION_NV                                   = 0x8E50,
    GL_PROGRAMMABLE_SAMPLE_LOCATION_NV                      = 0x9341,
    GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_NV         = 0x9342,
    GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_NV            = 0x9343,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferSampleLocationsfvNV) (GLenum target, GLuint start, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glNamedFramebufferSampleLocationsfvNV) (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glResolveDepthValuesNV) ();
#endif

#ifndef GL_NV_sample_mask_override_coverage
#define GL_NV_sample_mask_override_coverage 1
#endif

#ifndef GL_NV_scissor_exclusive
#define GL_NV_scissor_exclusive 1
enum : GLenum
{
    GL_SCISSOR_TEST_EXCLUSIVE_NV                            = 0x9555,
    GL_SCISSOR_BOX_EXCLUSIVE_NV                             = 0x9556,
};
extern void         (KHRONOS_APIENTRY* const& glScissorExclusiveNV) (GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glScissorExclusiveArrayvNV) (GLuint first, GLsizei count, const GLint *v);
#endif

#ifndef GL_NV_shader_atomic_counters
#define GL_NV_shader_atomic_counters 1
#endif

#ifndef GL_NV_shader_atomic_float
#define GL_NV_shader_atomic_float 1
#endif

#ifndef GL_NV_shader_atomic_float64
#define GL_NV_shader_atomic_float64 1
#endif

#ifndef GL_NV_shader_atomic_fp16_vector
#define GL_NV_shader_atomic_fp16_vector 1
#endif

#ifndef GL_NV_shader_atomic_int64
#define GL_NV_shader_atomic_int64 1
#endif

#ifndef GL_NV_shader_buffer_load
#define GL_NV_shader_buffer_load 1
enum : GLenum
{
    GL_BUFFER_GPU_ADDRESS_NV                                = 0x8F1D,
    GL_GPU_ADDRESS_NV                                       = 0x8F34,
    GL_MAX_SHADER_BUFFER_ADDRESS_NV                         = 0x8F35,
};
extern void         (KHRONOS_APIENTRY* const& glMakeBufferResidentNV) (GLenum target, GLenum access);
extern void         (KHRONOS_APIENTRY* const& glMakeBufferNonResidentNV) (GLenum target);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsBufferResidentNV) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glMakeNamedBufferResidentNV) (GLuint buffer, GLenum access);
extern void         (KHRONOS_APIENTRY* const& glMakeNamedBufferNonResidentNV) (GLuint buffer);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsNamedBufferResidentNV) (GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glGetBufferParameterui64vNV) (GLenum target, GLenum pname, GLuint64EXT *params);
extern void         (KHRONOS_APIENTRY* const& glGetNamedBufferParameterui64vNV) (GLuint buffer, GLenum pname, GLuint64EXT *params);
extern void         (KHRONOS_APIENTRY* const& glGetIntegerui64vNV) (GLenum value, GLuint64EXT *result);
extern void         (KHRONOS_APIENTRY* const& glUniformui64NV) (GLint location, GLuint64EXT value);
extern void         (KHRONOS_APIENTRY* const& glUniformui64vNV) (GLint location, GLsizei count, const GLuint64EXT *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformui64NV) (GLuint program, GLint location, GLuint64EXT value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformui64vNV) (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value);
#endif

#ifndef GL_NV_shader_buffer_store
#define GL_NV_shader_buffer_store 1
enum : GLenum
{
    GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV                  = 0x00000010,
};
#endif

#ifndef GL_NV_shader_storage_buffer_object
#define GL_NV_shader_storage_buffer_object 1
#endif

#ifndef GL_NV_shader_subgroup_partitioned
#define GL_NV_shader_subgroup_partitioned 1
enum : GLenum
{
    GL_SUBGROUP_FEATURE_PARTITIONED_BIT_NV                  = 0x00000100,
};
#endif

#ifndef GL_NV_shader_texture_footprint
#define GL_NV_shader_texture_footprint 1
#endif

#ifndef GL_NV_shader_thread_group
#define GL_NV_shader_thread_group 1
enum : GLenum
{
    GL_WARP_SIZE_NV                                         = 0x9339,
    GL_WARPS_PER_SM_NV                                      = 0x933A,
    GL_SM_COUNT_NV                                          = 0x933B,
};
#endif

#ifndef GL_NV_shader_thread_shuffle
#define GL_NV_shader_thread_shuffle 1
#endif

#ifndef GL_NV_shading_rate_image
#define GL_NV_shading_rate_image 1
enum : GLenum
{
    GL_SHADING_RATE_IMAGE_NV                                = 0x9563,
    GL_SHADING_RATE_NO_INVOCATIONS_NV                       = 0x9564,
    GL_SHADING_RATE_1_INVOCATION_PER_PIXEL_NV               = 0x9565,
    GL_SHADING_RATE_1_INVOCATION_PER_1X2_PIXELS_NV          = 0x9566,
    GL_SHADING_RATE_1_INVOCATION_PER_2X1_PIXELS_NV          = 0x9567,
    GL_SHADING_RATE_1_INVOCATION_PER_2X2_PIXELS_NV          = 0x9568,
    GL_SHADING_RATE_1_INVOCATION_PER_2X4_PIXELS_NV          = 0x9569,
    GL_SHADING_RATE_1_INVOCATION_PER_4X2_PIXELS_NV          = 0x956A,
    GL_SHADING_RATE_1_INVOCATION_PER_4X4_PIXELS_NV          = 0x956B,
    GL_SHADING_RATE_2_INVOCATIONS_PER_PIXEL_NV              = 0x956C,
    GL_SHADING_RATE_4_INVOCATIONS_PER_PIXEL_NV              = 0x956D,
    GL_SHADING_RATE_8_INVOCATIONS_PER_PIXEL_NV              = 0x956E,
    GL_SHADING_RATE_16_INVOCATIONS_PER_PIXEL_NV             = 0x956F,
    GL_SHADING_RATE_IMAGE_BINDING_NV                        = 0x955B,
    GL_SHADING_RATE_IMAGE_TEXEL_WIDTH_NV                    = 0x955C,
    GL_SHADING_RATE_IMAGE_TEXEL_HEIGHT_NV                   = 0x955D,
    GL_SHADING_RATE_IMAGE_PALETTE_SIZE_NV                   = 0x955E,
    GL_MAX_COARSE_FRAGMENT_SAMPLES_NV                       = 0x955F,
    GL_SHADING_RATE_SAMPLE_ORDER_DEFAULT_NV                 = 0x95AE,
    GL_SHADING_RATE_SAMPLE_ORDER_PIXEL_MAJOR_NV             = 0x95AF,
    GL_SHADING_RATE_SAMPLE_ORDER_SAMPLE_MAJOR_NV            = 0x95B0,
};
extern void         (KHRONOS_APIENTRY* const& glBindShadingRateImageNV) (GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glGetShadingRateImagePaletteNV) (GLuint viewport, GLuint entry, GLenum *rate);
extern void         (KHRONOS_APIENTRY* const& glGetShadingRateSampleLocationivNV) (GLenum rate, GLuint samples, GLuint index, GLint *location);
extern void         (KHRONOS_APIENTRY* const& glShadingRateImageBarrierNV) (GLboolean synchronize);
extern void         (KHRONOS_APIENTRY* const& glShadingRateImagePaletteNV) (GLuint viewport, GLuint first, GLsizei count, const GLenum *rates);
extern void         (KHRONOS_APIENTRY* const& glShadingRateSampleOrderNV) (GLenum order);
extern void         (KHRONOS_APIENTRY* const& glShadingRateSampleOrderCustomNV) (GLenum rate, GLuint samples, const GLint *locations);
#endif

#ifndef GL_NV_stereo_view_rendering
#define GL_NV_stereo_view_rendering 1
#endif

#ifndef GL_NV_tessellation_program5
#define GL_NV_tessellation_program5 1
enum : GLenum
{
    GL_MAX_PROGRAM_PATCH_ATTRIBS_NV                         = 0x86D8,
    GL_TESS_CONTROL_PROGRAM_NV                              = 0x891E,
    GL_TESS_EVALUATION_PROGRAM_NV                           = 0x891F,
    GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV             = 0x8C74,
    GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV          = 0x8C75,
};
#endif

#ifndef GL_NV_texgen_emboss
#define GL_NV_texgen_emboss 1
enum : GLenum
{
    GL_EMBOSS_LIGHT_NV                                      = 0x855D,
    GL_EMBOSS_CONSTANT_NV                                   = 0x855E,
    GL_EMBOSS_MAP_NV                                        = 0x855F,
};
#endif

#ifndef GL_NV_texgen_reflection
#define GL_NV_texgen_reflection 1
enum : GLenum
{
    GL_NORMAL_MAP_NV                                        = 0x8511,
    GL_REFLECTION_MAP_NV                                    = 0x8512,
};
#endif

#ifndef GL_NV_texture_barrier
#define GL_NV_texture_barrier 1
extern void         (KHRONOS_APIENTRY* const& glTextureBarrierNV) ();
#endif

#ifndef GL_NV_texture_compression_vtc
#define GL_NV_texture_compression_vtc 1
#endif

#ifndef GL_NV_texture_env_combine4
#define GL_NV_texture_env_combine4 1
enum : GLenum
{
    GL_COMBINE4_NV                                          = 0x8503,
    GL_SOURCE3_RGB_NV                                       = 0x8583,
    GL_SOURCE3_ALPHA_NV                                     = 0x858B,
    GL_OPERAND3_RGB_NV                                      = 0x8593,
    GL_OPERAND3_ALPHA_NV                                    = 0x859B,
};
#endif

#ifndef GL_NV_texture_expand_normal
#define GL_NV_texture_expand_normal 1
enum : GLenum
{
    GL_TEXTURE_UNSIGNED_REMAP_MODE_NV                       = 0x888F,
};
#endif

#ifndef GL_NV_texture_multisample
#define GL_NV_texture_multisample 1
enum : GLenum
{
    GL_TEXTURE_COVERAGE_SAMPLES_NV                          = 0x9045,
    GL_TEXTURE_COLOR_SAMPLES_NV                             = 0x9046,
};
extern void         (KHRONOS_APIENTRY* const& glTexImage2DMultisampleCoverageNV) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations);
extern void         (KHRONOS_APIENTRY* const& glTexImage3DMultisampleCoverageNV) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations);
extern void         (KHRONOS_APIENTRY* const& glTextureImage2DMultisampleNV) (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations);
extern void         (KHRONOS_APIENTRY* const& glTextureImage3DMultisampleNV) (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations);
extern void         (KHRONOS_APIENTRY* const& glTextureImage2DMultisampleCoverageNV) (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations);
extern void         (KHRONOS_APIENTRY* const& glTextureImage3DMultisampleCoverageNV) (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations);
#endif

#ifndef GL_NV_texture_rectangle
#define GL_NV_texture_rectangle 1
enum : GLenum
{
    GL_TEXTURE_RECTANGLE_NV                                 = 0x84F5,
    GL_TEXTURE_BINDING_RECTANGLE_NV                         = 0x84F6,
    GL_PROXY_TEXTURE_RECTANGLE_NV                           = 0x84F7,
    GL_MAX_RECTANGLE_TEXTURE_SIZE_NV                        = 0x84F8,
};
#endif

#ifndef GL_NV_texture_rectangle_compressed
#define GL_NV_texture_rectangle_compressed 1
#endif

#ifndef GL_NV_texture_shader
#define GL_NV_texture_shader 1
enum : GLenum
{
    GL_OFFSET_TEXTURE_RECTANGLE_NV                          = 0x864C,
    GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV                    = 0x864D,
    GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV                     = 0x864E,
    GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV                 = 0x86D9,
    GL_UNSIGNED_INT_S8_S8_8_8_NV                            = 0x86DA,
    GL_UNSIGNED_INT_8_8_S8_S8_REV_NV                        = 0x86DB,
    GL_DSDT_MAG_INTENSITY_NV                                = 0x86DC,
    GL_SHADER_CONSISTENT_NV                                 = 0x86DD,
    GL_TEXTURE_SHADER_NV                                    = 0x86DE,
    GL_SHADER_OPERATION_NV                                  = 0x86DF,
    GL_CULL_MODES_NV                                        = 0x86E0,
    GL_OFFSET_TEXTURE_MATRIX_NV                             = 0x86E1,
    GL_OFFSET_TEXTURE_SCALE_NV                              = 0x86E2,
    GL_OFFSET_TEXTURE_BIAS_NV                               = 0x86E3,
    GL_OFFSET_TEXTURE_2D_MATRIX_NV                          = 0x86E1,
    GL_OFFSET_TEXTURE_2D_SCALE_NV                           = 0x86E2,
    GL_OFFSET_TEXTURE_2D_BIAS_NV                            = 0x86E3,
    GL_PREVIOUS_TEXTURE_INPUT_NV                            = 0x86E4,
    GL_CONST_EYE_NV                                         = 0x86E5,
    GL_PASS_THROUGH_NV                                      = 0x86E6,
    GL_CULL_FRAGMENT_NV                                     = 0x86E7,
    GL_OFFSET_TEXTURE_2D_NV                                 = 0x86E8,
    GL_DEPENDENT_AR_TEXTURE_2D_NV                           = 0x86E9,
    GL_DEPENDENT_GB_TEXTURE_2D_NV                           = 0x86EA,
    GL_DOT_PRODUCT_NV                                       = 0x86EC,
    GL_DOT_PRODUCT_DEPTH_REPLACE_NV                         = 0x86ED,
    GL_DOT_PRODUCT_TEXTURE_2D_NV                            = 0x86EE,
    GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV                      = 0x86F0,
    GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV                      = 0x86F1,
    GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV                      = 0x86F2,
    GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV            = 0x86F3,
    GL_HILO_NV                                              = 0x86F4,
    GL_DSDT_NV                                              = 0x86F5,
    GL_DSDT_MAG_NV                                          = 0x86F6,
    GL_DSDT_MAG_VIB_NV                                      = 0x86F7,
    GL_HILO16_NV                                            = 0x86F8,
    GL_SIGNED_HILO_NV                                       = 0x86F9,
    GL_SIGNED_HILO16_NV                                     = 0x86FA,
    GL_SIGNED_RGBA_NV                                       = 0x86FB,
    GL_SIGNED_RGBA8_NV                                      = 0x86FC,
    GL_SIGNED_RGB_NV                                        = 0x86FE,
    GL_SIGNED_RGB8_NV                                       = 0x86FF,
    GL_SIGNED_LUMINANCE_NV                                  = 0x8701,
    GL_SIGNED_LUMINANCE8_NV                                 = 0x8702,
    GL_SIGNED_LUMINANCE_ALPHA_NV                            = 0x8703,
    GL_SIGNED_LUMINANCE8_ALPHA8_NV                          = 0x8704,
    GL_SIGNED_ALPHA_NV                                      = 0x8705,
    GL_SIGNED_ALPHA8_NV                                     = 0x8706,
    GL_SIGNED_INTENSITY_NV                                  = 0x8707,
    GL_SIGNED_INTENSITY8_NV                                 = 0x8708,
    GL_DSDT8_NV                                             = 0x8709,
    GL_DSDT8_MAG8_NV                                        = 0x870A,
    GL_DSDT8_MAG8_INTENSITY8_NV                             = 0x870B,
    GL_SIGNED_RGB_UNSIGNED_ALPHA_NV                         = 0x870C,
    GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV                       = 0x870D,
    GL_HI_SCALE_NV                                          = 0x870E,
    GL_LO_SCALE_NV                                          = 0x870F,
    GL_DS_SCALE_NV                                          = 0x8710,
    GL_DT_SCALE_NV                                          = 0x8711,
    GL_MAGNITUDE_SCALE_NV                                   = 0x8712,
    GL_VIBRANCE_SCALE_NV                                    = 0x8713,
    GL_HI_BIAS_NV                                           = 0x8714,
    GL_LO_BIAS_NV                                           = 0x8715,
    GL_DS_BIAS_NV                                           = 0x8716,
    GL_DT_BIAS_NV                                           = 0x8717,
    GL_MAGNITUDE_BIAS_NV                                    = 0x8718,
    GL_VIBRANCE_BIAS_NV                                     = 0x8719,
    GL_TEXTURE_BORDER_VALUES_NV                             = 0x871A,
    GL_TEXTURE_HI_SIZE_NV                                   = 0x871B,
    GL_TEXTURE_LO_SIZE_NV                                   = 0x871C,
    GL_TEXTURE_DS_SIZE_NV                                   = 0x871D,
    GL_TEXTURE_DT_SIZE_NV                                   = 0x871E,
    GL_TEXTURE_MAG_SIZE_NV                                  = 0x871F,
};
#endif

#ifndef GL_NV_texture_shader2
#define GL_NV_texture_shader2 1
enum : GLenum
{
    GL_DOT_PRODUCT_TEXTURE_3D_NV                            = 0x86EF,
};
#endif

#ifndef GL_NV_texture_shader3
#define GL_NV_texture_shader3 1
enum : GLenum
{
    GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV                      = 0x8850,
    GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV                = 0x8851,
    GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV               = 0x8852,
    GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV         = 0x8853,
    GL_OFFSET_HILO_TEXTURE_2D_NV                            = 0x8854,
    GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV                     = 0x8855,
    GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV                 = 0x8856,
    GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV          = 0x8857,
    GL_DEPENDENT_HILO_TEXTURE_2D_NV                         = 0x8858,
    GL_DEPENDENT_RGB_TEXTURE_3D_NV                          = 0x8859,
    GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV                    = 0x885A,
    GL_DOT_PRODUCT_PASS_THROUGH_NV                          = 0x885B,
    GL_DOT_PRODUCT_TEXTURE_1D_NV                            = 0x885C,
    GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV                  = 0x885D,
    GL_HILO8_NV                                             = 0x885E,
    GL_SIGNED_HILO8_NV                                      = 0x885F,
    GL_FORCE_BLUE_TO_ONE_NV                                 = 0x8860,
};
#endif

#ifndef GL_NV_transform_feedback
#define GL_NV_transform_feedback 1
enum : GLenum
{
    GL_BACK_PRIMARY_COLOR_NV                                = 0x8C77,
    GL_BACK_SECONDARY_COLOR_NV                              = 0x8C78,
    GL_TEXTURE_COORD_NV                                     = 0x8C79,
    GL_CLIP_DISTANCE_NV                                     = 0x8C7A,
    GL_VERTEX_ID_NV                                         = 0x8C7B,
    GL_PRIMITIVE_ID_NV                                      = 0x8C7C,
    GL_GENERIC_ATTRIB_NV                                    = 0x8C7D,
    GL_TRANSFORM_FEEDBACK_ATTRIBS_NV                        = 0x8C7E,
    GL_TRANSFORM_FEEDBACK_BUFFER_MODE_NV                    = 0x8C7F,
    GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV        = 0x8C80,
    GL_ACTIVE_VARYINGS_NV                                   = 0x8C81,
    GL_ACTIVE_VARYING_MAX_LENGTH_NV                         = 0x8C82,
    GL_TRANSFORM_FEEDBACK_VARYINGS_NV                       = 0x8C83,
    GL_TRANSFORM_FEEDBACK_BUFFER_START_NV                   = 0x8C84,
    GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_NV                    = 0x8C85,
    GL_TRANSFORM_FEEDBACK_RECORD_NV                         = 0x8C86,
    GL_PRIMITIVES_GENERATED_NV                              = 0x8C87,
    GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV             = 0x8C88,
    GL_RASTERIZER_DISCARD_NV                                = 0x8C89,
    GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV     = 0x8C8A,
    GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV           = 0x8C8B,
    GL_INTERLEAVED_ATTRIBS_NV                               = 0x8C8C,
    GL_SEPARATE_ATTRIBS_NV                                  = 0x8C8D,
    GL_TRANSFORM_FEEDBACK_BUFFER_NV                         = 0x8C8E,
    GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_NV                 = 0x8C8F,
    GL_LAYER_NV                                             = 0x8DAA,
};
enum : GLint
{
    GL_NEXT_BUFFER_NV                                       = -2,
    GL_SKIP_COMPONENTS4_NV                                  = -3,
    GL_SKIP_COMPONENTS3_NV                                  = -4,
    GL_SKIP_COMPONENTS2_NV                                  = -5,
    GL_SKIP_COMPONENTS1_NV                                  = -6,
};
extern void         (KHRONOS_APIENTRY* const& glBeginTransformFeedbackNV) (GLenum primitiveMode);
extern void         (KHRONOS_APIENTRY* const& glEndTransformFeedbackNV) ();
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackAttribsNV) (GLsizei count, const GLint *attribs, GLenum bufferMode);
extern void         (KHRONOS_APIENTRY* const& glBindBufferRangeNV) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glBindBufferOffsetNV) (GLenum target, GLuint index, GLuint buffer, GLintptr offset);
extern void         (KHRONOS_APIENTRY* const& glBindBufferBaseNV) (GLenum target, GLuint index, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackVaryingsNV) (GLuint program, GLsizei count, const GLint *locations, GLenum bufferMode);
extern void         (KHRONOS_APIENTRY* const& glActiveVaryingNV) (GLuint program, const GLchar *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetVaryingLocationNV) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetActiveVaryingNV) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetTransformFeedbackVaryingNV) (GLuint program, GLuint index, GLint *location);
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackStreamAttribsNV) (GLsizei count, const GLint *attribs, GLsizei nbuffers, const GLint *bufstreams, GLenum bufferMode);
#endif

#ifndef GL_NV_transform_feedback2
#define GL_NV_transform_feedback2 1
enum : GLenum
{
    GL_TRANSFORM_FEEDBACK_NV                                = 0x8E22,
    GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV                  = 0x8E23,
    GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV                  = 0x8E24,
    GL_TRANSFORM_FEEDBACK_BINDING_NV                        = 0x8E25,
};
extern void         (KHRONOS_APIENTRY* const& glBindTransformFeedbackNV) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDeleteTransformFeedbacksNV) (GLsizei n, const GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glGenTransformFeedbacksNV) (GLsizei n, GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTransformFeedbackNV) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glPauseTransformFeedbackNV) ();
extern void         (KHRONOS_APIENTRY* const& glResumeTransformFeedbackNV) ();
extern void         (KHRONOS_APIENTRY* const& glDrawTransformFeedbackNV) (GLenum mode, GLuint id);
#endif

#ifndef GL_NV_uniform_buffer_unified_memory
#define GL_NV_uniform_buffer_unified_memory 1
enum : GLenum
{
    GL_UNIFORM_BUFFER_UNIFIED_NV                            = 0x936E,
    GL_UNIFORM_BUFFER_ADDRESS_NV                            = 0x936F,
    GL_UNIFORM_BUFFER_LENGTH_NV                             = 0x9370,
};
#endif

#ifndef GL_NV_vdpau_interop
#define GL_NV_vdpau_interop 1
enum : GLenum
{
    GL_SURFACE_STATE_NV                                     = 0x86EB,
    GL_SURFACE_REGISTERED_NV                                = 0x86FD,
    GL_SURFACE_MAPPED_NV                                    = 0x8700,
    GL_WRITE_DISCARD_NV                                     = 0x88BE,
};
extern void         (KHRONOS_APIENTRY* const& glVDPAUInitNV) (const void *vdpDevice, const void *getProcAddress);
extern void         (KHRONOS_APIENTRY* const& glVDPAUFiniNV) ();
extern GLvdpauSurfaceNV (KHRONOS_APIENTRY* const& glVDPAURegisterVideoSurfaceNV) (const void *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames);
extern GLvdpauSurfaceNV (KHRONOS_APIENTRY* const& glVDPAURegisterOutputSurfaceNV) (const void *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames);
extern GLboolean    (KHRONOS_APIENTRY* const& glVDPAUIsSurfaceNV) (GLvdpauSurfaceNV surface);
extern void         (KHRONOS_APIENTRY* const& glVDPAUUnregisterSurfaceNV) (GLvdpauSurfaceNV surface);
extern void         (KHRONOS_APIENTRY* const& glVDPAUGetSurfaceivNV) (GLvdpauSurfaceNV surface, GLenum pname, GLsizei count, GLsizei *length, GLint *values);
extern void         (KHRONOS_APIENTRY* const& glVDPAUSurfaceAccessNV) (GLvdpauSurfaceNV surface, GLenum access);
extern void         (KHRONOS_APIENTRY* const& glVDPAUMapSurfacesNV) (GLsizei numSurfaces, const GLvdpauSurfaceNV *surfaces);
extern void         (KHRONOS_APIENTRY* const& glVDPAUUnmapSurfacesNV) (GLsizei numSurface, const GLvdpauSurfaceNV *surfaces);
#endif

#ifndef GL_NV_vdpau_interop2
#define GL_NV_vdpau_interop2 1
extern GLvdpauSurfaceNV (KHRONOS_APIENTRY* const& glVDPAURegisterVideoSurfaceWithPictureStructureNV) (const void *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames, GLboolean isFrameStructure);
#endif

#ifndef GL_NV_vertex_array_range
#define GL_NV_vertex_array_range 1
enum : GLenum
{
    GL_VERTEX_ARRAY_RANGE_NV                                = 0x851D,
    GL_VERTEX_ARRAY_RANGE_LENGTH_NV                         = 0x851E,
    GL_VERTEX_ARRAY_RANGE_VALID_NV                          = 0x851F,
    GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV                    = 0x8520,
    GL_VERTEX_ARRAY_RANGE_POINTER_NV                        = 0x8521,
};
extern void         (KHRONOS_APIENTRY* const& glFlushVertexArrayRangeNV) ();
extern void         (KHRONOS_APIENTRY* const& glVertexArrayRangeNV) (GLsizei length, const void *pointer);
#endif

#ifndef GL_NV_vertex_array_range2
#define GL_NV_vertex_array_range2 1
enum : GLenum
{
    GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV                  = 0x8533,
};
#endif

#ifndef GL_NV_vertex_attrib_integer_64bit
#define GL_NV_vertex_attrib_integer_64bit 1
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1i64NV) (GLuint index, GLint64EXT x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2i64NV) (GLuint index, GLint64EXT x, GLint64EXT y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3i64NV) (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4i64NV) (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1i64vNV) (GLuint index, const GLint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2i64vNV) (GLuint index, const GLint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3i64vNV) (GLuint index, const GLint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4i64vNV) (GLuint index, const GLint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1ui64NV) (GLuint index, GLuint64EXT x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2ui64NV) (GLuint index, GLuint64EXT x, GLuint64EXT y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3ui64NV) (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4ui64NV) (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL1ui64vNV) (GLuint index, const GLuint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL2ui64vNV) (GLuint index, const GLuint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL3ui64vNV) (GLuint index, const GLuint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribL4ui64vNV) (GLuint index, const GLuint64EXT *v);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribLi64vNV) (GLuint index, GLenum pname, GLint64EXT *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribLui64vNV) (GLuint index, GLenum pname, GLuint64EXT *params);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribLFormatNV) (GLuint index, GLint size, GLenum type, GLsizei stride);
#endif

#ifndef GL_NV_vertex_buffer_unified_memory
#define GL_NV_vertex_buffer_unified_memory 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV                       = 0x8F1E,
    GL_ELEMENT_ARRAY_UNIFIED_NV                             = 0x8F1F,
    GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV                       = 0x8F20,
    GL_VERTEX_ARRAY_ADDRESS_NV                              = 0x8F21,
    GL_NORMAL_ARRAY_ADDRESS_NV                              = 0x8F22,
    GL_COLOR_ARRAY_ADDRESS_NV                               = 0x8F23,
    GL_INDEX_ARRAY_ADDRESS_NV                               = 0x8F24,
    GL_TEXTURE_COORD_ARRAY_ADDRESS_NV                       = 0x8F25,
    GL_EDGE_FLAG_ARRAY_ADDRESS_NV                           = 0x8F26,
    GL_SECONDARY_COLOR_ARRAY_ADDRESS_NV                     = 0x8F27,
    GL_FOG_COORD_ARRAY_ADDRESS_NV                           = 0x8F28,
    GL_ELEMENT_ARRAY_ADDRESS_NV                             = 0x8F29,
    GL_VERTEX_ATTRIB_ARRAY_LENGTH_NV                        = 0x8F2A,
    GL_VERTEX_ARRAY_LENGTH_NV                               = 0x8F2B,
    GL_NORMAL_ARRAY_LENGTH_NV                               = 0x8F2C,
    GL_COLOR_ARRAY_LENGTH_NV                                = 0x8F2D,
    GL_INDEX_ARRAY_LENGTH_NV                                = 0x8F2E,
    GL_TEXTURE_COORD_ARRAY_LENGTH_NV                        = 0x8F2F,
    GL_EDGE_FLAG_ARRAY_LENGTH_NV                            = 0x8F30,
    GL_SECONDARY_COLOR_ARRAY_LENGTH_NV                      = 0x8F31,
    GL_FOG_COORD_ARRAY_LENGTH_NV                            = 0x8F32,
    GL_ELEMENT_ARRAY_LENGTH_NV                              = 0x8F33,
    GL_DRAW_INDIRECT_UNIFIED_NV                             = 0x8F40,
    GL_DRAW_INDIRECT_ADDRESS_NV                             = 0x8F41,
    GL_DRAW_INDIRECT_LENGTH_NV                              = 0x8F42,
};
extern void         (KHRONOS_APIENTRY* const& glBufferAddressRangeNV) (GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length);
extern void         (KHRONOS_APIENTRY* const& glVertexFormatNV) (GLint size, GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glNormalFormatNV) (GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glColorFormatNV) (GLint size, GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glIndexFormatNV) (GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glTexCoordFormatNV) (GLint size, GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glEdgeFlagFormatNV) (GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glSecondaryColorFormatNV) (GLint size, GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glFogCoordFormatNV) (GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribFormatNV) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribIFormatNV) (GLuint index, GLint size, GLenum type, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glGetIntegerui64i_vNV) (GLenum value, GLuint index, GLuint64EXT *result);
#endif

#ifndef GL_NV_vertex_program
#define GL_NV_vertex_program 1
enum : GLenum
{
    GL_VERTEX_PROGRAM_NV                                    = 0x8620,
    GL_VERTEX_STATE_PROGRAM_NV                              = 0x8621,
    GL_ATTRIB_ARRAY_SIZE_NV                                 = 0x8623,
    GL_ATTRIB_ARRAY_STRIDE_NV                               = 0x8624,
    GL_ATTRIB_ARRAY_TYPE_NV                                 = 0x8625,
    GL_CURRENT_ATTRIB_NV                                    = 0x8626,
    GL_PROGRAM_LENGTH_NV                                    = 0x8627,
    GL_PROGRAM_STRING_NV                                    = 0x8628,
    GL_MODELVIEW_PROJECTION_NV                              = 0x8629,
    GL_IDENTITY_NV                                          = 0x862A,
    GL_INVERSE_NV                                           = 0x862B,
    GL_TRANSPOSE_NV                                         = 0x862C,
    GL_INVERSE_TRANSPOSE_NV                                 = 0x862D,
    GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV                      = 0x862E,
    GL_MAX_TRACK_MATRICES_NV                                = 0x862F,
    GL_MATRIX0_NV                                           = 0x8630,
    GL_MATRIX1_NV                                           = 0x8631,
    GL_MATRIX2_NV                                           = 0x8632,
    GL_MATRIX3_NV                                           = 0x8633,
    GL_MATRIX4_NV                                           = 0x8634,
    GL_MATRIX5_NV                                           = 0x8635,
    GL_MATRIX6_NV                                           = 0x8636,
    GL_MATRIX7_NV                                           = 0x8637,
    GL_CURRENT_MATRIX_STACK_DEPTH_NV                        = 0x8640,
    GL_CURRENT_MATRIX_NV                                    = 0x8641,
    GL_VERTEX_PROGRAM_POINT_SIZE_NV                         = 0x8642,
    GL_VERTEX_PROGRAM_TWO_SIDE_NV                           = 0x8643,
    GL_PROGRAM_PARAMETER_NV                                 = 0x8644,
    GL_ATTRIB_ARRAY_POINTER_NV                              = 0x8645,
    GL_PROGRAM_TARGET_NV                                    = 0x8646,
    GL_PROGRAM_RESIDENT_NV                                  = 0x8647,
    GL_TRACK_MATRIX_NV                                      = 0x8648,
    GL_TRACK_MATRIX_TRANSFORM_NV                            = 0x8649,
    GL_VERTEX_PROGRAM_BINDING_NV                            = 0x864A,
    GL_PROGRAM_ERROR_POSITION_NV                            = 0x864B,
    GL_VERTEX_ATTRIB_ARRAY0_NV                              = 0x8650,
    GL_VERTEX_ATTRIB_ARRAY1_NV                              = 0x8651,
    GL_VERTEX_ATTRIB_ARRAY2_NV                              = 0x8652,
    GL_VERTEX_ATTRIB_ARRAY3_NV                              = 0x8653,
    GL_VERTEX_ATTRIB_ARRAY4_NV                              = 0x8654,
    GL_VERTEX_ATTRIB_ARRAY5_NV                              = 0x8655,
    GL_VERTEX_ATTRIB_ARRAY6_NV                              = 0x8656,
    GL_VERTEX_ATTRIB_ARRAY7_NV                              = 0x8657,
    GL_VERTEX_ATTRIB_ARRAY8_NV                              = 0x8658,
    GL_VERTEX_ATTRIB_ARRAY9_NV                              = 0x8659,
    GL_VERTEX_ATTRIB_ARRAY10_NV                             = 0x865A,
    GL_VERTEX_ATTRIB_ARRAY11_NV                             = 0x865B,
    GL_VERTEX_ATTRIB_ARRAY12_NV                             = 0x865C,
    GL_VERTEX_ATTRIB_ARRAY13_NV                             = 0x865D,
    GL_VERTEX_ATTRIB_ARRAY14_NV                             = 0x865E,
    GL_VERTEX_ATTRIB_ARRAY15_NV                             = 0x865F,
    GL_MAP1_VERTEX_ATTRIB0_4_NV                             = 0x8660,
    GL_MAP1_VERTEX_ATTRIB1_4_NV                             = 0x8661,
    GL_MAP1_VERTEX_ATTRIB2_4_NV                             = 0x8662,
    GL_MAP1_VERTEX_ATTRIB3_4_NV                             = 0x8663,
    GL_MAP1_VERTEX_ATTRIB4_4_NV                             = 0x8664,
    GL_MAP1_VERTEX_ATTRIB5_4_NV                             = 0x8665,
    GL_MAP1_VERTEX_ATTRIB6_4_NV                             = 0x8666,
    GL_MAP1_VERTEX_ATTRIB7_4_NV                             = 0x8667,
    GL_MAP1_VERTEX_ATTRIB8_4_NV                             = 0x8668,
    GL_MAP1_VERTEX_ATTRIB9_4_NV                             = 0x8669,
    GL_MAP1_VERTEX_ATTRIB10_4_NV                            = 0x866A,
    GL_MAP1_VERTEX_ATTRIB11_4_NV                            = 0x866B,
    GL_MAP1_VERTEX_ATTRIB12_4_NV                            = 0x866C,
    GL_MAP1_VERTEX_ATTRIB13_4_NV                            = 0x866D,
    GL_MAP1_VERTEX_ATTRIB14_4_NV                            = 0x866E,
    GL_MAP1_VERTEX_ATTRIB15_4_NV                            = 0x866F,
    GL_MAP2_VERTEX_ATTRIB0_4_NV                             = 0x8670,
    GL_MAP2_VERTEX_ATTRIB1_4_NV                             = 0x8671,
    GL_MAP2_VERTEX_ATTRIB2_4_NV                             = 0x8672,
    GL_MAP2_VERTEX_ATTRIB3_4_NV                             = 0x8673,
    GL_MAP2_VERTEX_ATTRIB4_4_NV                             = 0x8674,
    GL_MAP2_VERTEX_ATTRIB5_4_NV                             = 0x8675,
    GL_MAP2_VERTEX_ATTRIB6_4_NV                             = 0x8676,
    GL_MAP2_VERTEX_ATTRIB7_4_NV                             = 0x8677,
    GL_MAP2_VERTEX_ATTRIB8_4_NV                             = 0x8678,
    GL_MAP2_VERTEX_ATTRIB9_4_NV                             = 0x8679,
    GL_MAP2_VERTEX_ATTRIB10_4_NV                            = 0x867A,
    GL_MAP2_VERTEX_ATTRIB11_4_NV                            = 0x867B,
    GL_MAP2_VERTEX_ATTRIB12_4_NV                            = 0x867C,
    GL_MAP2_VERTEX_ATTRIB13_4_NV                            = 0x867D,
    GL_MAP2_VERTEX_ATTRIB14_4_NV                            = 0x867E,
    GL_MAP2_VERTEX_ATTRIB15_4_NV                            = 0x867F,
};
extern GLboolean    (KHRONOS_APIENTRY* const& glAreProgramsResidentNV) (GLsizei n, const GLuint *programs, GLboolean *residences);
extern void         (KHRONOS_APIENTRY* const& glBindProgramNV) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDeleteProgramsNV) (GLsizei n, const GLuint *programs);
extern void         (KHRONOS_APIENTRY* const& glExecuteProgramNV) (GLenum target, GLuint id, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGenProgramsNV) (GLsizei n, GLuint *programs);
extern void         (KHRONOS_APIENTRY* const& glGetProgramParameterdvNV) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramParameterfvNV) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramivNV) (GLuint id, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramStringNV) (GLuint id, GLenum pname, GLubyte *program);
extern void         (KHRONOS_APIENTRY* const& glGetTrackMatrixivNV) (GLenum target, GLuint address, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribdvNV) (GLuint index, GLenum pname, GLdouble *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribfvNV) (GLuint index, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribivNV) (GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribPointervNV) (GLuint index, GLenum pname, void **pointer);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsProgramNV) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glLoadProgramNV) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
extern void         (KHRONOS_APIENTRY* const& glProgramParameter4dNV) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glProgramParameter4dvNV) (GLenum target, GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glProgramParameter4fNV) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glProgramParameter4fvNV) (GLenum target, GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glProgramParameters4dvNV) (GLenum target, GLuint index, GLsizei count, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glProgramParameters4fvNV) (GLenum target, GLuint index, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glRequestResidentProgramsNV) (GLsizei n, const GLuint *programs);
extern void         (KHRONOS_APIENTRY* const& glTrackMatrixNV) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribPointerNV) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1dNV) (GLuint index, GLdouble x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1dvNV) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1fNV) (GLuint index, GLfloat x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1fvNV) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1sNV) (GLuint index, GLshort x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1svNV) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2dNV) (GLuint index, GLdouble x, GLdouble y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2dvNV) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2fNV) (GLuint index, GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2fvNV) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2sNV) (GLuint index, GLshort x, GLshort y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2svNV) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3dNV) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3dvNV) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3fNV) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3fvNV) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3sNV) (GLuint index, GLshort x, GLshort y, GLshort z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3svNV) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4dNV) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4dvNV) (GLuint index, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4fNV) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4fvNV) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4sNV) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4svNV) (GLuint index, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4ubNV) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4ubvNV) (GLuint index, const GLubyte *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs1dvNV) (GLuint index, GLsizei count, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs1fvNV) (GLuint index, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs1svNV) (GLuint index, GLsizei count, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs2dvNV) (GLuint index, GLsizei count, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs2fvNV) (GLuint index, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs2svNV) (GLuint index, GLsizei count, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs3dvNV) (GLuint index, GLsizei count, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs3fvNV) (GLuint index, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs3svNV) (GLuint index, GLsizei count, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs4dvNV) (GLuint index, GLsizei count, const GLdouble *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs4fvNV) (GLuint index, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs4svNV) (GLuint index, GLsizei count, const GLshort *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribs4ubvNV) (GLuint index, GLsizei count, const GLubyte *v);
#endif

#ifndef GL_NV_vertex_program1_1
#define GL_NV_vertex_program1_1 1
#endif

#ifndef GL_NV_vertex_program2
#define GL_NV_vertex_program2 1
#endif

#ifndef GL_NV_vertex_program2_option
#define GL_NV_vertex_program2_option 1
#endif

#ifndef GL_NV_vertex_program3
#define GL_NV_vertex_program3 1
#endif

#ifndef GL_NV_vertex_program4
#define GL_NV_vertex_program4 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_INTEGER_NV                       = 0x88FD,
};
#endif

#ifndef GL_NV_video_capture
#define GL_NV_video_capture 1
enum : GLenum
{
    GL_VIDEO_BUFFER_NV                                      = 0x9020,
    GL_VIDEO_BUFFER_BINDING_NV                              = 0x9021,
    GL_FIELD_UPPER_NV                                       = 0x9022,
    GL_FIELD_LOWER_NV                                       = 0x9023,
    GL_NUM_VIDEO_CAPTURE_STREAMS_NV                         = 0x9024,
    GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV                  = 0x9025,
    GL_VIDEO_CAPTURE_TO_422_SUPPORTED_NV                    = 0x9026,
    GL_LAST_VIDEO_CAPTURE_STATUS_NV                         = 0x9027,
    GL_VIDEO_BUFFER_PITCH_NV                                = 0x9028,
    GL_VIDEO_COLOR_CONVERSION_MATRIX_NV                     = 0x9029,
    GL_VIDEO_COLOR_CONVERSION_MAX_NV                        = 0x902A,
    GL_VIDEO_COLOR_CONVERSION_MIN_NV                        = 0x902B,
    GL_VIDEO_COLOR_CONVERSION_OFFSET_NV                     = 0x902C,
    GL_VIDEO_BUFFER_INTERNAL_FORMAT_NV                      = 0x902D,
    GL_PARTIAL_SUCCESS_NV                                   = 0x902E,
    GL_SUCCESS_NV                                           = 0x902F,
    GL_FAILURE_NV                                           = 0x9030,
    GL_YCBYCR8_422_NV                                       = 0x9031,
    GL_YCBAYCR8A_4224_NV                                    = 0x9032,
    GL_Z6Y10Z6CB10Z6Y10Z6CR10_422_NV                        = 0x9033,
    GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV             = 0x9034,
    GL_Z4Y12Z4CB12Z4Y12Z4CR12_422_NV                        = 0x9035,
    GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV             = 0x9036,
    GL_Z4Y12Z4CB12Z4CR12_444_NV                             = 0x9037,
    GL_VIDEO_CAPTURE_FRAME_WIDTH_NV                         = 0x9038,
    GL_VIDEO_CAPTURE_FRAME_HEIGHT_NV                        = 0x9039,
    GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV                  = 0x903A,
    GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV                  = 0x903B,
    GL_VIDEO_CAPTURE_SURFACE_ORIGIN_NV                      = 0x903C,
};
extern void         (KHRONOS_APIENTRY* const& glBeginVideoCaptureNV) (GLuint video_capture_slot);
extern void         (KHRONOS_APIENTRY* const& glBindVideoCaptureStreamBufferNV) (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset);
extern void         (KHRONOS_APIENTRY* const& glBindVideoCaptureStreamTextureNV) (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glEndVideoCaptureNV) (GLuint video_capture_slot);
extern void         (KHRONOS_APIENTRY* const& glGetVideoCaptureivNV) (GLuint video_capture_slot, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVideoCaptureStreamivNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVideoCaptureStreamfvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetVideoCaptureStreamdvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble *params);
extern GLenum       (KHRONOS_APIENTRY* const& glVideoCaptureNV) (GLuint video_capture_slot, GLuint *sequence_num, GLuint64EXT *capture_time);
extern void         (KHRONOS_APIENTRY* const& glVideoCaptureStreamParameterivNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glVideoCaptureStreamParameterfvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glVideoCaptureStreamParameterdvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble *params);
#endif

#ifndef GL_NV_viewport_array2
#define GL_NV_viewport_array2 1
#endif

#ifndef GL_NV_viewport_swizzle
#define GL_NV_viewport_swizzle 1
enum : GLenum
{
    GL_VIEWPORT_SWIZZLE_POSITIVE_X_NV                       = 0x9350,
    GL_VIEWPORT_SWIZZLE_NEGATIVE_X_NV                       = 0x9351,
    GL_VIEWPORT_SWIZZLE_POSITIVE_Y_NV                       = 0x9352,
    GL_VIEWPORT_SWIZZLE_NEGATIVE_Y_NV                       = 0x9353,
    GL_VIEWPORT_SWIZZLE_POSITIVE_Z_NV                       = 0x9354,
    GL_VIEWPORT_SWIZZLE_NEGATIVE_Z_NV                       = 0x9355,
    GL_VIEWPORT_SWIZZLE_POSITIVE_W_NV                       = 0x9356,
    GL_VIEWPORT_SWIZZLE_NEGATIVE_W_NV                       = 0x9357,
    GL_VIEWPORT_SWIZZLE_X_NV                                = 0x9358,
    GL_VIEWPORT_SWIZZLE_Y_NV                                = 0x9359,
    GL_VIEWPORT_SWIZZLE_Z_NV                                = 0x935A,
    GL_VIEWPORT_SWIZZLE_W_NV                                = 0x935B,
};
extern void         (KHRONOS_APIENTRY* const& glViewportSwizzleNV) (GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew);
#endif

#ifndef GL_OES_byte_coordinates
#define GL_OES_byte_coordinates 1
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1bOES) (GLenum texture, GLbyte s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1bvOES) (GLenum texture, const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2bOES) (GLenum texture, GLbyte s, GLbyte t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2bvOES) (GLenum texture, const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3bOES) (GLenum texture, GLbyte s, GLbyte t, GLbyte r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3bvOES) (GLenum texture, const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4bOES) (GLenum texture, GLbyte s, GLbyte t, GLbyte r, GLbyte q);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4bvOES) (GLenum texture, const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1bOES) (GLbyte s);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1bvOES) (const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2bOES) (GLbyte s, GLbyte t);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2bvOES) (const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3bOES) (GLbyte s, GLbyte t, GLbyte r);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3bvOES) (const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4bOES) (GLbyte s, GLbyte t, GLbyte r, GLbyte q);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4bvOES) (const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glVertex2bOES) (GLbyte x, GLbyte y);
extern void         (KHRONOS_APIENTRY* const& glVertex2bvOES) (const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glVertex3bOES) (GLbyte x, GLbyte y, GLbyte z);
extern void         (KHRONOS_APIENTRY* const& glVertex3bvOES) (const GLbyte *coords);
extern void         (KHRONOS_APIENTRY* const& glVertex4bOES) (GLbyte x, GLbyte y, GLbyte z, GLbyte w);
extern void         (KHRONOS_APIENTRY* const& glVertex4bvOES) (const GLbyte *coords);
#endif

#ifndef GL_OES_compressed_paletted_texture
#define GL_OES_compressed_paletted_texture 1
enum : GLenum
{
    GL_PALETTE4_RGB8_OES                                    = 0x8B90,
    GL_PALETTE4_RGBA8_OES                                   = 0x8B91,
    GL_PALETTE4_R5_G6_B5_OES                                = 0x8B92,
    GL_PALETTE4_RGBA4_OES                                   = 0x8B93,
    GL_PALETTE4_RGB5_A1_OES                                 = 0x8B94,
    GL_PALETTE8_RGB8_OES                                    = 0x8B95,
    GL_PALETTE8_RGBA8_OES                                   = 0x8B96,
    GL_PALETTE8_R5_G6_B5_OES                                = 0x8B97,
    GL_PALETTE8_RGBA4_OES                                   = 0x8B98,
    GL_PALETTE8_RGB5_A1_OES                                 = 0x8B99,
};
#endif

#ifndef GL_OES_fixed_point
#define GL_OES_fixed_point 1
enum : GLenum
{
    GL_FIXED_OES                                            = 0x140C,
};
extern void         (KHRONOS_APIENTRY* const& glAlphaFuncxOES) (GLenum func, GLfixed ref);
extern void         (KHRONOS_APIENTRY* const& glClearColorxOES) (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
extern void         (KHRONOS_APIENTRY* const& glClearDepthxOES) (GLfixed depth);
extern void         (KHRONOS_APIENTRY* const& glClipPlanexOES) (GLenum plane, const GLfixed *equation);
extern void         (KHRONOS_APIENTRY* const& glColor4xOES) (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
extern void         (KHRONOS_APIENTRY* const& glDepthRangexOES) (GLfixed n, GLfixed f);
extern void         (KHRONOS_APIENTRY* const& glFogxOES) (GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glFogxvOES) (GLenum pname, const GLfixed *param);
extern void         (KHRONOS_APIENTRY* const& glFrustumxOES) (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
extern void         (KHRONOS_APIENTRY* const& glGetClipPlanexOES) (GLenum plane, GLfixed *equation);
extern void         (KHRONOS_APIENTRY* const& glGetFixedvOES) (GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexEnvxvOES) (GLenum target, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterxvOES) (GLenum target, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glLightModelxOES) (GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glLightModelxvOES) (GLenum pname, const GLfixed *param);
extern void         (KHRONOS_APIENTRY* const& glLightxOES) (GLenum light, GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glLightxvOES) (GLenum light, GLenum pname, const GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glLineWidthxOES) (GLfixed width);
extern void         (KHRONOS_APIENTRY* const& glLoadMatrixxOES) (const GLfixed *m);
extern void         (KHRONOS_APIENTRY* const& glMaterialxOES) (GLenum face, GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glMaterialxvOES) (GLenum face, GLenum pname, const GLfixed *param);
extern void         (KHRONOS_APIENTRY* const& glMultMatrixxOES) (const GLfixed *m);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4xOES) (GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
extern void         (KHRONOS_APIENTRY* const& glNormal3xOES) (GLfixed nx, GLfixed ny, GLfixed nz);
extern void         (KHRONOS_APIENTRY* const& glOrthoxOES) (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
extern void         (KHRONOS_APIENTRY* const& glPointParameterxvOES) (GLenum pname, const GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glPointSizexOES) (GLfixed size);
extern void         (KHRONOS_APIENTRY* const& glPolygonOffsetxOES) (GLfixed factor, GLfixed units);
extern void         (KHRONOS_APIENTRY* const& glRotatexOES) (GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
extern void         (KHRONOS_APIENTRY* const& glScalexOES) (GLfixed x, GLfixed y, GLfixed z);
extern void         (KHRONOS_APIENTRY* const& glTexEnvxOES) (GLenum target, GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glTexEnvxvOES) (GLenum target, GLenum pname, const GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameterxOES) (GLenum target, GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glTexParameterxvOES) (GLenum target, GLenum pname, const GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glTranslatexOES) (GLfixed x, GLfixed y, GLfixed z);
extern void         (KHRONOS_APIENTRY* const& glGetLightxvOES) (GLenum light, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glGetMaterialxvOES) (GLenum face, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glPointParameterxOES) (GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glSampleCoveragexOES) (GLclampx value, GLboolean invert);
extern void         (KHRONOS_APIENTRY* const& glAccumxOES) (GLenum op, GLfixed value);
extern void         (KHRONOS_APIENTRY* const& glBitmapxOES) (GLsizei width, GLsizei height, GLfixed xorig, GLfixed yorig, GLfixed xmove, GLfixed ymove, const GLubyte *bitmap);
extern void         (KHRONOS_APIENTRY* const& glBlendColorxOES) (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
extern void         (KHRONOS_APIENTRY* const& glClearAccumxOES) (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
extern void         (KHRONOS_APIENTRY* const& glColor3xOES) (GLfixed red, GLfixed green, GLfixed blue);
extern void         (KHRONOS_APIENTRY* const& glColor3xvOES) (const GLfixed *components);
extern void         (KHRONOS_APIENTRY* const& glColor4xvOES) (const GLfixed *components);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameterxOES) (GLenum target, GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glConvolutionParameterxvOES) (GLenum target, GLenum pname, const GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord1xOES) (GLfixed u);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord1xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord2xOES) (GLfixed u, GLfixed v);
extern void         (KHRONOS_APIENTRY* const& glEvalCoord2xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glFeedbackBufferxOES) (GLsizei n, GLenum type, const GLfixed *buffer);
extern void         (KHRONOS_APIENTRY* const& glGetConvolutionParameterxvOES) (GLenum target, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glGetHistogramParameterxvOES) (GLenum target, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glGetLightxOES) (GLenum light, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glGetMapxvOES) (GLenum target, GLenum query, GLfixed *v);
extern void         (KHRONOS_APIENTRY* const& glGetMaterialxOES) (GLenum face, GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glGetPixelMapxv) (GLenum map, GLint size, GLfixed *values);
extern void         (KHRONOS_APIENTRY* const& glGetTexGenxvOES) (GLenum coord, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexLevelParameterxvOES) (GLenum target, GLint level, GLenum pname, GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glIndexxOES) (GLfixed component);
extern void         (KHRONOS_APIENTRY* const& glIndexxvOES) (const GLfixed *component);
extern void         (KHRONOS_APIENTRY* const& glLoadTransposeMatrixxOES) (const GLfixed *m);
extern void         (KHRONOS_APIENTRY* const& glMap1xOES) (GLenum target, GLfixed u1, GLfixed u2, GLint stride, GLint order, GLfixed points);
extern void         (KHRONOS_APIENTRY* const& glMap2xOES) (GLenum target, GLfixed u1, GLfixed u2, GLint ustride, GLint uorder, GLfixed v1, GLfixed v2, GLint vstride, GLint vorder, GLfixed points);
extern void         (KHRONOS_APIENTRY* const& glMapGrid1xOES) (GLint n, GLfixed u1, GLfixed u2);
extern void         (KHRONOS_APIENTRY* const& glMapGrid2xOES) (GLint n, GLfixed u1, GLfixed u2, GLfixed v1, GLfixed v2);
extern void         (KHRONOS_APIENTRY* const& glMultTransposeMatrixxOES) (const GLfixed *m);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1xOES) (GLenum texture, GLfixed s);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord1xvOES) (GLenum texture, const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2xOES) (GLenum texture, GLfixed s, GLfixed t);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord2xvOES) (GLenum texture, const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3xOES) (GLenum texture, GLfixed s, GLfixed t, GLfixed r);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord3xvOES) (GLenum texture, const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glMultiTexCoord4xvOES) (GLenum texture, const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glNormal3xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glPassThroughxOES) (GLfixed token);
extern void         (KHRONOS_APIENTRY* const& glPixelMapx) (GLenum map, GLint size, const GLfixed *values);
extern void         (KHRONOS_APIENTRY* const& glPixelStorex) (GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glPixelTransferxOES) (GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glPixelZoomxOES) (GLfixed xfactor, GLfixed yfactor);
extern void         (KHRONOS_APIENTRY* const& glPrioritizeTexturesxOES) (GLsizei n, const GLuint *textures, const GLfixed *priorities);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2xOES) (GLfixed x, GLfixed y);
extern void         (KHRONOS_APIENTRY* const& glRasterPos2xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3xOES) (GLfixed x, GLfixed y, GLfixed z);
extern void         (KHRONOS_APIENTRY* const& glRasterPos3xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4xOES) (GLfixed x, GLfixed y, GLfixed z, GLfixed w);
extern void         (KHRONOS_APIENTRY* const& glRasterPos4xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glRectxOES) (GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2);
extern void         (KHRONOS_APIENTRY* const& glRectxvOES) (const GLfixed *v1, const GLfixed *v2);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1xOES) (GLfixed s);
extern void         (KHRONOS_APIENTRY* const& glTexCoord1xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2xOES) (GLfixed s, GLfixed t);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3xOES) (GLfixed s, GLfixed t, GLfixed r);
extern void         (KHRONOS_APIENTRY* const& glTexCoord3xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4xOES) (GLfixed s, GLfixed t, GLfixed r, GLfixed q);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glTexGenxOES) (GLenum coord, GLenum pname, GLfixed param);
extern void         (KHRONOS_APIENTRY* const& glTexGenxvOES) (GLenum coord, GLenum pname, const GLfixed *params);
extern void         (KHRONOS_APIENTRY* const& glVertex2xOES) (GLfixed x);
extern void         (KHRONOS_APIENTRY* const& glVertex2xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glVertex3xOES) (GLfixed x, GLfixed y);
extern void         (KHRONOS_APIENTRY* const& glVertex3xvOES) (const GLfixed *coords);
extern void         (KHRONOS_APIENTRY* const& glVertex4xOES) (GLfixed x, GLfixed y, GLfixed z);
extern void         (KHRONOS_APIENTRY* const& glVertex4xvOES) (const GLfixed *coords);
#endif

#ifndef GL_OES_query_matrix
#define GL_OES_query_matrix 1
extern GLbitfield   (KHRONOS_APIENTRY* const& glQueryMatrixxOES) (GLfixed *mantissa, GLint *exponent);
#endif

#ifndef GL_OES_read_format
#define GL_OES_read_format 1
enum : GLenum
{
    GL_IMPLEMENTATION_COLOR_READ_TYPE_OES                   = 0x8B9A,
    GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES                 = 0x8B9B,
};
#endif

#ifndef GL_OES_single_precision
#define GL_OES_single_precision 1
extern void         (KHRONOS_APIENTRY* const& glClearDepthfOES) (GLclampf depth);
extern void         (KHRONOS_APIENTRY* const& glClipPlanefOES) (GLenum plane, const GLfloat *equation);
extern void         (KHRONOS_APIENTRY* const& glDepthRangefOES) (GLclampf n, GLclampf f);
extern void         (KHRONOS_APIENTRY* const& glFrustumfOES) (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
extern void         (KHRONOS_APIENTRY* const& glGetClipPlanefOES) (GLenum plane, GLfloat *equation);
extern void         (KHRONOS_APIENTRY* const& glOrthofOES) (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
#endif

#ifndef GL_OML_interlace
#define GL_OML_interlace 1
enum : GLenum
{
    GL_INTERLACE_OML                                        = 0x8980,
    GL_INTERLACE_READ_OML                                   = 0x8981,
};
#endif

#ifndef GL_OML_resample
#define GL_OML_resample 1
enum : GLenum
{
    GL_PACK_RESAMPLE_OML                                    = 0x8984,
    GL_UNPACK_RESAMPLE_OML                                  = 0x8985,
    GL_RESAMPLE_REPLICATE_OML                               = 0x8986,
    GL_RESAMPLE_ZERO_FILL_OML                               = 0x8987,
    GL_RESAMPLE_AVERAGE_OML                                 = 0x8988,
    GL_RESAMPLE_DECIMATE_OML                                = 0x8989,
};
#endif

#ifndef GL_OML_subsample
#define GL_OML_subsample 1
enum : GLenum
{
    GL_FORMAT_SUBSAMPLE_24_24_OML                           = 0x8982,
    GL_FORMAT_SUBSAMPLE_244_244_OML                         = 0x8983,
};
#endif

#ifndef GL_OVR_multiview
#define GL_OVR_multiview 1
enum : GLenum
{
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR         = 0x9630,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR   = 0x9632,
    GL_MAX_VIEWS_OVR                                        = 0x9631,
    GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR              = 0x9633,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureMultiviewOVR) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
#endif

#ifndef GL_OVR_multiview2
#define GL_OVR_multiview2 1
#endif

#ifndef GL_PGI_misc_hints
#define GL_PGI_misc_hints 1
enum : GLenum
{
    GL_PREFER_DOUBLEBUFFER_HINT_PGI                         = 0x1A1F8,
    GL_CONSERVE_MEMORY_HINT_PGI                             = 0x1A1FD,
    GL_RECLAIM_MEMORY_HINT_PGI                              = 0x1A1FE,
    GL_NATIVE_GRAPHICS_HANDLE_PGI                           = 0x1A202,
    GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI                       = 0x1A203,
    GL_NATIVE_GRAPHICS_END_HINT_PGI                         = 0x1A204,
    GL_ALWAYS_FAST_HINT_PGI                                 = 0x1A20C,
    GL_ALWAYS_SOFT_HINT_PGI                                 = 0x1A20D,
    GL_ALLOW_DRAW_OBJ_HINT_PGI                              = 0x1A20E,
    GL_ALLOW_DRAW_WIN_HINT_PGI                              = 0x1A20F,
    GL_ALLOW_DRAW_FRG_HINT_PGI                              = 0x1A210,
    GL_ALLOW_DRAW_MEM_HINT_PGI                              = 0x1A211,
    GL_STRICT_DEPTHFUNC_HINT_PGI                            = 0x1A216,
    GL_STRICT_LIGHTING_HINT_PGI                             = 0x1A217,
    GL_STRICT_SCISSOR_HINT_PGI                              = 0x1A218,
    GL_FULL_STIPPLE_HINT_PGI                                = 0x1A219,
    GL_CLIP_NEAR_HINT_PGI                                   = 0x1A220,
    GL_CLIP_FAR_HINT_PGI                                    = 0x1A221,
    GL_WIDE_LINE_HINT_PGI                                   = 0x1A222,
    GL_BACK_NORMALS_HINT_PGI                                = 0x1A223,
};
extern void         (KHRONOS_APIENTRY* const& glHintPGI) (GLenum target, GLint mode);
#endif

#ifndef GL_PGI_vertex_hints
#define GL_PGI_vertex_hints 1
enum : GLenum
{
    GL_VERTEX_DATA_HINT_PGI                                 = 0x1A22A,
    GL_VERTEX_CONSISTENT_HINT_PGI                           = 0x1A22B,
    GL_MATERIAL_SIDE_HINT_PGI                               = 0x1A22C,
    GL_MAX_VERTEX_HINT_PGI                                  = 0x1A22D,
    GL_COLOR3_BIT_PGI                                       = 0x00010000,
    GL_COLOR4_BIT_PGI                                       = 0x00020000,
    GL_EDGEFLAG_BIT_PGI                                     = 0x00040000,
    GL_INDEX_BIT_PGI                                        = 0x00080000,
    GL_MAT_AMBIENT_BIT_PGI                                  = 0x00100000,
    GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI                      = 0x00200000,
    GL_MAT_DIFFUSE_BIT_PGI                                  = 0x00400000,
    GL_MAT_EMISSION_BIT_PGI                                 = 0x00800000,
    GL_MAT_COLOR_INDEXES_BIT_PGI                            = 0x01000000,
    GL_MAT_SHININESS_BIT_PGI                                = 0x02000000,
    GL_MAT_SPECULAR_BIT_PGI                                 = 0x04000000,
    GL_NORMAL_BIT_PGI                                       = 0x08000000,
    GL_TEXCOORD1_BIT_PGI                                    = 0x10000000,
    GL_TEXCOORD2_BIT_PGI                                    = 0x20000000,
    GL_TEXCOORD3_BIT_PGI                                    = 0x40000000,
    GL_TEXCOORD4_BIT_PGI                                    = 0x80000000,
    GL_VERTEX23_BIT_PGI                                     = 0x00000004,
    GL_VERTEX4_BIT_PGI                                      = 0x00000008,
};
#endif

#ifndef GL_REND_screen_coordinates
#define GL_REND_screen_coordinates 1
enum : GLenum
{
    GL_SCREEN_COORDINATES_REND                              = 0x8490,
    GL_INVERTED_SCREEN_W_REND                               = 0x8491,
};
#endif

#ifndef GL_S3_s3tc
#define GL_S3_s3tc 1
enum : GLenum
{
    GL_RGB_S3TC                                             = 0x83A0,
    GL_RGB4_S3TC                                            = 0x83A1,
    GL_RGBA_S3TC                                            = 0x83A2,
    GL_RGBA4_S3TC                                           = 0x83A3,
    GL_RGBA_DXT5_S3TC                                       = 0x83A4,
    GL_RGBA4_DXT5_S3TC                                      = 0x83A5,
};
#endif

#ifndef GL_SGIS_detail_texture
#define GL_SGIS_detail_texture 1
enum : GLenum
{
    GL_DETAIL_TEXTURE_2D_SGIS                               = 0x8095,
    GL_DETAIL_TEXTURE_2D_BINDING_SGIS                       = 0x8096,
    GL_LINEAR_DETAIL_SGIS                                   = 0x8097,
    GL_LINEAR_DETAIL_ALPHA_SGIS                             = 0x8098,
    GL_LINEAR_DETAIL_COLOR_SGIS                             = 0x8099,
    GL_DETAIL_TEXTURE_LEVEL_SGIS                            = 0x809A,
    GL_DETAIL_TEXTURE_MODE_SGIS                             = 0x809B,
    GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS                      = 0x809C,
};
extern void         (KHRONOS_APIENTRY* const& glDetailTexFuncSGIS) (GLenum target, GLsizei n, const GLfloat *points);
extern void         (KHRONOS_APIENTRY* const& glGetDetailTexFuncSGIS) (GLenum target, GLfloat *points);
#endif

#ifndef GL_SGIS_fog_function
#define GL_SGIS_fog_function 1
enum : GLenum
{
    GL_FOG_FUNC_SGIS                                        = 0x812A,
    GL_FOG_FUNC_POINTS_SGIS                                 = 0x812B,
    GL_MAX_FOG_FUNC_POINTS_SGIS                             = 0x812C,
};
extern void         (KHRONOS_APIENTRY* const& glFogFuncSGIS) (GLsizei n, const GLfloat *points);
extern void         (KHRONOS_APIENTRY* const& glGetFogFuncSGIS) (GLfloat *points);
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_SGIS_generate_mipmap 1
enum : GLenum
{
    GL_GENERATE_MIPMAP_SGIS                                 = 0x8191,
    GL_GENERATE_MIPMAP_HINT_SGIS                            = 0x8192,
};
#endif

#ifndef GL_SGIS_multisample
#define GL_SGIS_multisample 1
enum : GLenum
{
    GL_MULTISAMPLE_SGIS                                     = 0x809D,
    GL_SAMPLE_ALPHA_TO_MASK_SGIS                            = 0x809E,
    GL_SAMPLE_ALPHA_TO_ONE_SGIS                             = 0x809F,
    GL_SAMPLE_MASK_SGIS                                     = 0x80A0,
    GL_1PASS_SGIS                                           = 0x80A1,
    GL_2PASS_0_SGIS                                         = 0x80A2,
    GL_2PASS_1_SGIS                                         = 0x80A3,
    GL_4PASS_0_SGIS                                         = 0x80A4,
    GL_4PASS_1_SGIS                                         = 0x80A5,
    GL_4PASS_2_SGIS                                         = 0x80A6,
    GL_4PASS_3_SGIS                                         = 0x80A7,
    GL_SAMPLE_BUFFERS_SGIS                                  = 0x80A8,
    GL_SAMPLES_SGIS                                         = 0x80A9,
    GL_SAMPLE_MASK_VALUE_SGIS                               = 0x80AA,
    GL_SAMPLE_MASK_INVERT_SGIS                              = 0x80AB,
    GL_SAMPLE_PATTERN_SGIS                                  = 0x80AC,
};
extern void         (KHRONOS_APIENTRY* const& glSampleMaskSGIS) (GLclampf value, GLboolean invert);
extern void         (KHRONOS_APIENTRY* const& glSamplePatternSGIS) (GLenum pattern);
#endif

#ifndef GL_SGIS_pixel_texture
#define GL_SGIS_pixel_texture 1
enum : GLenum
{
    GL_PIXEL_TEXTURE_SGIS                                   = 0x8353,
    GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS                       = 0x8354,
    GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS                     = 0x8355,
    GL_PIXEL_GROUP_COLOR_SGIS                               = 0x8356,
};
extern void         (KHRONOS_APIENTRY* const& glPixelTexGenParameteriSGIS) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glPixelTexGenParameterivSGIS) (GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glPixelTexGenParameterfSGIS) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPixelTexGenParameterfvSGIS) (GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetPixelTexGenParameterivSGIS) (GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetPixelTexGenParameterfvSGIS) (GLenum pname, GLfloat *params);
#endif

#ifndef GL_SGIS_point_line_texgen
#define GL_SGIS_point_line_texgen 1
enum : GLenum
{
    GL_EYE_DISTANCE_TO_POINT_SGIS                           = 0x81F0,
    GL_OBJECT_DISTANCE_TO_POINT_SGIS                        = 0x81F1,
    GL_EYE_DISTANCE_TO_LINE_SGIS                            = 0x81F2,
    GL_OBJECT_DISTANCE_TO_LINE_SGIS                         = 0x81F3,
    GL_EYE_POINT_SGIS                                       = 0x81F4,
    GL_OBJECT_POINT_SGIS                                    = 0x81F5,
    GL_EYE_LINE_SGIS                                        = 0x81F6,
    GL_OBJECT_LINE_SGIS                                     = 0x81F7,
};
#endif

#ifndef GL_SGIS_point_parameters
#define GL_SGIS_point_parameters 1
enum : GLenum
{
    GL_POINT_SIZE_MIN_SGIS                                  = 0x8126,
    GL_POINT_SIZE_MAX_SGIS                                  = 0x8127,
    GL_POINT_FADE_THRESHOLD_SIZE_SGIS                       = 0x8128,
    GL_DISTANCE_ATTENUATION_SGIS                            = 0x8129,
};
extern void         (KHRONOS_APIENTRY* const& glPointParameterfSGIS) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glPointParameterfvSGIS) (GLenum pname, const GLfloat *params);
#endif

#ifndef GL_SGIS_sharpen_texture
#define GL_SGIS_sharpen_texture 1
enum : GLenum
{
    GL_LINEAR_SHARPEN_SGIS                                  = 0x80AD,
    GL_LINEAR_SHARPEN_ALPHA_SGIS                            = 0x80AE,
    GL_LINEAR_SHARPEN_COLOR_SGIS                            = 0x80AF,
    GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS                     = 0x80B0,
};
extern void         (KHRONOS_APIENTRY* const& glSharpenTexFuncSGIS) (GLenum target, GLsizei n, const GLfloat *points);
extern void         (KHRONOS_APIENTRY* const& glGetSharpenTexFuncSGIS) (GLenum target, GLfloat *points);
#endif

#ifndef GL_SGIS_texture4D
#define GL_SGIS_texture4D 1
enum : GLenum
{
    GL_PACK_SKIP_VOLUMES_SGIS                               = 0x8130,
    GL_PACK_IMAGE_DEPTH_SGIS                                = 0x8131,
    GL_UNPACK_SKIP_VOLUMES_SGIS                             = 0x8132,
    GL_UNPACK_IMAGE_DEPTH_SGIS                              = 0x8133,
    GL_TEXTURE_4D_SGIS                                      = 0x8134,
    GL_PROXY_TEXTURE_4D_SGIS                                = 0x8135,
    GL_TEXTURE_4DSIZE_SGIS                                  = 0x8136,
    GL_TEXTURE_WRAP_Q_SGIS                                  = 0x8137,
    GL_MAX_4D_TEXTURE_SIZE_SGIS                             = 0x8138,
    GL_TEXTURE_4D_BINDING_SGIS                              = 0x814F,
};
extern void         (KHRONOS_APIENTRY* const& glTexImage4DSGIS) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage4DSGIS) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const void *pixels);
#endif

#ifndef GL_SGIS_texture_border_clamp
#define GL_SGIS_texture_border_clamp 1
enum : GLenum
{
    GL_CLAMP_TO_BORDER_SGIS                                 = 0x812D,
};
#endif

#ifndef GL_SGIS_texture_color_mask
#define GL_SGIS_texture_color_mask 1
enum : GLenum
{
    GL_TEXTURE_COLOR_WRITEMASK_SGIS                         = 0x81EF,
};
extern void         (KHRONOS_APIENTRY* const& glTextureColorMaskSGIS) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
#endif

#ifndef GL_SGIS_texture_edge_clamp
#define GL_SGIS_texture_edge_clamp 1
enum : GLenum
{
    GL_CLAMP_TO_EDGE_SGIS                                   = 0x812F,
};
#endif

#ifndef GL_SGIS_texture_filter4
#define GL_SGIS_texture_filter4 1
enum : GLenum
{
    GL_FILTER4_SGIS                                         = 0x8146,
    GL_TEXTURE_FILTER4_SIZE_SGIS                            = 0x8147,
};
extern void         (KHRONOS_APIENTRY* const& glGetTexFilterFuncSGIS) (GLenum target, GLenum filter, GLfloat *weights);
extern void         (KHRONOS_APIENTRY* const& glTexFilterFuncSGIS) (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights);
#endif

#ifndef GL_SGIS_texture_lod
#define GL_SGIS_texture_lod 1
enum : GLenum
{
    GL_TEXTURE_MIN_LOD_SGIS                                 = 0x813A,
    GL_TEXTURE_MAX_LOD_SGIS                                 = 0x813B,
    GL_TEXTURE_BASE_LEVEL_SGIS                              = 0x813C,
    GL_TEXTURE_MAX_LEVEL_SGIS                               = 0x813D,
};
#endif

#ifndef GL_SGIS_texture_select
#define GL_SGIS_texture_select 1
enum : GLenum
{
    GL_DUAL_ALPHA4_SGIS                                     = 0x8110,
    GL_DUAL_ALPHA8_SGIS                                     = 0x8111,
    GL_DUAL_ALPHA12_SGIS                                    = 0x8112,
    GL_DUAL_ALPHA16_SGIS                                    = 0x8113,
    GL_DUAL_LUMINANCE4_SGIS                                 = 0x8114,
    GL_DUAL_LUMINANCE8_SGIS                                 = 0x8115,
    GL_DUAL_LUMINANCE12_SGIS                                = 0x8116,
    GL_DUAL_LUMINANCE16_SGIS                                = 0x8117,
    GL_DUAL_INTENSITY4_SGIS                                 = 0x8118,
    GL_DUAL_INTENSITY8_SGIS                                 = 0x8119,
    GL_DUAL_INTENSITY12_SGIS                                = 0x811A,
    GL_DUAL_INTENSITY16_SGIS                                = 0x811B,
    GL_DUAL_LUMINANCE_ALPHA4_SGIS                           = 0x811C,
    GL_DUAL_LUMINANCE_ALPHA8_SGIS                           = 0x811D,
    GL_QUAD_ALPHA4_SGIS                                     = 0x811E,
    GL_QUAD_ALPHA8_SGIS                                     = 0x811F,
    GL_QUAD_LUMINANCE4_SGIS                                 = 0x8120,
    GL_QUAD_LUMINANCE8_SGIS                                 = 0x8121,
    GL_QUAD_INTENSITY4_SGIS                                 = 0x8122,
    GL_QUAD_INTENSITY8_SGIS                                 = 0x8123,
    GL_DUAL_TEXTURE_SELECT_SGIS                             = 0x8124,
    GL_QUAD_TEXTURE_SELECT_SGIS                             = 0x8125,
};
#endif

#ifndef GL_SGIX_async
#define GL_SGIX_async 1
enum : GLenum
{
    GL_ASYNC_MARKER_SGIX                                    = 0x8329,
};
extern void         (KHRONOS_APIENTRY* const& glAsyncMarkerSGIX) (GLuint marker);
extern GLint        (KHRONOS_APIENTRY* const& glFinishAsyncSGIX) (GLuint *markerp);
extern GLint        (KHRONOS_APIENTRY* const& glPollAsyncSGIX) (GLuint *markerp);
extern GLuint       (KHRONOS_APIENTRY* const& glGenAsyncMarkersSGIX) (GLsizei range);
extern void         (KHRONOS_APIENTRY* const& glDeleteAsyncMarkersSGIX) (GLuint marker, GLsizei range);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsAsyncMarkerSGIX) (GLuint marker);
#endif

#ifndef GL_SGIX_async_histogram
#define GL_SGIX_async_histogram 1
enum : GLenum
{
    GL_ASYNC_HISTOGRAM_SGIX                                 = 0x832C,
    GL_MAX_ASYNC_HISTOGRAM_SGIX                             = 0x832D,
};
#endif

#ifndef GL_SGIX_async_pixel
#define GL_SGIX_async_pixel 1
enum : GLenum
{
    GL_ASYNC_TEX_IMAGE_SGIX                                 = 0x835C,
    GL_ASYNC_DRAW_PIXELS_SGIX                               = 0x835D,
    GL_ASYNC_READ_PIXELS_SGIX                               = 0x835E,
    GL_MAX_ASYNC_TEX_IMAGE_SGIX                             = 0x835F,
    GL_MAX_ASYNC_DRAW_PIXELS_SGIX                           = 0x8360,
    GL_MAX_ASYNC_READ_PIXELS_SGIX                           = 0x8361,
};
#endif

#ifndef GL_SGIX_blend_alpha_minmax
#define GL_SGIX_blend_alpha_minmax 1
enum : GLenum
{
    GL_ALPHA_MIN_SGIX                                       = 0x8320,
    GL_ALPHA_MAX_SGIX                                       = 0x8321,
};
#endif

#ifndef GL_SGIX_calligraphic_fragment
#define GL_SGIX_calligraphic_fragment 1
enum : GLenum
{
    GL_CALLIGRAPHIC_FRAGMENT_SGIX                           = 0x8183,
};
#endif

#ifndef GL_SGIX_clipmap
#define GL_SGIX_clipmap 1
enum : GLenum
{
    GL_LINEAR_CLIPMAP_LINEAR_SGIX                           = 0x8170,
    GL_TEXTURE_CLIPMAP_CENTER_SGIX                          = 0x8171,
    GL_TEXTURE_CLIPMAP_FRAME_SGIX                           = 0x8172,
    GL_TEXTURE_CLIPMAP_OFFSET_SGIX                          = 0x8173,
    GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX                   = 0x8174,
    GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX                      = 0x8175,
    GL_TEXTURE_CLIPMAP_DEPTH_SGIX                           = 0x8176,
    GL_MAX_CLIPMAP_DEPTH_SGIX                               = 0x8177,
    GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX                       = 0x8178,
    GL_NEAREST_CLIPMAP_NEAREST_SGIX                         = 0x844D,
    GL_NEAREST_CLIPMAP_LINEAR_SGIX                          = 0x844E,
    GL_LINEAR_CLIPMAP_NEAREST_SGIX                          = 0x844F,
};
#endif

#ifndef GL_SGIX_convolution_accuracy
#define GL_SGIX_convolution_accuracy 1
enum : GLenum
{
    GL_CONVOLUTION_HINT_SGIX                                = 0x8316,
};
#endif

#ifndef GL_SGIX_depth_pass_instrument
#define GL_SGIX_depth_pass_instrument 1
#endif

#ifndef GL_SGIX_depth_texture
#define GL_SGIX_depth_texture 1
enum : GLenum
{
    GL_DEPTH_COMPONENT16_SGIX                               = 0x81A5,
    GL_DEPTH_COMPONENT24_SGIX                               = 0x81A6,
    GL_DEPTH_COMPONENT32_SGIX                               = 0x81A7,
};
#endif

#ifndef GL_SGIX_flush_raster
#define GL_SGIX_flush_raster 1
extern void         (KHRONOS_APIENTRY* const& glFlushRasterSGIX) ();
#endif

#ifndef GL_SGIX_fog_offset
#define GL_SGIX_fog_offset 1
enum : GLenum
{
    GL_FOG_OFFSET_SGIX                                      = 0x8198,
    GL_FOG_OFFSET_VALUE_SGIX                                = 0x8199,
};
#endif

#ifndef GL_SGIX_fragment_lighting
#define GL_SGIX_fragment_lighting 1
enum : GLenum
{
    GL_FRAGMENT_LIGHTING_SGIX                               = 0x8400,
    GL_FRAGMENT_COLOR_MATERIAL_SGIX                         = 0x8401,
    GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX                    = 0x8402,
    GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX               = 0x8403,
    GL_MAX_FRAGMENT_LIGHTS_SGIX                             = 0x8404,
    GL_MAX_ACTIVE_LIGHTS_SGIX                               = 0x8405,
    GL_CURRENT_RASTER_NORMAL_SGIX                           = 0x8406,
    GL_LIGHT_ENV_MODE_SGIX                                  = 0x8407,
    GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX               = 0x8408,
    GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX                   = 0x8409,
    GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX                    = 0x840A,
    GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX       = 0x840B,
    GL_FRAGMENT_LIGHT0_SGIX                                 = 0x840C,
    GL_FRAGMENT_LIGHT1_SGIX                                 = 0x840D,
    GL_FRAGMENT_LIGHT2_SGIX                                 = 0x840E,
    GL_FRAGMENT_LIGHT3_SGIX                                 = 0x840F,
    GL_FRAGMENT_LIGHT4_SGIX                                 = 0x8410,
    GL_FRAGMENT_LIGHT5_SGIX                                 = 0x8411,
    GL_FRAGMENT_LIGHT6_SGIX                                 = 0x8412,
    GL_FRAGMENT_LIGHT7_SGIX                                 = 0x8413,
};
extern void         (KHRONOS_APIENTRY* const& glFragmentColorMaterialSGIX) (GLenum face, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightfSGIX) (GLenum light, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightfvSGIX) (GLenum light, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightiSGIX) (GLenum light, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightivSGIX) (GLenum light, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightModelfSGIX) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightModelfvSGIX) (GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightModeliSGIX) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glFragmentLightModelivSGIX) (GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glFragmentMaterialfSGIX) (GLenum face, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glFragmentMaterialfvSGIX) (GLenum face, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glFragmentMaterialiSGIX) (GLenum face, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glFragmentMaterialivSGIX) (GLenum face, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetFragmentLightfvSGIX) (GLenum light, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetFragmentLightivSGIX) (GLenum light, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetFragmentMaterialfvSGIX) (GLenum face, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetFragmentMaterialivSGIX) (GLenum face, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glLightEnviSGIX) (GLenum pname, GLint param);
#endif

#ifndef GL_SGIX_framezoom
#define GL_SGIX_framezoom 1
enum : GLenum
{
    GL_FRAMEZOOM_SGIX                                       = 0x818B,
    GL_FRAMEZOOM_FACTOR_SGIX                                = 0x818C,
    GL_MAX_FRAMEZOOM_FACTOR_SGIX                            = 0x818D,
};
extern void         (KHRONOS_APIENTRY* const& glFrameZoomSGIX) (GLint factor);
#endif

#ifndef GL_SGIX_igloo_interface
#define GL_SGIX_igloo_interface 1
extern void         (KHRONOS_APIENTRY* const& glIglooInterfaceSGIX) (GLenum pname, const void *params);
#endif

#ifndef GL_SGIX_instruments
#define GL_SGIX_instruments 1
enum : GLenum
{
    GL_INSTRUMENT_BUFFER_POINTER_SGIX                       = 0x8180,
    GL_INSTRUMENT_MEASUREMENTS_SGIX                         = 0x8181,
};
extern GLint        (KHRONOS_APIENTRY* const& glGetInstrumentsSGIX) ();
extern void         (KHRONOS_APIENTRY* const& glInstrumentsBufferSGIX) (GLsizei size, GLint *buffer);
extern GLint        (KHRONOS_APIENTRY* const& glPollInstrumentsSGIX) (GLint *marker_p);
extern void         (KHRONOS_APIENTRY* const& glReadInstrumentsSGIX) (GLint marker);
extern void         (KHRONOS_APIENTRY* const& glStartInstrumentsSGIX) ();
extern void         (KHRONOS_APIENTRY* const& glStopInstrumentsSGIX) (GLint marker);
#endif

#ifndef GL_SGIX_interlace
#define GL_SGIX_interlace 1
enum : GLenum
{
    GL_INTERLACE_SGIX                                       = 0x8094,
};
#endif

#ifndef GL_SGIX_ir_instrument1
#define GL_SGIX_ir_instrument1 1
enum : GLenum
{
    GL_IR_INSTRUMENT1_SGIX                                  = 0x817F,
};
#endif

#ifndef GL_SGIX_list_priority
#define GL_SGIX_list_priority 1
enum : GLenum
{
    GL_LIST_PRIORITY_SGIX                                   = 0x8182,
};
extern void         (KHRONOS_APIENTRY* const& glGetListParameterfvSGIX) (GLuint list, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetListParameterivSGIX) (GLuint list, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glListParameterfSGIX) (GLuint list, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glListParameterfvSGIX) (GLuint list, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glListParameteriSGIX) (GLuint list, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glListParameterivSGIX) (GLuint list, GLenum pname, const GLint *params);
#endif

#ifndef GL_SGIX_pixel_texture
#define GL_SGIX_pixel_texture 1
enum : GLenum
{
    GL_PIXEL_TEX_GEN_SGIX                                   = 0x8139,
    GL_PIXEL_TEX_GEN_MODE_SGIX                              = 0x832B,
};
extern void         (KHRONOS_APIENTRY* const& glPixelTexGenSGIX) (GLenum mode);
#endif

#ifndef GL_SGIX_pixel_tiles
#define GL_SGIX_pixel_tiles 1
enum : GLenum
{
    GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX                       = 0x813E,
    GL_PIXEL_TILE_CACHE_INCREMENT_SGIX                      = 0x813F,
    GL_PIXEL_TILE_WIDTH_SGIX                                = 0x8140,
    GL_PIXEL_TILE_HEIGHT_SGIX                               = 0x8141,
    GL_PIXEL_TILE_GRID_WIDTH_SGIX                           = 0x8142,
    GL_PIXEL_TILE_GRID_HEIGHT_SGIX                          = 0x8143,
    GL_PIXEL_TILE_GRID_DEPTH_SGIX                           = 0x8144,
    GL_PIXEL_TILE_CACHE_SIZE_SGIX                           = 0x8145,
};
#endif

#ifndef GL_SGIX_polynomial_ffd
#define GL_SGIX_polynomial_ffd 1
enum : GLenum
{
    GL_TEXTURE_DEFORMATION_BIT_SGIX                         = 0x00000001,
    GL_GEOMETRY_DEFORMATION_BIT_SGIX                        = 0x00000002,
    GL_GEOMETRY_DEFORMATION_SGIX                            = 0x8194,
    GL_TEXTURE_DEFORMATION_SGIX                             = 0x8195,
    GL_DEFORMATIONS_MASK_SGIX                               = 0x8196,
    GL_MAX_DEFORMATION_ORDER_SGIX                           = 0x8197,
};
extern void         (KHRONOS_APIENTRY* const& glDeformationMap3dSGIX) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble *points);
extern void         (KHRONOS_APIENTRY* const& glDeformationMap3fSGIX) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat *points);
extern void         (KHRONOS_APIENTRY* const& glDeformSGIX) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glLoadIdentityDeformationMapSGIX) (GLbitfield mask);
#endif

#ifndef GL_SGIX_reference_plane
#define GL_SGIX_reference_plane 1
enum : GLenum
{
    GL_REFERENCE_PLANE_SGIX                                 = 0x817D,
    GL_REFERENCE_PLANE_EQUATION_SGIX                        = 0x817E,
};
extern void         (KHRONOS_APIENTRY* const& glReferencePlaneSGIX) (const GLdouble *equation);
#endif

#ifndef GL_SGIX_resample
#define GL_SGIX_resample 1
enum : GLenum
{
    GL_PACK_RESAMPLE_SGIX                                   = 0x842E,
    GL_UNPACK_RESAMPLE_SGIX                                 = 0x842F,
    GL_RESAMPLE_REPLICATE_SGIX                              = 0x8433,
    GL_RESAMPLE_ZERO_FILL_SGIX                              = 0x8434,
    GL_RESAMPLE_DECIMATE_SGIX                               = 0x8430,
};
#endif

#ifndef GL_SGIX_scalebias_hint
#define GL_SGIX_scalebias_hint 1
enum : GLenum
{
    GL_SCALEBIAS_HINT_SGIX                                  = 0x8322,
};
#endif

#ifndef GL_SGIX_shadow
#define GL_SGIX_shadow 1
enum : GLenum
{
    GL_TEXTURE_COMPARE_SGIX                                 = 0x819A,
    GL_TEXTURE_COMPARE_OPERATOR_SGIX                        = 0x819B,
    GL_TEXTURE_LEQUAL_R_SGIX                                = 0x819C,
    GL_TEXTURE_GEQUAL_R_SGIX                                = 0x819D,
};
#endif

#ifndef GL_SGIX_shadow_ambient
#define GL_SGIX_shadow_ambient 1
enum : GLenum
{
    GL_SHADOW_AMBIENT_SGIX                                  = 0x80BF,
};
#endif

#ifndef GL_SGIX_sprite
#define GL_SGIX_sprite 1
enum : GLenum
{
    GL_SPRITE_SGIX                                          = 0x8148,
    GL_SPRITE_MODE_SGIX                                     = 0x8149,
    GL_SPRITE_AXIS_SGIX                                     = 0x814A,
    GL_SPRITE_TRANSLATION_SGIX                              = 0x814B,
    GL_SPRITE_AXIAL_SGIX                                    = 0x814C,
    GL_SPRITE_OBJECT_ALIGNED_SGIX                           = 0x814D,
    GL_SPRITE_EYE_ALIGNED_SGIX                              = 0x814E,
};
extern void         (KHRONOS_APIENTRY* const& glSpriteParameterfSGIX) (GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glSpriteParameterfvSGIX) (GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glSpriteParameteriSGIX) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glSpriteParameterivSGIX) (GLenum pname, const GLint *params);
#endif

#ifndef GL_SGIX_subsample
#define GL_SGIX_subsample 1
enum : GLenum
{
    GL_PACK_SUBSAMPLE_RATE_SGIX                             = 0x85A0,
    GL_UNPACK_SUBSAMPLE_RATE_SGIX                           = 0x85A1,
    GL_PIXEL_SUBSAMPLE_4444_SGIX                            = 0x85A2,
    GL_PIXEL_SUBSAMPLE_2424_SGIX                            = 0x85A3,
    GL_PIXEL_SUBSAMPLE_4242_SGIX                            = 0x85A4,
};
#endif

#ifndef GL_SGIX_tag_sample_buffer
#define GL_SGIX_tag_sample_buffer 1
extern void         (KHRONOS_APIENTRY* const& glTagSampleBufferSGIX) ();
#endif

#ifndef GL_SGIX_texture_add_env
#define GL_SGIX_texture_add_env 1
enum : GLenum
{
    GL_TEXTURE_ENV_BIAS_SGIX                                = 0x80BE,
};
#endif

#ifndef GL_SGIX_texture_coordinate_clamp
#define GL_SGIX_texture_coordinate_clamp 1
enum : GLenum
{
    GL_TEXTURE_MAX_CLAMP_S_SGIX                             = 0x8369,
    GL_TEXTURE_MAX_CLAMP_T_SGIX                             = 0x836A,
    GL_TEXTURE_MAX_CLAMP_R_SGIX                             = 0x836B,
};
#endif

#ifndef GL_SGIX_texture_lod_bias
#define GL_SGIX_texture_lod_bias 1
enum : GLenum
{
    GL_TEXTURE_LOD_BIAS_S_SGIX                              = 0x818E,
    GL_TEXTURE_LOD_BIAS_T_SGIX                              = 0x818F,
    GL_TEXTURE_LOD_BIAS_R_SGIX                              = 0x8190,
};
#endif

#ifndef GL_SGIX_texture_multi_buffer
#define GL_SGIX_texture_multi_buffer 1
enum : GLenum
{
    GL_TEXTURE_MULTI_BUFFER_HINT_SGIX                       = 0x812E,
};
#endif

#ifndef GL_SGIX_texture_scale_bias
#define GL_SGIX_texture_scale_bias 1
enum : GLenum
{
    GL_POST_TEXTURE_FILTER_BIAS_SGIX                        = 0x8179,
    GL_POST_TEXTURE_FILTER_SCALE_SGIX                       = 0x817A,
    GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX                  = 0x817B,
    GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX                 = 0x817C,
};
#endif

#ifndef GL_SGIX_vertex_preclip
#define GL_SGIX_vertex_preclip 1
enum : GLenum
{
    GL_VERTEX_PRECLIP_SGIX                                  = 0x83EE,
    GL_VERTEX_PRECLIP_HINT_SGIX                             = 0x83EF,
};
#endif

#ifndef GL_SGIX_ycrcb
#define GL_SGIX_ycrcb 1
enum : GLenum
{
    GL_YCRCB_422_SGIX                                       = 0x81BB,
    GL_YCRCB_444_SGIX                                       = 0x81BC,
};
#endif

#ifndef GL_SGIX_ycrcb_subsample
#define GL_SGIX_ycrcb_subsample 1
#endif

#ifndef GL_SGIX_ycrcba
#define GL_SGIX_ycrcba 1
enum : GLenum
{
    GL_YCRCB_SGIX                                           = 0x8318,
    GL_YCRCBA_SGIX                                          = 0x8319,
};
#endif

#ifndef GL_SGI_color_matrix
#define GL_SGI_color_matrix 1
enum : GLenum
{
    GL_COLOR_MATRIX_SGI                                     = 0x80B1,
    GL_COLOR_MATRIX_STACK_DEPTH_SGI                         = 0x80B2,
    GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI                     = 0x80B3,
    GL_POST_COLOR_MATRIX_RED_SCALE_SGI                      = 0x80B4,
    GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI                    = 0x80B5,
    GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI                     = 0x80B6,
    GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI                    = 0x80B7,
    GL_POST_COLOR_MATRIX_RED_BIAS_SGI                       = 0x80B8,
    GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI                     = 0x80B9,
    GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI                      = 0x80BA,
    GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI                     = 0x80BB,
};
#endif

#ifndef GL_SGI_color_table
#define GL_SGI_color_table 1
enum : GLenum
{
    GL_COLOR_TABLE_SGI                                      = 0x80D0,
    GL_POST_CONVOLUTION_COLOR_TABLE_SGI                     = 0x80D1,
    GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI                    = 0x80D2,
    GL_PROXY_COLOR_TABLE_SGI                                = 0x80D3,
    GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI               = 0x80D4,
    GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI              = 0x80D5,
    GL_COLOR_TABLE_SCALE_SGI                                = 0x80D6,
    GL_COLOR_TABLE_BIAS_SGI                                 = 0x80D7,
    GL_COLOR_TABLE_FORMAT_SGI                               = 0x80D8,
    GL_COLOR_TABLE_WIDTH_SGI                                = 0x80D9,
    GL_COLOR_TABLE_RED_SIZE_SGI                             = 0x80DA,
    GL_COLOR_TABLE_GREEN_SIZE_SGI                           = 0x80DB,
    GL_COLOR_TABLE_BLUE_SIZE_SGI                            = 0x80DC,
    GL_COLOR_TABLE_ALPHA_SIZE_SGI                           = 0x80DD,
    GL_COLOR_TABLE_LUMINANCE_SIZE_SGI                       = 0x80DE,
    GL_COLOR_TABLE_INTENSITY_SIZE_SGI                       = 0x80DF,
};
extern void         (KHRONOS_APIENTRY* const& glColorTableSGI) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *table);
extern void         (KHRONOS_APIENTRY* const& glColorTableParameterfvSGI) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glColorTableParameterivSGI) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glCopyColorTableSGI) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableSGI) (GLenum target, GLenum format, GLenum type, void *table);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableParameterfvSGI) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetColorTableParameterivSGI) (GLenum target, GLenum pname, GLint *params);
#endif

#ifndef GL_SGI_texture_color_table
#define GL_SGI_texture_color_table 1
enum : GLenum
{
    GL_TEXTURE_COLOR_TABLE_SGI                              = 0x80BC,
    GL_PROXY_TEXTURE_COLOR_TABLE_SGI                        = 0x80BD,
};
#endif

#ifndef GL_SUNX_constant_data
#define GL_SUNX_constant_data 1
enum : GLenum
{
    GL_UNPACK_CONSTANT_DATA_SUNX                            = 0x81D5,
    GL_TEXTURE_CONSTANT_DATA_SUNX                           = 0x81D6,
};
extern void         (KHRONOS_APIENTRY* const& glFinishTextureSUNX) ();
#endif

#ifndef GL_SUN_convolution_border_modes
#define GL_SUN_convolution_border_modes 1
enum : GLenum
{
    GL_WRAP_BORDER_SUN                                      = 0x81D4,
};
#endif

#ifndef GL_SUN_global_alpha
#define GL_SUN_global_alpha 1
enum : GLenum
{
    GL_GLOBAL_ALPHA_SUN                                     = 0x81D9,
    GL_GLOBAL_ALPHA_FACTOR_SUN                              = 0x81DA,
};
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactorbSUN) (GLbyte factor);
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactorsSUN) (GLshort factor);
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactoriSUN) (GLint factor);
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactorfSUN) (GLfloat factor);
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactordSUN) (GLdouble factor);
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactorubSUN) (GLubyte factor);
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactorusSUN) (GLushort factor);
extern void         (KHRONOS_APIENTRY* const& glGlobalAlphaFactoruiSUN) (GLuint factor);
#endif

#ifndef GL_SUN_mesh_array
#define GL_SUN_mesh_array 1
enum : GLenum
{
    GL_QUAD_MESH_SUN                                        = 0x8614,
    GL_TRIANGLE_MESH_SUN                                    = 0x8615,
};
extern void         (KHRONOS_APIENTRY* const& glDrawMeshArraysSUN) (GLenum mode, GLint first, GLsizei count, GLsizei width);
#endif

#ifndef GL_SUN_slice_accum
#define GL_SUN_slice_accum 1
enum : GLenum
{
    GL_SLICE_ACCUM_SUN                                      = 0x85CC,
};
#endif

#ifndef GL_SUN_triangle_list
#define GL_SUN_triangle_list 1
enum : GLenum
{
    GL_RESTART_SUN                                          = 0x0001,
    GL_REPLACE_MIDDLE_SUN                                   = 0x0002,
    GL_REPLACE_OLDEST_SUN                                   = 0x0003,
    GL_TRIANGLE_LIST_SUN                                    = 0x81D7,
    GL_REPLACEMENT_CODE_SUN                                 = 0x81D8,
    GL_REPLACEMENT_CODE_ARRAY_SUN                           = 0x85C0,
    GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN                      = 0x85C1,
    GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN                    = 0x85C2,
    GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN                   = 0x85C3,
    GL_R1UI_V3F_SUN                                         = 0x85C4,
    GL_R1UI_C4UB_V3F_SUN                                    = 0x85C5,
    GL_R1UI_C3F_V3F_SUN                                     = 0x85C6,
    GL_R1UI_N3F_V3F_SUN                                     = 0x85C7,
    GL_R1UI_C4F_N3F_V3F_SUN                                 = 0x85C8,
    GL_R1UI_T2F_V3F_SUN                                     = 0x85C9,
    GL_R1UI_T2F_N3F_V3F_SUN                                 = 0x85CA,
    GL_R1UI_T2F_C4F_N3F_V3F_SUN                             = 0x85CB,
};
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiSUN) (GLuint code);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeusSUN) (GLushort code);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeubSUN) (GLubyte code);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuivSUN) (const GLuint *code);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeusvSUN) (const GLushort *code);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeubvSUN) (const GLubyte *code);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodePointerSUN) (GLenum type, GLsizei stride, const void **pointer);
#endif

#ifndef GL_SUN_vertex
#define GL_SUN_vertex 1
extern void         (KHRONOS_APIENTRY* const& glColor4ubVertex2fSUN) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glColor4ubVertex2fvSUN) (const GLubyte *c, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glColor4ubVertex3fSUN) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glColor4ubVertex3fvSUN) (const GLubyte *c, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glColor3fVertex3fSUN) (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glColor3fVertex3fvSUN) (const GLfloat *c, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glNormal3fVertex3fSUN) (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glNormal3fVertex3fvSUN) (const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glColor4fNormal3fVertex3fSUN) (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glColor4fNormal3fVertex3fvSUN) (const GLfloat *c, const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fVertex3fvSUN) (const GLfloat *tc, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4fVertex4fSUN) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4fVertex4fvSUN) (const GLfloat *tc, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fColor4ubVertex3fSUN) (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fColor4ubVertex3fvSUN) (const GLfloat *tc, const GLubyte *c, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fColor3fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fColor3fVertex3fvSUN) (const GLfloat *tc, const GLfloat *c, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fNormal3fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fNormal3fVertex3fvSUN) (const GLfloat *tc, const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fColor4fNormal3fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glTexCoord2fColor4fNormal3fVertex3fvSUN) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4fColor4fNormal3fVertex4fSUN) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glTexCoord4fColor4fNormal3fVertex4fvSUN) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiVertex3fSUN) (GLuint rc, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiVertex3fvSUN) (const GLuint *rc, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiColor4ubVertex3fSUN) (GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiColor4ubVertex3fvSUN) (const GLuint *rc, const GLubyte *c, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiColor3fVertex3fSUN) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiColor3fVertex3fvSUN) (const GLuint *rc, const GLfloat *c, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiNormal3fVertex3fSUN) (GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiColor4fNormal3fVertex3fSUN) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiColor4fNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiTexCoord2fVertex3fSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiTexCoord2fVertex3fvSUN) (const GLuint *rc, const GLfloat *tc, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
#endif

#ifndef GL_WIN_phong_shading
#define GL_WIN_phong_shading 1
enum : GLenum
{
    GL_PHONG_WIN                                            = 0x80EA,
    GL_PHONG_HINT_WIN                                       = 0x80EB,
};
#endif

#ifndef GL_WIN_specular_fog
#define GL_WIN_specular_fog 1
enum : GLenum
{
    GL_FOG_SPECULAR_TEXTURE_WIN                             = 0x80EC,
};
#endif

#ifndef GL_EXT_texture_shadow_lod
#define GL_EXT_texture_shadow_lod 1
#endif


/** Load all available functions from the OpenGL core API.

    This will not load extensions!
*/
void loadFunctions();

/** Load all available OpenGL extension functions.

    It's probably a good idea to stick to the core API as much as possible.
    Extensions are not as portable, and it can be a little time-consuming to
    load all of the extension entry-points.
*/
void loadExtensions();

}
}
