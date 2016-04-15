/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifdef JUCE_OPENGL_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1

#include "juce_opengl.h"

//==============================================================================
#if JUCE_IOS
 #import <QuartzCore/QuartzCore.h>

//==============================================================================
#elif JUCE_WINDOWS
 #include <windowsx.h>

 #if JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "OpenGL32.Lib")
 #endif

//==============================================================================
#elif JUCE_LINUX
 /* Got an include error here?

    If you want to install OpenGL support, the packages to get are "mesa-common-dev"
    and "freeglut3-dev".
 */
 #include <GL/glx.h>

//==============================================================================
#elif JUCE_MAC
 #include <OpenGL/CGLCurrent.h> // These are both just needed with the 10.5 SDK
 #include <OpenGL/OpenGL.h>

//==============================================================================
#elif JUCE_ANDROID
 #ifndef GL_GLEXT_PROTOTYPES
  #define GL_GLEXT_PROTOTYPES 1
 #endif
 #include <GLES2/gl2.h>
#endif

namespace juce
{

//==============================================================================
#include "native/juce_OpenGLExtensions.h"

void OpenGLExtensionFunctions::initialise()
{
   #if JUCE_WINDOWS || JUCE_LINUX
    #define JUCE_INIT_GL_FUNCTION(name, returnType, params, callparams) \
        name = (type_ ## name) OpenGLHelpers::getExtensionFunction (#name);

    JUCE_GL_BASE_FUNCTIONS (JUCE_INIT_GL_FUNCTION)
    #undef JUCE_INIT_GL_FUNCTION

    #define JUCE_INIT_GL_FUNCTION(name, returnType, params, callparams) \
        name = (type_ ## name) OpenGLHelpers::getExtensionFunction (#name); \
        if (name == nullptr) \
            name = (type_ ## name) OpenGLHelpers::getExtensionFunction (JUCE_STRINGIFY (name ## EXT));

    JUCE_GL_EXTENSION_FUNCTIONS (JUCE_INIT_GL_FUNCTION)
    #if JUCE_OPENGL3
     JUCE_GL_VERTEXBUFFER_FUNCTIONS (JUCE_INIT_GL_FUNCTION)
    #endif

    #undef JUCE_INIT_GL_FUNCTION
   #endif
}

#if JUCE_OPENGL_ES
 #define JUCE_DECLARE_GL_FUNCTION(name, returnType, params, callparams) \
    returnType OpenGLExtensionFunctions::name params noexcept { return ::name callparams; }

 JUCE_GL_BASE_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
 JUCE_GL_EXTENSION_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
#if JUCE_OPENGL3
 JUCE_GL_VERTEXBUFFER_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
#endif

 #undef JUCE_DECLARE_GL_FUNCTION
#endif

#undef JUCE_GL_EXTENSION_FUNCTIONS

#if JUCE_DEBUG && ! defined (JUCE_CHECK_OPENGL_ERROR)
static const char* getGLErrorMessage (const GLenum e) noexcept
{
    switch (e)
    {
        case GL_INVALID_ENUM:                   return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                  return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:              return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:                  return "GL_OUT_OF_MEMORY";
       #ifdef GL_STACK_OVERFLOW
        case GL_STACK_OVERFLOW:                 return "GL_STACK_OVERFLOW";
       #endif
       #ifdef GL_STACK_UNDERFLOW
        case GL_STACK_UNDERFLOW:                return "GL_STACK_UNDERFLOW";
       #endif
       #ifdef GL_INVALID_FRAMEBUFFER_OPERATION
        case GL_INVALID_FRAMEBUFFER_OPERATION:  return "GL_INVALID_FRAMEBUFFER_OPERATION";
       #endif
        default: break;
    }

    return "Unknown error";
}

static void checkGLError (const char* file, const int line)
{
    for (;;)
    {
        const GLenum e = glGetError();

        if (e == GL_NO_ERROR)
            break;

        DBG ("***** " << getGLErrorMessage (e) << "  at " << file << " : " << line);
        jassertfalse;
    }
}

 #define JUCE_CHECK_OPENGL_ERROR checkGLError (__FILE__, __LINE__);
#else
 #define JUCE_CHECK_OPENGL_ERROR ;
#endif

static void clearGLError() noexcept
{
    while (glGetError() != GL_NO_ERROR) {}
}

struct OpenGLTargetSaver
{
    OpenGLTargetSaver (const OpenGLContext& c) noexcept
        : context (c), oldFramebuffer (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
    {
        glGetIntegerv (GL_VIEWPORT, oldViewport);
    }

    ~OpenGLTargetSaver() noexcept
    {
        context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, oldFramebuffer);
        glViewport (oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
    }

private:
    const OpenGLContext& context;
    GLuint oldFramebuffer;
    GLint oldViewport[4];

    OpenGLTargetSaver& operator= (const OpenGLTargetSaver&);
};

//==============================================================================
#include "opengl/juce_OpenGLFrameBuffer.cpp"
#include "opengl/juce_OpenGLGraphicsContext.cpp"
#include "opengl/juce_OpenGLHelpers.cpp"
#include "opengl/juce_OpenGLImage.cpp"
#include "opengl/juce_OpenGLPixelFormat.cpp"
#include "opengl/juce_OpenGLShaderProgram.cpp"
#include "opengl/juce_OpenGLTexture.cpp"

//==============================================================================
#if JUCE_MAC || JUCE_IOS

 #if JUCE_MAC
  #include "native/juce_OpenGL_osx.h"
 #else
  #include "native/juce_OpenGL_ios.h"
 #endif

#elif JUCE_WINDOWS
 #include "native/juce_OpenGL_win32.h"

#elif JUCE_LINUX
 #include "native/juce_OpenGL_linux.h"

#elif JUCE_ANDROID
 #include "native/juce_OpenGL_android.h"

#endif

#include "opengl/juce_OpenGLContext.cpp"
#include "utils/juce_OpenGLAppComponent.cpp"

}
