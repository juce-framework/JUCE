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

#if JUCE_INTERNAL_HAS_VST

#include <juce_audio_processors_headless/format_types/juce_VSTPluginFormatImpl.h>
#include <juce_audio_processors_headless/utilities/juce_CommonProcessorUtilities.h>

namespace juce
{

void VSTPluginFormatHeadless::findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                                   const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uniqueId = desc.deprecatedUid = 0;

    auto instance = createAndUpdateDesc (*this, desc);

    if (instance == nullptr)
        return;

    if (instance->getVstCategory() != Vst2::kPlugCategShell)
    {
        // Normal plugin...
        results.add (new PluginDescription (desc));

        instance->dispatch (Vst2::effOpen, 0, 0, nullptr, 0);
    }
    else
    {
        // It's a shell plugin, so iterate all the subtypes...
        for (;;)
        {
            char shellEffectName [256] = { 0 };
            auto uid = (int) instance->dispatch (Vst2::effShellGetNextPlugin, 0, 0, shellEffectName, 0);

            if (uid == 0)
                break;

            desc.uniqueId = desc.deprecatedUid = uid;
            desc.name = shellEffectName;

            aboutToScanVSTShellPlugin (desc);

            std::unique_ptr<VSTPluginInstanceHeadless> shellInstance (createAndUpdateDesc (*this, desc));

            if (shellInstance != nullptr)
            {
                jassert (desc.deprecatedUid == uid);
                jassert (desc.uniqueId == uid);
                desc.hasSharedContainer = true;
                desc.name = shellEffectName;

                if (! arrayContainsPlugin (results, desc))
                    results.add (new PluginDescription (desc));
            }
        }
    }
}

void VSTPluginFormatHeadless::createPluginInstance (const PluginDescription& desc,
                                                    double sampleRate,
                                                    int blockSize,
                                                    PluginCreationCallback callback)
{
    createVstPluginInstance<VSTPluginInstanceHeadless> (*this, desc, sampleRate, blockSize, callback);
}

bool VSTPluginFormatHeadless::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

bool VSTPluginFormatHeadless::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);

  #if JUCE_MAC || JUCE_IOS
    return f.isDirectory() && f.hasFileExtension (".vst");
  #elif JUCE_WINDOWS
    return f.existsAsFile() && f.hasFileExtension (".dll");
  #elif JUCE_LINUX || JUCE_BSD || JUCE_ANDROID
    return f.existsAsFile() && f.hasFileExtension (".so");
  #endif
}

String VSTPluginFormatHeadless::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier;
}

bool VSTPluginFormatHeadless::pluginNeedsRescanning (const PluginDescription& desc)
{
    return File (desc.fileOrIdentifier).getLastModificationTime() != desc.lastFileModTime;
}

bool VSTPluginFormatHeadless::doesPluginStillExist (const PluginDescription& desc)
{
    return File (desc.fileOrIdentifier).exists();
}

StringArray VSTPluginFormatHeadless::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive, bool)
{
    StringArray results;

    for (int j = 0; j < directoriesToSearch.getNumPaths(); ++j)
        recursiveFileSearch (results, directoriesToSearch [j], recursive);

    return results;
}

void VSTPluginFormatHeadless::recursiveFileSearch (StringArray& results, const File& dir, const bool recursive)
{
    // avoid allowing the dir iterator to be recursive, because we want to avoid letting it delve inside
    // .component or .vst directories.
    for (const auto& iter : RangedDirectoryIterator (dir, false, "*", File::findFilesAndDirectories))
    {
        auto f = iter.getFile();
        bool isPlugin = false;

        if (fileMightContainThisPluginType (f.getFullPathName()))
        {
            isPlugin = true;
            results.add (f.getFullPathName());
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (results, f, true);
    }
}

FileSearchPath VSTPluginFormatHeadless::getDefaultLocationsToSearch()
{
   #if JUCE_MAC
    return FileSearchPath ("~/Library/Audio/Plug-Ins/VST;/Library/Audio/Plug-Ins/VST");
   #elif JUCE_LINUX || JUCE_BSD || JUCE_ANDROID
    return FileSearchPath (SystemStats::getEnvironmentVariable ("VST_PATH",
                                                                "/usr/lib/vst;/usr/local/lib/vst;~/.vst")
                             .replace (":", ";"));
   #elif JUCE_WINDOWS
    auto programFiles = File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName();

    FileSearchPath paths;
    paths.add (WindowsRegistry::getValue ("HKEY_LOCAL_MACHINE\\Software\\VST\\VSTPluginsPath"));
    paths.addIfNotAlreadyThere (programFiles + "\\Steinberg\\VstPlugins");
    paths.addIfNotAlreadyThere (programFiles + "\\VstPlugins");
    paths.removeRedundantPaths();
    return paths;
   #elif JUCE_IOS
    // on iOS you can only load plug-ins inside the hosts bundle folder
    CFUniquePtr<CFURLRef> relativePluginDir (CFBundleCopyBuiltInPlugInsURL (CFBundleGetMainBundle()));
    CFUniquePtr<CFURLRef> pluginDir (CFURLCopyAbsoluteURL (relativePluginDir.get()));

    CFUniquePtr<CFStringRef> path (CFURLCopyFileSystemPath (pluginDir.get(), kCFURLPOSIXPathStyle));
    FileSearchPath retval (String (CFStringGetCStringPtr (path.get(), kCFStringEncodingUTF8)));
    return retval;
   #endif
}

const XmlElement* VSTPluginFormatHeadless::getVSTXML (AudioPluginInstance* plugin)
{
    if (auto* vst = dynamic_cast<VSTPluginInstanceHeadless*> (plugin))
        if (vst->vstModule != nullptr)
            return vst->vstModule->vstXml.get();

    return nullptr;
}

bool VSTPluginFormatHeadless::loadFromFXBFile (AudioPluginInstance* plugin, const void* data, size_t dataSize)
{
    if (auto* vst = dynamic_cast<VSTPluginInstanceHeadless*> (plugin))
        return vst->loadFromFXBFile (data, dataSize);

    return false;
}

bool VSTPluginFormatHeadless::saveToFXBFile (AudioPluginInstance* plugin, MemoryBlock& dest, bool asFXB)
{
    if (auto* vst = dynamic_cast<VSTPluginInstanceHeadless*> (plugin))
        return vst->saveToFXBFile (dest, asFXB);

    return false;
}

bool VSTPluginFormatHeadless::getChunkData (AudioPluginInstance* plugin, MemoryBlock& result, bool isPreset)
{
    if (auto* vst = dynamic_cast<VSTPluginInstanceHeadless*> (plugin))
        return vst->getChunkData (result, isPreset, 128);

    return false;
}

bool VSTPluginFormatHeadless::setChunkData (AudioPluginInstance* plugin, const void* data, int size, bool isPreset)
{
    if (auto* vst = dynamic_cast<VSTPluginInstanceHeadless*> (plugin))
        return vst->setChunkData (data, size, isPreset);

    return false;
}

AudioPluginInstance* VSTPluginFormatHeadless::createCustomVSTFromMainCall (void* entryPointFunction,
                                                                           double initialSampleRate,
                                                                           int initialBufferSize)
{
    return createCustomVSTFromMainCallImpl<VSTPluginInstanceHeadless> (entryPointFunction, initialSampleRate, initialBufferSize).release();
}

void VSTPluginFormatHeadless::setExtraFunctions (AudioPluginInstance* plugin, ExtraFunctions* functions)
{
    std::unique_ptr<ExtraFunctions> f (functions);

    if (auto* vst = dynamic_cast<VSTPluginInstanceHeadless*> (plugin))
        std::swap (vst->extraFunctions, f);
}

AudioPluginInstance* VSTPluginFormatHeadless::getPluginInstanceFromVstEffectInterface (void* aEffect)
{
    if (auto* vstAEffect = reinterpret_cast<Vst2::AEffect*> (aEffect))
        if (auto* instanceVST = reinterpret_cast<VSTPluginInstanceHeadless*> (vstAEffect->resvd2))
            return dynamic_cast<AudioPluginInstance*> (instanceVST);

    return nullptr;
}

pointer_sized_int JUCE_CALLTYPE VSTPluginFormatHeadless::dispatcher (AudioPluginInstance* plugin,
                                                                     int32 opcode,
                                                                     int32 index,
                                                                     pointer_sized_int value,
                                                                     void* ptr,
                                                                     float opt)
{
    if (auto* vst = dynamic_cast<VSTPluginInstanceHeadless*> (plugin))
        return vst->dispatch (opcode, index, value, ptr, opt);

    return {};
}

void VSTPluginFormatHeadless::aboutToScanVSTShellPlugin (const PluginDescription&) {}

} // namespace juce

#endif
