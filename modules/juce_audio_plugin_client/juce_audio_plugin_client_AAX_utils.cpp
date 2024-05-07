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

#include <juce_core/system/juce_TargetPlatform.h>

#if JucePlugin_Build_AAX

#include <AAX_Version.h>

static_assert (AAX_SDK_CURRENT_REVISION >= AAX_SDK_2p4p0_REVISION, "JUCE requires AAX SDK version 2.4.0 or higher");

#if JUCE_INTEL || (JUCE_MAC && JUCE_ARM)

#include <juce_core/system/juce_CompilerWarnings.h>

// Utilities
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")
#include <Libs/AAXLibrary/source/AAX_CAutoreleasePool.Win.cpp>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations",
                                     "-Wextra-semi",
                                     "-Wfloat-equal",
                                     "-Winconsistent-missing-destructor-override",
                                     "-Wshift-sign-overflow",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wfour-char-constants",
                                     "-Wdeprecated-copy-with-user-provided-dtor",
                                     "-Wdeprecated",
                                     "-Wlanguage-extension-token",
                                     "-Wmicrosoft-enum-value",
                                     "-Wmisleading-indentation",
                                     "-Wregister")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6001 6053 4996 5033 4068 4996 5272)

#include <Libs/AAXLibrary/source/AAX_CChunkDataParser.cpp>
#include <Libs/AAXLibrary/source/AAX_CHostServices.cpp>

#if defined (_WIN32) && ! defined (WIN32)
 #define WIN32
#endif
#include <Libs/AAXLibrary/source/AAX_CMutex.cpp>

#include <Libs/AAXLibrary/source/AAX_CommonConversions.cpp>
#include <Libs/AAXLibrary/source/AAX_CPacketDispatcher.cpp>
#include <Libs/AAXLibrary/source/AAX_CString.cpp>

// Versioned Interfaces
#include <Interfaces/ACF/CACFClassFactory.cpp>
#include <Libs/AAXLibrary/source/AAX_CACFUnknown.cpp>

#include <Libs/AAXLibrary/source/AAX_CUIDs.cpp>
#include <Libs/AAXLibrary/source/AAX_IEffectDirectData.cpp>
#include <Libs/AAXLibrary/source/AAX_IEffectGUI.cpp>
#include <Libs/AAXLibrary/source/AAX_IEffectParameters.cpp>
#include <Libs/AAXLibrary/source/AAX_IHostProcessor.cpp>
#include <Libs/AAXLibrary/source/AAX_Properties.cpp>
#include <Libs/AAXLibrary/source/AAX_VAutomationDelegate.cpp>
#include <Libs/AAXLibrary/source/AAX_VCollection.cpp>
#include <Libs/AAXLibrary/source/AAX_VComponentDescriptor.cpp>
#include <Libs/AAXLibrary/source/AAX_VController.cpp>
#include <Libs/AAXLibrary/source/AAX_VDescriptionHost.cpp>
#include <Libs/AAXLibrary/source/AAX_VEffectDescriptor.cpp>
#include <Libs/AAXLibrary/source/AAX_VFeatureInfo.cpp>
#include <Libs/AAXLibrary/source/AAX_VHostProcessorDelegate.cpp>
#include <Libs/AAXLibrary/source/AAX_VHostServices.cpp>
#include <Libs/AAXLibrary/source/AAX_VPageTable.cpp>
#include <Libs/AAXLibrary/source/AAX_VPrivateDataAccess.cpp>
#include <Libs/AAXLibrary/source/AAX_VPropertyMap.cpp>
#include <Libs/AAXLibrary/source/AAX_VTransport.cpp>
#include <Libs/AAXLibrary/source/AAX_VViewContainer.cpp>
#include <Libs/AAXLibrary/source/AAX_CEffectDirectData.cpp>
#include <Libs/AAXLibrary/source/AAX_CEffectGUI.cpp>

#include <Libs/AAXLibrary/source/AAX_CEffectParameters.cpp>
#include <Libs/AAXLibrary/source/AAX_CHostProcessor.cpp>
#include <Libs/AAXLibrary/source/AAX_CParameter.cpp>
#include <Libs/AAXLibrary/source/AAX_CParameterManager.cpp>
#include <Libs/AAXLibrary/source/AAX_Init.cpp>
#include <Libs/AAXLibrary/source/AAX_SliderConversions.cpp>

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#else
 #error "This version of the AAX SDK does not support the current platform."
#endif
#endif
