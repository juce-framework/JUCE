/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_VST3HEADERS_H_INCLUDED
#define JUCE_VST3HEADERS_H_INCLUDED

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
 #include <public.sdk/source/common/memorystream.h>
 #include <public.sdk/source/vst/vsteditcontroller.h>
#else
 #include <base/source/baseiids.cpp>
 #include <base/source/fatomic.cpp>
 #include <base/source/fbuffer.cpp>
 #include <base/source/fdebug.cpp>
 #include <base/source/fobject.cpp>
 #include <base/source/frect.cpp>
 #include <base/source/fstreamer.cpp>
 #include <base/source/fstring.cpp>
 #include <base/source/fthread.cpp>
 #include <base/source/updatehandler.cpp>
 #include <pluginterfaces/base/conststringtable.cpp>
 #include <pluginterfaces/base/funknown.cpp>
 #include <pluginterfaces/base/ipluginbase.h>
 #include <pluginterfaces/base/ustring.cpp>
 #include <pluginterfaces/gui/iplugview.h>
 #include <pluginterfaces/vst/ivstmidicontrollers.h>
 #include <public.sdk/source/common/memorystream.cpp>
 #include <public.sdk/source/common/pluginview.cpp>
 #include <public.sdk/source/vst/vsteditcontroller.cpp>
 #include <public.sdk/source/vst/vstbus.cpp>
 #include <public.sdk/source/vst/vstinitiids.cpp>
 #include <public.sdk/source/vst/vstcomponent.cpp>
 #include <public.sdk/source/vst/vstcomponentbase.cpp>
 #include <public.sdk/source/vst/vstparameters.cpp>
 #include <public.sdk/source/vst/hosting/hostclasses.cpp>

//==============================================================================
namespace Steinberg
{
    /** Missing IIDs */
    DEF_CLASS_IID (IPluginBase)
    DEF_CLASS_IID (IPlugView)
    DEF_CLASS_IID (IPlugFrame)
    DEF_CLASS_IID (IBStream)
    DEF_CLASS_IID (IPluginFactory)
    DEF_CLASS_IID (IPluginFactory2)
    DEF_CLASS_IID (IPluginFactory3)
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

#endif   // JUCE_VST3HEADERS_H_INCLUDED
