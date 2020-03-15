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

  ID:                 juce_osc
  vendor:             juce
  version:            5.4.7
  name:               JUCE OSC classes
  description:        Open Sound Control implementation.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial

  dependencies:       juce_core, juce_events

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_OSC_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

/** Config: JUCE_ALLOW_SPECIAL_CHARS_IN_ADDRESS
	Enables the use of characters in adress that are not allowed by the OSC specifications (like spaces), but that are used
	by some applications anyway (e.g. /my spaced/address)
*/
#ifndef JUCE_ALLOW_SPECIAL_CHARS_IN_ADDRESS
#define JUCE_ALLOW_SPECIAL_CHARS_IN_ADDRESS 0
#endif

/** Config: JUCE_ENABLE_BROADCAST_BY_DEFAULT
	Automatically enables broadcast on bound port in OSCReceiver
*/
#ifndef JUCE_ENABLE_BROADCAST_BY_DEFAULT
#define JUCE_ENABLE_BROADCAST_BY_DEFAULT 0
#endif

/** Config: JUCE_EXCLUSIVE_BINDING_BY_DEFAULT
	If enabled, this will make the binding of this port exclusive, so no other process can bind it.
*/
#ifndef JUCE_EXCLUSIVE_BINDING_BY_DEFAULT
#define JUCE_EXCLUSIVE_BINDING_BY_DEFAULT 0
#endif

//==============================================================================
#include "osc/juce_OSCTypes.h"
#include "osc/juce_OSCTimeTag.h"
#include "osc/juce_OSCArgument.h"
#include "osc/juce_OSCAddress.h"
#include "osc/juce_OSCMessage.h"
#include "osc/juce_OSCBundle.h"
#include "osc/juce_OSCReceiver.h"
#include "osc/juce_OSCSender.h"
