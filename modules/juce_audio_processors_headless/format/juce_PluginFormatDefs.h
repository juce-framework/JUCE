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

#pragma once

// The following definitions are PRIVATE and should not be queried or modified
// by user code. These are exclusively used to enable and disable JUCE
// implementation details.

#if defined (JUCE_INTERNAL_HAS_VST) \
 || defined (JUCE_INTERNAL_HAS_VST3) \
 || defined (JUCE_INTERNAL_HAS_AU) \
 || defined (JUCE_INTERNAL_HAS_LADSPA) \
 || defined (JUCE_INTERNAL_HAS_LV2)
 #error These preprocessor definitions should not be set by the build system. Use the JUCE_PLUGINHOST_* definitions instead.
#endif

#if JUCE_PLUGINHOST_VST && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD || JUCE_IOS)
 #define JUCE_INTERNAL_HAS_VST 1
#else
 #define JUCE_INTERNAL_HAS_VST 0
#endif

#if JUCE_PLUGINHOST_VST3 && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD)
 #define JUCE_INTERNAL_HAS_VST3 1
#else
 #define JUCE_INTERNAL_HAS_VST3 0
#endif

#if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)
 #define JUCE_INTERNAL_HAS_AU 1
#else
 #define JUCE_INTERNAL_HAS_AU 0
#endif

#if JUCE_PLUGINHOST_LADSPA && (JUCE_LINUX || JUCE_BSD)
 #define JUCE_INTERNAL_HAS_LADSPA 1
#else
 #define JUCE_INTERNAL_HAS_LADSPA 0
#endif

#if JUCE_PLUGINHOST_LV2 && (JUCE_MAC || JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS)
 #define JUCE_INTERNAL_HAS_LV2 1
#else
 #define JUCE_INTERNAL_HAS_LV2 0
#endif

#if JUCE_PLUGINHOST_ARA && (JUCE_INTERNAL_HAS_VST3 || JUCE_INTERNAL_HAS_AU) && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)
 #define JUCE_INTERNAL_HAS_ARA 1
#else
 #define JUCE_INTERNAL_HAS_ARA 0
#endif
