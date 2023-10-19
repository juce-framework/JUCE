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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_data_structures
  vendor:             juce
  version:            7.0.8
  name:               JUCE data model helper classes
  description:        Classes for undo/redo management, and smart data structures.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial
  minimumCppStandard: 17

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
