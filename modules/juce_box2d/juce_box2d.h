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

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_box2d
  vendor:           juce
  version:          4.3.0
  name:             JUCE wrapper for the Box2D physics engine
  description:      The Box2D physics engine and some utility classes.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_graphics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_BOX2D_H_INCLUDED
#define JUCE_BOX2D_H_INCLUDED

//==============================================================================
#include <juce_graphics/juce_graphics.h>

#include "box2d/Box2D.h"

#ifndef DOXYGEN // for some reason, Doxygen sees this as a re-definition of Box2DRenderer
namespace juce
{
  #include "utils/juce_Box2DRenderer.h"
}
#endif // DOXYGEN

#endif   // JUCE_BOX2D_H_INCLUDED
