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

  ID:                 juce_product_unlocking
  vendor:             juce
  version:            7.0.6
  name:               JUCE Online marketplace support
  description:        Classes for online product authentication
  website:            http://www.juce.com/juce
  license:            GPL/Commercial
  minimumCppStandard: 17

  dependencies:       juce_cryptography

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_PRODUCT_UNLOCKING_H_INCLUDED

/**
    The juce_product_unlocking module provides simple user-registration classes
    for allowing you to build apps/plugins with features that are unlocked by a
    user having a suitable account on a webserver.

    Although originally designed for use with products that are sold on the
    Tracktion Marketplace web-store, the module itself is fully open, and can
    be used to connect to your own web-store instead, if you implement your
    own compatible web-server back-end.

    In additional, the module supports in-app purchases both on iOS and Android
    platforms.
*/

//==============================================================================
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_events/juce_events.h>

#if JUCE_MODULE_AVAILABLE_juce_data_structures
 #include <juce_data_structures/juce_data_structures.h>
#endif

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #include <juce_gui_extra/juce_gui_extra.h>
#endif

#if JUCE_IN_APP_PURCHASES
 #include "in_app_purchases/juce_InAppPurchases.h"
#endif

#if JUCE_MODULE_AVAILABLE_juce_data_structures
 #include "marketplace/juce_OnlineUnlockStatus.h"
 #include "marketplace/juce_TracktionMarketplaceStatus.h"
#endif

#include "marketplace/juce_KeyFileGeneration.h"

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #include "marketplace/juce_OnlineUnlockForm.h"
#endif
