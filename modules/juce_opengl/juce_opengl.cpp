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

#ifdef __JUCE_OPENGL_JUCEHEADER__
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "../juce_core/native/juce_BasicNativeHeaders.h"
#include "juce_opengl.h"

//==============================================================================
#if JUCE_IOS
 #import <QuartzCore/QuartzCore.h>

//==============================================================================
#elif JUCE_WINDOWS
 #include <windowsx.h>
 #include <vfw.h>
 #include <commdlg.h>

 #if JUCE_WEB_BROWSER
  #include <Exdisp.h>
  #include <exdispid.h>
 #endif

 #if JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "vfw32.lib")
  #pragma comment(lib, "imm32.lib")
  #pragma comment(lib, "OpenGL32.Lib")
  #pragma comment(lib, "GlU32.Lib")
 #endif

 #if JUCE_QUICKTIME && JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment (lib, "QTMLClient.lib")
 #endif

 #if JUCE_DIRECT2D && JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment (lib, "Dwrite.lib")
  #pragma comment (lib, "D2d1.lib")
 #endif

//==============================================================================
#elif JUCE_LINUX
 #include <X11/Xlib.h>
 #include <X11/Xatom.h>
 #include <X11/Xresource.h>
 #include <X11/Xutil.h>
 #include <X11/Xmd.h>
 #include <X11/keysym.h>
 #include <X11/cursorfont.h>

 #if JUCE_USE_XINERAMA
  /* If you're trying to use Xinerama, you'll need to install the "libxinerama-dev" package..  */
  #include <X11/extensions/Xinerama.h>
 #endif

 #if JUCE_USE_XSHM
  #include <X11/extensions/XShm.h>
  #include <sys/shm.h>
  #include <sys/ipc.h>
 #endif

 #if JUCE_USE_XRENDER
  // If you're missing these headers, try installing the libxrender-dev and libxcomposite-dev
  #include <X11/extensions/Xrender.h>
  #include <X11/extensions/Xcomposite.h>
 #endif

 #if JUCE_USE_XCURSOR
  // If you're missing this header, try installing the libxcursor-dev package
  #include <X11/Xcursor/Xcursor.h>
 #endif

 /* Got an include error here?

    If you want to install OpenGL support, the packages to get are "mesa-common-dev"
    and "freeglut3-dev".
 */
 #include <GL/glx.h>

 #undef SIZEOF
 #undef KeyPress

//==============================================================================
#elif JUCE_ANDROID
 #include <GLES/glext.h>
#endif

#if JUCE_WINDOWS
 #define JUCE_DECLARE_GL_EXTENSION_FUNCTION(name, returnType, params) \
   typedef returnType (__stdcall *type_ ## name) params; static type_ ## name name;
#else
 #define JUCE_DECLARE_GL_EXTENSION_FUNCTION(name, returnType, params) \
   typedef returnType (*type_ ## name) params; static type_ ## name name;
#endif

#define JUCE_INSTANTIATE_GL_EXTENSION(name) \
   name = (type_ ## name) OpenGLHelpers::getExtensionFunction (#name);

//==============================================================================
// START_AUTOINCLUDE opengl/*.cpp
#include "opengl/juce_OpenGLComponent.cpp"
#include "opengl/juce_OpenGLFrameBuffer.cpp"
#include "opengl/juce_OpenGLGraphicsContext.cpp"
#include "opengl/juce_OpenGLHelpers.cpp"
#include "opengl/juce_OpenGLImage.cpp"
#include "opengl/juce_OpenGLTexture.cpp"
// END_AUTOINCLUDE

using namespace juce;

//==============================================================================
BEGIN_JUCE_NAMESPACE

#if JUCE_MAC || JUCE_IOS
 #include "../juce_core/native/juce_osx_ObjCHelpers.h"
 #include "../juce_core/native/juce_mac_ObjCSuffix.h"
 #include "../juce_graphics/native/juce_mac_CoreGraphicsHelpers.h"

 #if JUCE_MAC
  #include "native/juce_mac_OpenGLComponent.mm"
 #else
  #include "native/juce_ios_OpenGLComponent.mm"
 #endif

#elif JUCE_WINDOWS
 #include "native/juce_win32_OpenGLComponent.cpp"

#elif JUCE_LINUX
 #include "native/juce_linux_OpenGLComponent.cpp"

#elif JUCE_ANDROID
 #include "native/juce_android_OpenGLComponent.cpp"

#endif

END_JUCE_NAMESPACE
