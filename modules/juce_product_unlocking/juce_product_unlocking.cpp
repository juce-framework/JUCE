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

#ifdef JUCE_PRODUCT_UNLOCKING_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_JNI_HELPERS    1
#define JUCE_CORE_INCLUDE_OBJC_HELPERS   1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1

// Set this flag to 1 to use test servers on iOS
#ifndef JUCE_IN_APP_PURCHASES_USE_SANDBOX_ENVIRONMENT
    #define JUCE_IN_APP_PURCHASES_USE_SANDBOX_ENVIRONMENT 0
#endif

#include "juce_product_unlocking.h"

#if JUCE_IOS || JUCE_MAC
 #import <StoreKit/StoreKit.h>
#endif

#if JUCE_IN_APP_PURCHASES
 #if JUCE_ANDROID
  #include "native/juce_InAppPurchases_android.cpp"
 #elif JUCE_IOS || JUCE_MAC
  #include "native/juce_InAppPurchases_ios.cpp"
 #endif

 #include "in_app_purchases/juce_InAppPurchases.cpp"
#endif

#include "marketplace/juce_OnlineUnlockStatus.cpp"

#if JUCE_MODULE_AVAILABLE_juce_data_structures
 #include "marketplace/juce_TracktionMarketplaceStatus.cpp"
#endif

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #include "marketplace/juce_OnlineUnlockForm.cpp"
#endif
