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

  ID:               juce_box2d
  vendor:           juce
  version:          5.4.1
  name:             JUCE wrapper for the Box2D physics engine
  description:      The Box2D physics engine and some utility classes.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_graphics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_BOX2D_H_INCLUDED

//==============================================================================
#include <juce_graphics/juce_graphics.h>

#ifdef __GNUC__
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wconversion"
 #if defined (__clang__)
  #if __has_warning("-Wzero-as-null-pointer-constant")
   #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
  #endif
 #endif
#endif

#include <climits>
#include <cfloat>

#include "box2d/Box2D.h"

#ifdef __GNUC__
 #pragma GCC diagnostic pop
#endif

#ifndef DOXYGEN // for some reason, Doxygen sees this as a re-definition of Box2DRenderer
 #include "utils/juce_Box2DRenderer.h"
#endif // DOXYGEN
