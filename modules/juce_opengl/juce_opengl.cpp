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
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1
#define JUCE_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1

#include "juce_opengl.h"

#define JUCE_STATIC_LINK_GL_VERSION_1_0 1
#define JUCE_STATIC_LINK_GL_VERSION_1_1 1

#if JUCE_MAC
 #define JUCE_STATIC_LINK_GL_VERSION_1_2 1
 #define JUCE_STATIC_LINK_GL_VERSION_1_3 1
 #define JUCE_STATIC_LINK_GL_VERSION_1_4 1
 #define JUCE_STATIC_LINK_GL_VERSION_1_5 1
 #define JUCE_STATIC_LINK_GL_VERSION_2_0 1
 #define JUCE_STATIC_LINK_GL_VERSION_2_1 1
 #define JUCE_STATIC_LINK_GL_VERSION_3_0 1
 #define JUCE_STATIC_LINK_GL_VERSION_3_1 1
 #define JUCE_STATIC_LINK_GL_VERSION_3_2 1
#endif

#define JUCE_STATIC_LINK_GL_ES_VERSION_2_0 1
#if !JUCE_ANDROID || JUCE_ANDROID_GL_ES_VERSION_3_0
#define JUCE_STATIC_LINK_GL_ES_VERSION_3_0 1
#endif

#if JUCE_OPENGL_ES
 #include "opengl/juce_gles2.cpp"
#else
 #include "opengl/juce_gl.cpp"
#endif

//==============================================================================
#if JUCE_IOS
 #import <QuartzCore/QuartzCore.h>

//==============================================================================
#elif JUCE_WINDOWS
 #include <windowsx.h>

 #if ! JUCE_MINGW && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "OpenGL32.Lib")
 #endif

//==============================================================================
#elif JUCE_LINUX || JUCE_BSD
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
 #include <android/native_window.h>
 #include <android/native_window_jni.h>
 #include <EGL/egl.h>
#endif

//==============================================================================
namespace juce
{

using namespace ::juce::gl;

void OpenGLExtensionFunctions::initialise()
{
    gl::loadFunctions();
}

#define X(name) decltype (::juce::gl::name)& OpenGLExtensionFunctions::name = ::juce::gl::name;
JUCE_GL_BASE_FUNCTIONS
JUCE_GL_EXTENSION_FUNCTIONS
JUCE_GL_VERTEXBUFFER_FUNCTIONS
#undef X

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

#if JUCE_MAC || JUCE_IOS

 #ifndef JUCE_IOS_MAC_VIEW
  #if JUCE_IOS
   #define JUCE_IOS_MAC_VIEW    UIView
   #define JUCE_IOS_MAC_WINDOW  UIWindow
  #else
   #define JUCE_IOS_MAC_VIEW    NSView
   #define JUCE_IOS_MAC_WINDOW  NSWindow
  #endif
 #endif

#endif

static bool checkPeerIsValid (OpenGLContext* context)
{
    jassert (context != nullptr);

    if (context != nullptr)
    {
        if (auto* comp = context->getTargetComponent())
        {
            if (auto* peer [[maybe_unused]] = comp->getPeer())
            {
               #if JUCE_MAC || JUCE_IOS
                if (auto* nsView = (JUCE_IOS_MAC_VIEW*) peer->getNativeHandle())
                {
                    if ([[maybe_unused]] auto nsWindow = [nsView window])
                    {
                       #if JUCE_MAC
                        return ([nsWindow isVisible]
                                  && (! [nsWindow hidesOnDeactivate] || [NSApp isActive]));
                       #else
                        return true;
                       #endif
                    }
                }
               #else
                return true;
               #endif
            }
        }
    }

    return false;
}

static void checkGLError (const char* file, const int line)
{
    for (;;)
    {
        const GLenum e = glGetError();

        if (e == GL_NO_ERROR)
            break;

        // if the peer is not valid then ignore errors
        if (! checkPeerIsValid (OpenGLContext::getCurrentContext()))
            continue;

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
   #if JUCE_DEBUG
    while (glGetError() != GL_NO_ERROR) {}
   #endif
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

} // namespace juce

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
  #include "native/juce_OpenGL_mac.h"
 #else
  #include "native/juce_OpenGL_ios.h"
 #endif

#elif JUCE_WINDOWS
 #include "opengl/juce_wgl.h"
 #include "native/juce_OpenGL_windows.h"

#define JUCE_IMPL_WGL_EXTENSION_FUNCTION(name) \
    decltype (juce::OpenGLContext::NativeContext::name) juce::OpenGLContext::NativeContext::name = nullptr;

JUCE_IMPL_WGL_EXTENSION_FUNCTION (wglChoosePixelFormatARB)
JUCE_IMPL_WGL_EXTENSION_FUNCTION (wglSwapIntervalEXT)
JUCE_IMPL_WGL_EXTENSION_FUNCTION (wglGetSwapIntervalEXT)
JUCE_IMPL_WGL_EXTENSION_FUNCTION (wglCreateContextAttribsARB)

#undef JUCE_IMPL_WGL_EXTENSION_FUNCTION

#elif JUCE_LINUX || JUCE_BSD
 #include <juce_gui_basics/native/juce_ScopedWindowAssociation_linux.h>
 #include "native/juce_OpenGL_linux.h"

#elif JUCE_ANDROID
 #include "native/juce_OpenGL_android.h"

#endif

#include "opengl/juce_OpenGLContext.cpp"
#include "utils/juce_OpenGLAppComponent.cpp"
