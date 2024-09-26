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

  ID:                 juce_product_unlocking
  vendor:             juce
  version:            8.0.2
  name:               JUCE Online marketplace support
  description:        Classes for online product authentication
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
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
