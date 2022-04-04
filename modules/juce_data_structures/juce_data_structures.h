/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_data_structures
  vendor:             juce
  version:            6.1.6
  name:               JUCE data model helper classes
  description:        Classes for undo/redo management, and smart data structures.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial
  minimumCppStandard: 14

  dependencies:       juce_events

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_DATA_STRUCTURES_H_INCLUDED

//==============================================================================
#include <juce_events/juce_events.h>

#include "undomanager/juce_UndoableAction.h"
#include "undomanager/juce_UndoManager.h"
#include "values/juce_Value.h"
#include "values/juce_ValueTree.h"
#include "values/juce_ValueTreeSynchroniser.h"
#include "values/juce_CachedValue.h"
#include "values/juce_ValueTreePropertyWithDefault.h"
#include "app_properties/juce_PropertiesFile.h"
#include "app_properties/juce_ApplicationProperties.h"
