/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_opengl
  vendor:           juce
  version:          5.3.2
  name:             JUCE OpenGL classes
  description:      Classes for rendering OpenGL in a JUCE window.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_gui_extra
  OSXFrameworks:    OpenGL
  iOSFrameworks:    OpenGLES
  linuxLibs:        GL
  mingwLibs:        opengl32

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_OPENGL_H_INCLUDED

#include <juce_gui_extra/juce_gui_extra.h>

#undef JUCE_OPENGL
#define JUCE_OPENGL 1

#if JUCE_IOS || JUCE_ANDROID
 #define JUCE_OPENGL_ES 1
#endif

#if JUCE_WINDOWS
 #ifndef APIENTRY
  #define APIENTRY __stdcall
  #define CLEAR_TEMP_APIENTRY 1
 #endif
 #ifndef WINGDIAPI
  #define WINGDIAPI __declspec(dllimport)
  #define CLEAR_TEMP_WINGDIAPI 1
 #endif

 #if JUCE_MINGW
  #include <GL/gl.h>
 #else
  #include <gl/GL.h>
 #endif

 #ifdef CLEAR_TEMP_WINGDIAPI
  #undef WINGDIAPI
  #undef CLEAR_TEMP_WINGDIAPI
 #endif
 #ifdef CLEAR_TEMP_APIENTRY
  #undef APIENTRY
  #undef CLEAR_TEMP_APIENTRY
 #endif
#elif JUCE_LINUX
 #include <GL/gl.h>
 #undef KeyPress
#elif JUCE_IOS
 #if defined (__IPHONE_7_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_7_0
  #include <OpenGLES/ES3/gl.h>
 #else
  #include <OpenGLES/ES2/gl.h>
 #endif
#elif JUCE_MAC
 #if defined (MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7)
  #define JUCE_OPENGL3 1
  #include <OpenGL/gl3.h>
  #include <OpenGL/gl3ext.h>
 #else
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
 #endif
#elif JUCE_ANDROID
 #include <android/native_window.h>
 #include <android/native_window_jni.h>
 #if JUCE_ANDROID_GL_ES_VERSION_3_0
  #define JUCE_OPENGL3 1
  #include <GLES3/gl3.h>
 #else
  #include <GLES2/gl2.h>
 #endif
 #include <EGL/egl.h>
#endif

#if GL_ES_VERSION_3_0
 #define JUCE_OPENGL3 1
#endif

//==============================================================================
/** This macro is a helper for use in GLSL shader code which needs to compile on both OpenGL 2.1 and OpenGL 3.0.
    It's mandatory in OpenGL 3.0 to specify the GLSL version.
*/
#if JUCE_OPENGL3
 #if JUCE_OPENGL_ES
  #define JUCE_GLSL_VERSION "#version 300 es"
 #else
  #define JUCE_GLSL_VERSION "#version 150"
 #endif
#else
 #define JUCE_GLSL_VERSION ""
#endif

//==============================================================================
#if JUCE_OPENGL_ES || defined (DOXYGEN)
 /** This macro is a helper for use in GLSL shader code which needs to compile on both GLES and desktop GL.
     Since it's mandatory in GLES to mark a variable with a precision, but the keywords don't exist in normal GLSL,
     these macros define the various precision keywords only on GLES.
 */
 #define JUCE_MEDIUMP  "mediump"

 /** This macro is a helper for use in GLSL shader code which needs to compile on both GLES and desktop GL.
     Since it's mandatory in GLES to mark a variable with a precision, but the keywords don't exist in normal GLSL,
     these macros define the various precision keywords only on GLES.
 */
 #define JUCE_HIGHP    "highp"

 /** This macro is a helper for use in GLSL shader code which needs to compile on both GLES and desktop GL.
     Since it's mandatory in GLES to mark a variable with a precision, but the keywords don't exist in normal GLSL,
     these macros define the various precision keywords only on GLES.
 */
 #define JUCE_LOWP     "lowp"
#else
 #define JUCE_MEDIUMP
 #define JUCE_HIGHP
 #define JUCE_LOWP
#endif

//==============================================================================
namespace juce
{
    class OpenGLTexture;
    class OpenGLFrameBuffer;
    class OpenGLShaderProgram;
}

#include "geometry/juce_Vector3D.h"
#include "geometry/juce_Matrix3D.h"
#include "geometry/juce_Quaternion.h"
#include "geometry/juce_Draggable3DOrientation.h"
#include "native/juce_MissingGLDefinitions.h"
#include "opengl/juce_OpenGLHelpers.h"
#include "opengl/juce_OpenGLPixelFormat.h"
#include "native/juce_OpenGLExtensions.h"
#include "opengl/juce_OpenGLRenderer.h"
#include "opengl/juce_OpenGLContext.h"
#include "opengl/juce_OpenGLFrameBuffer.h"
#include "opengl/juce_OpenGLGraphicsContext.h"
#include "opengl/juce_OpenGLImage.h"
#include "opengl/juce_OpenGLShaderProgram.h"
#include "opengl/juce_OpenGLTexture.h"
#include "utils/juce_OpenGLAppComponent.h"
