/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_opengl
  vendor:             juce
  version:            8.0.10
  name:               JUCE OpenGL classes
  description:        Classes for rendering OpenGL in a JUCE window.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_gui_extra
  OSXFrameworks:      OpenGL
  iOSFrameworks:      OpenGLES
  linuxPackages:      gl

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_OPENGL_H_INCLUDED

#include <juce_core/system/juce_TargetPlatform.h>

#undef JUCE_OPENGL
#define JUCE_OPENGL 1

#if JUCE_IOS || JUCE_ANDROID
 #define JUCE_OPENGL_ES 1
 #include "opengl/juce_gles2.h"
#else
 #include "opengl/juce_gl.h"
#endif

#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
#if JUCE_OPENGL_ES || DOXYGEN
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
