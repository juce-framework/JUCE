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

// This suppresses a warning in juce_TargetPlatform.h
#ifndef JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED
 #define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1
#endif

#include <juce_core/system/juce_CompilerWarnings.h>
#include <juce_core/system/juce_CompilerSupport.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wc++98-compat-extra-semi",
                                     "-Wdeprecated-declarations",
                                     "-Wexpansion-to-defined",
                                     "-Wfloat-equal",
                                     "-Wformat",
                                     "-Wmissing-prototypes",
                                     "-Wpragma-pack",
                                     "-Wredundant-decls",
                                     "-Wshadow",
                                     "-Wshadow-field",
                                     "-Wshorten-64-to-32",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant")

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6387 6031)

// As of at least 3.7.12 there is a bug in fplatform.h that leads to SMTG_CPP20
// having the wrong value when the /Zc:__cplusplus is not enabled. This work
// around prevents needing to provide that flag

#include <juce_audio_processors/format_types/VST3_SDK/pluginterfaces/base/fplatform.h>

#ifdef SMTG_CPP20
 #undef SMTG_CPP20
 #define SMTG_CPP20 JUCE_CXX20_IS_AVAILABLE
#endif

#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
 #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#ifndef NOMINMAX
 #define NOMINMAX 1
#endif

#if JUCE_MAC
 #include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module_mac.mm>
#elif JUCE_WINDOWS
 #include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module_win32.cpp>
#elif JUCE_LINUX
 #include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module_linux.cpp>
#endif

#include <juce_audio_processors/format_types/VST3_SDK/pluginterfaces/base/coreiids.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/pluginterfaces/base/funknown.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/samples/vst-utilities/moduleinfotool/source/main.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/common/commonstringconvert.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/common/memorystream.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/common/readfile.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/moduleinfo/moduleinfocreator.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/utility/stringconvert.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/vstinitiids.cpp>

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
