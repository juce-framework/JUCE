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

#if JUCE_INTERNAL_HAS_VST3

#include <juce_audio_processors_headless/format_types/juce_VST3PluginFormatImpl.h>

namespace juce
{

// UB Sanitizer doesn't necessarily have instrumentation for loaded plugins, so
// it won't recognize the dynamic types of pointers to the plugin's interfaces.
JUCE_BEGIN_NO_SANITIZE ("vptr")

bool VST3PluginFormatHeadless::setStateFromVSTPresetFile (AudioPluginInstance* api, const MemoryBlock& rawData)
{
    if (auto vst3 = dynamic_cast<VST3PluginInstanceHeadless*> (api))
        return vst3->setStateFromPresetFile (rawData);

    return false;
}

void VST3PluginFormatHeadless::findAllTypesForFile (OwnedArray<PluginDescription>& results, const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    if (const auto fast = DescriptionLister::findDescriptionsFast (File (fileOrIdentifier)); ! fast.empty())
    {
        for (const auto& d : fast)
            results.add (new PluginDescription (d));

        return;
    }

    for (const auto& file : getLibraryPaths (*this, fileOrIdentifier))
    {
        /**
            Since there is no apparent indication if a VST3 plugin is a shell or not,
            we're stuck iterating through a VST3's factory, creating a description
            for every housed plugin.
        */

        auto handle = RefCountedDllHandle::getHandle (file);

        if (handle == nullptr)
            continue;

        auto pluginFactory = handle->getPluginFactory();

        if (pluginFactory == nullptr)
            continue;

        VSTComSmartPtr host { new VST3HostContextHeadless(), IncrementRef::yes };

        for (const auto& d : DescriptionLister::findDescriptionsSlow (*host, *pluginFactory, File (file)))
            results.add (new PluginDescription (d));
    }
}

void VST3PluginFormatHeadless::createARAFactoryAsync (const PluginDescription& description, ARAFactoryCreationCallback callback)
{
    if (! description.hasARAExtension)
    {
        jassertfalse;
        callback ({ {}, "The provided plugin does not support ARA features" });
    }

    const File file (description.fileOrIdentifier);
    auto handle = RefCountedDllHandle::getHandle (file.getFullPathName());
    auto pluginFactory = handle->getPluginFactory();
    const auto* pluginName = description.name.toRawUTF8();

    callback ({ ARAFactoryWrapper { ::juce::getARAFactory (pluginFactory.get(), pluginName) }, {} });
}

void VST3PluginFormatHeadless::createPluginInstance (const PluginDescription& description,
                                                     double,
                                                     int,
                                                     PluginCreationCallback callback)
{
    createVst3InstanceImpl<VST3PluginInstanceHeadless> (*this,
                                                        { new VST3HostContextHeadless(), IncrementRef::no },
                                                        description,
                                                        callback);
}

bool VST3PluginFormatHeadless::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

bool VST3PluginFormatHeadless::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);

    return f.hasFileExtension (".vst3") && f.exists();
}

String VST3PluginFormatHeadless::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier; //Impossible to tell because every VST3 is a type of shell...
}

bool VST3PluginFormatHeadless::pluginNeedsRescanning (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).getLastModificationTime() != description.lastFileModTime;
}

bool VST3PluginFormatHeadless::doesPluginStillExist (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).exists();
}

StringArray VST3PluginFormatHeadless::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive, bool)
{
    StringArray results;

    for (int i = 0; i < directoriesToSearch.getNumPaths(); ++i)
        recursiveFileSearch (*this, results, directoriesToSearch[i], recursive);

    return results;
}

FileSearchPath VST3PluginFormatHeadless::getDefaultLocationsToSearch()
{
   #if JUCE_WINDOWS
    const auto localAppData = File::getSpecialLocation (File::windowsLocalAppData)        .getFullPathName();
    const auto programFiles = File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName();
    return FileSearchPath (localAppData + "\\Programs\\Common\\VST3;" + programFiles + "\\Common Files\\VST3");
   #elif JUCE_MAC
    return FileSearchPath ("~/Library/Audio/Plug-Ins/VST3;/Library/Audio/Plug-Ins/VST3");
   #else
    return FileSearchPath ("~/.vst3/;/usr/lib/vst3/;/usr/local/lib/vst3/");
   #endif
}

JUCE_END_NO_SANITIZE

} // namespace juce

#endif
