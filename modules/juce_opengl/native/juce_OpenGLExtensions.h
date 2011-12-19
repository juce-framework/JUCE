/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

namespace
{

#if JUCE_WINDOWS
 #define JUCE_DECLARE_GL_EXTENSION_FUNCTION(name, returnType, params) \
   typedef returnType (__stdcall *type_ ## name) params; static type_ ## name name = nullptr;
#else
 #define JUCE_DECLARE_GL_EXTENSION_FUNCTION(name, returnType, params) \
   typedef returnType (*type_ ## name) params; static type_ ## name name = nullptr;
#endif

#ifndef GL_BGRA_EXT
 enum { GL_BGRA_EXT = 0x80e1 };
#endif

#ifndef GL_CLAMP_TO_EDGE
 enum { GL_CLAMP_TO_EDGE = 0x812f };
#endif

#ifndef GL_RGBA8
 #define GL_RGBA8 GL_RGBA
#endif

#ifndef GL_DEPTH24_STENCIL8
 #define GL_DEPTH24_STENCIL8  GL_DEPTH24_STENCIL8_OES
#endif

#if JUCE_WINDOWS

enum
{
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
    GL_FRAMEBUFFER                  = 0x8D40,
    GL_RENDERBUFFER                 = 0x8D41,
    GL_FRAMEBUFFER_BINDING          = 0x8CA6,
    GL_COLOR_ATTACHMENT0            = 0x8CE0,
    GL_DEPTH_ATTACHMENT             = 0x8D00,
    GL_STENCIL_ATTACHMENT           = 0x8D20,
    GL_FRAMEBUFFER_COMPLETE         = 0x8CD5,
    GL_DEPTH24_STENCIL8             = 0x88F0,
    GL_RENDERBUFFER_DEPTH_SIZE      = 0x8D54,
    GL_ARRAY_BUFFER                 = 0x8892,
    GL_ELEMENT_ARRAY_BUFFER         = 0x8893,
    GL_STATIC_DRAW                  = 0x88E4,
    GL_DYNAMIC_DRAW                 = 0x88E8
};

typedef char GLchar;
typedef long GLsizeiptr;

#define WGL_FUNCTION_LIST(USE_FUNCTION) \
    USE_FUNCTION (wglChoosePixelFormatARB,  BOOL, (HDC, const int*, const FLOAT*, UINT, int*, UINT*))\
    USE_FUNCTION (wglSwapIntervalEXT,       BOOL, (int))\
    USE_FUNCTION (wglGetSwapIntervalEXT,    int, ())\

WGL_FUNCTION_LIST (JUCE_DECLARE_GL_EXTENSION_FUNCTION)
#endif

#if JUCE_WINDOWS || JUCE_LINUX

#define GL_EXTENSIONS_LIST(USE_FUNCTION) \
    USE_FUNCTION (glActiveTexture,          void, (GLenum))\
    USE_FUNCTION (glClientActiveTexture,    void, (GLenum))\
    USE_FUNCTION (glCreateProgram,          GLuint, ())\
    USE_FUNCTION (glDeleteProgram,          void, (GLuint))\
    USE_FUNCTION (glCreateShader,           GLuint, (GLenum))\
    USE_FUNCTION (glDeleteShader,           void, (GLuint))\
    USE_FUNCTION (glShaderSource,           void, (GLuint, GLsizei, const GLchar**, const GLint*))\
    USE_FUNCTION (glCompileShader,          void, (GLuint))\
    USE_FUNCTION (glAttachShader,           void, (GLuint, GLuint))\
    USE_FUNCTION (glLinkProgram,            void, (GLuint))\
    USE_FUNCTION (glUseProgram,             void, (GLuint))\
    USE_FUNCTION (glGetShaderiv,            void, (GLuint, GLenum, GLint*))\
    USE_FUNCTION (glGetShaderInfoLog,       void, (GLuint, GLsizei, GLsizei*, GLchar*))\
    USE_FUNCTION (glGetProgramiv,           void, (GLuint, GLenum, GLint*))\
    USE_FUNCTION (glGetUniformLocation,     GLint, (GLuint, const GLchar*))\
    USE_FUNCTION (glGetAttribLocation,      GLint, (GLuint, const GLchar*))\
    USE_FUNCTION (glVertexAttribPointer,    void, (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*))\
    USE_FUNCTION (glEnableVertexAttribArray, void, (GLuint))\
    USE_FUNCTION (glBindBuffer,             void, (GLenum, GLuint))\
    USE_FUNCTION (glDeleteBuffers,          void, (GLsizei, const GLuint*))\
    USE_FUNCTION (glGenBuffers,             void, (GLsizei, GLuint*))\
    USE_FUNCTION (glBufferData,             void, (GLenum, GLsizeiptr, const GLvoid*, GLenum))\
    USE_FUNCTION (glBufferSubData,          void, (GLenum, GLintptr, GLsizeiptr, const GLvoid*))\
    USE_FUNCTION (glUniform1f,              void, (GLint, GLfloat))\
    USE_FUNCTION (glUniform1i,              void, (GLint, GLint))\
    USE_FUNCTION (glUniform2f,              void, (GLint, GLfloat, GLfloat))\
    USE_FUNCTION (glUniform3f,              void, (GLint, GLfloat, GLfloat, GLfloat))\
    USE_FUNCTION (glUniform4f,              void, (GLint, GLfloat, GLfloat, GLfloat, GLfloat))\
    USE_FUNCTION (glUniform4i,              void, (GLint, GLint, GLint, GLint, GLint))\
    USE_FUNCTION (glUniform1fv,             void, (GLint, GLsizei, const GLfloat*))\
    USE_FUNCTION (glIsRenderbuffer,         GLboolean, (GLuint))\
    USE_FUNCTION (glBindRenderbuffer,       void, (GLenum, GLuint))\
    USE_FUNCTION (glDeleteRenderbuffers,    void, (GLsizei, const GLuint*))\
    USE_FUNCTION (glGenRenderbuffers,       void, (GLsizei, GLuint*))\
    USE_FUNCTION (glRenderbufferStorage,    void, (GLenum, GLenum, GLsizei, GLsizei))\
    USE_FUNCTION (glGetRenderbufferParameteriv,  void, (GLenum, GLenum, GLint*))\
    USE_FUNCTION (glIsFramebuffer,          GLboolean, (GLuint))\
    USE_FUNCTION (glBindFramebuffer,        void, (GLenum, GLuint))\
    USE_FUNCTION (glDeleteFramebuffers,     void, (GLsizei, const GLuint*))\
    USE_FUNCTION (glGenFramebuffers,        void, (GLsizei, GLuint*))\
    USE_FUNCTION (glCheckFramebufferStatus, GLenum, (GLenum))\
    USE_FUNCTION (glFramebufferTexture1D,   void, (GLenum, GLenum, GLenum, GLuint, GLint))\
    USE_FUNCTION (glFramebufferTexture2D,   void, (GLenum, GLenum, GLenum, GLuint, GLint))\
    USE_FUNCTION (glFramebufferTexture3D,   void, (GLenum, GLenum, GLenum, GLuint, GLint, GLint))\
    USE_FUNCTION (glFramebufferRenderbuffer,  void, (GLenum, GLenum, GLenum, GLuint))\
    USE_FUNCTION (glGetFramebufferAttachmentParameteriv, void, (GLenum, GLenum, GLenum, GLint*))\

GL_EXTENSIONS_LIST (JUCE_DECLARE_GL_EXTENSION_FUNCTION)

void initialiseGLExtensions()
{
    if (glActiveTexture == nullptr)
    {
        #define FIND_FUNCTION(name, returnType, params) name = (type_ ## name) OpenGLHelpers::getExtensionFunction (#name);

       #if JUCE_WINDOWS
        WGL_FUNCTION_LIST (FIND_FUNCTION)
       #endif
        GL_EXTENSIONS_LIST (FIND_FUNCTION)

        #undef FIND_FUNCTION
    }
}

#undef GL_EXTENSIONS_LIST
#undef WGL_FUNCTION_LIST

#else
void initialiseGLExtensions() {}
#endif

}
