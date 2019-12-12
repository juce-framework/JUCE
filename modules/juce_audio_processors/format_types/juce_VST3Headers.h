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

// Wow, those Steinberg guys really don't worry too much about compiler warnings.
#if _MSC_VER
 #pragma warning (disable: 4505)
 #pragma warning (push, 0)
 #pragma warning (disable: 4702)
#elif __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wreorder"
 #pragma clang diagnostic ignored "-Wunsequenced"
 #pragma clang diagnostic ignored "-Wint-to-pointer-cast"
 #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Woverloaded-virtual"
 #pragma clang diagnostic ignored "-Wshadow"
 #pragma clang diagnostic ignored "-Wdeprecated-register"
 #pragma clang diagnostic ignored "-Wunused-function"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wsign-compare"
 #pragma clang diagnostic ignored "-Wdelete-non-virtual-dtor"
 #pragma clang diagnostic ignored "-Wdeprecated-declarations"
 #pragma clang diagnostic ignored "-Wextra-semi"
 #pragma clang diagnostic ignored "-Wmissing-braces"
 #if __has_warning("-Wshadow-field")
  #pragma clang diagnostic ignored "-Wshadow-field"
 #endif
 #if __has_warning("-Wpragma-pack")
  #pragma clang diagnostic ignored "-Wpragma-pack"
 #endif
 #if __has_warning("-Wcomma")
  #pragma clang diagnostic ignored "-Wcomma"
 #endif
 #if __has_warning("-Wzero-as-null-pointer-constant")
  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
 #endif
 #if __has_warning("-Winconsistent-missing-destructor-override")
  #pragma clang diagnostic ignored "-Winconsistent-missing-destructor-override"
 #endif
 #if __has_warning("-Wcast-align")
  #pragma clang diagnostic ignored "-Wcast-align"
 #endif
 #if __has_warning("-Wignored-qualifiers")
  #pragma clang diagnostic ignored "-Wignored-qualifiers"
 #endif
 #if __has_warning("-Wmissing-field-initializers")
  #pragma clang diagnostic ignored "-Wmissing-field-initializers"
 #endif
#endif

#undef DEVELOPMENT
#define DEVELOPMENT 0  // This avoids a Clang warning in Steinberg code about unused values

/*  These files come with the Steinberg VST3 SDK - to get them, you'll need to
    visit the Steinberg website and agree to whatever is currently required to
    get them.

    Then, you'll need to make sure your include path contains your "VST3 SDK"
    directory (or whatever you've named it on your machine). The Projucer has
    a special box for setting this path.
*/
#if JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY
 #include <base/source/fstring.h>
 #include <pluginterfaces/base/conststringtable.h>
 #include <pluginterfaces/base/funknown.h>
 #include <pluginterfaces/base/ipluginbase.h>
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
 #include <pluginterfaces/vst/vsttypes.h>
 #include <pluginterfaces/vst/ivstunits.h>
 #include <pluginterfaces/vst/ivstmidicontrollers.h>
 #include <pluginterfaces/vst/ivstchannelcontextinfo.h>
 #include <public.sdk/source/common/memorystream.h>
 #include <public.sdk/source/vst/vsteditcontroller.h>
 #include <public.sdk/source/vst/vstpresetfile.h>
#else
 // needed for VST_VERSION
 #include <pluginterfaces/vst/vsttypes.h>

 #include <base/source/baseiids.cpp>
 #include <base/source/fbuffer.cpp>
 #include <base/source/fdebug.cpp>
 #include <base/source/fobject.cpp>
 #include <base/source/fstreamer.cpp>
 #include <base/source/fstring.cpp>
#if VST_VERSION >= 0x030608
 #include <base/thread/source/flock.cpp>
 #include <pluginterfaces/base/coreiids.cpp>
#else
 #include <base/source/flock.cpp>
#endif
 #include <base/source/updatehandler.cpp>
 #include <pluginterfaces/base/conststringtable.cpp>
 #include <pluginterfaces/base/funknown.cpp>
 #include <pluginterfaces/base/ipluginbase.h>
 #include <pluginterfaces/base/ustring.cpp>
 #include <pluginterfaces/gui/iplugview.h>
 #include <pluginterfaces/gui/iplugviewcontentscalesupport.h>
 #include <pluginterfaces/vst/ivstmidicontrollers.h>
 #include <pluginterfaces/vst/ivstchannelcontextinfo.h>
 #include <public.sdk/source/common/memorystream.cpp>
 #include <public.sdk/source/common/pluginview.cpp>
 #include <public.sdk/source/vst/vsteditcontroller.cpp>
 #include <public.sdk/source/vst/vstbus.cpp>
 #include <public.sdk/source/vst/vstinitiids.cpp>
 #include <public.sdk/source/vst/vstcomponent.cpp>
 #include <public.sdk/source/vst/vstcomponentbase.cpp>
 #include <public.sdk/source/vst/vstparameters.cpp>
 #include <public.sdk/source/vst/vstpresetfile.cpp>
 #include <public.sdk/source/vst/hosting/hostclasses.cpp>
#if VST_VERSION >= 0x03060c   // 3.6.12
 #include <public.sdk/source/vst/hosting/pluginterfacesupport.cpp>
#endif

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
}
#endif //JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY

#if _MSC_VER
 #pragma warning (pop)
#elif __clang__
 #pragma clang diagnostic pop
#endif

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
