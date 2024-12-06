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

#if JUCE_BSD && ! JUCE_CUSTOM_VST3_SDK
 #error To build JUCE VST3 plug-ins on BSD you must use an external BSD-compatible VST3 SDK with JUCE_CUSTOM_VST3_SDK=1
#endif

// It's important to include this *before* any of the Steinberg headers.
// On Windows, the VST3 headers might end up defining `stricmp` as `_stricmp` before including
// <cstring> or <string.h>, which prevents the use of stricmp in JUCE source.
#include <cstring>

// Wow, those Steinberg guys really don't worry too much about compiler warnings.
JUCE_BEGIN_IGNORE_WARNINGS_LEVEL_MSVC (0, 4505 4702 6011 6031 6221 6386 6387 6330 6001 28199)

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-W#warnings",
                                     "-Wcast-align",
                                     "-Wclass-memaccess",
                                     "-Wcomma",
                                     "-Wconversion",
                                     "-Wcpp",
                                     "-Wdelete-non-virtual-dtor",
                                     "-Wdeprecated",
                                     "-Wdeprecated-copy-dtor",
                                     "-Wdeprecated-declarations",
                                     "-Wdeprecated-register",
                                     "-Wextra",
                                     "-Wextra-semi",
                                     "-Wfloat-equal",
                                     "-Wformat",
                                     "-Wformat-truncation=",
                                     "-Wformat=",
                                     "-Wignored-qualifiers",
                                     "-Winconsistent-missing-destructor-override",
                                     "-Wint-to-pointer-cast",
                                     "-Wlogical-op-parentheses",
                                     "-Wmaybe-uninitialized",
                                     "-Wmissing-braces",
                                     "-Wmissing-field-initializers",
                                     "-Wmissing-prototypes",
                                     "-Wnon-virtual-dtor",
                                     "-Woverloaded-virtual",
                                     "-Wparentheses",
                                     "-Wpedantic",
                                     "-Wpragma-pack",
                                     "-Wredundant-decls",
                                     "-Wreorder",
                                     "-Wshadow",
                                     "-Wshadow-field",
                                     "-Wsign-compare",
                                     "-Wsign-conversion",
                                     "-Wswitch-default",
                                     "-Wtype-limits",
                                     "-Wunsequenced",
                                     "-Wunused-but-set-variable",
                                     "-Wunused-function",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wdangling-else")

#undef DEVELOPMENT
#define DEVELOPMENT 0  // This avoids a Clang warning in Steinberg code about unused values

// As of at least 3.7.12 there is a bug in fplatform.h that leads to SMTG_CPP20
// having the wrong value when the /Zc:__cplusplus is not enabled. This work
// around prevents needing to provide that flag

#include <juce_audio_processors/format_types/VST3_SDK/pluginterfaces/base/fplatform.h>

#ifdef SMTG_CPP20
 #undef SMTG_CPP20
 #define SMTG_CPP20 JUCE_CXX20_IS_AVAILABLE
#endif

#if JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY
 #include <base/source/fstring.h>
 #include <pluginterfaces/base/conststringtable.h>
 #include <pluginterfaces/base/funknown.h>
 #include <pluginterfaces/base/ipluginbase.h>
 #include <pluginterfaces/base/iplugincompatibility.h>
 #include <pluginterfaces/base/ustring.h>
 #include <pluginterfaces/gui/iplugview.h>
 #include <pluginterfaces/gui/iplugviewcontentscalesupport.h>
 #include <pluginterfaces/vst/ivstattributes.h>
 #include <pluginterfaces/vst/ivstaudioprocessor.h>
 #include <pluginterfaces/vst/ivstcomponent.h>
 #include <pluginterfaces/vst/ivstcontextmenu.h>
 #include <pluginterfaces/vst/ivsteditcontroller.h>
 #include <pluginterfaces/vst/ivstevents.h>
 #include <pluginterfaces/vst/ivsthostapplication.h>
 #include <pluginterfaces/vst/ivstmessage.h>
 #include <pluginterfaces/vst/ivstmidicontrollers.h>
 #include <pluginterfaces/vst/ivstparameterchanges.h>
 #include <pluginterfaces/vst/ivstplugview.h>
 #include <pluginterfaces/vst/ivstprocesscontext.h>
 #include <pluginterfaces/vst/ivstremapparamid.h>
 #include <pluginterfaces/vst/vsttypes.h>
 #include <pluginterfaces/vst/ivstunits.h>
 #include <pluginterfaces/vst/ivstmidicontrollers.h>
 #include <pluginterfaces/vst/ivstchannelcontextinfo.h>
 #include <public.sdk/source/common/memorystream.h>
 #include <public.sdk/source/vst/utility/uid.h>
 #include <public.sdk/source/vst/utility/vst2persistence.h>
 #include <public.sdk/source/vst/vsteditcontroller.h>
 #include <public.sdk/source/vst/vstpresetfile.h>

 #include "pslextensions/ipslviewembedding.h"
#else
 // needed for VST_VERSION
 #include <pluginterfaces/vst/vsttypes.h>

 #ifndef NOMINMAX
  #define NOMINMAX // Some of the steinberg sources don't set this before including windows.h
 #endif

 #include <base/source/baseiids.cpp>
 #include <base/source/fbuffer.cpp>
 #include <base/source/fdebug.cpp>
 #include <base/source/fobject.cpp>
 #include <base/source/fstreamer.cpp>
 #include <base/source/fstring.cpp>

 // The following shouldn't leak from fstring.cpp
 #undef stricmp
 #undef strnicmp
 #undef snprintf
 #undef vsnprintf
 #undef snwprintf
 #undef vsnwprintf

 #if VST_VERSION >= 0x030608
  #include <base/thread/source/flock.cpp>
  #include <pluginterfaces/base/coreiids.cpp>
 #else
  #include <base/source/flock.cpp>
 #endif

 #pragma push_macro ("True")
 #undef True
 #pragma push_macro ("False")
 #undef False

 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmultichar", "-Wfour-char-constants")

 #include <base/source/updatehandler.cpp>
 #include <pluginterfaces/base/conststringtable.cpp>
 #include <pluginterfaces/base/funknown.cpp>
 #include <pluginterfaces/base/ipluginbase.h>
 #include <pluginterfaces/base/ustring.cpp>
 #include <pluginterfaces/gui/iplugview.h>
 #include <pluginterfaces/gui/iplugviewcontentscalesupport.h>
 #include <pluginterfaces/vst/ivstchannelcontextinfo.h>
 #include <pluginterfaces/vst/ivstmidicontrollers.h>
 #include <public.sdk/source/common/commonstringconvert.cpp>
 #include <public.sdk/source/common/memorystream.cpp>
 #include <public.sdk/source/common/pluginview.cpp>
 #include <public.sdk/source/vst/hosting/hostclasses.cpp>
 #include <public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp>
 #include <public.sdk/source/vst/utility/stringconvert.cpp>
 #include <public.sdk/source/vst/utility/vst2persistence.cpp>
 #include <public.sdk/source/vst/utility/uid.h>
 #include <public.sdk/source/vst/vstbus.cpp>
 #include <public.sdk/source/vst/vstcomponent.cpp>
 #include <public.sdk/source/vst/vstcomponentbase.cpp>
 #include <public.sdk/source/vst/vsteditcontroller.cpp>
 #include <public.sdk/source/vst/vstinitiids.cpp>
 #include <public.sdk/source/vst/vstparameters.cpp>
 #include <public.sdk/source/vst/vstpresetfile.cpp>

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE

 #pragma pop_macro ("True")
 #pragma pop_macro ("False")

 #if VST_VERSION >= 0x03060c   // 3.6.12
  #include <public.sdk/source/vst/hosting/pluginterfacesupport.cpp>
 #endif

 #include "pslextensions/ipslviewembedding.h"

//==============================================================================
namespace Steinberg
{
    /** Missing IIDs */
  #if VST_VERSION < 0x03060d   // 3.6.13
    DEF_CLASS_IID (IPluginBase)
    DEF_CLASS_IID (IPluginFactory)
    DEF_CLASS_IID (IPluginFactory2)
    DEF_CLASS_IID (IPluginFactory3)
   #if VST_VERSION < 0x030608
    DEF_CLASS_IID (IBStream)
   #endif
  #endif
    DEF_CLASS_IID (IPlugView)
    DEF_CLASS_IID (IPlugFrame)
    DEF_CLASS_IID (IPlugViewContentScaleSupport)

   #if JUCE_LINUX || JUCE_BSD
    DEF_CLASS_IID (Linux::IRunLoop)
    DEF_CLASS_IID (Linux::IEventHandler)
   #endif
}

namespace Presonus
{
    DEF_CLASS_IID (IPlugInViewEmbedding)
}

#endif // JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#if JUCE_WINDOWS
 #include <windows.h>
#endif

//==============================================================================
#undef ASSERT
#undef WARNING
#undef PRINTSYSERROR
#undef DEBUGSTR
#undef DBPRT0
#undef DBPRT1
#undef DBPRT2
#undef DBPRT3
#undef DBPRT4
#undef DBPRT5
#undef min
#undef max
#undef MIN
#undef MAX
#undef calloc
#undef free
#undef malloc
#undef realloc
#undef NEW
#undef NEWVEC
#undef VERIFY
#undef VERIFY_IS
#undef VERIFY_NOT
#undef META_CREATE_FUNC
#undef CLASS_CREATE_FUNC
#undef SINGLE_CREATE_FUNC
#undef _META_CLASS
#undef _META_CLASS_IFACE
#undef _META_CLASS_SINGLE
#undef META_CLASS
#undef META_CLASS_IFACE
#undef META_CLASS_SINGLE
#undef SINGLETON
#undef OBJ_METHODS
#undef QUERY_INTERFACE
#undef LICENCE_UID
#undef BEGIN_FACTORY
#undef DEF_CLASS
#undef DEF_CLASS1
#undef DEF_CLASS2
#undef DEF_CLASS_W
#undef END_FACTORY

#ifdef atomic_thread_fence
 #undef atomic_thread_fence
#endif
