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

  ID:                 juce_box2d
  vendor:             juce
  version:            8.0.10
  name:               JUCE wrapper for the Box2D physics engine
  description:        The Box2D physics engine and some utility classes.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_graphics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_BOX2D_H_INCLUDED

//==============================================================================
#include <juce_graphics/juce_graphics.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                     "-Wdeprecated",
                                     "-Wfloat-equal",
                                     "-Wmaybe-uninitialized",
                                     "-Wshadow-field",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wsuggest-override")

#include <climits>
#include <cfloat>

#include "box2d/Box2D.h"

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

// For some reason, Doxygen sees this as a re-definition of Box2DRenderer
/** @cond */
#include "utils/juce_Box2DRenderer.h"
/** @endcond */
