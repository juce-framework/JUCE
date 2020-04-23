/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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

  ID:                 juce_osc
  vendor:             juce
  version:            6.0.0
  name:               JUCE OSC classes
  description:        Open Sound Control implementation.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial

  dependencies:       juce_events

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_OSC_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

//==============================================================================
#include "osc/juce_OSCTypes.h"
#include "osc/juce_OSCTimeTag.h"
#include "osc/juce_OSCArgument.h"
#include "osc/juce_OSCAddress.h"
#include "osc/juce_OSCMessage.h"
#include "osc/juce_OSCBundle.h"
#include "osc/juce_OSCReceiver.h"
#include "osc/juce_OSCSender.h"
