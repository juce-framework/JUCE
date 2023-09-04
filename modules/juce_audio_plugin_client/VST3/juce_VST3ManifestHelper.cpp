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

// This suppresses a warning in juce_TargetPlatform.h
#ifndef JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED
 #define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1
#endif

#include <juce_core/system/juce_CompilerWarnings.h>

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
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/common/memorystream.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/common/readfile.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/hosting/module.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/moduleinfo/moduleinfocreator.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/utility/stringconvert.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/public.sdk/source/vst/vstinitiids.cpp>

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
