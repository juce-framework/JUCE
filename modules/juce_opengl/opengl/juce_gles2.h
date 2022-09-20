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

#if defined (ES1_GL_H_GUARD) || defined (__gl_es20_h_) || defined (__gl_es30_h_)
#error gl.h included before juce_gles2.h
#endif
#if defined (__gltypes_h_)
#error gltypes.h included before juce_gles2.h
#endif

#define ES1_GL_H_GUARD
#define __gl_es20_h_
#define __gl_es30_h_

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

#ifndef GL_ES_VERSION_2_0
#define GL_ES_VERSION_2_0 1
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
    GL_FUNC_ADD                                             = 0x8006,
    GL_BLEND_EQUATION                                       = 0x8009,
    GL_BLEND_EQUATION_RGB                                   = 0x8009,
    GL_BLEND_EQUATION_ALPHA                                 = 0x883D,
    GL_FUNC_SUBTRACT                                        = 0x800A,
    GL_FUNC_REVERSE_SUBTRACT                                = 0x800B,
    GL_BLEND_DST_RGB                                        = 0x80C8,
    GL_BLEND_SRC_RGB                                        = 0x80C9,
    GL_BLEND_DST_ALPHA                                      = 0x80CA,
    GL_BLEND_SRC_ALPHA                                      = 0x80CB,
    GL_CONSTANT_COLOR                                       = 0x8001,
    GL_ONE_MINUS_CONSTANT_COLOR                             = 0x8002,
    GL_CONSTANT_ALPHA                                       = 0x8003,
    GL_ONE_MINUS_CONSTANT_ALPHA                             = 0x8004,
    GL_BLEND_COLOR                                          = 0x8005,
    GL_ARRAY_BUFFER                                         = 0x8892,
    GL_ELEMENT_ARRAY_BUFFER                                 = 0x8893,
    GL_ARRAY_BUFFER_BINDING                                 = 0x8894,
    GL_ELEMENT_ARRAY_BUFFER_BINDING                         = 0x8895,
    GL_STREAM_DRAW                                          = 0x88E0,
    GL_STATIC_DRAW                                          = 0x88E4,
    GL_DYNAMIC_DRAW                                         = 0x88E8,
    GL_BUFFER_SIZE                                          = 0x8764,
    GL_BUFFER_USAGE                                         = 0x8765,
    GL_CURRENT_VERTEX_ATTRIB                                = 0x8626,
    GL_FRONT                                                = 0x0404,
    GL_BACK                                                 = 0x0405,
    GL_FRONT_AND_BACK                                       = 0x0408,
    GL_TEXTURE_2D                                           = 0x0DE1,
    GL_CULL_FACE                                            = 0x0B44,
    GL_BLEND                                                = 0x0BE2,
    GL_DITHER                                               = 0x0BD0,
    GL_STENCIL_TEST                                         = 0x0B90,
    GL_DEPTH_TEST                                           = 0x0B71,
    GL_SCISSOR_TEST                                         = 0x0C11,
    GL_POLYGON_OFFSET_FILL                                  = 0x8037,
    GL_SAMPLE_ALPHA_TO_COVERAGE                             = 0x809E,
    GL_SAMPLE_COVERAGE                                      = 0x80A0,
    GL_NO_ERROR                                             = 0,
    GL_INVALID_ENUM                                         = 0x0500,
    GL_INVALID_VALUE                                        = 0x0501,
    GL_INVALID_OPERATION                                    = 0x0502,
    GL_OUT_OF_MEMORY                                        = 0x0505,
    GL_CW                                                   = 0x0900,
    GL_CCW                                                  = 0x0901,
    GL_LINE_WIDTH                                           = 0x0B21,
    GL_ALIASED_POINT_SIZE_RANGE                             = 0x846D,
    GL_ALIASED_LINE_WIDTH_RANGE                             = 0x846E,
    GL_CULL_FACE_MODE                                       = 0x0B45,
    GL_FRONT_FACE                                           = 0x0B46,
    GL_DEPTH_RANGE                                          = 0x0B70,
    GL_DEPTH_WRITEMASK                                      = 0x0B72,
    GL_DEPTH_CLEAR_VALUE                                    = 0x0B73,
    GL_DEPTH_FUNC                                           = 0x0B74,
    GL_STENCIL_CLEAR_VALUE                                  = 0x0B91,
    GL_STENCIL_FUNC                                         = 0x0B92,
    GL_STENCIL_FAIL                                         = 0x0B94,
    GL_STENCIL_PASS_DEPTH_FAIL                              = 0x0B95,
    GL_STENCIL_PASS_DEPTH_PASS                              = 0x0B96,
    GL_STENCIL_REF                                          = 0x0B97,
    GL_STENCIL_VALUE_MASK                                   = 0x0B93,
    GL_STENCIL_WRITEMASK                                    = 0x0B98,
    GL_STENCIL_BACK_FUNC                                    = 0x8800,
    GL_STENCIL_BACK_FAIL                                    = 0x8801,
    GL_STENCIL_BACK_PASS_DEPTH_FAIL                         = 0x8802,
    GL_STENCIL_BACK_PASS_DEPTH_PASS                         = 0x8803,
    GL_STENCIL_BACK_REF                                     = 0x8CA3,
    GL_STENCIL_BACK_VALUE_MASK                              = 0x8CA4,
    GL_STENCIL_BACK_WRITEMASK                               = 0x8CA5,
    GL_VIEWPORT                                             = 0x0BA2,
    GL_SCISSOR_BOX                                          = 0x0C10,
    GL_COLOR_CLEAR_VALUE                                    = 0x0C22,
    GL_COLOR_WRITEMASK                                      = 0x0C23,
    GL_UNPACK_ALIGNMENT                                     = 0x0CF5,
    GL_PACK_ALIGNMENT                                       = 0x0D05,
    GL_MAX_TEXTURE_SIZE                                     = 0x0D33,
    GL_MAX_VIEWPORT_DIMS                                    = 0x0D3A,
    GL_SUBPIXEL_BITS                                        = 0x0D50,
    GL_RED_BITS                                             = 0x0D52,
    GL_GREEN_BITS                                           = 0x0D53,
    GL_BLUE_BITS                                            = 0x0D54,
    GL_ALPHA_BITS                                           = 0x0D55,
    GL_DEPTH_BITS                                           = 0x0D56,
    GL_STENCIL_BITS                                         = 0x0D57,
    GL_POLYGON_OFFSET_UNITS                                 = 0x2A00,
    GL_POLYGON_OFFSET_FACTOR                                = 0x8038,
    GL_TEXTURE_BINDING_2D                                   = 0x8069,
    GL_SAMPLE_BUFFERS                                       = 0x80A8,
    GL_SAMPLES                                              = 0x80A9,
    GL_SAMPLE_COVERAGE_VALUE                                = 0x80AA,
    GL_SAMPLE_COVERAGE_INVERT                               = 0x80AB,
    GL_NUM_COMPRESSED_TEXTURE_FORMATS                       = 0x86A2,
    GL_COMPRESSED_TEXTURE_FORMATS                           = 0x86A3,
    GL_DONT_CARE                                            = 0x1100,
    GL_FASTEST                                              = 0x1101,
    GL_NICEST                                               = 0x1102,
    GL_GENERATE_MIPMAP_HINT                                 = 0x8192,
    GL_BYTE                                                 = 0x1400,
    GL_UNSIGNED_BYTE                                        = 0x1401,
    GL_SHORT                                                = 0x1402,
    GL_UNSIGNED_SHORT                                       = 0x1403,
    GL_INT                                                  = 0x1404,
    GL_UNSIGNED_INT                                         = 0x1405,
    GL_FLOAT                                                = 0x1406,
    GL_FIXED                                                = 0x140C,
    GL_DEPTH_COMPONENT                                      = 0x1902,
    GL_ALPHA                                                = 0x1906,
    GL_RGB                                                  = 0x1907,
    GL_RGBA                                                 = 0x1908,
    GL_LUMINANCE                                            = 0x1909,
    GL_LUMINANCE_ALPHA                                      = 0x190A,
    GL_UNSIGNED_SHORT_4_4_4_4                               = 0x8033,
    GL_UNSIGNED_SHORT_5_5_5_1                               = 0x8034,
    GL_UNSIGNED_SHORT_5_6_5                                 = 0x8363,
    GL_FRAGMENT_SHADER                                      = 0x8B30,
    GL_VERTEX_SHADER                                        = 0x8B31,
    GL_MAX_VERTEX_ATTRIBS                                   = 0x8869,
    GL_MAX_VERTEX_UNIFORM_VECTORS                           = 0x8DFB,
    GL_MAX_VARYING_VECTORS                                  = 0x8DFC,
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS                     = 0x8B4D,
    GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                       = 0x8B4C,
    GL_MAX_TEXTURE_IMAGE_UNITS                              = 0x8872,
    GL_MAX_FRAGMENT_UNIFORM_VECTORS                         = 0x8DFD,
    GL_SHADER_TYPE                                          = 0x8B4F,
    GL_DELETE_STATUS                                        = 0x8B80,
    GL_LINK_STATUS                                          = 0x8B82,
    GL_VALIDATE_STATUS                                      = 0x8B83,
    GL_ATTACHED_SHADERS                                     = 0x8B85,
    GL_ACTIVE_UNIFORMS                                      = 0x8B86,
    GL_ACTIVE_UNIFORM_MAX_LENGTH                            = 0x8B87,
    GL_ACTIVE_ATTRIBUTES                                    = 0x8B89,
    GL_ACTIVE_ATTRIBUTE_MAX_LENGTH                          = 0x8B8A,
    GL_SHADING_LANGUAGE_VERSION                             = 0x8B8C,
    GL_CURRENT_PROGRAM                                      = 0x8B8D,
    GL_NEVER                                                = 0x0200,
    GL_LESS                                                 = 0x0201,
    GL_EQUAL                                                = 0x0202,
    GL_LEQUAL                                               = 0x0203,
    GL_GREATER                                              = 0x0204,
    GL_NOTEQUAL                                             = 0x0205,
    GL_GEQUAL                                               = 0x0206,
    GL_ALWAYS                                               = 0x0207,
    GL_KEEP                                                 = 0x1E00,
    GL_REPLACE                                              = 0x1E01,
    GL_INCR                                                 = 0x1E02,
    GL_DECR                                                 = 0x1E03,
    GL_INVERT                                               = 0x150A,
    GL_INCR_WRAP                                            = 0x8507,
    GL_DECR_WRAP                                            = 0x8508,
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
    GL_TEXTURE                                              = 0x1702,
    GL_TEXTURE_CUBE_MAP                                     = 0x8513,
    GL_TEXTURE_BINDING_CUBE_MAP                             = 0x8514,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X                          = 0x8515,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X                          = 0x8516,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y                          = 0x8517,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                          = 0x8518,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z                          = 0x8519,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                          = 0x851A,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE                            = 0x851C,
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
    GL_REPEAT                                               = 0x2901,
    GL_CLAMP_TO_EDGE                                        = 0x812F,
    GL_MIRRORED_REPEAT                                      = 0x8370,
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
    GL_SAMPLER_2D                                           = 0x8B5E,
    GL_SAMPLER_CUBE                                         = 0x8B60,
    GL_VERTEX_ATTRIB_ARRAY_ENABLED                          = 0x8622,
    GL_VERTEX_ATTRIB_ARRAY_SIZE                             = 0x8623,
    GL_VERTEX_ATTRIB_ARRAY_STRIDE                           = 0x8624,
    GL_VERTEX_ATTRIB_ARRAY_TYPE                             = 0x8625,
    GL_VERTEX_ATTRIB_ARRAY_NORMALIZED                       = 0x886A,
    GL_VERTEX_ATTRIB_ARRAY_POINTER                          = 0x8645,
    GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING                   = 0x889F,
    GL_IMPLEMENTATION_COLOR_READ_TYPE                       = 0x8B9A,
    GL_IMPLEMENTATION_COLOR_READ_FORMAT                     = 0x8B9B,
    GL_COMPILE_STATUS                                       = 0x8B81,
    GL_INFO_LOG_LENGTH                                      = 0x8B84,
    GL_SHADER_SOURCE_LENGTH                                 = 0x8B88,
    GL_SHADER_COMPILER                                      = 0x8DFA,
    GL_SHADER_BINARY_FORMATS                                = 0x8DF8,
    GL_NUM_SHADER_BINARY_FORMATS                            = 0x8DF9,
    GL_LOW_FLOAT                                            = 0x8DF0,
    GL_MEDIUM_FLOAT                                         = 0x8DF1,
    GL_HIGH_FLOAT                                           = 0x8DF2,
    GL_LOW_INT                                              = 0x8DF3,
    GL_MEDIUM_INT                                           = 0x8DF4,
    GL_HIGH_INT                                             = 0x8DF5,
    GL_FRAMEBUFFER                                          = 0x8D40,
    GL_RENDERBUFFER                                         = 0x8D41,
    GL_RGBA4                                                = 0x8056,
    GL_RGB5_A1                                              = 0x8057,
    GL_RGB565                                               = 0x8D62,
    GL_DEPTH_COMPONENT16                                    = 0x81A5,
    GL_STENCIL_INDEX8                                       = 0x8D48,
    GL_RENDERBUFFER_WIDTH                                   = 0x8D42,
    GL_RENDERBUFFER_HEIGHT                                  = 0x8D43,
    GL_RENDERBUFFER_INTERNAL_FORMAT                         = 0x8D44,
    GL_RENDERBUFFER_RED_SIZE                                = 0x8D50,
    GL_RENDERBUFFER_GREEN_SIZE                              = 0x8D51,
    GL_RENDERBUFFER_BLUE_SIZE                               = 0x8D52,
    GL_RENDERBUFFER_ALPHA_SIZE                              = 0x8D53,
    GL_RENDERBUFFER_DEPTH_SIZE                              = 0x8D54,
    GL_RENDERBUFFER_STENCIL_SIZE                            = 0x8D55,
    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE                   = 0x8CD0,
    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME                   = 0x8CD1,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL                 = 0x8CD2,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE         = 0x8CD3,
    GL_COLOR_ATTACHMENT0                                    = 0x8CE0,
    GL_DEPTH_ATTACHMENT                                     = 0x8D00,
    GL_STENCIL_ATTACHMENT                                   = 0x8D20,
    GL_NONE                                                 = 0,
    GL_FRAMEBUFFER_COMPLETE                                 = 0x8CD5,
    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT                    = 0x8CD6,
    GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT            = 0x8CD7,
    GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS                    = 0x8CD9,
    GL_FRAMEBUFFER_UNSUPPORTED                              = 0x8CDD,
    GL_FRAMEBUFFER_BINDING                                  = 0x8CA6,
    GL_RENDERBUFFER_BINDING                                 = 0x8CA7,
    GL_MAX_RENDERBUFFER_SIZE                                = 0x84E8,
    GL_INVALID_FRAMEBUFFER_OPERATION                        = 0x0506,
};
extern void         (KHRONOS_APIENTRY* const& glActiveTexture) (GLenum texture);
extern void         (KHRONOS_APIENTRY* const& glAttachShader) (GLuint program, GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glBindAttribLocation) (GLuint program, GLuint index, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glBindBuffer) (GLenum target, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glBindFramebuffer) (GLenum target, GLuint framebuffer);
extern void         (KHRONOS_APIENTRY* const& glBindRenderbuffer) (GLenum target, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glBindTexture) (GLenum target, GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glBlendColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void         (KHRONOS_APIENTRY* const& glBlendEquation) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparate) (GLenum modeRGB, GLenum modeAlpha);
extern void         (KHRONOS_APIENTRY* const& glBlendFunc) (GLenum sfactor, GLenum dfactor);
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparate) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
extern void         (KHRONOS_APIENTRY* const& glBufferData) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern void         (KHRONOS_APIENTRY* const& glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
extern GLenum       (KHRONOS_APIENTRY* const& glCheckFramebufferStatus) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glClear) (GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glClearColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void         (KHRONOS_APIENTRY* const& glClearDepthf) (GLfloat d);
extern void         (KHRONOS_APIENTRY* const& glClearStencil) (GLint s);
extern void         (KHRONOS_APIENTRY* const& glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void         (KHRONOS_APIENTRY* const& glCompileShader) (GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern GLuint       (KHRONOS_APIENTRY* const& glCreateProgram) ();
extern GLuint       (KHRONOS_APIENTRY* const& glCreateShader) (GLenum type);
extern void         (KHRONOS_APIENTRY* const& glCullFace) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glDeleteBuffers) (GLsizei n, const GLuint *buffers);
extern void         (KHRONOS_APIENTRY* const& glDeleteFramebuffers) (GLsizei n, const GLuint *framebuffers);
extern void         (KHRONOS_APIENTRY* const& glDeleteProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glDeleteRenderbuffers) (GLsizei n, const GLuint *renderbuffers);
extern void         (KHRONOS_APIENTRY* const& glDeleteShader) (GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glDeleteTextures) (GLsizei n, const GLuint *textures);
extern void         (KHRONOS_APIENTRY* const& glDepthFunc) (GLenum func);
extern void         (KHRONOS_APIENTRY* const& glDepthMask) (GLboolean flag);
extern void         (KHRONOS_APIENTRY* const& glDepthRangef) (GLfloat n, GLfloat f);
extern void         (KHRONOS_APIENTRY* const& glDetachShader) (GLuint program, GLuint shader);
extern void         (KHRONOS_APIENTRY* const& glDisable) (GLenum cap);
extern void         (KHRONOS_APIENTRY* const& glDisableVertexAttribArray) (GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDrawArrays) (GLenum mode, GLint first, GLsizei count);
extern void         (KHRONOS_APIENTRY* const& glDrawElements) (GLenum mode, GLsizei count, GLenum type, const void *indices);
extern void         (KHRONOS_APIENTRY* const& glEnable) (GLenum cap);
extern void         (KHRONOS_APIENTRY* const& glEnableVertexAttribArray) (GLuint index);
extern void         (KHRONOS_APIENTRY* const& glFinish) ();
extern void         (KHRONOS_APIENTRY* const& glFlush) ();
extern void         (KHRONOS_APIENTRY* const& glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glFrontFace) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glGenBuffers) (GLsizei n, GLuint *buffers);
extern void         (KHRONOS_APIENTRY* const& glGenerateMipmap) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGenFramebuffers) (GLsizei n, GLuint *framebuffers);
extern void         (KHRONOS_APIENTRY* const& glGenRenderbuffers) (GLsizei n, GLuint *renderbuffers);
extern void         (KHRONOS_APIENTRY* const& glGenTextures) (GLsizei n, GLuint *textures);
extern void         (KHRONOS_APIENTRY* const& glGetActiveAttrib) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniform) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetAttachedShaders) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
extern GLint        (KHRONOS_APIENTRY* const& glGetAttribLocation) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetBooleanv) (GLenum pname, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glGetBufferParameteriv) (GLenum target, GLenum pname, GLint *params);
extern GLenum       (KHRONOS_APIENTRY* const& glGetError) ();
extern void         (KHRONOS_APIENTRY* const& glGetFloatv) (GLenum pname, GLfloat *data);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferAttachmentParameteriv) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetIntegerv) (GLenum pname, GLint *data);
extern void         (KHRONOS_APIENTRY* const& glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramInfoLog) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void         (KHRONOS_APIENTRY* const& glGetRenderbufferParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetShaderiv) (GLuint shader, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetShaderInfoLog) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void         (KHRONOS_APIENTRY* const& glGetShaderPrecisionFormat) (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
extern void         (KHRONOS_APIENTRY* const& glGetShaderSource) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
extern const GLubyte * (KHRONOS_APIENTRY* const& glGetString) (GLenum name);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetUniformfv) (GLuint program, GLint location, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetUniformiv) (GLuint program, GLint location, GLint *params);
extern GLint        (KHRONOS_APIENTRY* const& glGetUniformLocation) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribfv) (GLuint index, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribiv) (GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribPointerv) (GLuint index, GLenum pname, void **pointer);
extern void         (KHRONOS_APIENTRY* const& glHint) (GLenum target, GLenum mode);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsBuffer) (GLuint buffer);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnabled) (GLenum cap);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsFramebuffer) (GLuint framebuffer);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsProgram) (GLuint program);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsRenderbuffer) (GLuint renderbuffer);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsShader) (GLuint shader);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTexture) (GLuint texture);
extern void         (KHRONOS_APIENTRY* const& glLineWidth) (GLfloat width);
extern void         (KHRONOS_APIENTRY* const& glLinkProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glPixelStorei) (GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glPolygonOffset) (GLfloat factor, GLfloat units);
extern void         (KHRONOS_APIENTRY* const& glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
extern void         (KHRONOS_APIENTRY* const& glReleaseShaderCompiler) ();
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glSampleCoverage) (GLfloat value, GLboolean invert);
extern void         (KHRONOS_APIENTRY* const& glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glShaderBinary) (GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length);
extern void         (KHRONOS_APIENTRY* const& glShaderSource) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
extern void         (KHRONOS_APIENTRY* const& glStencilFunc) (GLenum func, GLint ref, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilFuncSeparate) (GLenum face, GLenum func, GLint ref, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilMask) (GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilMaskSeparate) (GLenum face, GLuint mask);
extern void         (KHRONOS_APIENTRY* const& glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
extern void         (KHRONOS_APIENTRY* const& glStencilOpSeparate) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
extern void         (KHRONOS_APIENTRY* const& glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameteri) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glTexParameteriv) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glUniform1f) (GLint location, GLfloat v0);
extern void         (KHRONOS_APIENTRY* const& glUniform1fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform1i) (GLint location, GLint v0);
extern void         (KHRONOS_APIENTRY* const& glUniform1iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2f) (GLint location, GLfloat v0, GLfloat v1);
extern void         (KHRONOS_APIENTRY* const& glUniform2fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2i) (GLint location, GLint v0, GLint v1);
extern void         (KHRONOS_APIENTRY* const& glUniform2iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glUniform3fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3i) (GLint location, GLint v0, GLint v1, GLint v2);
extern void         (KHRONOS_APIENTRY* const& glUniform3iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void         (KHRONOS_APIENTRY* const& glUniform4fv) (GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4i) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void         (KHRONOS_APIENTRY* const& glUniform4iv) (GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUseProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glValidateProgram) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1f) (GLuint index, GLfloat x);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib1fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2f) (GLuint index, GLfloat x, GLfloat y);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib2fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3f) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib3fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4f) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttrib4fv) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribPointer) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
#endif

#ifndef GL_ES_VERSION_3_0
#define GL_ES_VERSION_3_0 1
enum : GLenum
{
    GL_READ_BUFFER                                          = 0x0C02,
    GL_UNPACK_ROW_LENGTH                                    = 0x0CF2,
    GL_UNPACK_SKIP_ROWS                                     = 0x0CF3,
    GL_UNPACK_SKIP_PIXELS                                   = 0x0CF4,
    GL_PACK_ROW_LENGTH                                      = 0x0D02,
    GL_PACK_SKIP_ROWS                                       = 0x0D03,
    GL_PACK_SKIP_PIXELS                                     = 0x0D04,
    GL_COLOR                                                = 0x1800,
    GL_DEPTH                                                = 0x1801,
    GL_STENCIL                                              = 0x1802,
    GL_RED                                                  = 0x1903,
    GL_RGB8                                                 = 0x8051,
    GL_RGBA8                                                = 0x8058,
    GL_RGB10_A2                                             = 0x8059,
    GL_TEXTURE_BINDING_3D                                   = 0x806A,
    GL_UNPACK_SKIP_IMAGES                                   = 0x806D,
    GL_UNPACK_IMAGE_HEIGHT                                  = 0x806E,
    GL_TEXTURE_3D                                           = 0x806F,
    GL_TEXTURE_WRAP_R                                       = 0x8072,
    GL_MAX_3D_TEXTURE_SIZE                                  = 0x8073,
    GL_UNSIGNED_INT_2_10_10_10_REV                          = 0x8368,
    GL_MAX_ELEMENTS_VERTICES                                = 0x80E8,
    GL_MAX_ELEMENTS_INDICES                                 = 0x80E9,
    GL_TEXTURE_MIN_LOD                                      = 0x813A,
    GL_TEXTURE_MAX_LOD                                      = 0x813B,
    GL_TEXTURE_BASE_LEVEL                                   = 0x813C,
    GL_TEXTURE_MAX_LEVEL                                    = 0x813D,
    GL_MIN                                                  = 0x8007,
    GL_MAX                                                  = 0x8008,
    GL_DEPTH_COMPONENT24                                    = 0x81A6,
    GL_MAX_TEXTURE_LOD_BIAS                                 = 0x84FD,
    GL_TEXTURE_COMPARE_MODE                                 = 0x884C,
    GL_TEXTURE_COMPARE_FUNC                                 = 0x884D,
    GL_CURRENT_QUERY                                        = 0x8865,
    GL_QUERY_RESULT                                         = 0x8866,
    GL_QUERY_RESULT_AVAILABLE                               = 0x8867,
    GL_BUFFER_MAPPED                                        = 0x88BC,
    GL_BUFFER_MAP_POINTER                                   = 0x88BD,
    GL_STREAM_READ                                          = 0x88E1,
    GL_STREAM_COPY                                          = 0x88E2,
    GL_STATIC_READ                                          = 0x88E5,
    GL_STATIC_COPY                                          = 0x88E6,
    GL_DYNAMIC_READ                                         = 0x88E9,
    GL_DYNAMIC_COPY                                         = 0x88EA,
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
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS                      = 0x8B49,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS                        = 0x8B4A,
    GL_SAMPLER_3D                                           = 0x8B5F,
    GL_SAMPLER_2D_SHADOW                                    = 0x8B62,
    GL_FRAGMENT_SHADER_DERIVATIVE_HINT                      = 0x8B8B,
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
    GL_SRGB8_ALPHA8                                         = 0x8C43,
    GL_COMPARE_REF_TO_TEXTURE                               = 0x884E,
    GL_MAJOR_VERSION                                        = 0x821B,
    GL_MINOR_VERSION                                        = 0x821C,
    GL_NUM_EXTENSIONS                                       = 0x821D,
    GL_RGBA32F                                              = 0x8814,
    GL_RGB32F                                               = 0x8815,
    GL_RGBA16F                                              = 0x881A,
    GL_RGB16F                                               = 0x881B,
    GL_VERTEX_ATTRIB_ARRAY_INTEGER                          = 0x88FD,
    GL_MAX_ARRAY_TEXTURE_LAYERS                             = 0x88FF,
    GL_MIN_PROGRAM_TEXEL_OFFSET                             = 0x8904,
    GL_MAX_PROGRAM_TEXEL_OFFSET                             = 0x8905,
    GL_MAX_VARYING_COMPONENTS                               = 0x8B4B,
    GL_TEXTURE_2D_ARRAY                                     = 0x8C1A,
    GL_TEXTURE_BINDING_2D_ARRAY                             = 0x8C1D,
    GL_R11F_G11F_B10F                                       = 0x8C3A,
    GL_UNSIGNED_INT_10F_11F_11F_REV                         = 0x8C3B,
    GL_RGB9_E5                                              = 0x8C3D,
    GL_UNSIGNED_INT_5_9_9_9_REV                             = 0x8C3E,
    GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH                = 0x8C76,
    GL_TRANSFORM_FEEDBACK_BUFFER_MODE                       = 0x8C7F,
    GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS           = 0x8C80,
    GL_TRANSFORM_FEEDBACK_VARYINGS                          = 0x8C83,
    GL_TRANSFORM_FEEDBACK_BUFFER_START                      = 0x8C84,
    GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                       = 0x8C85,
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
    GL_RGB_INTEGER                                          = 0x8D98,
    GL_RGBA_INTEGER                                         = 0x8D99,
    GL_SAMPLER_2D_ARRAY                                     = 0x8DC1,
    GL_SAMPLER_2D_ARRAY_SHADOW                              = 0x8DC4,
    GL_SAMPLER_CUBE_SHADOW                                  = 0x8DC5,
    GL_UNSIGNED_INT_VEC2                                    = 0x8DC6,
    GL_UNSIGNED_INT_VEC3                                    = 0x8DC7,
    GL_UNSIGNED_INT_VEC4                                    = 0x8DC8,
    GL_INT_SAMPLER_2D                                       = 0x8DCA,
    GL_INT_SAMPLER_3D                                       = 0x8DCB,
    GL_INT_SAMPLER_CUBE                                     = 0x8DCC,
    GL_INT_SAMPLER_2D_ARRAY                                 = 0x8DCF,
    GL_UNSIGNED_INT_SAMPLER_2D                              = 0x8DD2,
    GL_UNSIGNED_INT_SAMPLER_3D                              = 0x8DD3,
    GL_UNSIGNED_INT_SAMPLER_CUBE                            = 0x8DD4,
    GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                        = 0x8DD7,
    GL_BUFFER_ACCESS_FLAGS                                  = 0x911F,
    GL_BUFFER_MAP_LENGTH                                    = 0x9120,
    GL_BUFFER_MAP_OFFSET                                    = 0x9121,
    GL_DEPTH_COMPONENT32F                                   = 0x8CAC,
    GL_DEPTH32F_STENCIL8                                    = 0x8CAD,
    GL_FLOAT_32_UNSIGNED_INT_24_8_REV                       = 0x8DAD,
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
    GL_DEPTH_STENCIL                                        = 0x84F9,
    GL_UNSIGNED_INT_24_8                                    = 0x84FA,
    GL_DEPTH24_STENCIL8                                     = 0x88F0,
    GL_UNSIGNED_NORMALIZED                                  = 0x8C17,
    GL_DRAW_FRAMEBUFFER_BINDING                             = 0x8CA6,
    GL_READ_FRAMEBUFFER                                     = 0x8CA8,
    GL_DRAW_FRAMEBUFFER                                     = 0x8CA9,
    GL_READ_FRAMEBUFFER_BINDING                             = 0x8CAA,
    GL_RENDERBUFFER_SAMPLES                                 = 0x8CAB,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER                 = 0x8CD4,
    GL_MAX_COLOR_ATTACHMENTS                                = 0x8CDF,
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
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE                   = 0x8D56,
    GL_MAX_SAMPLES                                          = 0x8D57,
    GL_HALF_FLOAT                                           = 0x140B,
    GL_MAP_READ_BIT                                         = 0x0001,
    GL_MAP_WRITE_BIT                                        = 0x0002,
    GL_MAP_INVALIDATE_RANGE_BIT                             = 0x0004,
    GL_MAP_INVALIDATE_BUFFER_BIT                            = 0x0008,
    GL_MAP_FLUSH_EXPLICIT_BIT                               = 0x0010,
    GL_MAP_UNSYNCHRONIZED_BIT                               = 0x0020,
    GL_RG                                                   = 0x8227,
    GL_RG_INTEGER                                           = 0x8228,
    GL_R8                                                   = 0x8229,
    GL_RG8                                                  = 0x822B,
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
    GL_R8_SNORM                                             = 0x8F94,
    GL_RG8_SNORM                                            = 0x8F95,
    GL_RGB8_SNORM                                           = 0x8F96,
    GL_RGBA8_SNORM                                          = 0x8F97,
    GL_SIGNED_NORMALIZED                                    = 0x8F9C,
    GL_PRIMITIVE_RESTART_FIXED_INDEX                        = 0x8D69,
    GL_COPY_READ_BUFFER                                     = 0x8F36,
    GL_COPY_WRITE_BUFFER                                    = 0x8F37,
    GL_COPY_READ_BUFFER_BINDING                             = 0x8F36,
    GL_COPY_WRITE_BUFFER_BINDING                            = 0x8F37,
    GL_UNIFORM_BUFFER                                       = 0x8A11,
    GL_UNIFORM_BUFFER_BINDING                               = 0x8A28,
    GL_UNIFORM_BUFFER_START                                 = 0x8A29,
    GL_UNIFORM_BUFFER_SIZE                                  = 0x8A2A,
    GL_MAX_VERTEX_UNIFORM_BLOCKS                            = 0x8A2B,
    GL_MAX_FRAGMENT_UNIFORM_BLOCKS                          = 0x8A2D,
    GL_MAX_COMBINED_UNIFORM_BLOCKS                          = 0x8A2E,
    GL_MAX_UNIFORM_BUFFER_BINDINGS                          = 0x8A2F,
    GL_MAX_UNIFORM_BLOCK_SIZE                               = 0x8A30,
    GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS               = 0x8A31,
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
    GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER          = 0x8A46,
    GL_INVALID_INDEX                                        = 0xFFFFFFFF,
    GL_MAX_VERTEX_OUTPUT_COMPONENTS                         = 0x9122,
    GL_MAX_FRAGMENT_INPUT_COMPONENTS                        = 0x9125,
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
    GL_SYNC_FLUSH_COMMANDS_BIT                              = 0x00000001,
};
enum : GLuint64
{
    GL_TIMEOUT_IGNORED                                      = 0xFFFFFFFFFFFFFFFF,
};
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_DIVISOR                          = 0x88FE,
    GL_ANY_SAMPLES_PASSED                                   = 0x8C2F,
    GL_ANY_SAMPLES_PASSED_CONSERVATIVE                      = 0x8D6A,
    GL_SAMPLER_BINDING                                      = 0x8919,
    GL_RGB10_A2UI                                           = 0x906F,
    GL_TEXTURE_SWIZZLE_R                                    = 0x8E42,
    GL_TEXTURE_SWIZZLE_G                                    = 0x8E43,
    GL_TEXTURE_SWIZZLE_B                                    = 0x8E44,
    GL_TEXTURE_SWIZZLE_A                                    = 0x8E45,
    GL_GREEN                                                = 0x1904,
    GL_BLUE                                                 = 0x1905,
    GL_INT_2_10_10_10_REV                                   = 0x8D9F,
    GL_TRANSFORM_FEEDBACK                                   = 0x8E22,
    GL_TRANSFORM_FEEDBACK_PAUSED                            = 0x8E23,
    GL_TRANSFORM_FEEDBACK_ACTIVE                            = 0x8E24,
    GL_TRANSFORM_FEEDBACK_BINDING                           = 0x8E25,
    GL_PROGRAM_BINARY_RETRIEVABLE_HINT                      = 0x8257,
    GL_PROGRAM_BINARY_LENGTH                                = 0x8741,
    GL_NUM_PROGRAM_BINARY_FORMATS                           = 0x87FE,
    GL_PROGRAM_BINARY_FORMATS                               = 0x87FF,
    GL_COMPRESSED_R11_EAC                                   = 0x9270,
    GL_COMPRESSED_SIGNED_R11_EAC                            = 0x9271,
    GL_COMPRESSED_RG11_EAC                                  = 0x9272,
    GL_COMPRESSED_SIGNED_RG11_EAC                           = 0x9273,
    GL_COMPRESSED_RGB8_ETC2                                 = 0x9274,
    GL_COMPRESSED_SRGB8_ETC2                                = 0x9275,
    GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2             = 0x9276,
    GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2            = 0x9277,
    GL_COMPRESSED_RGBA8_ETC2_EAC                            = 0x9278,
    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC                     = 0x9279,
    GL_TEXTURE_IMMUTABLE_FORMAT                             = 0x912F,
    GL_MAX_ELEMENT_INDEX                                    = 0x8D6B,
    GL_NUM_SAMPLE_COUNTS                                    = 0x9380,
    GL_TEXTURE_IMMUTABLE_LEVELS                             = 0x82DF,
};
extern void         (KHRONOS_APIENTRY* const& glReadBuffer) (GLenum src);
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
extern void         (KHRONOS_APIENTRY* const& glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glGenQueries) (GLsizei n, GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glDeleteQueries) (GLsizei n, const GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsQuery) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glBeginQuery) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glEndQuery) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetQueryiv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint *params);
extern GLboolean    (KHRONOS_APIENTRY* const& glUnmapBuffer) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetBufferPointerv) (GLenum target, GLenum pname, void **params);
extern void         (KHRONOS_APIENTRY* const& glDrawBuffers) (GLsizei n, const GLenum *bufs);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureLayer) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern void *       (KHRONOS_APIENTRY* const& glMapBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern void         (KHRONOS_APIENTRY* const& glFlushMappedBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length);
extern void         (KHRONOS_APIENTRY* const& glBindVertexArray) (GLuint array);
extern void         (KHRONOS_APIENTRY* const& glDeleteVertexArrays) (GLsizei n, const GLuint *arrays);
extern void         (KHRONOS_APIENTRY* const& glGenVertexArrays) (GLsizei n, GLuint *arrays);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsVertexArray) (GLuint array);
extern void         (KHRONOS_APIENTRY* const& glGetIntegeri_v) (GLenum target, GLuint index, GLint *data);
extern void         (KHRONOS_APIENTRY* const& glBeginTransformFeedback) (GLenum primitiveMode);
extern void         (KHRONOS_APIENTRY* const& glEndTransformFeedback) ();
extern void         (KHRONOS_APIENTRY* const& glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTransformFeedbackVaryings) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
extern void         (KHRONOS_APIENTRY* const& glGetTransformFeedbackVarying) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribIPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribIiv) (GLuint index, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetVertexAttribIuiv) (GLuint index, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4i) (GLuint index, GLint x, GLint y, GLint z, GLint w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4ui) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4iv) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribI4uiv) (GLuint index, const GLuint *v);
extern void         (KHRONOS_APIENTRY* const& glGetUniformuiv) (GLuint program, GLint location, GLuint *params);
extern GLint        (KHRONOS_APIENTRY* const& glGetFragDataLocation) (GLuint program, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glUniform1ui) (GLint location, GLuint v0);
extern void         (KHRONOS_APIENTRY* const& glUniform2ui) (GLint location, GLuint v0, GLuint v1);
extern void         (KHRONOS_APIENTRY* const& glUniform3ui) (GLint location, GLuint v0, GLuint v1, GLuint v2);
extern void         (KHRONOS_APIENTRY* const& glUniform4ui) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern void         (KHRONOS_APIENTRY* const& glUniform1uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform2uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform3uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glUniform4uiv) (GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glClearBufferiv) (GLenum buffer, GLint drawbuffer, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glClearBufferuiv) (GLenum buffer, GLint drawbuffer, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glClearBufferfv) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glClearBufferfi) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
extern const GLubyte * (KHRONOS_APIENTRY* const& glGetStringi) (GLenum name, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glCopyBufferSubData) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glGetUniformIndices) (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformsiv) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
extern GLuint       (KHRONOS_APIENTRY* const& glGetUniformBlockIndex) (GLuint program, const GLchar *uniformBlockName);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformBlockiv) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetActiveUniformBlockName) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
extern void         (KHRONOS_APIENTRY* const& glUniformBlockBinding) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstanced) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstanced) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
extern GLsync       (KHRONOS_APIENTRY* const& glFenceSync) (GLenum condition, GLbitfield flags);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsSync) (GLsync sync);
extern void         (KHRONOS_APIENTRY* const& glDeleteSync) (GLsync sync);
extern GLenum       (KHRONOS_APIENTRY* const& glClientWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void         (KHRONOS_APIENTRY* const& glWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void         (KHRONOS_APIENTRY* const& glGetInteger64v) (GLenum pname, GLint64 *data);
extern void         (KHRONOS_APIENTRY* const& glGetSynciv) (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values);
extern void         (KHRONOS_APIENTRY* const& glGetInteger64i_v) (GLenum target, GLuint index, GLint64 *data);
extern void         (KHRONOS_APIENTRY* const& glGetBufferParameteri64v) (GLenum target, GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGenSamplers) (GLsizei count, GLuint *samplers);
extern void         (KHRONOS_APIENTRY* const& glDeleteSamplers) (GLsizei count, const GLuint *samplers);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsSampler) (GLuint sampler);
extern void         (KHRONOS_APIENTRY* const& glBindSampler) (GLuint unit, GLuint sampler);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameteri) (GLuint sampler, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameteriv) (GLuint sampler, GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterf) (GLuint sampler, GLenum pname, GLfloat param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterfv) (GLuint sampler, GLenum pname, const GLfloat *param);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameteriv) (GLuint sampler, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterfv) (GLuint sampler, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribDivisor) (GLuint index, GLuint divisor);
extern void         (KHRONOS_APIENTRY* const& glBindTransformFeedback) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDeleteTransformFeedbacks) (GLsizei n, const GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glGenTransformFeedbacks) (GLsizei n, GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsTransformFeedback) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glPauseTransformFeedback) ();
extern void         (KHRONOS_APIENTRY* const& glResumeTransformFeedback) ();
extern void         (KHRONOS_APIENTRY* const& glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
extern void         (KHRONOS_APIENTRY* const& glProgramBinary) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
extern void         (KHRONOS_APIENTRY* const& glProgramParameteri) (GLuint program, GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glInvalidateFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
extern void         (KHRONOS_APIENTRY* const& glInvalidateSubFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTexStorage2D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTexStorage3D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern void         (KHRONOS_APIENTRY* const& glGetInternalformativ) (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params);
#endif

#ifndef GL_ES_VERSION_3_1
#define GL_ES_VERSION_3_1 1
enum : GLenum
{
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
    GL_DISPATCH_INDIRECT_BUFFER                             = 0x90EE,
    GL_DISPATCH_INDIRECT_BUFFER_BINDING                     = 0x90EF,
    GL_COMPUTE_SHADER_BIT                                   = 0x00000020,
    GL_DRAW_INDIRECT_BUFFER                                 = 0x8F3F,
    GL_DRAW_INDIRECT_BUFFER_BINDING                         = 0x8F43,
    GL_MAX_UNIFORM_LOCATIONS                                = 0x826E,
    GL_FRAMEBUFFER_DEFAULT_WIDTH                            = 0x9310,
    GL_FRAMEBUFFER_DEFAULT_HEIGHT                           = 0x9311,
    GL_FRAMEBUFFER_DEFAULT_SAMPLES                          = 0x9313,
    GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS           = 0x9314,
    GL_MAX_FRAMEBUFFER_WIDTH                                = 0x9315,
    GL_MAX_FRAMEBUFFER_HEIGHT                               = 0x9316,
    GL_MAX_FRAMEBUFFER_SAMPLES                              = 0x9318,
    GL_UNIFORM                                              = 0x92E1,
    GL_UNIFORM_BLOCK                                        = 0x92E2,
    GL_PROGRAM_INPUT                                        = 0x92E3,
    GL_PROGRAM_OUTPUT                                       = 0x92E4,
    GL_BUFFER_VARIABLE                                      = 0x92E5,
    GL_SHADER_STORAGE_BLOCK                                 = 0x92E6,
    GL_ATOMIC_COUNTER_BUFFER                                = 0x92C0,
    GL_TRANSFORM_FEEDBACK_VARYING                           = 0x92F4,
    GL_ACTIVE_RESOURCES                                     = 0x92F5,
    GL_MAX_NAME_LENGTH                                      = 0x92F6,
    GL_MAX_NUM_ACTIVE_VARIABLES                             = 0x92F7,
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
    GL_REFERENCED_BY_FRAGMENT_SHADER                        = 0x930A,
    GL_REFERENCED_BY_COMPUTE_SHADER                         = 0x930B,
    GL_TOP_LEVEL_ARRAY_SIZE                                 = 0x930C,
    GL_TOP_LEVEL_ARRAY_STRIDE                               = 0x930D,
    GL_LOCATION                                             = 0x930E,
    GL_VERTEX_SHADER_BIT                                    = 0x00000001,
    GL_FRAGMENT_SHADER_BIT                                  = 0x00000002,
    GL_ALL_SHADER_BITS                                      = 0xFFFFFFFF,
    GL_PROGRAM_SEPARABLE                                    = 0x8258,
    GL_ACTIVE_PROGRAM                                       = 0x8259,
    GL_PROGRAM_PIPELINE_BINDING                             = 0x825A,
    GL_ATOMIC_COUNTER_BUFFER_BINDING                        = 0x92C1,
    GL_ATOMIC_COUNTER_BUFFER_START                          = 0x92C2,
    GL_ATOMIC_COUNTER_BUFFER_SIZE                           = 0x92C3,
    GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS                    = 0x92CC,
    GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS                  = 0x92D0,
    GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS                  = 0x92D1,
    GL_MAX_VERTEX_ATOMIC_COUNTERS                           = 0x92D2,
    GL_MAX_FRAGMENT_ATOMIC_COUNTERS                         = 0x92D6,
    GL_MAX_COMBINED_ATOMIC_COUNTERS                         = 0x92D7,
    GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE                       = 0x92D8,
    GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS                   = 0x92DC,
    GL_ACTIVE_ATOMIC_COUNTER_BUFFERS                        = 0x92D9,
    GL_UNSIGNED_INT_ATOMIC_COUNTER                          = 0x92DB,
    GL_MAX_IMAGE_UNITS                                      = 0x8F38,
    GL_MAX_VERTEX_IMAGE_UNIFORMS                            = 0x90CA,
    GL_MAX_FRAGMENT_IMAGE_UNIFORMS                          = 0x90CE,
    GL_MAX_COMBINED_IMAGE_UNIFORMS                          = 0x90CF,
    GL_IMAGE_BINDING_NAME                                   = 0x8F3A,
    GL_IMAGE_BINDING_LEVEL                                  = 0x8F3B,
    GL_IMAGE_BINDING_LAYERED                                = 0x8F3C,
    GL_IMAGE_BINDING_LAYER                                  = 0x8F3D,
    GL_IMAGE_BINDING_ACCESS                                 = 0x8F3E,
    GL_IMAGE_BINDING_FORMAT                                 = 0x906E,
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
    GL_IMAGE_2D                                             = 0x904D,
    GL_IMAGE_3D                                             = 0x904E,
    GL_IMAGE_CUBE                                           = 0x9050,
    GL_IMAGE_2D_ARRAY                                       = 0x9053,
    GL_INT_IMAGE_2D                                         = 0x9058,
    GL_INT_IMAGE_3D                                         = 0x9059,
    GL_INT_IMAGE_CUBE                                       = 0x905B,
    GL_INT_IMAGE_2D_ARRAY                                   = 0x905E,
    GL_UNSIGNED_INT_IMAGE_2D                                = 0x9063,
    GL_UNSIGNED_INT_IMAGE_3D                                = 0x9064,
    GL_UNSIGNED_INT_IMAGE_CUBE                              = 0x9066,
    GL_UNSIGNED_INT_IMAGE_2D_ARRAY                          = 0x9069,
    GL_IMAGE_FORMAT_COMPATIBILITY_TYPE                      = 0x90C7,
    GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE                   = 0x90C8,
    GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS                  = 0x90C9,
    GL_READ_ONLY                                            = 0x88B8,
    GL_WRITE_ONLY                                           = 0x88B9,
    GL_READ_WRITE                                           = 0x88BA,
    GL_SHADER_STORAGE_BUFFER                                = 0x90D2,
    GL_SHADER_STORAGE_BUFFER_BINDING                        = 0x90D3,
    GL_SHADER_STORAGE_BUFFER_START                          = 0x90D4,
    GL_SHADER_STORAGE_BUFFER_SIZE                           = 0x90D5,
    GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS                     = 0x90D6,
    GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS                   = 0x90DA,
    GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS                    = 0x90DB,
    GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS                   = 0x90DC,
    GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS                   = 0x90DD,
    GL_MAX_SHADER_STORAGE_BLOCK_SIZE                        = 0x90DE,
    GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT               = 0x90DF,
    GL_SHADER_STORAGE_BARRIER_BIT                           = 0x00002000,
    GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES                 = 0x8F39,
    GL_DEPTH_STENCIL_TEXTURE_MODE                           = 0x90EA,
    GL_STENCIL_INDEX                                        = 0x1901,
    GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET                    = 0x8E5E,
    GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET                    = 0x8E5F,
    GL_SAMPLE_POSITION                                      = 0x8E50,
    GL_SAMPLE_MASK                                          = 0x8E51,
    GL_SAMPLE_MASK_VALUE                                    = 0x8E52,
    GL_TEXTURE_2D_MULTISAMPLE                               = 0x9100,
    GL_MAX_SAMPLE_MASK_WORDS                                = 0x8E59,
    GL_MAX_COLOR_TEXTURE_SAMPLES                            = 0x910E,
    GL_MAX_DEPTH_TEXTURE_SAMPLES                            = 0x910F,
    GL_MAX_INTEGER_SAMPLES                                  = 0x9110,
    GL_TEXTURE_BINDING_2D_MULTISAMPLE                       = 0x9104,
    GL_TEXTURE_SAMPLES                                      = 0x9106,
    GL_TEXTURE_FIXED_SAMPLE_LOCATIONS                       = 0x9107,
    GL_TEXTURE_WIDTH                                        = 0x1000,
    GL_TEXTURE_HEIGHT                                       = 0x1001,
    GL_TEXTURE_DEPTH                                        = 0x8071,
    GL_TEXTURE_INTERNAL_FORMAT                              = 0x1003,
    GL_TEXTURE_RED_SIZE                                     = 0x805C,
    GL_TEXTURE_GREEN_SIZE                                   = 0x805D,
    GL_TEXTURE_BLUE_SIZE                                    = 0x805E,
    GL_TEXTURE_ALPHA_SIZE                                   = 0x805F,
    GL_TEXTURE_DEPTH_SIZE                                   = 0x884A,
    GL_TEXTURE_STENCIL_SIZE                                 = 0x88F1,
    GL_TEXTURE_SHARED_SIZE                                  = 0x8C3F,
    GL_TEXTURE_RED_TYPE                                     = 0x8C10,
    GL_TEXTURE_GREEN_TYPE                                   = 0x8C11,
    GL_TEXTURE_BLUE_TYPE                                    = 0x8C12,
    GL_TEXTURE_ALPHA_TYPE                                   = 0x8C13,
    GL_TEXTURE_DEPTH_TYPE                                   = 0x8C16,
    GL_TEXTURE_COMPRESSED                                   = 0x86A1,
    GL_SAMPLER_2D_MULTISAMPLE                               = 0x9108,
    GL_INT_SAMPLER_2D_MULTISAMPLE                           = 0x9109,
    GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE                  = 0x910A,
    GL_VERTEX_ATTRIB_BINDING                                = 0x82D4,
    GL_VERTEX_ATTRIB_RELATIVE_OFFSET                        = 0x82D5,
    GL_VERTEX_BINDING_DIVISOR                               = 0x82D6,
    GL_VERTEX_BINDING_OFFSET                                = 0x82D7,
    GL_VERTEX_BINDING_STRIDE                                = 0x82D8,
    GL_VERTEX_BINDING_BUFFER                                = 0x8F4F,
    GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET                    = 0x82D9,
    GL_MAX_VERTEX_ATTRIB_BINDINGS                           = 0x82DA,
    GL_MAX_VERTEX_ATTRIB_STRIDE                             = 0x82E5,
};
extern void         (KHRONOS_APIENTRY* const& glDispatchCompute) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
extern void         (KHRONOS_APIENTRY* const& glDispatchComputeIndirect) (GLintptr indirect);
extern void         (KHRONOS_APIENTRY* const& glDrawArraysIndirect) (GLenum mode, const void *indirect);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect);
extern void         (KHRONOS_APIENTRY* const& glFramebufferParameteri) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glGetFramebufferParameteriv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetProgramInterfaceiv) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
extern GLuint       (KHRONOS_APIENTRY* const& glGetProgramResourceIndex) (GLuint program, GLenum programInterface, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetProgramResourceName) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glGetProgramResourceiv) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params);
extern GLint        (KHRONOS_APIENTRY* const& glGetProgramResourceLocation) (GLuint program, GLenum programInterface, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glUseProgramStages) (GLuint pipeline, GLbitfield stages, GLuint program);
extern void         (KHRONOS_APIENTRY* const& glActiveShaderProgram) (GLuint pipeline, GLuint program);
extern GLuint       (KHRONOS_APIENTRY* const& glCreateShaderProgramv) (GLenum type, GLsizei count, const GLchar *const*strings);
extern void         (KHRONOS_APIENTRY* const& glBindProgramPipeline) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glDeleteProgramPipelines) (GLsizei n, const GLuint *pipelines);
extern void         (KHRONOS_APIENTRY* const& glGenProgramPipelines) (GLsizei n, GLuint *pipelines);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsProgramPipeline) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glGetProgramPipelineiv) (GLuint pipeline, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1i) (GLuint program, GLint location, GLint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2i) (GLuint program, GLint location, GLint v0, GLint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ui) (GLuint program, GLint location, GLuint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ui) (GLuint program, GLint location, GLuint v0, GLuint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1f) (GLuint program, GLint location, GLfloat v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2f) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glValidateProgramPipeline) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glGetProgramPipelineInfoLog) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void         (KHRONOS_APIENTRY* const& glBindImageTexture) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
extern void         (KHRONOS_APIENTRY* const& glGetBooleani_v) (GLenum target, GLuint index, GLboolean *data);
extern void         (KHRONOS_APIENTRY* const& glMemoryBarrier) (GLbitfield barriers);
extern void         (KHRONOS_APIENTRY* const& glMemoryBarrierByRegion) (GLbitfield barriers);
extern void         (KHRONOS_APIENTRY* const& glTexStorage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
extern void         (KHRONOS_APIENTRY* const& glGetMultisamplefv) (GLenum pname, GLuint index, GLfloat *val);
extern void         (KHRONOS_APIENTRY* const& glSampleMaski) (GLuint maskNumber, GLbitfield mask);
extern void         (KHRONOS_APIENTRY* const& glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glBindVertexBuffer) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribFormat) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribIFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribBinding) (GLuint attribindex, GLuint bindingindex);
extern void         (KHRONOS_APIENTRY* const& glVertexBindingDivisor) (GLuint bindingindex, GLuint divisor);
#endif

#ifndef GL_ES_VERSION_3_2
#define GL_ES_VERSION_3_2 1
enum : GLenum
{
    GL_MULTISAMPLE_LINE_WIDTH_RANGE                         = 0x9381,
    GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY                   = 0x9382,
    GL_MULTIPLY                                             = 0x9294,
    GL_SCREEN                                               = 0x9295,
    GL_OVERLAY                                              = 0x9296,
    GL_DARKEN                                               = 0x9297,
    GL_LIGHTEN                                              = 0x9298,
    GL_COLORDODGE                                           = 0x9299,
    GL_COLORBURN                                            = 0x929A,
    GL_HARDLIGHT                                            = 0x929B,
    GL_SOFTLIGHT                                            = 0x929C,
    GL_DIFFERENCE                                           = 0x929E,
    GL_EXCLUSION                                            = 0x92A0,
    GL_HSL_HUE                                              = 0x92AD,
    GL_HSL_SATURATION                                       = 0x92AE,
    GL_HSL_COLOR                                            = 0x92AF,
    GL_HSL_LUMINOSITY                                       = 0x92B0,
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
    GL_DEBUG_TYPE_MARKER                                    = 0x8268,
    GL_DEBUG_TYPE_PUSH_GROUP                                = 0x8269,
    GL_DEBUG_TYPE_POP_GROUP                                 = 0x826A,
    GL_DEBUG_SEVERITY_NOTIFICATION                          = 0x826B,
    GL_MAX_DEBUG_GROUP_STACK_DEPTH                          = 0x826C,
    GL_DEBUG_GROUP_STACK_DEPTH                              = 0x826D,
    GL_BUFFER                                               = 0x82E0,
    GL_SHADER                                               = 0x82E1,
    GL_PROGRAM                                              = 0x82E2,
    GL_VERTEX_ARRAY                                         = 0x8074,
    GL_QUERY                                                = 0x82E3,
    GL_PROGRAM_PIPELINE                                     = 0x82E4,
    GL_SAMPLER                                              = 0x82E6,
    GL_MAX_LABEL_LENGTH                                     = 0x82E8,
    GL_MAX_DEBUG_MESSAGE_LENGTH                             = 0x9143,
    GL_MAX_DEBUG_LOGGED_MESSAGES                            = 0x9144,
    GL_DEBUG_LOGGED_MESSAGES                                = 0x9145,
    GL_DEBUG_SEVERITY_HIGH                                  = 0x9146,
    GL_DEBUG_SEVERITY_MEDIUM                                = 0x9147,
    GL_DEBUG_SEVERITY_LOW                                   = 0x9148,
    GL_DEBUG_OUTPUT                                         = 0x92E0,
    GL_CONTEXT_FLAG_DEBUG_BIT                               = 0x00000002,
    GL_STACK_OVERFLOW                                       = 0x0503,
    GL_STACK_UNDERFLOW                                      = 0x0504,
    GL_GEOMETRY_SHADER                                      = 0x8DD9,
    GL_GEOMETRY_SHADER_BIT                                  = 0x00000004,
    GL_GEOMETRY_VERTICES_OUT                                = 0x8916,
    GL_GEOMETRY_INPUT_TYPE                                  = 0x8917,
    GL_GEOMETRY_OUTPUT_TYPE                                 = 0x8918,
    GL_GEOMETRY_SHADER_INVOCATIONS                          = 0x887F,
    GL_LAYER_PROVOKING_VERTEX                               = 0x825E,
    GL_LINES_ADJACENCY                                      = 0x000A,
    GL_LINE_STRIP_ADJACENCY                                 = 0x000B,
    GL_TRIANGLES_ADJACENCY                                  = 0x000C,
    GL_TRIANGLE_STRIP_ADJACENCY                             = 0x000D,
    GL_MAX_GEOMETRY_UNIFORM_COMPONENTS                      = 0x8DDF,
    GL_MAX_GEOMETRY_UNIFORM_BLOCKS                          = 0x8A2C,
    GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS             = 0x8A32,
    GL_MAX_GEOMETRY_INPUT_COMPONENTS                        = 0x9123,
    GL_MAX_GEOMETRY_OUTPUT_COMPONENTS                       = 0x9124,
    GL_MAX_GEOMETRY_OUTPUT_VERTICES                         = 0x8DE0,
    GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS                 = 0x8DE1,
    GL_MAX_GEOMETRY_SHADER_INVOCATIONS                      = 0x8E5A,
    GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS                     = 0x8C29,
    GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS                  = 0x92CF,
    GL_MAX_GEOMETRY_ATOMIC_COUNTERS                         = 0x92D5,
    GL_MAX_GEOMETRY_IMAGE_UNIFORMS                          = 0x90CD,
    GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS                   = 0x90D7,
    GL_FIRST_VERTEX_CONVENTION                              = 0x8E4D,
    GL_LAST_VERTEX_CONVENTION                               = 0x8E4E,
    GL_UNDEFINED_VERTEX                                     = 0x8260,
    GL_PRIMITIVES_GENERATED                                 = 0x8C87,
    GL_FRAMEBUFFER_DEFAULT_LAYERS                           = 0x9312,
    GL_MAX_FRAMEBUFFER_LAYERS                               = 0x9317,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS                 = 0x8DA8,
    GL_FRAMEBUFFER_ATTACHMENT_LAYERED                       = 0x8DA7,
    GL_REFERENCED_BY_GEOMETRY_SHADER                        = 0x9309,
    GL_PRIMITIVE_BOUNDING_BOX                               = 0x92BE,
    GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT                       = 0x00000004,
    GL_CONTEXT_FLAGS                                        = 0x821E,
    GL_LOSE_CONTEXT_ON_RESET                                = 0x8252,
    GL_GUILTY_CONTEXT_RESET                                 = 0x8253,
    GL_INNOCENT_CONTEXT_RESET                               = 0x8254,
    GL_UNKNOWN_CONTEXT_RESET                                = 0x8255,
    GL_RESET_NOTIFICATION_STRATEGY                          = 0x8256,
    GL_NO_RESET_NOTIFICATION                                = 0x8261,
    GL_CONTEXT_LOST                                         = 0x0507,
    GL_SAMPLE_SHADING                                       = 0x8C36,
    GL_MIN_SAMPLE_SHADING_VALUE                             = 0x8C37,
    GL_MIN_FRAGMENT_INTERPOLATION_OFFSET                    = 0x8E5B,
    GL_MAX_FRAGMENT_INTERPOLATION_OFFSET                    = 0x8E5C,
    GL_FRAGMENT_INTERPOLATION_OFFSET_BITS                   = 0x8E5D,
    GL_PATCHES                                              = 0x000E,
    GL_PATCH_VERTICES                                       = 0x8E72,
    GL_TESS_CONTROL_OUTPUT_VERTICES                         = 0x8E75,
    GL_TESS_GEN_MODE                                        = 0x8E76,
    GL_TESS_GEN_SPACING                                     = 0x8E77,
    GL_TESS_GEN_VERTEX_ORDER                                = 0x8E78,
    GL_TESS_GEN_POINT_MODE                                  = 0x8E79,
    GL_ISOLINES                                             = 0x8E7A,
    GL_QUADS                                                = 0x0007,
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
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS              = 0x92CD,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS           = 0x92CE,
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS                     = 0x92D3,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS                  = 0x92D4,
    GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS                      = 0x90CB,
    GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS                   = 0x90CC,
    GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS               = 0x90D8,
    GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS            = 0x90D9,
    GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED              = 0x8221,
    GL_IS_PER_PATCH                                         = 0x92E7,
    GL_REFERENCED_BY_TESS_CONTROL_SHADER                    = 0x9307,
    GL_REFERENCED_BY_TESS_EVALUATION_SHADER                 = 0x9308,
    GL_TESS_CONTROL_SHADER                                  = 0x8E88,
    GL_TESS_EVALUATION_SHADER                               = 0x8E87,
    GL_TESS_CONTROL_SHADER_BIT                              = 0x00000008,
    GL_TESS_EVALUATION_SHADER_BIT                           = 0x00000010,
    GL_TEXTURE_BORDER_COLOR                                 = 0x1004,
    GL_CLAMP_TO_BORDER                                      = 0x812D,
    GL_TEXTURE_BUFFER                                       = 0x8C2A,
    GL_TEXTURE_BUFFER_BINDING                               = 0x8C2A,
    GL_MAX_TEXTURE_BUFFER_SIZE                              = 0x8C2B,
    GL_TEXTURE_BINDING_BUFFER                               = 0x8C2C,
    GL_TEXTURE_BUFFER_DATA_STORE_BINDING                    = 0x8C2D,
    GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT                      = 0x919F,
    GL_SAMPLER_BUFFER                                       = 0x8DC2,
    GL_INT_SAMPLER_BUFFER                                   = 0x8DD0,
    GL_UNSIGNED_INT_SAMPLER_BUFFER                          = 0x8DD8,
    GL_IMAGE_BUFFER                                         = 0x9051,
    GL_INT_IMAGE_BUFFER                                     = 0x905C,
    GL_UNSIGNED_INT_IMAGE_BUFFER                            = 0x9067,
    GL_TEXTURE_BUFFER_OFFSET                                = 0x919D,
    GL_TEXTURE_BUFFER_SIZE                                  = 0x919E,
    GL_COMPRESSED_RGBA_ASTC_4x4                             = 0x93B0,
    GL_COMPRESSED_RGBA_ASTC_5x4                             = 0x93B1,
    GL_COMPRESSED_RGBA_ASTC_5x5                             = 0x93B2,
    GL_COMPRESSED_RGBA_ASTC_6x5                             = 0x93B3,
    GL_COMPRESSED_RGBA_ASTC_6x6                             = 0x93B4,
    GL_COMPRESSED_RGBA_ASTC_8x5                             = 0x93B5,
    GL_COMPRESSED_RGBA_ASTC_8x6                             = 0x93B6,
    GL_COMPRESSED_RGBA_ASTC_8x8                             = 0x93B7,
    GL_COMPRESSED_RGBA_ASTC_10x5                            = 0x93B8,
    GL_COMPRESSED_RGBA_ASTC_10x6                            = 0x93B9,
    GL_COMPRESSED_RGBA_ASTC_10x8                            = 0x93BA,
    GL_COMPRESSED_RGBA_ASTC_10x10                           = 0x93BB,
    GL_COMPRESSED_RGBA_ASTC_12x10                           = 0x93BC,
    GL_COMPRESSED_RGBA_ASTC_12x12                           = 0x93BD,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4                     = 0x93D0,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4                     = 0x93D1,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5                     = 0x93D2,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5                     = 0x93D3,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6                     = 0x93D4,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5                     = 0x93D5,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6                     = 0x93D6,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8                     = 0x93D7,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5                    = 0x93D8,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6                    = 0x93D9,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8                    = 0x93DA,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10                   = 0x93DB,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10                   = 0x93DC,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12                   = 0x93DD,
    GL_TEXTURE_CUBE_MAP_ARRAY                               = 0x9009,
    GL_TEXTURE_BINDING_CUBE_MAP_ARRAY                       = 0x900A,
    GL_SAMPLER_CUBE_MAP_ARRAY                               = 0x900C,
    GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW                        = 0x900D,
    GL_INT_SAMPLER_CUBE_MAP_ARRAY                           = 0x900E,
    GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY                  = 0x900F,
    GL_IMAGE_CUBE_MAP_ARRAY                                 = 0x9054,
    GL_INT_IMAGE_CUBE_MAP_ARRAY                             = 0x905F,
    GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY                    = 0x906A,
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY                         = 0x9102,
    GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY                 = 0x9105,
    GL_SAMPLER_2D_MULTISAMPLE_ARRAY                         = 0x910B,
    GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY                     = 0x910C,
    GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY            = 0x910D,
};
extern void         (KHRONOS_APIENTRY* const& glBlendBarrier) ();
extern void         (KHRONOS_APIENTRY* const& glCopyImageSubData) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
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
extern void         (KHRONOS_APIENTRY* const& glGetPointerv) (GLenum pname, void **params);
extern void         (KHRONOS_APIENTRY* const& glEnablei) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisablei) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationi) (GLuint buf, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparatei) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
extern void         (KHRONOS_APIENTRY* const& glBlendFunci) (GLuint buf, GLenum src, GLenum dst);
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparatei) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern void         (KHRONOS_APIENTRY* const& glColorMaski) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnabledi) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElementsBaseVertex) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture) (GLenum target, GLenum attachment, GLuint texture, GLint level);
extern void         (KHRONOS_APIENTRY* const& glPrimitiveBoundingBox) (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);
extern GLenum       (KHRONOS_APIENTRY* const& glGetGraphicsResetStatus) ();
extern void         (KHRONOS_APIENTRY* const& glReadnPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformfv) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformiv) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformuiv) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glMinSampleShading) (GLfloat value);
extern void         (KHRONOS_APIENTRY* const& glPatchParameteri) (GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glTexParameterIiv) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameterIuiv) (GLenum target, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIiv) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIuiv) (GLenum target, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIiv) (GLuint sampler, GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIuiv) (GLuint sampler, GLenum pname, const GLuint *param);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIiv) (GLuint sampler, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIuiv) (GLuint sampler, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glTexBuffer) (GLenum target, GLenum internalformat, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTexBufferRange) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void         (KHRONOS_APIENTRY* const& glTexStorage3DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
#endif

#ifndef GL_AMD_compressed_3DC_texture
#define GL_AMD_compressed_3DC_texture 1
enum : GLenum
{
    GL_3DC_X_AMD                                            = 0x87F9,
    GL_3DC_XY_AMD                                           = 0x87FA,
};
#endif

#ifndef GL_AMD_compressed_ATC_texture
#define GL_AMD_compressed_ATC_texture 1
enum : GLenum
{
    GL_ATC_RGB_AMD                                          = 0x8C92,
    GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                          = 0x8C93,
    GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD                      = 0x87EE,
};
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

#ifndef GL_AMD_program_binary_Z400
#define GL_AMD_program_binary_Z400 1
enum : GLenum
{
    GL_Z400_BINARY_AMD                                      = 0x8740,
};
#endif

#ifndef GL_ANDROID_extension_pack_es31a
#define GL_ANDROID_extension_pack_es31a 1
#endif

#ifndef GL_ANGLE_depth_texture
#define GL_ANGLE_depth_texture 1
enum : GLenum
{
    GL_DEPTH_STENCIL_OES                                    = 0x84F9,
    GL_UNSIGNED_INT_24_8_OES                                = 0x84FA,
    GL_DEPTH_COMPONENT32_OES                                = 0x81A7,
    GL_DEPTH24_STENCIL8_OES                                 = 0x88F0,
};
#endif

#ifndef GL_ANGLE_framebuffer_blit
#define GL_ANGLE_framebuffer_blit 1
enum : GLenum
{
    GL_READ_FRAMEBUFFER_ANGLE                               = 0x8CA8,
    GL_DRAW_FRAMEBUFFER_ANGLE                               = 0x8CA9,
    GL_DRAW_FRAMEBUFFER_BINDING_ANGLE                       = 0x8CA6,
    GL_READ_FRAMEBUFFER_BINDING_ANGLE                       = 0x8CAA,
};
extern void         (KHRONOS_APIENTRY* const& glBlitFramebufferANGLE) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#endif

#ifndef GL_ANGLE_framebuffer_multisample
#define GL_ANGLE_framebuffer_multisample 1
enum : GLenum
{
    GL_RENDERBUFFER_SAMPLES_ANGLE                           = 0x8CAB,
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE             = 0x8D56,
    GL_MAX_SAMPLES_ANGLE                                    = 0x8D57,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleANGLE) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif

#ifndef GL_ANGLE_instanced_arrays
#define GL_ANGLE_instanced_arrays 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE                    = 0x88FE,
};
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstancedANGLE) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedANGLE) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glVertexAttribDivisorANGLE) (GLuint index, GLuint divisor);
#endif

#ifndef GL_ANGLE_pack_reverse_row_order
#define GL_ANGLE_pack_reverse_row_order 1
enum : GLenum
{
    GL_PACK_REVERSE_ROW_ORDER_ANGLE                         = 0x93A4,
};
#endif

#ifndef GL_ANGLE_program_binary
#define GL_ANGLE_program_binary 1
enum : GLenum
{
    GL_PROGRAM_BINARY_ANGLE                                 = 0x93A6,
};
#endif

#ifndef GL_ANGLE_texture_compression_dxt3
#define GL_ANGLE_texture_compression_dxt3 1
enum : GLenum
{
    GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE                      = 0x83F2,
};
#endif

#ifndef GL_ANGLE_texture_compression_dxt5
#define GL_ANGLE_texture_compression_dxt5 1
enum : GLenum
{
    GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE                      = 0x83F3,
};
#endif

#ifndef GL_ANGLE_texture_usage
#define GL_ANGLE_texture_usage 1
enum : GLenum
{
    GL_TEXTURE_USAGE_ANGLE                                  = 0x93A2,
    GL_FRAMEBUFFER_ATTACHMENT_ANGLE                         = 0x93A3,
};
#endif

#ifndef GL_ANGLE_translated_shader_source
#define GL_ANGLE_translated_shader_source 1
enum : GLenum
{
    GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE                = 0x93A0,
};
extern void         (KHRONOS_APIENTRY* const& glGetTranslatedShaderSourceANGLE) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
#endif

#ifndef GL_APPLE_clip_distance
#define GL_APPLE_clip_distance 1
enum : GLenum
{
    GL_MAX_CLIP_DISTANCES_APPLE                             = 0x0D32,
    GL_CLIP_DISTANCE0_APPLE                                 = 0x3000,
    GL_CLIP_DISTANCE1_APPLE                                 = 0x3001,
    GL_CLIP_DISTANCE2_APPLE                                 = 0x3002,
    GL_CLIP_DISTANCE3_APPLE                                 = 0x3003,
    GL_CLIP_DISTANCE4_APPLE                                 = 0x3004,
    GL_CLIP_DISTANCE5_APPLE                                 = 0x3005,
    GL_CLIP_DISTANCE6_APPLE                                 = 0x3006,
    GL_CLIP_DISTANCE7_APPLE                                 = 0x3007,
};
#endif

#ifndef GL_APPLE_color_buffer_packed_float
#define GL_APPLE_color_buffer_packed_float 1
#endif

#ifndef GL_APPLE_copy_texture_levels
#define GL_APPLE_copy_texture_levels 1
extern void         (KHRONOS_APIENTRY* const& glCopyTextureLevelsAPPLE) (GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount);
#endif

#ifndef GL_APPLE_framebuffer_multisample
#define GL_APPLE_framebuffer_multisample 1
enum : GLenum
{
    GL_RENDERBUFFER_SAMPLES_APPLE                           = 0x8CAB,
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_APPLE             = 0x8D56,
    GL_MAX_SAMPLES_APPLE                                    = 0x8D57,
    GL_READ_FRAMEBUFFER_APPLE                               = 0x8CA8,
    GL_DRAW_FRAMEBUFFER_APPLE                               = 0x8CA9,
    GL_DRAW_FRAMEBUFFER_BINDING_APPLE                       = 0x8CA6,
    GL_READ_FRAMEBUFFER_BINDING_APPLE                       = 0x8CAA,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleAPPLE) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glResolveMultisampleFramebufferAPPLE) ();
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

#ifndef GL_APPLE_sync
#define GL_APPLE_sync 1
enum : GLenum
{
    GL_SYNC_OBJECT_APPLE                                    = 0x8A53,
    GL_MAX_SERVER_WAIT_TIMEOUT_APPLE                        = 0x9111,
    GL_OBJECT_TYPE_APPLE                                    = 0x9112,
    GL_SYNC_CONDITION_APPLE                                 = 0x9113,
    GL_SYNC_STATUS_APPLE                                    = 0x9114,
    GL_SYNC_FLAGS_APPLE                                     = 0x9115,
    GL_SYNC_FENCE_APPLE                                     = 0x9116,
    GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE                     = 0x9117,
    GL_UNSIGNALED_APPLE                                     = 0x9118,
    GL_SIGNALED_APPLE                                       = 0x9119,
    GL_ALREADY_SIGNALED_APPLE                               = 0x911A,
    GL_TIMEOUT_EXPIRED_APPLE                                = 0x911B,
    GL_CONDITION_SATISFIED_APPLE                            = 0x911C,
    GL_WAIT_FAILED_APPLE                                    = 0x911D,
    GL_SYNC_FLUSH_COMMANDS_BIT_APPLE                        = 0x00000001,
};
enum : GLuint64
{
    GL_TIMEOUT_IGNORED_APPLE                                = 0xFFFFFFFFFFFFFFFF,
};
extern GLsync       (KHRONOS_APIENTRY* const& glFenceSyncAPPLE) (GLenum condition, GLbitfield flags);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsSyncAPPLE) (GLsync sync);
extern void         (KHRONOS_APIENTRY* const& glDeleteSyncAPPLE) (GLsync sync);
extern GLenum       (KHRONOS_APIENTRY* const& glClientWaitSyncAPPLE) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void         (KHRONOS_APIENTRY* const& glWaitSyncAPPLE) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void         (KHRONOS_APIENTRY* const& glGetInteger64vAPPLE) (GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetSyncivAPPLE) (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values);
#endif

#ifndef GL_APPLE_texture_format_BGRA8888
#define GL_APPLE_texture_format_BGRA8888 1
enum : GLenum
{
    GL_BGRA_EXT                                             = 0x80E1,
    GL_BGRA8_EXT                                            = 0x93A1,
};
#endif

#ifndef GL_APPLE_texture_max_level
#define GL_APPLE_texture_max_level 1
enum : GLenum
{
    GL_TEXTURE_MAX_LEVEL_APPLE                              = 0x813D,
};
#endif

#ifndef GL_APPLE_texture_packed_float
#define GL_APPLE_texture_packed_float 1
enum : GLenum
{
    GL_UNSIGNED_INT_10F_11F_11F_REV_APPLE                   = 0x8C3B,
    GL_UNSIGNED_INT_5_9_9_9_REV_APPLE                       = 0x8C3E,
    GL_R11F_G11F_B10F_APPLE                                 = 0x8C3A,
    GL_RGB9_E5_APPLE                                        = 0x8C3D,
};
#endif

#ifndef GL_ARM_mali_program_binary
#define GL_ARM_mali_program_binary 1
enum : GLenum
{
    GL_MALI_PROGRAM_BINARY_ARM                              = 0x8F61,
};
#endif

#ifndef GL_ARM_mali_shader_binary
#define GL_ARM_mali_shader_binary 1
enum : GLenum
{
    GL_MALI_SHADER_BINARY_ARM                               = 0x8F60,
};
#endif

#ifndef GL_ARM_rgba8
#define GL_ARM_rgba8 1
#endif

#ifndef GL_ARM_shader_framebuffer_fetch
#define GL_ARM_shader_framebuffer_fetch 1
enum : GLenum
{
    GL_FETCH_PER_SAMPLE_ARM                                 = 0x8F65,
    GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM            = 0x8F66,
};
#endif

#ifndef GL_ARM_shader_framebuffer_fetch_depth_stencil
#define GL_ARM_shader_framebuffer_fetch_depth_stencil 1
#endif

#ifndef GL_ARM_texture_unnormalized_coordinates
#define GL_ARM_texture_unnormalized_coordinates 1
enum : GLenum
{
    GL_TEXTURE_UNNORMALIZED_COORDINATES_ARM                 = 0x8F6A,
};
#endif

#ifndef GL_DMP_program_binary
#define GL_DMP_program_binary 1
enum : GLenum
{
    GL_SMAPHS30_PROGRAM_BINARY_DMP                          = 0x9251,
    GL_SMAPHS_PROGRAM_BINARY_DMP                            = 0x9252,
    GL_DMP_PROGRAM_BINARY_DMP                               = 0x9253,
};
#endif

#ifndef GL_DMP_shader_binary
#define GL_DMP_shader_binary 1
enum : GLenum
{
    GL_SHADER_BINARY_DMP                                    = 0x9250,
};
#endif

#ifndef GL_EXT_EGL_image_array
#define GL_EXT_EGL_image_array 1
#endif

#ifndef GL_EXT_EGL_image_storage
#define GL_EXT_EGL_image_storage 1
extern void         (KHRONOS_APIENTRY* const& glEGLImageTargetTexStorageEXT) (GLenum target, GLeglImageOES image, const GLint* attrib_list);
extern void         (KHRONOS_APIENTRY* const& glEGLImageTargetTextureStorageEXT) (GLuint texture, GLeglImageOES image, const GLint* attrib_list);
#endif

#ifndef GL_EXT_YUV_target
#define GL_EXT_YUV_target 1
enum : GLenum
{
    GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT                          = 0x8BE7,
    GL_TEXTURE_EXTERNAL_OES                                 = 0x8D65,
    GL_TEXTURE_BINDING_EXTERNAL_OES                         = 0x8D67,
    GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES                     = 0x8D68,
};
#endif

#ifndef GL_EXT_base_instance
#define GL_EXT_base_instance 1
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstancedBaseInstanceEXT) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseInstanceEXT) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseVertexBaseInstanceEXT) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
#endif

#ifndef GL_EXT_blend_func_extended
#define GL_EXT_blend_func_extended 1
enum : GLenum
{
    GL_SRC1_COLOR_EXT                                       = 0x88F9,
    GL_SRC1_ALPHA_EXT                                       = 0x8589,
    GL_ONE_MINUS_SRC1_COLOR_EXT                             = 0x88FA,
    GL_ONE_MINUS_SRC1_ALPHA_EXT                             = 0x88FB,
    GL_SRC_ALPHA_SATURATE_EXT                               = 0x0308,
    GL_LOCATION_INDEX_EXT                                   = 0x930F,
    GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT                     = 0x88FC,
};
extern void         (KHRONOS_APIENTRY* const& glBindFragDataLocationIndexedEXT) (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glBindFragDataLocationEXT) (GLuint program, GLuint color, const GLchar *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetProgramResourceLocationIndexEXT) (GLuint program, GLenum programInterface, const GLchar *name);
extern GLint        (KHRONOS_APIENTRY* const& glGetFragDataIndexEXT) (GLuint program, const GLchar *name);
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

#ifndef GL_EXT_buffer_storage
#define GL_EXT_buffer_storage 1
enum : GLenum
{
    GL_MAP_PERSISTENT_BIT_EXT                               = 0x0040,
    GL_MAP_COHERENT_BIT_EXT                                 = 0x0080,
    GL_DYNAMIC_STORAGE_BIT_EXT                              = 0x0100,
    GL_CLIENT_STORAGE_BIT_EXT                               = 0x0200,
    GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT                 = 0x00004000,
    GL_BUFFER_IMMUTABLE_STORAGE_EXT                         = 0x821F,
    GL_BUFFER_STORAGE_FLAGS_EXT                             = 0x8220,
};
extern void         (KHRONOS_APIENTRY* const& glBufferStorageEXT) (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
#endif

#ifndef GL_EXT_clear_texture
#define GL_EXT_clear_texture 1
extern void         (KHRONOS_APIENTRY* const& glClearTexImageEXT) (GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
extern void         (KHRONOS_APIENTRY* const& glClearTexSubImageEXT) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
#endif

#ifndef GL_EXT_clip_control
#define GL_EXT_clip_control 1
enum : GLenum
{
    GL_LOWER_LEFT_EXT                                       = 0x8CA1,
    GL_UPPER_LEFT_EXT                                       = 0x8CA2,
    GL_NEGATIVE_ONE_TO_ONE_EXT                              = 0x935E,
    GL_ZERO_TO_ONE_EXT                                      = 0x935F,
    GL_CLIP_ORIGIN_EXT                                      = 0x935C,
    GL_CLIP_DEPTH_MODE_EXT                                  = 0x935D,
};
extern void         (KHRONOS_APIENTRY* const& glClipControlEXT) (GLenum origin, GLenum depth);
#endif

#ifndef GL_EXT_clip_cull_distance
#define GL_EXT_clip_cull_distance 1
enum : GLenum
{
    GL_MAX_CLIP_DISTANCES_EXT                               = 0x0D32,
    GL_MAX_CULL_DISTANCES_EXT                               = 0x82F9,
    GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT             = 0x82FA,
    GL_CLIP_DISTANCE0_EXT                                   = 0x3000,
    GL_CLIP_DISTANCE1_EXT                                   = 0x3001,
    GL_CLIP_DISTANCE2_EXT                                   = 0x3002,
    GL_CLIP_DISTANCE3_EXT                                   = 0x3003,
    GL_CLIP_DISTANCE4_EXT                                   = 0x3004,
    GL_CLIP_DISTANCE5_EXT                                   = 0x3005,
    GL_CLIP_DISTANCE6_EXT                                   = 0x3006,
    GL_CLIP_DISTANCE7_EXT                                   = 0x3007,
};
#endif

#ifndef GL_EXT_color_buffer_float
#define GL_EXT_color_buffer_float 1
#endif

#ifndef GL_EXT_color_buffer_half_float
#define GL_EXT_color_buffer_half_float 1
enum : GLenum
{
    GL_RGBA16F_EXT                                          = 0x881A,
    GL_RGB16F_EXT                                           = 0x881B,
    GL_RG16F_EXT                                            = 0x822F,
    GL_R16F_EXT                                             = 0x822D,
    GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT            = 0x8211,
    GL_UNSIGNED_NORMALIZED_EXT                              = 0x8C17,
};
#endif

#ifndef GL_EXT_conservative_depth
#define GL_EXT_conservative_depth 1
#endif

#ifndef GL_EXT_copy_image
#define GL_EXT_copy_image 1
extern void         (KHRONOS_APIENTRY* const& glCopyImageSubDataEXT) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
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

#ifndef GL_EXT_depth_clamp
#define GL_EXT_depth_clamp 1
enum : GLenum
{
    GL_DEPTH_CLAMP_EXT                                      = 0x864F,
};
#endif

#ifndef GL_EXT_discard_framebuffer
#define GL_EXT_discard_framebuffer 1
enum : GLenum
{
    GL_COLOR_EXT                                            = 0x1800,
    GL_DEPTH_EXT                                            = 0x1801,
    GL_STENCIL_EXT                                          = 0x1802,
};
extern void         (KHRONOS_APIENTRY* const& glDiscardFramebufferEXT) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
#endif

#ifndef GL_EXT_disjoint_timer_query
#define GL_EXT_disjoint_timer_query 1
enum : GLenum
{
    GL_QUERY_COUNTER_BITS_EXT                               = 0x8864,
    GL_CURRENT_QUERY_EXT                                    = 0x8865,
    GL_QUERY_RESULT_EXT                                     = 0x8866,
    GL_QUERY_RESULT_AVAILABLE_EXT                           = 0x8867,
    GL_TIME_ELAPSED_EXT                                     = 0x88BF,
    GL_TIMESTAMP_EXT                                        = 0x8E28,
    GL_GPU_DISJOINT_EXT                                     = 0x8FBB,
};
extern void         (KHRONOS_APIENTRY* const& glGenQueriesEXT) (GLsizei n, GLuint *ids);
extern void         (KHRONOS_APIENTRY* const& glDeleteQueriesEXT) (GLsizei n, const GLuint *ids);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsQueryEXT) (GLuint id);
extern void         (KHRONOS_APIENTRY* const& glBeginQueryEXT) (GLenum target, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glEndQueryEXT) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glQueryCounterEXT) (GLuint id, GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetQueryivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectivEXT) (GLuint id, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectuivEXT) (GLuint id, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjecti64vEXT) (GLuint id, GLenum pname, GLint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetQueryObjectui64vEXT) (GLuint id, GLenum pname, GLuint64 *params);
extern void         (KHRONOS_APIENTRY* const& glGetInteger64vEXT) (GLenum pname, GLint64 *data);
#endif

#ifndef GL_EXT_draw_buffers
#define GL_EXT_draw_buffers 1
enum : GLenum
{
    GL_MAX_COLOR_ATTACHMENTS_EXT                            = 0x8CDF,
    GL_MAX_DRAW_BUFFERS_EXT                                 = 0x8824,
    GL_DRAW_BUFFER0_EXT                                     = 0x8825,
    GL_DRAW_BUFFER1_EXT                                     = 0x8826,
    GL_DRAW_BUFFER2_EXT                                     = 0x8827,
    GL_DRAW_BUFFER3_EXT                                     = 0x8828,
    GL_DRAW_BUFFER4_EXT                                     = 0x8829,
    GL_DRAW_BUFFER5_EXT                                     = 0x882A,
    GL_DRAW_BUFFER6_EXT                                     = 0x882B,
    GL_DRAW_BUFFER7_EXT                                     = 0x882C,
    GL_DRAW_BUFFER8_EXT                                     = 0x882D,
    GL_DRAW_BUFFER9_EXT                                     = 0x882E,
    GL_DRAW_BUFFER10_EXT                                    = 0x882F,
    GL_DRAW_BUFFER11_EXT                                    = 0x8830,
    GL_DRAW_BUFFER12_EXT                                    = 0x8831,
    GL_DRAW_BUFFER13_EXT                                    = 0x8832,
    GL_DRAW_BUFFER14_EXT                                    = 0x8833,
    GL_DRAW_BUFFER15_EXT                                    = 0x8834,
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
};
extern void         (KHRONOS_APIENTRY* const& glDrawBuffersEXT) (GLsizei n, const GLenum *bufs);
#endif

#ifndef GL_EXT_draw_buffers_indexed
#define GL_EXT_draw_buffers_indexed 1
extern void         (KHRONOS_APIENTRY* const& glEnableiEXT) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableiEXT) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationiEXT) (GLuint buf, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparateiEXT) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
extern void         (KHRONOS_APIENTRY* const& glBlendFunciEXT) (GLuint buf, GLenum src, GLenum dst);
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparateiEXT) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern void         (KHRONOS_APIENTRY* const& glColorMaskiEXT) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnablediEXT) (GLenum target, GLuint index);
#endif

#ifndef GL_EXT_draw_elements_base_vertex
#define GL_EXT_draw_elements_base_vertex 1
extern void         (KHRONOS_APIENTRY* const& glDrawElementsBaseVertexEXT) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElementsBaseVertexEXT) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseVertexEXT) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsBaseVertexEXT) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
#endif

#ifndef GL_EXT_draw_instanced
#define GL_EXT_draw_instanced 1
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstancedEXT) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedEXT) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#endif

#ifndef GL_EXT_draw_transform_feedback
#define GL_EXT_draw_transform_feedback 1
extern void         (KHRONOS_APIENTRY* const& glDrawTransformFeedbackEXT) (GLenum mode, GLuint id);
extern void         (KHRONOS_APIENTRY* const& glDrawTransformFeedbackInstancedEXT) (GLenum mode, GLuint id, GLsizei instancecount);
#endif

#ifndef GL_EXT_external_buffer
#define GL_EXT_external_buffer 1
extern void         (KHRONOS_APIENTRY* const& glBufferStorageExternalEXT) (GLenum target, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags);
extern void         (KHRONOS_APIENTRY* const& glNamedBufferStorageExternalEXT) (GLuint buffer, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags);
#endif

#ifndef GL_EXT_float_blend
#define GL_EXT_float_blend 1
#endif

#ifndef GL_EXT_geometry_point_size
#define GL_EXT_geometry_point_size 1
#endif

#ifndef GL_EXT_geometry_shader
#define GL_EXT_geometry_shader 1
enum : GLenum
{
    GL_GEOMETRY_SHADER_EXT                                  = 0x8DD9,
    GL_GEOMETRY_SHADER_BIT_EXT                              = 0x00000004,
    GL_GEOMETRY_LINKED_VERTICES_OUT_EXT                     = 0x8916,
    GL_GEOMETRY_LINKED_INPUT_TYPE_EXT                       = 0x8917,
    GL_GEOMETRY_LINKED_OUTPUT_TYPE_EXT                      = 0x8918,
    GL_GEOMETRY_SHADER_INVOCATIONS_EXT                      = 0x887F,
    GL_LAYER_PROVOKING_VERTEX_EXT                           = 0x825E,
    GL_LINES_ADJACENCY_EXT                                  = 0x000A,
    GL_LINE_STRIP_ADJACENCY_EXT                             = 0x000B,
    GL_TRIANGLES_ADJACENCY_EXT                              = 0x000C,
    GL_TRIANGLE_STRIP_ADJACENCY_EXT                         = 0x000D,
    GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT                  = 0x8DDF,
    GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT                      = 0x8A2C,
    GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT         = 0x8A32,
    GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT                    = 0x9123,
    GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT                   = 0x9124,
    GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT                     = 0x8DE0,
    GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT             = 0x8DE1,
    GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT                  = 0x8E5A,
    GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT                 = 0x8C29,
    GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT              = 0x92CF,
    GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT                     = 0x92D5,
    GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT                      = 0x90CD,
    GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT               = 0x90D7,
    GL_FIRST_VERTEX_CONVENTION_EXT                          = 0x8E4D,
    GL_LAST_VERTEX_CONVENTION_EXT                           = 0x8E4E,
    GL_UNDEFINED_VERTEX_EXT                                 = 0x8260,
    GL_PRIMITIVES_GENERATED_EXT                             = 0x8C87,
    GL_FRAMEBUFFER_DEFAULT_LAYERS_EXT                       = 0x9312,
    GL_MAX_FRAMEBUFFER_LAYERS_EXT                           = 0x9317,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT             = 0x8DA8,
    GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT                   = 0x8DA7,
    GL_REFERENCED_BY_GEOMETRY_SHADER_EXT                    = 0x9309,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureEXT) (GLenum target, GLenum attachment, GLuint texture, GLint level);
#endif

#ifndef GL_EXT_gpu_shader5
#define GL_EXT_gpu_shader5 1
#endif

#ifndef GL_EXT_instanced_arrays
#define GL_EXT_instanced_arrays 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_DIVISOR_EXT                      = 0x88FE,
};
extern void         (KHRONOS_APIENTRY* const& glVertexAttribDivisorEXT) (GLuint index, GLuint divisor);
#endif

#ifndef GL_EXT_map_buffer_range
#define GL_EXT_map_buffer_range 1
enum : GLenum
{
    GL_MAP_READ_BIT_EXT                                     = 0x0001,
    GL_MAP_WRITE_BIT_EXT                                    = 0x0002,
    GL_MAP_INVALIDATE_RANGE_BIT_EXT                         = 0x0004,
    GL_MAP_INVALIDATE_BUFFER_BIT_EXT                        = 0x0008,
    GL_MAP_FLUSH_EXPLICIT_BIT_EXT                           = 0x0010,
    GL_MAP_UNSYNCHRONIZED_BIT_EXT                           = 0x0020,
};
extern void *       (KHRONOS_APIENTRY* const& glMapBufferRangeEXT) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern void         (KHRONOS_APIENTRY* const& glFlushMappedBufferRangeEXT) (GLenum target, GLintptr offset, GLsizeiptr length);
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

#ifndef GL_EXT_multi_draw_arrays
#define GL_EXT_multi_draw_arrays 1
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysEXT) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsEXT) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount);
#endif

#ifndef GL_EXT_multi_draw_indirect
#define GL_EXT_multi_draw_indirect 1
extern void         (KHRONOS_APIENTRY* const& glMultiDrawArraysIndirectEXT) (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
extern void         (KHRONOS_APIENTRY* const& glMultiDrawElementsIndirectEXT) (GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
#endif

#ifndef GL_EXT_multisampled_compatibility
#define GL_EXT_multisampled_compatibility 1
enum : GLenum
{
    GL_MULTISAMPLE_EXT                                      = 0x809D,
    GL_SAMPLE_ALPHA_TO_ONE_EXT                              = 0x809F,
};
#endif

#ifndef GL_EXT_multisampled_render_to_texture
#define GL_EXT_multisampled_render_to_texture 1
enum : GLenum
{
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT           = 0x8D6C,
    GL_RENDERBUFFER_SAMPLES_EXT                             = 0x8CAB,
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT               = 0x8D56,
    GL_MAX_SAMPLES_EXT                                      = 0x8D57,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleEXT) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture2DMultisampleEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
#endif

#ifndef GL_EXT_multisampled_render_to_texture2
#define GL_EXT_multisampled_render_to_texture2 1
#endif

#ifndef GL_EXT_multiview_draw_buffers
#define GL_EXT_multiview_draw_buffers 1
enum : GLenum
{
    GL_COLOR_ATTACHMENT_EXT                                 = 0x90F0,
    GL_MULTIVIEW_EXT                                        = 0x90F1,
    GL_DRAW_BUFFER_EXT                                      = 0x0C01,
    GL_READ_BUFFER_EXT                                      = 0x0C02,
    GL_MAX_MULTIVIEW_BUFFERS_EXT                            = 0x90F2,
};
extern void         (KHRONOS_APIENTRY* const& glReadBufferIndexedEXT) (GLenum src, GLint index);
extern void         (KHRONOS_APIENTRY* const& glDrawBuffersIndexedEXT) (GLint n, const GLenum *location, const GLint *indices);
extern void         (KHRONOS_APIENTRY* const& glGetIntegeri_vEXT) (GLenum target, GLuint index, GLint *data);
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

#ifndef GL_EXT_occlusion_query_boolean
#define GL_EXT_occlusion_query_boolean 1
enum : GLenum
{
    GL_ANY_SAMPLES_PASSED_EXT                               = 0x8C2F,
    GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT                  = 0x8D6A,
};
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

#ifndef GL_EXT_primitive_bounding_box
#define GL_EXT_primitive_bounding_box 1
enum : GLenum
{
    GL_PRIMITIVE_BOUNDING_BOX_EXT                           = 0x92BE,
};
extern void         (KHRONOS_APIENTRY* const& glPrimitiveBoundingBoxEXT) (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);
#endif

#ifndef GL_EXT_protected_textures
#define GL_EXT_protected_textures 1
enum : GLenum
{
    GL_CONTEXT_FLAG_PROTECTED_CONTENT_BIT_EXT               = 0x00000010,
    GL_TEXTURE_PROTECTED_EXT                                = 0x8BFA,
};
#endif

#ifndef GL_EXT_pvrtc_sRGB
#define GL_EXT_pvrtc_sRGB 1
enum : GLenum
{
    GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT                     = 0x8A54,
    GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT                     = 0x8A55,
    GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT               = 0x8A56,
    GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT               = 0x8A57,
    GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG               = 0x93F0,
    GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG               = 0x93F1,
};
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

#ifndef GL_EXT_read_format_bgra
#define GL_EXT_read_format_bgra 1
enum : GLenum
{
    GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT                       = 0x8365,
    GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT                       = 0x8366,
};
#endif

#ifndef GL_EXT_render_snorm
#define GL_EXT_render_snorm 1
enum : GLenum
{
    GL_R16_SNORM_EXT                                        = 0x8F98,
    GL_RG16_SNORM_EXT                                       = 0x8F99,
    GL_RGBA16_SNORM_EXT                                     = 0x8F9B,
};
#endif

#ifndef GL_EXT_robustness
#define GL_EXT_robustness 1
enum : GLenum
{
    GL_GUILTY_CONTEXT_RESET_EXT                             = 0x8253,
    GL_INNOCENT_CONTEXT_RESET_EXT                           = 0x8254,
    GL_UNKNOWN_CONTEXT_RESET_EXT                            = 0x8255,
    GL_CONTEXT_ROBUST_ACCESS_EXT                            = 0x90F3,
    GL_RESET_NOTIFICATION_STRATEGY_EXT                      = 0x8256,
    GL_LOSE_CONTEXT_ON_RESET_EXT                            = 0x8252,
    GL_NO_RESET_NOTIFICATION_EXT                            = 0x8261,
};
extern GLenum       (KHRONOS_APIENTRY* const& glGetGraphicsResetStatusEXT) ();
extern void         (KHRONOS_APIENTRY* const& glReadnPixelsEXT) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformfvEXT) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
extern void         (KHRONOS_APIENTRY* const& glGetnUniformivEXT) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
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

#ifndef GL_EXT_sRGB
#define GL_EXT_sRGB 1
enum : GLenum
{
    GL_SRGB_EXT                                             = 0x8C40,
    GL_SRGB_ALPHA_EXT                                       = 0x8C42,
    GL_SRGB8_ALPHA8_EXT                                     = 0x8C43,
    GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT            = 0x8210,
};
#endif

#ifndef GL_EXT_sRGB_write_control
#define GL_EXT_sRGB_write_control 1
enum : GLenum
{
    GL_FRAMEBUFFER_SRGB_EXT                                 = 0x8DB9,
};
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
extern void         (KHRONOS_APIENTRY* const& glProgramParameteriEXT) (GLuint program, GLenum pname, GLint value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1fEXT) (GLuint program, GLint location, GLfloat v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1iEXT) (GLuint program, GLint location, GLint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2iEXT) (GLuint program, GLint location, GLint v0, GLint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3iEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4iEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUseProgramStagesEXT) (GLuint pipeline, GLbitfield stages, GLuint program);
extern void         (KHRONOS_APIENTRY* const& glValidateProgramPipelineEXT) (GLuint pipeline);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1uiEXT) (GLuint program, GLint location, GLuint v0);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform1uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform2uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform3uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniform4uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix2x4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix3x4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformMatrix4x3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
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

#ifndef GL_EXT_shader_group_vote
#define GL_EXT_shader_group_vote 1
#endif

#ifndef GL_EXT_shader_implicit_conversions
#define GL_EXT_shader_implicit_conversions 1
#endif

#ifndef GL_EXT_shader_integer_mix
#define GL_EXT_shader_integer_mix 1
#endif

#ifndef GL_EXT_shader_io_blocks
#define GL_EXT_shader_io_blocks 1
#endif

#ifndef GL_EXT_shader_non_constant_global_initializers
#define GL_EXT_shader_non_constant_global_initializers 1
#endif

#ifndef GL_EXT_shader_pixel_local_storage
#define GL_EXT_shader_pixel_local_storage 1
enum : GLenum
{
    GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT         = 0x8F63,
    GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT              = 0x8F67,
    GL_SHADER_PIXEL_LOCAL_STORAGE_EXT                       = 0x8F64,
};
#endif

#ifndef GL_EXT_shader_pixel_local_storage2
#define GL_EXT_shader_pixel_local_storage2 1
enum : GLenum
{
    GL_MAX_SHADER_COMBINED_LOCAL_STORAGE_FAST_SIZE_EXT      = 0x9650,
    GL_MAX_SHADER_COMBINED_LOCAL_STORAGE_SIZE_EXT           = 0x9651,
    GL_FRAMEBUFFER_INCOMPLETE_INSUFFICIENT_SHADER_COMBINED_LOCAL_STORAGE_EXT = 0x9652,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferPixelLocalStorageSizeEXT) (GLuint target, GLsizei size);
extern GLsizei      (KHRONOS_APIENTRY* const& glGetFramebufferPixelLocalStorageSizeEXT) (GLuint target);
extern void         (KHRONOS_APIENTRY* const& glClearPixelLocalStorageuiEXT) (GLsizei offset, GLsizei n, const GLuint *values);
#endif

#ifndef GL_EXT_shader_texture_lod
#define GL_EXT_shader_texture_lod 1
#endif

#ifndef GL_EXT_shadow_samplers
#define GL_EXT_shadow_samplers 1
enum : GLenum
{
    GL_TEXTURE_COMPARE_MODE_EXT                             = 0x884C,
    GL_TEXTURE_COMPARE_FUNC_EXT                             = 0x884D,
    GL_COMPARE_REF_TO_TEXTURE_EXT                           = 0x884E,
    GL_SAMPLER_2D_SHADOW_EXT                                = 0x8B62,
};
#endif

#ifndef GL_EXT_sparse_texture
#define GL_EXT_sparse_texture 1
enum : GLenum
{
    GL_TEXTURE_SPARSE_EXT                                   = 0x91A6,
    GL_VIRTUAL_PAGE_SIZE_INDEX_EXT                          = 0x91A7,
    GL_NUM_SPARSE_LEVELS_EXT                                = 0x91AA,
    GL_NUM_VIRTUAL_PAGE_SIZES_EXT                           = 0x91A8,
    GL_VIRTUAL_PAGE_SIZE_X_EXT                              = 0x9195,
    GL_VIRTUAL_PAGE_SIZE_Y_EXT                              = 0x9196,
    GL_VIRTUAL_PAGE_SIZE_Z_EXT                              = 0x9197,
    GL_TEXTURE_CUBE_MAP_ARRAY_OES                           = 0x9009,
    GL_MAX_SPARSE_TEXTURE_SIZE_EXT                          = 0x9198,
    GL_MAX_SPARSE_3D_TEXTURE_SIZE_EXT                       = 0x9199,
    GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_EXT                  = 0x919A,
    GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_EXT           = 0x91A9,
};
extern void         (KHRONOS_APIENTRY* const& glTexPageCommitmentEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
#endif

#ifndef GL_EXT_sparse_texture2
#define GL_EXT_sparse_texture2 1
#endif

#ifndef GL_EXT_tessellation_point_size
#define GL_EXT_tessellation_point_size 1
#endif

#ifndef GL_EXT_tessellation_shader
#define GL_EXT_tessellation_shader 1
enum : GLenum
{
    GL_PATCHES_EXT                                          = 0x000E,
    GL_PATCH_VERTICES_EXT                                   = 0x8E72,
    GL_TESS_CONTROL_OUTPUT_VERTICES_EXT                     = 0x8E75,
    GL_TESS_GEN_MODE_EXT                                    = 0x8E76,
    GL_TESS_GEN_SPACING_EXT                                 = 0x8E77,
    GL_TESS_GEN_VERTEX_ORDER_EXT                            = 0x8E78,
    GL_TESS_GEN_POINT_MODE_EXT                              = 0x8E79,
    GL_ISOLINES_EXT                                         = 0x8E7A,
    GL_QUADS_EXT                                            = 0x0007,
    GL_FRACTIONAL_ODD_EXT                                   = 0x8E7B,
    GL_FRACTIONAL_EVEN_EXT                                  = 0x8E7C,
    GL_MAX_PATCH_VERTICES_EXT                               = 0x8E7D,
    GL_MAX_TESS_GEN_LEVEL_EXT                               = 0x8E7E,
    GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_EXT              = 0x8E7F,
    GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT           = 0x8E80,
    GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_EXT             = 0x8E81,
    GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_EXT          = 0x8E82,
    GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_EXT               = 0x8E83,
    GL_MAX_TESS_PATCH_COMPONENTS_EXT                        = 0x8E84,
    GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_EXT         = 0x8E85,
    GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_EXT            = 0x8E86,
    GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_EXT                  = 0x8E89,
    GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_EXT               = 0x8E8A,
    GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_EXT                = 0x886C,
    GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_EXT             = 0x886D,
    GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_EXT     = 0x8E1E,
    GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT  = 0x8E1F,
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_EXT          = 0x92CD,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_EXT       = 0x92CE,
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_EXT                 = 0x92D3,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_EXT              = 0x92D4,
    GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_EXT                  = 0x90CB,
    GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_EXT               = 0x90CC,
    GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_EXT           = 0x90D8,
    GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_EXT        = 0x90D9,
    GL_IS_PER_PATCH_EXT                                     = 0x92E7,
    GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT                = 0x9307,
    GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT             = 0x9308,
    GL_TESS_CONTROL_SHADER_EXT                              = 0x8E88,
    GL_TESS_EVALUATION_SHADER_EXT                           = 0x8E87,
    GL_TESS_CONTROL_SHADER_BIT_EXT                          = 0x00000008,
    GL_TESS_EVALUATION_SHADER_BIT_EXT                       = 0x00000010,
};
extern void         (KHRONOS_APIENTRY* const& glPatchParameteriEXT) (GLenum pname, GLint value);
#endif

#ifndef GL_EXT_texture_border_clamp
#define GL_EXT_texture_border_clamp 1
enum : GLenum
{
    GL_TEXTURE_BORDER_COLOR_EXT                             = 0x1004,
    GL_CLAMP_TO_BORDER_EXT                                  = 0x812D,
};
extern void         (KHRONOS_APIENTRY* const& glTexParameterIivEXT) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameterIuivEXT) (GLenum target, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIivEXT) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIuivEXT) (GLenum target, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIivEXT) (GLuint sampler, GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIuivEXT) (GLuint sampler, GLenum pname, const GLuint *param);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIivEXT) (GLuint sampler, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIuivEXT) (GLuint sampler, GLenum pname, GLuint *params);
#endif

#ifndef GL_EXT_texture_buffer
#define GL_EXT_texture_buffer 1
enum : GLenum
{
    GL_TEXTURE_BUFFER_EXT                                   = 0x8C2A,
    GL_TEXTURE_BUFFER_BINDING_EXT                           = 0x8C2A,
    GL_MAX_TEXTURE_BUFFER_SIZE_EXT                          = 0x8C2B,
    GL_TEXTURE_BINDING_BUFFER_EXT                           = 0x8C2C,
    GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT                = 0x8C2D,
    GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_EXT                  = 0x919F,
    GL_SAMPLER_BUFFER_EXT                                   = 0x8DC2,
    GL_INT_SAMPLER_BUFFER_EXT                               = 0x8DD0,
    GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT                      = 0x8DD8,
    GL_IMAGE_BUFFER_EXT                                     = 0x9051,
    GL_INT_IMAGE_BUFFER_EXT                                 = 0x905C,
    GL_UNSIGNED_INT_IMAGE_BUFFER_EXT                        = 0x9067,
    GL_TEXTURE_BUFFER_OFFSET_EXT                            = 0x919D,
    GL_TEXTURE_BUFFER_SIZE_EXT                              = 0x919E,
};
extern void         (KHRONOS_APIENTRY* const& glTexBufferEXT) (GLenum target, GLenum internalformat, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTexBufferRangeEXT) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
#endif

#ifndef GL_EXT_texture_compression_astc_decode_mode
#define GL_EXT_texture_compression_astc_decode_mode 1
enum : GLenum
{
    GL_TEXTURE_ASTC_DECODE_PRECISION_EXT                    = 0x8F69,
};
#endif

#ifndef GL_EXT_texture_compression_bptc
#define GL_EXT_texture_compression_bptc 1
enum : GLenum
{
    GL_COMPRESSED_RGBA_BPTC_UNORM_EXT                       = 0x8E8C,
    GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT                 = 0x8E8D,
    GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT                 = 0x8E8E,
    GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT               = 0x8E8F,
};
#endif

#ifndef GL_EXT_texture_compression_dxt1
#define GL_EXT_texture_compression_dxt1 1
enum : GLenum
{
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT                         = 0x83F0,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                        = 0x83F1,
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
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                        = 0x83F2,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                        = 0x83F3,
};
#endif

#ifndef GL_EXT_texture_compression_s3tc_srgb
#define GL_EXT_texture_compression_s3tc_srgb 1
enum : GLenum
{
    GL_COMPRESSED_SRGB_S3TC_DXT1_EXT                        = 0x8C4C,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT                  = 0x8C4D,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT                  = 0x8C4E,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT                  = 0x8C4F,
};
#endif

#ifndef GL_EXT_texture_cube_map_array
#define GL_EXT_texture_cube_map_array 1
enum : GLenum
{
    GL_TEXTURE_CUBE_MAP_ARRAY_EXT                           = 0x9009,
    GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_EXT                   = 0x900A,
    GL_SAMPLER_CUBE_MAP_ARRAY_EXT                           = 0x900C,
    GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_EXT                    = 0x900D,
    GL_INT_SAMPLER_CUBE_MAP_ARRAY_EXT                       = 0x900E,
    GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_EXT              = 0x900F,
    GL_IMAGE_CUBE_MAP_ARRAY_EXT                             = 0x9054,
    GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT                         = 0x905F,
    GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT                = 0x906A,
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

#ifndef GL_EXT_texture_format_BGRA8888
#define GL_EXT_texture_format_BGRA8888 1
#endif

#ifndef GL_EXT_texture_format_sRGB_override
#define GL_EXT_texture_format_sRGB_override 1
enum : GLenum
{
    GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT                     = 0x8FBF,
};
#endif

#ifndef GL_EXT_texture_mirror_clamp_to_edge
#define GL_EXT_texture_mirror_clamp_to_edge 1
enum : GLenum
{
    GL_MIRROR_CLAMP_TO_EDGE_EXT                             = 0x8743,
};
#endif

#ifndef GL_EXT_texture_norm16
#define GL_EXT_texture_norm16 1
enum : GLenum
{
    GL_R16_EXT                                              = 0x822A,
    GL_RG16_EXT                                             = 0x822C,
    GL_RGBA16_EXT                                           = 0x805B,
    GL_RGB16_EXT                                            = 0x8054,
    GL_RGB16_SNORM_EXT                                      = 0x8F9A,
};
#endif

#ifndef GL_EXT_texture_query_lod
#define GL_EXT_texture_query_lod 1
#endif

#ifndef GL_EXT_texture_rg
#define GL_EXT_texture_rg 1
enum : GLenum
{
    GL_RED_EXT                                              = 0x1903,
    GL_RG_EXT                                               = 0x8227,
    GL_R8_EXT                                               = 0x8229,
    GL_RG8_EXT                                              = 0x822B,
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

#ifndef GL_EXT_texture_storage
#define GL_EXT_texture_storage 1
enum : GLenum
{
    GL_TEXTURE_IMMUTABLE_FORMAT_EXT                         = 0x912F,
    GL_ALPHA8_EXT                                           = 0x803C,
    GL_LUMINANCE8_EXT                                       = 0x8040,
    GL_LUMINANCE8_ALPHA8_EXT                                = 0x8045,
    GL_RGBA32F_EXT                                          = 0x8814,
    GL_RGB32F_EXT                                           = 0x8815,
    GL_ALPHA32F_EXT                                         = 0x8816,
    GL_LUMINANCE32F_EXT                                     = 0x8818,
    GL_LUMINANCE_ALPHA32F_EXT                               = 0x8819,
    GL_ALPHA16F_EXT                                         = 0x881C,
    GL_LUMINANCE16F_EXT                                     = 0x881E,
    GL_LUMINANCE_ALPHA16F_EXT                               = 0x881F,
    GL_RGB10_A2_EXT                                         = 0x8059,
    GL_RGB10_EXT                                            = 0x8052,
    GL_R32F_EXT                                             = 0x822E,
    GL_RG32F_EXT                                            = 0x8230,
};
extern void         (KHRONOS_APIENTRY* const& glTexStorage1DEXT) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glTexStorage2DEXT) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTexStorage3DEXT) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage1DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage2DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glTextureStorage3DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
#endif

#ifndef GL_EXT_texture_type_2_10_10_10_REV
#define GL_EXT_texture_type_2_10_10_10_REV 1
enum : GLenum
{
    GL_UNSIGNED_INT_2_10_10_10_REV_EXT                      = 0x8368,
};
#endif

#ifndef GL_EXT_texture_view
#define GL_EXT_texture_view 1
enum : GLenum
{
    GL_TEXTURE_VIEW_MIN_LEVEL_EXT                           = 0x82DB,
    GL_TEXTURE_VIEW_NUM_LEVELS_EXT                          = 0x82DC,
    GL_TEXTURE_VIEW_MIN_LAYER_EXT                           = 0x82DD,
    GL_TEXTURE_VIEW_NUM_LAYERS_EXT                          = 0x82DE,
};
extern void         (KHRONOS_APIENTRY* const& glTextureViewEXT) (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
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

#ifndef GL_EXT_unpack_subimage
#define GL_EXT_unpack_subimage 1
enum : GLenum
{
    GL_UNPACK_ROW_LENGTH_EXT                                = 0x0CF2,
    GL_UNPACK_SKIP_ROWS_EXT                                 = 0x0CF3,
    GL_UNPACK_SKIP_PIXELS_EXT                               = 0x0CF4,
};
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

#ifndef GL_FJ_shader_binary_GCCSO
#define GL_FJ_shader_binary_GCCSO 1
enum : GLenum
{
    GL_GCCSO_SHADER_BINARY_FJ                               = 0x9260,
};
#endif

#ifndef GL_IMG_bindless_texture
#define GL_IMG_bindless_texture 1
extern GLuint64     (KHRONOS_APIENTRY* const& glGetTextureHandleIMG) (GLuint texture);
extern GLuint64     (KHRONOS_APIENTRY* const& glGetTextureSamplerHandleIMG) (GLuint texture, GLuint sampler);
extern void         (KHRONOS_APIENTRY* const& glUniformHandleui64IMG) (GLint location, GLuint64 value);
extern void         (KHRONOS_APIENTRY* const& glUniformHandleui64vIMG) (GLint location, GLsizei count, const GLuint64 *value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformHandleui64IMG) (GLuint program, GLint location, GLuint64 value);
extern void         (KHRONOS_APIENTRY* const& glProgramUniformHandleui64vIMG) (GLuint program, GLint location, GLsizei count, const GLuint64 *values);
#endif

#ifndef GL_IMG_framebuffer_downsample
#define GL_IMG_framebuffer_downsample 1
enum : GLenum
{
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_AND_DOWNSAMPLE_IMG = 0x913C,
    GL_NUM_DOWNSAMPLE_SCALES_IMG                            = 0x913D,
    GL_DOWNSAMPLE_SCALES_IMG                                = 0x913E,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SCALE_IMG             = 0x913F,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture2DDownsampleIMG) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint xscale, GLint yscale);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureLayerDownsampleIMG) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer, GLint xscale, GLint yscale);
#endif

#ifndef GL_IMG_multisampled_render_to_texture
#define GL_IMG_multisampled_render_to_texture 1
enum : GLenum
{
    GL_RENDERBUFFER_SAMPLES_IMG                             = 0x9133,
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG               = 0x9134,
    GL_MAX_SAMPLES_IMG                                      = 0x9135,
    GL_TEXTURE_SAMPLES_IMG                                  = 0x9136,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleIMG) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture2DMultisampleIMG) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
#endif

#ifndef GL_IMG_program_binary
#define GL_IMG_program_binary 1
enum : GLenum
{
    GL_SGX_PROGRAM_BINARY_IMG                               = 0x9130,
};
#endif

#ifndef GL_IMG_read_format
#define GL_IMG_read_format 1
enum : GLenum
{
    GL_BGRA_IMG                                             = 0x80E1,
    GL_UNSIGNED_SHORT_4_4_4_4_REV_IMG                       = 0x8365,
};
#endif

#ifndef GL_IMG_shader_binary
#define GL_IMG_shader_binary 1
enum : GLenum
{
    GL_SGX_BINARY_IMG                                       = 0x8C0A,
};
#endif

#ifndef GL_IMG_texture_compression_pvrtc
#define GL_IMG_texture_compression_pvrtc 1
enum : GLenum
{
    GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      = 0x8C00,
    GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      = 0x8C01,
    GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     = 0x8C02,
    GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     = 0x8C03,
};
#endif

#ifndef GL_IMG_texture_compression_pvrtc2
#define GL_IMG_texture_compression_pvrtc2 1
enum : GLenum
{
    GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG                     = 0x9137,
    GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG                     = 0x9138,
};
#endif

#ifndef GL_IMG_texture_filter_cubic
#define GL_IMG_texture_filter_cubic 1
enum : GLenum
{
    GL_CUBIC_IMG                                            = 0x9139,
    GL_CUBIC_MIPMAP_NEAREST_IMG                             = 0x913A,
    GL_CUBIC_MIPMAP_LINEAR_IMG                              = 0x913B,
};
#endif

#ifndef GL_INTEL_conservative_rasterization
#define GL_INTEL_conservative_rasterization 1
enum : GLenum
{
    GL_CONSERVATIVE_RASTERIZATION_INTEL                     = 0x83FE,
};
#endif

#ifndef GL_INTEL_framebuffer_CMAA
#define GL_INTEL_framebuffer_CMAA 1
extern void         (KHRONOS_APIENTRY* const& glApplyFramebufferAttachmentCMAAINTEL) ();
#endif

#ifndef GL_INTEL_blackhole_render
#define GL_INTEL_blackhole_render 1
enum : GLenum
{
    GL_BLACKHOLE_RENDER_INTEL                               = 0x83FC,
};
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
    GL_CONTEXT_RELEASE_BEHAVIOR                             = 0x82FB,
    GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH                       = 0x82FC,
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
    GL_DISPLAY_LIST                                         = 0x82E7,
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

#ifndef GL_MESA_bgra
#define GL_MESA_bgra 1
enum : GLenum
{
    GL_BGR_EXT                                              = 0x80E0,
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

#ifndef GL_MESA_program_binary_formats
#define GL_MESA_program_binary_formats 1
enum : GLenum
{
    GL_PROGRAM_BINARY_FORMAT_MESA                           = 0x875F,
};
#endif

#ifndef GL_MESA_shader_integer_functions
#define GL_MESA_shader_integer_functions 1
#endif

#ifndef GL_NVX_blend_equation_advanced_multi_draw_buffers
#define GL_NVX_blend_equation_advanced_multi_draw_buffers 1
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
enum : GLenum
{
    GL_FACTOR_MIN_AMD                                       = 0x901C,
    GL_FACTOR_MAX_AMD                                       = 0x901D,
};
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

#ifndef GL_NV_copy_buffer
#define GL_NV_copy_buffer 1
enum : GLenum
{
    GL_COPY_READ_BUFFER_NV                                  = 0x8F36,
    GL_COPY_WRITE_BUFFER_NV                                 = 0x8F37,
};
extern void         (KHRONOS_APIENTRY* const& glCopyBufferSubDataNV) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
#endif

#ifndef GL_NV_coverage_sample
#define GL_NV_coverage_sample 1
enum : GLenum
{
    GL_COVERAGE_COMPONENT_NV                                = 0x8ED0,
    GL_COVERAGE_COMPONENT4_NV                               = 0x8ED1,
    GL_COVERAGE_ATTACHMENT_NV                               = 0x8ED2,
    GL_COVERAGE_BUFFERS_NV                                  = 0x8ED3,
    GL_COVERAGE_SAMPLES_NV                                  = 0x8ED4,
    GL_COVERAGE_ALL_FRAGMENTS_NV                            = 0x8ED5,
    GL_COVERAGE_EDGE_FRAGMENTS_NV                           = 0x8ED6,
    GL_COVERAGE_AUTOMATIC_NV                                = 0x8ED7,
    GL_COVERAGE_BUFFER_BIT_NV                               = 0x00008000,
};
extern void         (KHRONOS_APIENTRY* const& glCoverageMaskNV) (GLboolean mask);
extern void         (KHRONOS_APIENTRY* const& glCoverageOperationNV) (GLenum operation);
#endif

#ifndef GL_NV_depth_nonlinear
#define GL_NV_depth_nonlinear 1
enum : GLenum
{
    GL_DEPTH_COMPONENT16_NONLINEAR_NV                       = 0x8E2C,
};
#endif

#ifndef GL_NV_draw_buffers
#define GL_NV_draw_buffers 1
enum : GLenum
{
    GL_MAX_DRAW_BUFFERS_NV                                  = 0x8824,
    GL_DRAW_BUFFER0_NV                                      = 0x8825,
    GL_DRAW_BUFFER1_NV                                      = 0x8826,
    GL_DRAW_BUFFER2_NV                                      = 0x8827,
    GL_DRAW_BUFFER3_NV                                      = 0x8828,
    GL_DRAW_BUFFER4_NV                                      = 0x8829,
    GL_DRAW_BUFFER5_NV                                      = 0x882A,
    GL_DRAW_BUFFER6_NV                                      = 0x882B,
    GL_DRAW_BUFFER7_NV                                      = 0x882C,
    GL_DRAW_BUFFER8_NV                                      = 0x882D,
    GL_DRAW_BUFFER9_NV                                      = 0x882E,
    GL_DRAW_BUFFER10_NV                                     = 0x882F,
    GL_DRAW_BUFFER11_NV                                     = 0x8830,
    GL_DRAW_BUFFER12_NV                                     = 0x8831,
    GL_DRAW_BUFFER13_NV                                     = 0x8832,
    GL_DRAW_BUFFER14_NV                                     = 0x8833,
    GL_DRAW_BUFFER15_NV                                     = 0x8834,
    GL_COLOR_ATTACHMENT0_NV                                 = 0x8CE0,
    GL_COLOR_ATTACHMENT1_NV                                 = 0x8CE1,
    GL_COLOR_ATTACHMENT2_NV                                 = 0x8CE2,
    GL_COLOR_ATTACHMENT3_NV                                 = 0x8CE3,
    GL_COLOR_ATTACHMENT4_NV                                 = 0x8CE4,
    GL_COLOR_ATTACHMENT5_NV                                 = 0x8CE5,
    GL_COLOR_ATTACHMENT6_NV                                 = 0x8CE6,
    GL_COLOR_ATTACHMENT7_NV                                 = 0x8CE7,
    GL_COLOR_ATTACHMENT8_NV                                 = 0x8CE8,
    GL_COLOR_ATTACHMENT9_NV                                 = 0x8CE9,
    GL_COLOR_ATTACHMENT10_NV                                = 0x8CEA,
    GL_COLOR_ATTACHMENT11_NV                                = 0x8CEB,
    GL_COLOR_ATTACHMENT12_NV                                = 0x8CEC,
    GL_COLOR_ATTACHMENT13_NV                                = 0x8CED,
    GL_COLOR_ATTACHMENT14_NV                                = 0x8CEE,
    GL_COLOR_ATTACHMENT15_NV                                = 0x8CEF,
};
extern void         (KHRONOS_APIENTRY* const& glDrawBuffersNV) (GLsizei n, const GLenum *bufs);
#endif

#ifndef GL_NV_draw_instanced
#define GL_NV_draw_instanced 1
extern void         (KHRONOS_APIENTRY* const& glDrawArraysInstancedNV) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedNV) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
#endif

#ifndef GL_NV_draw_vulkan_image
#define GL_NV_draw_vulkan_image 1
extern void         (KHRONOS_APIENTRY* const& glDrawVkImageNV) (GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1);
extern GLVULKANPROCNV (KHRONOS_APIENTRY* const& glGetVkProcAddrNV) (const GLchar *name);
extern void         (KHRONOS_APIENTRY* const& glWaitVkSemaphoreNV) (GLuint64 vkSemaphore);
extern void         (KHRONOS_APIENTRY* const& glSignalVkSemaphoreNV) (GLuint64 vkSemaphore);
extern void         (KHRONOS_APIENTRY* const& glSignalVkFenceNV) (GLuint64 vkFence);
#endif

#ifndef GL_NV_explicit_attrib_location
#define GL_NV_explicit_attrib_location 1
#endif

#ifndef GL_NV_fbo_color_attachments
#define GL_NV_fbo_color_attachments 1
enum : GLenum
{
    GL_MAX_COLOR_ATTACHMENTS_NV                             = 0x8CDF,
};
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

#ifndef GL_NV_fragment_coverage_to_color
#define GL_NV_fragment_coverage_to_color 1
enum : GLenum
{
    GL_FRAGMENT_COVERAGE_TO_COLOR_NV                        = 0x92DD,
    GL_FRAGMENT_COVERAGE_COLOR_NV                           = 0x92DE,
};
extern void         (KHRONOS_APIENTRY* const& glFragmentCoverageColorNV) (GLuint color);
#endif

#ifndef GL_NV_fragment_shader_barycentric
#define GL_NV_fragment_shader_barycentric 1
#endif

#ifndef GL_NV_fragment_shader_interlock
#define GL_NV_fragment_shader_interlock 1
#endif

#ifndef GL_NV_framebuffer_blit
#define GL_NV_framebuffer_blit 1
enum : GLenum
{
    GL_READ_FRAMEBUFFER_NV                                  = 0x8CA8,
    GL_DRAW_FRAMEBUFFER_NV                                  = 0x8CA9,
    GL_DRAW_FRAMEBUFFER_BINDING_NV                          = 0x8CA6,
    GL_READ_FRAMEBUFFER_BINDING_NV                          = 0x8CAA,
};
extern void         (KHRONOS_APIENTRY* const& glBlitFramebufferNV) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
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

#ifndef GL_NV_framebuffer_multisample
#define GL_NV_framebuffer_multisample 1
enum : GLenum
{
    GL_RENDERBUFFER_SAMPLES_NV                              = 0x8CAB,
    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_NV                = 0x8D56,
    GL_MAX_SAMPLES_NV                                       = 0x8D57,
};
extern void         (KHRONOS_APIENTRY* const& glRenderbufferStorageMultisampleNV) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif

#ifndef GL_NV_generate_mipmap_sRGB
#define GL_NV_generate_mipmap_sRGB 1
#endif

#ifndef GL_NV_geometry_shader_passthrough
#define GL_NV_geometry_shader_passthrough 1
#endif

#ifndef GL_NV_gpu_shader5
#define GL_NV_gpu_shader5 1
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
    GL_FLOAT16_NV                                           = 0x8FF8,
    GL_FLOAT16_VEC2_NV                                      = 0x8FF9,
    GL_FLOAT16_VEC3_NV                                      = 0x8FFA,
    GL_FLOAT16_VEC4_NV                                      = 0x8FFB,
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

#ifndef GL_NV_image_formats
#define GL_NV_image_formats 1
#endif

#ifndef GL_NV_instanced_arrays
#define GL_NV_instanced_arrays 1
enum : GLenum
{
    GL_VERTEX_ATTRIB_ARRAY_DIVISOR_NV                       = 0x88FE,
};
extern void         (KHRONOS_APIENTRY* const& glVertexAttribDivisorNV) (GLuint index, GLuint divisor);
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

#ifndef GL_NV_non_square_matrices
#define GL_NV_non_square_matrices 1
enum : GLenum
{
    GL_FLOAT_MAT2x3_NV                                      = 0x8B65,
    GL_FLOAT_MAT2x4_NV                                      = 0x8B66,
    GL_FLOAT_MAT3x2_NV                                      = 0x8B67,
    GL_FLOAT_MAT3x4_NV                                      = 0x8B68,
    GL_FLOAT_MAT4x2_NV                                      = 0x8B69,
    GL_FLOAT_MAT4x3_NV                                      = 0x8B6A,
};
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x3fvNV) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x2fvNV) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix2x4fvNV) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x2fvNV) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix3x4fvNV) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void         (KHRONOS_APIENTRY* const& glUniformMatrix4x3fvNV) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
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
    GL_PRIMARY_COLOR                                        = 0x8577,
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
extern void         (KHRONOS_APIENTRY* const& glMatrixFrustumEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadIdentityEXT) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadTransposefEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadTransposedEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoadfEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixLoaddEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultTransposefEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultTransposedEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultfEXT) (GLenum mode, const GLfloat *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixMultdEXT) (GLenum mode, const GLdouble *m);
extern void         (KHRONOS_APIENTRY* const& glMatrixOrthoEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void         (KHRONOS_APIENTRY* const& glMatrixPopEXT) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glMatrixPushEXT) (GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glMatrixRotatefEXT) (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glMatrixRotatedEXT) (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glMatrixScalefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glMatrixScaledEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
extern void         (KHRONOS_APIENTRY* const& glMatrixTranslatefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
extern void         (KHRONOS_APIENTRY* const& glMatrixTranslatedEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
#endif

#ifndef GL_NV_path_rendering_shared_edge
#define GL_NV_path_rendering_shared_edge 1
enum : GLenum
{
    GL_SHARED_EDGE_NV                                       = 0xC0,
};
#endif

#ifndef GL_NV_pixel_buffer_object
#define GL_NV_pixel_buffer_object 1
enum : GLenum
{
    GL_PIXEL_PACK_BUFFER_NV                                 = 0x88EB,
    GL_PIXEL_UNPACK_BUFFER_NV                               = 0x88EC,
    GL_PIXEL_PACK_BUFFER_BINDING_NV                         = 0x88ED,
    GL_PIXEL_UNPACK_BUFFER_BINDING_NV                       = 0x88EF,
};
#endif

#ifndef GL_NV_polygon_mode
#define GL_NV_polygon_mode 1
enum : GLenum
{
    GL_POLYGON_MODE_NV                                      = 0x0B40,
    GL_POLYGON_OFFSET_POINT_NV                              = 0x2A01,
    GL_POLYGON_OFFSET_LINE_NV                               = 0x2A02,
    GL_POINT_NV                                             = 0x1B00,
    GL_LINE_NV                                              = 0x1B01,
    GL_FILL_NV                                              = 0x1B02,
};
extern void         (KHRONOS_APIENTRY* const& glPolygonModeNV) (GLenum face, GLenum mode);
#endif

#ifndef GL_NV_primitive_shading_rate
#define GL_NV_primitive_shading_rate 1
enum : GLenum
{
    GL_SHADING_RATE_IMAGE_PER_PRIMITIVE_NV                  = 0x95B1,
    GL_SHADING_RATE_IMAGE_PALETTE_COUNT_NV                  = 0x95B2,
};
#endif

#ifndef GL_NV_read_buffer
#define GL_NV_read_buffer 1
enum : GLenum
{
    GL_READ_BUFFER_NV                                       = 0x0C02,
};
extern void         (KHRONOS_APIENTRY* const& glReadBufferNV) (GLenum mode);
#endif

#ifndef GL_NV_read_buffer_front
#define GL_NV_read_buffer_front 1
#endif

#ifndef GL_NV_read_depth
#define GL_NV_read_depth 1
#endif

#ifndef GL_NV_read_depth_stencil
#define GL_NV_read_depth_stencil 1
#endif

#ifndef GL_NV_read_stencil
#define GL_NV_read_stencil 1
#endif

#ifndef GL_NV_representative_fragment_test
#define GL_NV_representative_fragment_test 1
enum : GLenum
{
    GL_REPRESENTATIVE_FRAGMENT_TEST_NV                      = 0x937F,
};
#endif

#ifndef GL_NV_sRGB_formats
#define GL_NV_sRGB_formats 1
enum : GLenum
{
    GL_SLUMINANCE_NV                                        = 0x8C46,
    GL_SLUMINANCE_ALPHA_NV                                  = 0x8C44,
    GL_SRGB8_NV                                             = 0x8C41,
    GL_SLUMINANCE8_NV                                       = 0x8C47,
    GL_SLUMINANCE8_ALPHA8_NV                                = 0x8C45,
    GL_COMPRESSED_SRGB_S3TC_DXT1_NV                         = 0x8C4C,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV                   = 0x8C4D,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV                   = 0x8C4E,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV                   = 0x8C4F,
    GL_ETC1_SRGB8_NV                                        = 0x88EE,
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

#ifndef GL_NV_shader_atomic_fp16_vector
#define GL_NV_shader_atomic_fp16_vector 1
#endif

#ifndef GL_NV_shader_noperspective_interpolation
#define GL_NV_shader_noperspective_interpolation 1
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

#ifndef GL_NV_shadow_samplers_array
#define GL_NV_shadow_samplers_array 1
enum : GLenum
{
    GL_SAMPLER_2D_ARRAY_SHADOW_NV                           = 0x8DC4,
};
#endif

#ifndef GL_NV_shadow_samplers_cube
#define GL_NV_shadow_samplers_cube 1
enum : GLenum
{
    GL_SAMPLER_CUBE_SHADOW_NV                               = 0x8DC5,
};
#endif

#ifndef GL_NV_stereo_view_rendering
#define GL_NV_stereo_view_rendering 1
#endif

#ifndef GL_NV_texture_border_clamp
#define GL_NV_texture_border_clamp 1
enum : GLenum
{
    GL_TEXTURE_BORDER_COLOR_NV                              = 0x1004,
    GL_CLAMP_TO_BORDER_NV                                   = 0x812D,
};
#endif

#ifndef GL_NV_texture_compression_s3tc_update
#define GL_NV_texture_compression_s3tc_update 1
#endif

#ifndef GL_NV_texture_npot_2D_mipmap
#define GL_NV_texture_npot_2D_mipmap 1
#endif

#ifndef GL_NV_viewport_array
#define GL_NV_viewport_array 1
enum : GLenum
{
    GL_MAX_VIEWPORTS_NV                                     = 0x825B,
    GL_VIEWPORT_SUBPIXEL_BITS_NV                            = 0x825C,
    GL_VIEWPORT_BOUNDS_RANGE_NV                             = 0x825D,
    GL_VIEWPORT_INDEX_PROVOKING_VERTEX_NV                   = 0x825F,
};
extern void         (KHRONOS_APIENTRY* const& glViewportArrayvNV) (GLuint first, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glViewportIndexedfNV) (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
extern void         (KHRONOS_APIENTRY* const& glViewportIndexedfvNV) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glScissorArrayvNV) (GLuint first, GLsizei count, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glScissorIndexedNV) (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glScissorIndexedvNV) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glDepthRangeArrayfvNV) (GLuint first, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glDepthRangeIndexedfNV) (GLuint index, GLfloat n, GLfloat f);
extern void         (KHRONOS_APIENTRY* const& glGetFloati_vNV) (GLenum target, GLuint index, GLfloat *data);
extern void         (KHRONOS_APIENTRY* const& glEnableiNV) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableiNV) (GLenum target, GLuint index);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnablediNV) (GLenum target, GLuint index);
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

#ifndef GL_OES_EGL_image
#define GL_OES_EGL_image 1
extern void         (KHRONOS_APIENTRY* const& glEGLImageTargetTexture2DOES) (GLenum target, GLeglImageOES image);
extern void         (KHRONOS_APIENTRY* const& glEGLImageTargetRenderbufferStorageOES) (GLenum target, GLeglImageOES image);
#endif

#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1
enum : GLenum
{
    GL_SAMPLER_EXTERNAL_OES                                 = 0x8D66,
};
#endif

#ifndef GL_OES_EGL_image_external_essl3
#define GL_OES_EGL_image_external_essl3 1
#endif

#ifndef GL_OES_compressed_ETC1_RGB8_sub_texture
#define GL_OES_compressed_ETC1_RGB8_sub_texture 1
#endif

#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_OES_compressed_ETC1_RGB8_texture 1
enum : GLenum
{
    GL_ETC1_RGB8_OES                                        = 0x8D64,
};
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

#ifndef GL_OES_copy_image
#define GL_OES_copy_image 1
extern void         (KHRONOS_APIENTRY* const& glCopyImageSubDataOES) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
#endif

#ifndef GL_OES_depth24
#define GL_OES_depth24 1
enum : GLenum
{
    GL_DEPTH_COMPONENT24_OES                                = 0x81A6,
};
#endif

#ifndef GL_OES_depth32
#define GL_OES_depth32 1
#endif

#ifndef GL_OES_depth_texture
#define GL_OES_depth_texture 1
#endif

#ifndef GL_OES_draw_buffers_indexed
#define GL_OES_draw_buffers_indexed 1
extern void         (KHRONOS_APIENTRY* const& glEnableiOES) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glDisableiOES) (GLenum target, GLuint index);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationiOES) (GLuint buf, GLenum mode);
extern void         (KHRONOS_APIENTRY* const& glBlendEquationSeparateiOES) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
extern void         (KHRONOS_APIENTRY* const& glBlendFunciOES) (GLuint buf, GLenum src, GLenum dst);
extern void         (KHRONOS_APIENTRY* const& glBlendFuncSeparateiOES) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern void         (KHRONOS_APIENTRY* const& glColorMaskiOES) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsEnablediOES) (GLenum target, GLuint index);
#endif

#ifndef GL_OES_draw_elements_base_vertex
#define GL_OES_draw_elements_base_vertex 1
extern void         (KHRONOS_APIENTRY* const& glDrawElementsBaseVertexOES) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawRangeElementsBaseVertexOES) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void         (KHRONOS_APIENTRY* const& glDrawElementsInstancedBaseVertexOES) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
#endif

#ifndef GL_OES_element_index_uint
#define GL_OES_element_index_uint 1
#endif

#ifndef GL_OES_fbo_render_mipmap
#define GL_OES_fbo_render_mipmap 1
#endif

#ifndef GL_OES_fragment_precision_high
#define GL_OES_fragment_precision_high 1
#endif

#ifndef GL_OES_geometry_point_size
#define GL_OES_geometry_point_size 1
#endif

#ifndef GL_OES_geometry_shader
#define GL_OES_geometry_shader 1
enum : GLenum
{
    GL_GEOMETRY_SHADER_OES                                  = 0x8DD9,
    GL_GEOMETRY_SHADER_BIT_OES                              = 0x00000004,
    GL_GEOMETRY_LINKED_VERTICES_OUT_OES                     = 0x8916,
    GL_GEOMETRY_LINKED_INPUT_TYPE_OES                       = 0x8917,
    GL_GEOMETRY_LINKED_OUTPUT_TYPE_OES                      = 0x8918,
    GL_GEOMETRY_SHADER_INVOCATIONS_OES                      = 0x887F,
    GL_LAYER_PROVOKING_VERTEX_OES                           = 0x825E,
    GL_LINES_ADJACENCY_OES                                  = 0x000A,
    GL_LINE_STRIP_ADJACENCY_OES                             = 0x000B,
    GL_TRIANGLES_ADJACENCY_OES                              = 0x000C,
    GL_TRIANGLE_STRIP_ADJACENCY_OES                         = 0x000D,
    GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_OES                  = 0x8DDF,
    GL_MAX_GEOMETRY_UNIFORM_BLOCKS_OES                      = 0x8A2C,
    GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_OES         = 0x8A32,
    GL_MAX_GEOMETRY_INPUT_COMPONENTS_OES                    = 0x9123,
    GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_OES                   = 0x9124,
    GL_MAX_GEOMETRY_OUTPUT_VERTICES_OES                     = 0x8DE0,
    GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_OES             = 0x8DE1,
    GL_MAX_GEOMETRY_SHADER_INVOCATIONS_OES                  = 0x8E5A,
    GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_OES                 = 0x8C29,
    GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_OES              = 0x92CF,
    GL_MAX_GEOMETRY_ATOMIC_COUNTERS_OES                     = 0x92D5,
    GL_MAX_GEOMETRY_IMAGE_UNIFORMS_OES                      = 0x90CD,
    GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_OES               = 0x90D7,
    GL_FIRST_VERTEX_CONVENTION_OES                          = 0x8E4D,
    GL_LAST_VERTEX_CONVENTION_OES                           = 0x8E4E,
    GL_UNDEFINED_VERTEX_OES                                 = 0x8260,
    GL_PRIMITIVES_GENERATED_OES                             = 0x8C87,
    GL_FRAMEBUFFER_DEFAULT_LAYERS_OES                       = 0x9312,
    GL_MAX_FRAMEBUFFER_LAYERS_OES                           = 0x9317,
    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_OES             = 0x8DA8,
    GL_FRAMEBUFFER_ATTACHMENT_LAYERED_OES                   = 0x8DA7,
    GL_REFERENCED_BY_GEOMETRY_SHADER_OES                    = 0x9309,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureOES) (GLenum target, GLenum attachment, GLuint texture, GLint level);
#endif

#ifndef GL_OES_get_program_binary
#define GL_OES_get_program_binary 1
enum : GLenum
{
    GL_PROGRAM_BINARY_LENGTH_OES                            = 0x8741,
    GL_NUM_PROGRAM_BINARY_FORMATS_OES                       = 0x87FE,
    GL_PROGRAM_BINARY_FORMATS_OES                           = 0x87FF,
};
extern void         (KHRONOS_APIENTRY* const& glGetProgramBinaryOES) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
extern void         (KHRONOS_APIENTRY* const& glProgramBinaryOES) (GLuint program, GLenum binaryFormat, const void *binary, GLint length);
#endif

#ifndef GL_OES_gpu_shader5
#define GL_OES_gpu_shader5 1
#endif

#ifndef GL_OES_mapbuffer
#define GL_OES_mapbuffer 1
enum : GLenum
{
    GL_WRITE_ONLY_OES                                       = 0x88B9,
    GL_BUFFER_ACCESS_OES                                    = 0x88BB,
    GL_BUFFER_MAPPED_OES                                    = 0x88BC,
    GL_BUFFER_MAP_POINTER_OES                               = 0x88BD,
};
extern void *       (KHRONOS_APIENTRY* const& glMapBufferOES) (GLenum target, GLenum access);
extern GLboolean    (KHRONOS_APIENTRY* const& glUnmapBufferOES) (GLenum target);
extern void         (KHRONOS_APIENTRY* const& glGetBufferPointervOES) (GLenum target, GLenum pname, void **params);
#endif

#ifndef GL_OES_packed_depth_stencil
#define GL_OES_packed_depth_stencil 1
#endif

#ifndef GL_OES_primitive_bounding_box
#define GL_OES_primitive_bounding_box 1
enum : GLenum
{
    GL_PRIMITIVE_BOUNDING_BOX_OES                           = 0x92BE,
};
extern void         (KHRONOS_APIENTRY* const& glPrimitiveBoundingBoxOES) (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);
#endif

#ifndef GL_OES_required_internalformat
#define GL_OES_required_internalformat 1
enum : GLenum
{
    GL_ALPHA8_OES                                           = 0x803C,
    GL_DEPTH_COMPONENT16_OES                                = 0x81A5,
    GL_LUMINANCE4_ALPHA4_OES                                = 0x8043,
    GL_LUMINANCE8_ALPHA8_OES                                = 0x8045,
    GL_LUMINANCE8_OES                                       = 0x8040,
    GL_RGBA4_OES                                            = 0x8056,
    GL_RGB5_A1_OES                                          = 0x8057,
    GL_RGB565_OES                                           = 0x8D62,
    GL_RGB8_OES                                             = 0x8051,
    GL_RGBA8_OES                                            = 0x8058,
};
#endif

#ifndef GL_OES_rgb8_rgba8
#define GL_OES_rgb8_rgba8 1
#endif

#ifndef GL_OES_sample_shading
#define GL_OES_sample_shading 1
enum : GLenum
{
    GL_SAMPLE_SHADING_OES                                   = 0x8C36,
    GL_MIN_SAMPLE_SHADING_VALUE_OES                         = 0x8C37,
};
extern void         (KHRONOS_APIENTRY* const& glMinSampleShadingOES) (GLfloat value);
#endif

#ifndef GL_OES_sample_variables
#define GL_OES_sample_variables 1
#endif

#ifndef GL_OES_shader_image_atomic
#define GL_OES_shader_image_atomic 1
#endif

#ifndef GL_OES_shader_io_blocks
#define GL_OES_shader_io_blocks 1
#endif

#ifndef GL_OES_shader_multisample_interpolation
#define GL_OES_shader_multisample_interpolation 1
enum : GLenum
{
    GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES                = 0x8E5B,
    GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES                = 0x8E5C,
    GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES               = 0x8E5D,
};
#endif

#ifndef GL_OES_standard_derivatives
#define GL_OES_standard_derivatives 1
enum : GLenum
{
    GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES                  = 0x8B8B,
};
#endif

#ifndef GL_OES_stencil1
#define GL_OES_stencil1 1
enum : GLenum
{
    GL_STENCIL_INDEX1_OES                                   = 0x8D46,
};
#endif

#ifndef GL_OES_stencil4
#define GL_OES_stencil4 1
enum : GLenum
{
    GL_STENCIL_INDEX4_OES                                   = 0x8D47,
};
#endif

#ifndef GL_OES_surfaceless_context
#define GL_OES_surfaceless_context 1
enum : GLenum
{
    GL_FRAMEBUFFER_UNDEFINED_OES                            = 0x8219,
};
#endif

#ifndef GL_OES_tessellation_point_size
#define GL_OES_tessellation_point_size 1
#endif

#ifndef GL_OES_tessellation_shader
#define GL_OES_tessellation_shader 1
enum : GLenum
{
    GL_PATCHES_OES                                          = 0x000E,
    GL_PATCH_VERTICES_OES                                   = 0x8E72,
    GL_TESS_CONTROL_OUTPUT_VERTICES_OES                     = 0x8E75,
    GL_TESS_GEN_MODE_OES                                    = 0x8E76,
    GL_TESS_GEN_SPACING_OES                                 = 0x8E77,
    GL_TESS_GEN_VERTEX_ORDER_OES                            = 0x8E78,
    GL_TESS_GEN_POINT_MODE_OES                              = 0x8E79,
    GL_ISOLINES_OES                                         = 0x8E7A,
    GL_QUADS_OES                                            = 0x0007,
    GL_FRACTIONAL_ODD_OES                                   = 0x8E7B,
    GL_FRACTIONAL_EVEN_OES                                  = 0x8E7C,
    GL_MAX_PATCH_VERTICES_OES                               = 0x8E7D,
    GL_MAX_TESS_GEN_LEVEL_OES                               = 0x8E7E,
    GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_OES              = 0x8E7F,
    GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_OES           = 0x8E80,
    GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_OES             = 0x8E81,
    GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_OES          = 0x8E82,
    GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_OES               = 0x8E83,
    GL_MAX_TESS_PATCH_COMPONENTS_OES                        = 0x8E84,
    GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_OES         = 0x8E85,
    GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_OES            = 0x8E86,
    GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_OES                  = 0x8E89,
    GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_OES               = 0x8E8A,
    GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_OES                = 0x886C,
    GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_OES             = 0x886D,
    GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_OES     = 0x8E1E,
    GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_OES  = 0x8E1F,
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_OES          = 0x92CD,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_OES       = 0x92CE,
    GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_OES                 = 0x92D3,
    GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_OES              = 0x92D4,
    GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_OES                  = 0x90CB,
    GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_OES               = 0x90CC,
    GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_OES           = 0x90D8,
    GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_OES        = 0x90D9,
    GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED_OES          = 0x8221,
    GL_IS_PER_PATCH_OES                                     = 0x92E7,
    GL_REFERENCED_BY_TESS_CONTROL_SHADER_OES                = 0x9307,
    GL_REFERENCED_BY_TESS_EVALUATION_SHADER_OES             = 0x9308,
    GL_TESS_CONTROL_SHADER_OES                              = 0x8E88,
    GL_TESS_EVALUATION_SHADER_OES                           = 0x8E87,
    GL_TESS_CONTROL_SHADER_BIT_OES                          = 0x00000008,
    GL_TESS_EVALUATION_SHADER_BIT_OES                       = 0x00000010,
};
extern void         (KHRONOS_APIENTRY* const& glPatchParameteriOES) (GLenum pname, GLint value);
#endif

#ifndef GL_OES_texture_3D
#define GL_OES_texture_3D 1
enum : GLenum
{
    GL_TEXTURE_WRAP_R_OES                                   = 0x8072,
    GL_TEXTURE_3D_OES                                       = 0x806F,
    GL_TEXTURE_BINDING_3D_OES                               = 0x806A,
    GL_MAX_3D_TEXTURE_SIZE_OES                              = 0x8073,
    GL_SAMPLER_3D_OES                                       = 0x8B5F,
    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES        = 0x8CD4,
};
extern void         (KHRONOS_APIENTRY* const& glTexImage3DOES) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void         (KHRONOS_APIENTRY* const& glCopyTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexImage3DOES) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glCompressedTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
extern void         (KHRONOS_APIENTRY* const& glFramebufferTexture3DOES) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
#endif

#ifndef GL_OES_texture_border_clamp
#define GL_OES_texture_border_clamp 1
enum : GLenum
{
    GL_TEXTURE_BORDER_COLOR_OES                             = 0x1004,
    GL_CLAMP_TO_BORDER_OES                                  = 0x812D,
};
extern void         (KHRONOS_APIENTRY* const& glTexParameterIivOES) (GLenum target, GLenum pname, const GLint *params);
extern void         (KHRONOS_APIENTRY* const& glTexParameterIuivOES) (GLenum target, GLenum pname, const GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIivOES) (GLenum target, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetTexParameterIuivOES) (GLenum target, GLenum pname, GLuint *params);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIivOES) (GLuint sampler, GLenum pname, const GLint *param);
extern void         (KHRONOS_APIENTRY* const& glSamplerParameterIuivOES) (GLuint sampler, GLenum pname, const GLuint *param);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIivOES) (GLuint sampler, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glGetSamplerParameterIuivOES) (GLuint sampler, GLenum pname, GLuint *params);
#endif

#ifndef GL_OES_texture_buffer
#define GL_OES_texture_buffer 1
enum : GLenum
{
    GL_TEXTURE_BUFFER_OES                                   = 0x8C2A,
    GL_TEXTURE_BUFFER_BINDING_OES                           = 0x8C2A,
    GL_MAX_TEXTURE_BUFFER_SIZE_OES                          = 0x8C2B,
    GL_TEXTURE_BINDING_BUFFER_OES                           = 0x8C2C,
    GL_TEXTURE_BUFFER_DATA_STORE_BINDING_OES                = 0x8C2D,
    GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_OES                  = 0x919F,
    GL_SAMPLER_BUFFER_OES                                   = 0x8DC2,
    GL_INT_SAMPLER_BUFFER_OES                               = 0x8DD0,
    GL_UNSIGNED_INT_SAMPLER_BUFFER_OES                      = 0x8DD8,
    GL_IMAGE_BUFFER_OES                                     = 0x9051,
    GL_INT_IMAGE_BUFFER_OES                                 = 0x905C,
    GL_UNSIGNED_INT_IMAGE_BUFFER_OES                        = 0x9067,
    GL_TEXTURE_BUFFER_OFFSET_OES                            = 0x919D,
    GL_TEXTURE_BUFFER_SIZE_OES                              = 0x919E,
};
extern void         (KHRONOS_APIENTRY* const& glTexBufferOES) (GLenum target, GLenum internalformat, GLuint buffer);
extern void         (KHRONOS_APIENTRY* const& glTexBufferRangeOES) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
#endif

#ifndef GL_OES_texture_compression_astc
#define GL_OES_texture_compression_astc 1
enum : GLenum
{
    GL_COMPRESSED_RGBA_ASTC_3x3x3_OES                       = 0x93C0,
    GL_COMPRESSED_RGBA_ASTC_4x3x3_OES                       = 0x93C1,
    GL_COMPRESSED_RGBA_ASTC_4x4x3_OES                       = 0x93C2,
    GL_COMPRESSED_RGBA_ASTC_4x4x4_OES                       = 0x93C3,
    GL_COMPRESSED_RGBA_ASTC_5x4x4_OES                       = 0x93C4,
    GL_COMPRESSED_RGBA_ASTC_5x5x4_OES                       = 0x93C5,
    GL_COMPRESSED_RGBA_ASTC_5x5x5_OES                       = 0x93C6,
    GL_COMPRESSED_RGBA_ASTC_6x5x5_OES                       = 0x93C7,
    GL_COMPRESSED_RGBA_ASTC_6x6x5_OES                       = 0x93C8,
    GL_COMPRESSED_RGBA_ASTC_6x6x6_OES                       = 0x93C9,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES               = 0x93E0,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES               = 0x93E1,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES               = 0x93E2,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES               = 0x93E3,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES               = 0x93E4,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES               = 0x93E5,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES               = 0x93E6,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES               = 0x93E7,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES               = 0x93E8,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES               = 0x93E9,
};
#endif

#ifndef GL_OES_texture_cube_map_array
#define GL_OES_texture_cube_map_array 1
enum : GLenum
{
    GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_OES                   = 0x900A,
    GL_SAMPLER_CUBE_MAP_ARRAY_OES                           = 0x900C,
    GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_OES                    = 0x900D,
    GL_INT_SAMPLER_CUBE_MAP_ARRAY_OES                       = 0x900E,
    GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_OES              = 0x900F,
    GL_IMAGE_CUBE_MAP_ARRAY_OES                             = 0x9054,
    GL_INT_IMAGE_CUBE_MAP_ARRAY_OES                         = 0x905F,
    GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_OES                = 0x906A,
};
#endif

#ifndef GL_OES_texture_float
#define GL_OES_texture_float 1
#endif

#ifndef GL_OES_texture_float_linear
#define GL_OES_texture_float_linear 1
#endif

#ifndef GL_OES_texture_half_float
#define GL_OES_texture_half_float 1
enum : GLenum
{
    GL_HALF_FLOAT_OES                                       = 0x8D61,
};
#endif

#ifndef GL_OES_texture_half_float_linear
#define GL_OES_texture_half_float_linear 1
#endif

#ifndef GL_OES_texture_npot
#define GL_OES_texture_npot 1
#endif

#ifndef GL_OES_texture_stencil8
#define GL_OES_texture_stencil8 1
enum : GLenum
{
    GL_STENCIL_INDEX_OES                                    = 0x1901,
    GL_STENCIL_INDEX8_OES                                   = 0x8D48,
};
#endif

#ifndef GL_OES_texture_storage_multisample_2d_array
#define GL_OES_texture_storage_multisample_2d_array 1
enum : GLenum
{
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES                     = 0x9102,
    GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY_OES             = 0x9105,
    GL_SAMPLER_2D_MULTISAMPLE_ARRAY_OES                     = 0x910B,
    GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES                 = 0x910C,
    GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES        = 0x910D,
};
extern void         (KHRONOS_APIENTRY* const& glTexStorage3DMultisampleOES) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
#endif

#ifndef GL_OES_texture_view
#define GL_OES_texture_view 1
enum : GLenum
{
    GL_TEXTURE_VIEW_MIN_LEVEL_OES                           = 0x82DB,
    GL_TEXTURE_VIEW_NUM_LEVELS_OES                          = 0x82DC,
    GL_TEXTURE_VIEW_MIN_LAYER_OES                           = 0x82DD,
    GL_TEXTURE_VIEW_NUM_LAYERS_OES                          = 0x82DE,
};
extern void         (KHRONOS_APIENTRY* const& glTextureViewOES) (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
#endif

#ifndef GL_OES_vertex_array_object
#define GL_OES_vertex_array_object 1
enum : GLenum
{
    GL_VERTEX_ARRAY_BINDING_OES                             = 0x85B5,
};
extern void         (KHRONOS_APIENTRY* const& glBindVertexArrayOES) (GLuint array);
extern void         (KHRONOS_APIENTRY* const& glDeleteVertexArraysOES) (GLsizei n, const GLuint *arrays);
extern void         (KHRONOS_APIENTRY* const& glGenVertexArraysOES) (GLsizei n, GLuint *arrays);
extern GLboolean    (KHRONOS_APIENTRY* const& glIsVertexArrayOES) (GLuint array);
#endif

#ifndef GL_OES_vertex_half_float
#define GL_OES_vertex_half_float 1
#endif

#ifndef GL_OES_vertex_type_10_10_10_2
#define GL_OES_vertex_type_10_10_10_2 1
enum : GLenum
{
    GL_UNSIGNED_INT_10_10_10_2_OES                          = 0x8DF6,
    GL_INT_10_10_10_2_OES                                   = 0x8DF7,
};
#endif

#ifndef GL_OES_viewport_array
#define GL_OES_viewport_array 1
enum : GLenum
{
    GL_MAX_VIEWPORTS_OES                                    = 0x825B,
    GL_VIEWPORT_SUBPIXEL_BITS_OES                           = 0x825C,
    GL_VIEWPORT_BOUNDS_RANGE_OES                            = 0x825D,
    GL_VIEWPORT_INDEX_PROVOKING_VERTEX_OES                  = 0x825F,
};
extern void         (KHRONOS_APIENTRY* const& glViewportArrayvOES) (GLuint first, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glViewportIndexedfOES) (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
extern void         (KHRONOS_APIENTRY* const& glViewportIndexedfvOES) (GLuint index, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glScissorArrayvOES) (GLuint first, GLsizei count, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glScissorIndexedOES) (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
extern void         (KHRONOS_APIENTRY* const& glScissorIndexedvOES) (GLuint index, const GLint *v);
extern void         (KHRONOS_APIENTRY* const& glDepthRangeArrayfvOES) (GLuint first, GLsizei count, const GLfloat *v);
extern void         (KHRONOS_APIENTRY* const& glDepthRangeIndexedfOES) (GLuint index, GLfloat n, GLfloat f);
extern void         (KHRONOS_APIENTRY* const& glGetFloati_vOES) (GLenum target, GLuint index, GLfloat *data);
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

#ifndef GL_OVR_multiview_multisampled_render_to_texture
#define GL_OVR_multiview_multisampled_render_to_texture 1
extern void         (KHRONOS_APIENTRY* const& glFramebufferTextureMultisampleMultiviewOVR) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei samples, GLint baseViewIndex, GLsizei numViews);
#endif

#ifndef GL_QCOM_alpha_test
#define GL_QCOM_alpha_test 1
enum : GLenum
{
    GL_ALPHA_TEST_QCOM                                      = 0x0BC0,
    GL_ALPHA_TEST_FUNC_QCOM                                 = 0x0BC1,
    GL_ALPHA_TEST_REF_QCOM                                  = 0x0BC2,
};
extern void         (KHRONOS_APIENTRY* const& glAlphaFuncQCOM) (GLenum func, GLclampf ref);
#endif

#ifndef GL_QCOM_binning_control
#define GL_QCOM_binning_control 1
enum : GLenum
{
    GL_BINNING_CONTROL_HINT_QCOM                            = 0x8FB0,
    GL_CPU_OPTIMIZED_QCOM                                   = 0x8FB1,
    GL_GPU_OPTIMIZED_QCOM                                   = 0x8FB2,
    GL_RENDER_DIRECT_TO_FRAMEBUFFER_QCOM                    = 0x8FB3,
};
#endif

#ifndef GL_QCOM_driver_control
#define GL_QCOM_driver_control 1
extern void         (KHRONOS_APIENTRY* const& glGetDriverControlsQCOM) (GLint *num, GLsizei size, GLuint *driverControls);
extern void         (KHRONOS_APIENTRY* const& glGetDriverControlStringQCOM) (GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString);
extern void         (KHRONOS_APIENTRY* const& glEnableDriverControlQCOM) (GLuint driverControl);
extern void         (KHRONOS_APIENTRY* const& glDisableDriverControlQCOM) (GLuint driverControl);
#endif

#ifndef GL_QCOM_extended_get
#define GL_QCOM_extended_get 1
enum : GLenum
{
    GL_TEXTURE_WIDTH_QCOM                                   = 0x8BD2,
    GL_TEXTURE_HEIGHT_QCOM                                  = 0x8BD3,
    GL_TEXTURE_DEPTH_QCOM                                   = 0x8BD4,
    GL_TEXTURE_INTERNAL_FORMAT_QCOM                         = 0x8BD5,
    GL_TEXTURE_FORMAT_QCOM                                  = 0x8BD6,
    GL_TEXTURE_TYPE_QCOM                                    = 0x8BD7,
    GL_TEXTURE_IMAGE_VALID_QCOM                             = 0x8BD8,
    GL_TEXTURE_NUM_LEVELS_QCOM                              = 0x8BD9,
    GL_TEXTURE_TARGET_QCOM                                  = 0x8BDA,
    GL_TEXTURE_OBJECT_VALID_QCOM                            = 0x8BDB,
    GL_STATE_RESTORE                                        = 0x8BDC,
};
extern void         (KHRONOS_APIENTRY* const& glExtGetTexturesQCOM) (GLuint *textures, GLint maxTextures, GLint *numTextures);
extern void         (KHRONOS_APIENTRY* const& glExtGetBuffersQCOM) (GLuint *buffers, GLint maxBuffers, GLint *numBuffers);
extern void         (KHRONOS_APIENTRY* const& glExtGetRenderbuffersQCOM) (GLuint *renderbuffers, GLint maxRenderbuffers, GLint *numRenderbuffers);
extern void         (KHRONOS_APIENTRY* const& glExtGetFramebuffersQCOM) (GLuint *framebuffers, GLint maxFramebuffers, GLint *numFramebuffers);
extern void         (KHRONOS_APIENTRY* const& glExtGetTexLevelParameterivQCOM) (GLuint texture, GLenum face, GLint level, GLenum pname, GLint *params);
extern void         (KHRONOS_APIENTRY* const& glExtTexObjectStateOverrideiQCOM) (GLenum target, GLenum pname, GLint param);
extern void         (KHRONOS_APIENTRY* const& glExtGetTexSubImageQCOM) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void *texels);
extern void         (KHRONOS_APIENTRY* const& glExtGetBufferPointervQCOM) (GLenum target, void **params);
#endif

#ifndef GL_QCOM_extended_get2
#define GL_QCOM_extended_get2 1
extern void         (KHRONOS_APIENTRY* const& glExtGetShadersQCOM) (GLuint *shaders, GLint maxShaders, GLint *numShaders);
extern void         (KHRONOS_APIENTRY* const& glExtGetProgramsQCOM) (GLuint *programs, GLint maxPrograms, GLint *numPrograms);
extern GLboolean    (KHRONOS_APIENTRY* const& glExtIsProgramBinaryQCOM) (GLuint program);
extern void         (KHRONOS_APIENTRY* const& glExtGetProgramBinarySourceQCOM) (GLuint program, GLenum shadertype, GLchar *source, GLint *length);
#endif

#ifndef GL_QCOM_framebuffer_foveated
#define GL_QCOM_framebuffer_foveated 1
enum : GLenum
{
    GL_FOVEATION_ENABLE_BIT_QCOM                            = 0x00000001,
    GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM                 = 0x00000002,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferFoveationConfigQCOM) (GLuint framebuffer, GLuint numLayers, GLuint focalPointsPerLayer, GLuint requestedFeatures, GLuint *providedFeatures);
extern void         (KHRONOS_APIENTRY* const& glFramebufferFoveationParametersQCOM) (GLuint framebuffer, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea);
#endif

#ifndef GL_QCOM_motion_estimation
#define GL_QCOM_motion_estimation 1
enum : GLenum
{
    GL_MOTION_ESTIMATION_SEARCH_BLOCK_X_QCOM                = 0x8C90,
    GL_MOTION_ESTIMATION_SEARCH_BLOCK_Y_QCOM                = 0x8C91,
};
extern void         (KHRONOS_APIENTRY* const& glTexEstimateMotionQCOM) (GLuint ref, GLuint target, GLuint output);
extern void         (KHRONOS_APIENTRY* const& glTexEstimateMotionRegionsQCOM) (GLuint ref, GLuint target, GLuint output, GLuint mask);
#endif

#ifndef GL_QCOM_frame_extrapolation
#define GL_QCOM_frame_extrapolation 1
extern void         (KHRONOS_APIENTRY* const& glExtrapolateTex2DQCOM) (GLuint src1, GLuint src2, GLuint output, GLfloat scaleFactor);
#endif

#ifndef GL_QCOM_render_shared_exponent
#define GL_QCOM_render_shared_exponent 1
#endif

#ifndef GL_QCOM_texture_foveated
#define GL_QCOM_texture_foveated 1
enum : GLenum
{
    GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM                   = 0x8BFB,
    GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM              = 0x8BFC,
    GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM                  = 0x8BFD,
    GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM         = 0x8BFE,
    GL_FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM                = 0x8BFF,
};
extern void         (KHRONOS_APIENTRY* const& glTextureFoveationParametersQCOM) (GLuint texture, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea);
#endif

#ifndef GL_QCOM_texture_foveated2
#define GL_QCOM_texture_foveated2 1
enum : GLenum
{
    GL_TEXTURE_FOVEATED_CUTOFF_DENSITY_QCOM                 = 0x96A0,
};
#endif

#ifndef GL_QCOM_texture_foveated_subsampled_layout
#define GL_QCOM_texture_foveated_subsampled_layout 1
enum : GLenum
{
    GL_FOVEATION_SUBSAMPLED_LAYOUT_METHOD_BIT_QCOM          = 0x00000004,
    GL_MAX_SHADER_SUBSAMPLED_IMAGE_UNITS_QCOM               = 0x8FA1,
};
#endif

#ifndef GL_QCOM_perfmon_global_mode
#define GL_QCOM_perfmon_global_mode 1
enum : GLenum
{
    GL_PERFMON_GLOBAL_MODE_QCOM                             = 0x8FA0,
};
#endif

#ifndef GL_QCOM_shader_framebuffer_fetch_noncoherent
#define GL_QCOM_shader_framebuffer_fetch_noncoherent 1
enum : GLenum
{
    GL_FRAMEBUFFER_FETCH_NONCOHERENT_QCOM                   = 0x96A2,
};
extern void         (KHRONOS_APIENTRY* const& glFramebufferFetchBarrierQCOM) ();
#endif

#ifndef GL_QCOM_shader_framebuffer_fetch_rate
#define GL_QCOM_shader_framebuffer_fetch_rate 1
#endif

#ifndef GL_QCOM_shading_rate
#define GL_QCOM_shading_rate 1
enum : GLenum
{
    GL_SHADING_RATE_QCOM                                    = 0x96A4,
    GL_SHADING_RATE_PRESERVE_ASPECT_RATIO_QCOM              = 0x96A5,
    GL_SHADING_RATE_1X1_PIXELS_QCOM                         = 0x96A6,
    GL_SHADING_RATE_1X2_PIXELS_QCOM                         = 0x96A7,
    GL_SHADING_RATE_2X1_PIXELS_QCOM                         = 0x96A8,
    GL_SHADING_RATE_2X2_PIXELS_QCOM                         = 0x96A9,
    GL_SHADING_RATE_4X2_PIXELS_QCOM                         = 0x96AC,
    GL_SHADING_RATE_4X4_PIXELS_QCOM                         = 0x96AE,
};
extern void         (KHRONOS_APIENTRY* const& glShadingRateQCOM) (GLenum rate);
#endif

#ifndef GL_QCOM_tiled_rendering
#define GL_QCOM_tiled_rendering 1
enum : GLenum
{
    GL_COLOR_BUFFER_BIT0_QCOM                               = 0x00000001,
    GL_COLOR_BUFFER_BIT1_QCOM                               = 0x00000002,
    GL_COLOR_BUFFER_BIT2_QCOM                               = 0x00000004,
    GL_COLOR_BUFFER_BIT3_QCOM                               = 0x00000008,
    GL_COLOR_BUFFER_BIT4_QCOM                               = 0x00000010,
    GL_COLOR_BUFFER_BIT5_QCOM                               = 0x00000020,
    GL_COLOR_BUFFER_BIT6_QCOM                               = 0x00000040,
    GL_COLOR_BUFFER_BIT7_QCOM                               = 0x00000080,
    GL_DEPTH_BUFFER_BIT0_QCOM                               = 0x00000100,
    GL_DEPTH_BUFFER_BIT1_QCOM                               = 0x00000200,
    GL_DEPTH_BUFFER_BIT2_QCOM                               = 0x00000400,
    GL_DEPTH_BUFFER_BIT3_QCOM                               = 0x00000800,
    GL_DEPTH_BUFFER_BIT4_QCOM                               = 0x00001000,
    GL_DEPTH_BUFFER_BIT5_QCOM                               = 0x00002000,
    GL_DEPTH_BUFFER_BIT6_QCOM                               = 0x00004000,
    GL_DEPTH_BUFFER_BIT7_QCOM                               = 0x00008000,
    GL_STENCIL_BUFFER_BIT0_QCOM                             = 0x00010000,
    GL_STENCIL_BUFFER_BIT1_QCOM                             = 0x00020000,
    GL_STENCIL_BUFFER_BIT2_QCOM                             = 0x00040000,
    GL_STENCIL_BUFFER_BIT3_QCOM                             = 0x00080000,
    GL_STENCIL_BUFFER_BIT4_QCOM                             = 0x00100000,
    GL_STENCIL_BUFFER_BIT5_QCOM                             = 0x00200000,
    GL_STENCIL_BUFFER_BIT6_QCOM                             = 0x00400000,
    GL_STENCIL_BUFFER_BIT7_QCOM                             = 0x00800000,
    GL_MULTISAMPLE_BUFFER_BIT0_QCOM                         = 0x01000000,
    GL_MULTISAMPLE_BUFFER_BIT1_QCOM                         = 0x02000000,
    GL_MULTISAMPLE_BUFFER_BIT2_QCOM                         = 0x04000000,
    GL_MULTISAMPLE_BUFFER_BIT3_QCOM                         = 0x08000000,
    GL_MULTISAMPLE_BUFFER_BIT4_QCOM                         = 0x10000000,
    GL_MULTISAMPLE_BUFFER_BIT5_QCOM                         = 0x20000000,
    GL_MULTISAMPLE_BUFFER_BIT6_QCOM                         = 0x40000000,
    GL_MULTISAMPLE_BUFFER_BIT7_QCOM                         = 0x80000000,
};
extern void         (KHRONOS_APIENTRY* const& glStartTilingQCOM) (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
extern void         (KHRONOS_APIENTRY* const& glEndTilingQCOM) (GLbitfield preserveMask);
#endif

#ifndef GL_QCOM_writeonly_rendering
#define GL_QCOM_writeonly_rendering 1
enum : GLenum
{
    GL_WRITEONLY_RENDERING_QCOM                             = 0x8823,
};
#endif

#ifndef GL_QCOM_YUV_texture_gather
#define GL_QCOM_YUV_texture_gather 1
#endif

#ifndef GL_VIV_shader_binary
#define GL_VIV_shader_binary 1
enum : GLenum
{
    GL_SHADER_BINARY_VIV                                    = 0x8FC4,
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
