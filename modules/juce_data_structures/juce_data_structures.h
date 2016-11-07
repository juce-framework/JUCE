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

  ID:               juce_data_structures
  vendor:           juce
  version:          4.3.0
  name:             JUCE data model helper classes
  description:      Classes for undo/redo management, and smart data structures.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_events

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_DATA_STRUCTURES_H_INCLUDED
#define JUCE_DATA_STRUCTURES_H_INCLUDED

//==============================================================================
#include <juce_events/juce_events.h>

namespace juce
{

#include "undomanager/juce_UndoableAction.h"
#include "undomanager/juce_UndoManager.h"
#include "values/juce_Value.h"
#include "values/juce_ValueTree.h"
#include "values/juce_ValueTreeSynchroniser.h"
#include "values/juce_CachedValue.h"
#include "app_properties/juce_PropertiesFile.h"
#include "app_properties/juce_ApplicationProperties.h"

}

#endif   // JUCE_DATA_STRUCTURES_H_INCLUDED
