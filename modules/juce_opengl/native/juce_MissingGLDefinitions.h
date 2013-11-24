/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

/** These are important openGL values that aren't defined by default
    by the GL headers on various platforms.
*/
enum MissingOpenGLDefinitions
{
   #ifndef GL_CLAMP_TO_EDGE
    GL_CLAMP_TO_EDGE                = 0x812f,
   #endif

   #ifndef GL_NUM_EXTENSIONS
    GL_NUM_EXTENSIONS               = 0x821d,
   #endif

   #ifndef GL_BGRA_EXT
    GL_BGRA_EXT                     = 0x80e1,
   #endif

   #ifndef GL_DEPTH24_STENCIL8
    GL_DEPTH24_STENCIL8             = 0x88F0,
   #endif

   #ifndef GL_RGBA8
    GL_RGBA8                        = GL_RGBA,
   #endif

   #ifndef GL_COLOR_ATTACHMENT0
    GL_COLOR_ATTACHMENT0            = 0x8CE0,
   #endif

   #ifndef GL_DEPTH_ATTACHMENT
    GL_DEPTH_ATTACHMENT             = 0x8D00,
   #endif

   #ifndef GL_FRAMEBUFFER
    GL_FRAMEBUFFER                  = 0x8D40,
   #endif

   #ifndef GL_FRAMEBUFFER_BINDING
    GL_FRAMEBUFFER_BINDING          = 0x8CA6,
   #endif

   #ifndef GL_FRAMEBUFFER_COMPLETE
    GL_FRAMEBUFFER_COMPLETE         = 0x8CD5,
   #endif

   #ifndef GL_RENDERBUFFER
    GL_RENDERBUFFER                 = 0x8D41,
   #endif

   #ifndef GL_RENDERBUFFER_DEPTH_SIZE
    GL_RENDERBUFFER_DEPTH_SIZE      = 0x8D54,
   #endif

   #ifndef GL_STENCIL_ATTACHMENT
    GL_STENCIL_ATTACHMENT           = 0x8D20,
   #endif

   #if JUCE_WINDOWS
    GL_OPERAND0_RGB                 = 0x8590,
    GL_OPERAND1_RGB                 = 0x8591,
    GL_OPERAND0_ALPHA               = 0x8598,
    GL_OPERAND1_ALPHA               = 0x8599,
    GL_SRC0_RGB                     = 0x8580,
    GL_SRC1_RGB                     = 0x8581,
    GL_SRC0_ALPHA                   = 0x8588,
    GL_SRC1_ALPHA                   = 0x8589,
    GL_TEXTURE0                     = 0x84C0,
    GL_TEXTURE1                     = 0x84C1,
    GL_TEXTURE2                     = 0x84C2,
    GL_COMBINE                      = 0x8570,
    GL_COMBINE_RGB                  = 0x8571,
    GL_COMBINE_ALPHA                = 0x8572,
    GL_PREVIOUS                     = 0x8578,
    GL_COMPILE_STATUS               = 0x8B81,
    GL_LINK_STATUS                  = 0x8B82,
    GL_SHADING_LANGUAGE_VERSION     = 0x8B8C,
    GL_FRAGMENT_SHADER              = 0x8B30,
    GL_VERTEX_SHADER                = 0x8B31,
    GL_ARRAY_BUFFER                 = 0x8892,
    GL_ELEMENT_ARRAY_BUFFER         = 0x8893,
    GL_STATIC_DRAW                  = 0x88E4,
    GL_DYNAMIC_DRAW                 = 0x88E8,
    GL_STREAM_DRAW                  = 0x88E0,

    WGL_NUMBER_PIXEL_FORMATS_ARB    = 0x2000,
    WGL_DRAW_TO_WINDOW_ARB          = 0x2001,
    WGL_ACCELERATION_ARB            = 0x2003,
    WGL_SWAP_METHOD_ARB             = 0x2007,
    WGL_SUPPORT_OPENGL_ARB          = 0x2010,
    WGL_PIXEL_TYPE_ARB              = 0x2013,
    WGL_DOUBLE_BUFFER_ARB           = 0x2011,
    WGL_COLOR_BITS_ARB              = 0x2014,
    WGL_RED_BITS_ARB                = 0x2015,
    WGL_GREEN_BITS_ARB              = 0x2017,
    WGL_BLUE_BITS_ARB               = 0x2019,
    WGL_ALPHA_BITS_ARB              = 0x201B,
    WGL_DEPTH_BITS_ARB              = 0x2022,
    WGL_STENCIL_BITS_ARB            = 0x2023,
    WGL_FULL_ACCELERATION_ARB       = 0x2027,
    WGL_ACCUM_RED_BITS_ARB          = 0x201E,
    WGL_ACCUM_GREEN_BITS_ARB        = 0x201F,
    WGL_ACCUM_BLUE_BITS_ARB         = 0x2020,
    WGL_ACCUM_ALPHA_BITS_ARB        = 0x2021,
    WGL_STEREO_ARB                  = 0x2012,
    WGL_SAMPLE_BUFFERS_ARB          = 0x2041,
    WGL_SAMPLES_ARB                 = 0x2042,
    WGL_TYPE_RGBA_ARB               = 0x202B,
    WGL_CONTEXT_MAJOR_VERSION_ARB   = 0x2091,
    WGL_CONTEXT_MINOR_VERSION_ARB   = 0x2092,
    WGL_CONTEXT_PROFILE_MASK_ARB    = 0x9126,
   #endif

   #if JUCE_ANDROID
    JUCE_RGBA_FORMAT                = GL_RGBA
   #else
    JUCE_RGBA_FORMAT                = GL_BGRA_EXT
   #endif
};

#if JUCE_WINDOWS
 typedef char GLchar;
 typedef pointer_sized_int GLsizeiptr;
 typedef pointer_sized_int GLintptr;
#endif
