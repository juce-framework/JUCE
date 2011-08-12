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

#ifndef __JUCE_OPENGL_JUCEHEADER__
#define __JUCE_OPENGL_JUCEHEADER__

#include "../juce_gui_extra/juce_gui_extra.h"

#undef JUCE_OPENGL
#if ! JUCE_ANDROID
 #define JUCE_OPENGL 1
#endif

//=============================================================================
BEGIN_JUCE_NAMESPACE

// START_AUTOINCLUDE opengl
#ifndef __JUCE_OPENGLCOMPONENT_JUCEHEADER__
 #include "opengl/juce_OpenGLComponent.h"
#endif
#ifndef __JUCE_OPENGLCONTEXT_JUCEHEADER__
 #include "opengl/juce_OpenGLContext.h"
#endif
#ifndef __JUCE_OPENGLPIXELFORMAT_JUCEHEADER__
 #include "opengl/juce_OpenGLPixelFormat.h"
#endif
// END_AUTOINCLUDE

END_JUCE_NAMESPACE

#endif   // __JUCE_OPENGL_JUCEHEADER__
