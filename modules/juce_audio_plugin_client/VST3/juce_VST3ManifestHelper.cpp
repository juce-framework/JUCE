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

//==============================================================================
// This suppresses a warning caused by some of the Steinberg source code
#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
 #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

//==============================================================================
// This suppresses a warning in juce_TargetPlatform.h
#ifndef JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED
 #define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1
#endif

#include <juce_core/system/juce_CompilerWarnings.h>
#include <juce_core/system/juce_CompilerSupport.h>

//==============================================================================
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
 #define NOMINMAX
#endif

#include <juce_audio_processors/format_types/VST3_SDK/pluginterfaces/base/coreiids.cpp>
#include <juce_audio_processors/format_types/VST3_SDK/pluginterfaces/base/funknown.cpp>
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

//==============================================================================
#if JucePlugin_Enable_ARA
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpragma-pack")
 #include <ARA_API/ARAVST3.h>
 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif // JucePlugin_Enable_ARA

//==============================================================================
#define jassert(x) assert ((x))
#define jassertfalse assert (false)
#define DBG(x)
#define JUCE_DECLARE_NON_COPYABLE(x)
#define JUCE_API

#if __has_include ("JucePluginDefines.h")
 #include "JucePluginDefines.h"
#endif

#include <juce_audio_processors/format_types/juce_VST3Utilities.h>
#include <juce_audio_processors/utilities/juce_VST3Interface.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include "juce_VST3ModuleInfo.h"

//==============================================================================
class JucePluginModule : public VST3::Hosting::Module
{
public:
    JucePluginModule()
    {
        using namespace Steinberg;
        using namespace VST3::Hosting;
        PluginFactory tmp { owned (new juce::JucePluginFactoryBase()) };
        factory = std::move (tmp);
    }

private:
    bool load (const std::string&, std::string&) final { return {}; }
};

static std::optional<Steinberg::ModuleInfo::CompatibilityList> loadCompatibilityFromModule (const VST3::Hosting::Module& pluginModule)
{
    const auto& factory = pluginModule.getFactory();
    const auto& infos = factory.classInfos();

    const auto iter = std::find_if (infos.begin(), infos.end(), [&] (const auto& info)
    {
        return info.category() == kPluginCompatibilityClass;
    });

    if (iter == infos.end())
        return {};

    const auto compatibility = factory.createInstance<Steinberg::IPluginCompatibility> (iter->ID());

    if (compatibility == nullptr)
        return {};

    Steinberg::MemoryStream stream;

    if (compatibility->getCompatibilityJSON (&stream) != Steinberg::kResultOk)
        return {};

    const std::string_view streamView (stream.getData(), (size_t) stream.getSize());

    return Steinberg::ModuleInfoLib::parseCompatibilityJson (streamView, nullptr);
}

//==============================================================================
int main()
{
    const JucePluginModule pluginModule;

    auto moduleInfo = Steinberg::ModuleInfoLib::createModuleInfo (pluginModule, false);

    if (auto compatibility = loadCompatibilityFromModule (pluginModule))
        moduleInfo.compatibility = *compatibility;

    moduleInfo.name = JucePlugin_Name;
    moduleInfo.version = JucePlugin_VersionString;

    std::stringstream output;
    Steinberg::ModuleInfoLib::outputJson (moduleInfo, output);
    std::cout << output.str() << std::endl;
    return 0;
}

//==============================================================================
namespace VST3::Hosting
{
    Module::SnapshotList Module::getSnapshots (const std::string&) { return {}; }
    Optional<std::string> Module::getModuleInfoPath (const std::string&) { return {}; }
    Module::Ptr Module::create (const std::string&, std::string&) { return {}; }
} // namespace VST3::Hosting
