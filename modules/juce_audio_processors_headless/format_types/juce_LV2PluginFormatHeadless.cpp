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

#if JUCE_INTERNAL_HAS_LV2

#include <juce_audio_processors_headless/format_types/juce_LV2PluginFormatImpl.h>

namespace juce
{

//==============================================================================
LV2PluginFormatHeadless::LV2PluginFormatHeadless()
    : pimpl (std::make_unique<Pimpl>()) {}

LV2PluginFormatHeadless::~LV2PluginFormatHeadless() = default;

void LV2PluginFormatHeadless::findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                                   const String& fileOrIdentifier)
{
    pimpl->findAllTypesForFile (results, fileOrIdentifier);
}

bool LV2PluginFormatHeadless::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    return pimpl->fileMightContainThisPluginType (fileOrIdentifier);
}

String LV2PluginFormatHeadless::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return pimpl->getNameOfPluginFromIdentifier (fileOrIdentifier);
}

bool LV2PluginFormatHeadless::pluginNeedsRescanning (const PluginDescription& desc)
{
    return pimpl->pluginNeedsRescanning (desc);
}

bool LV2PluginFormatHeadless::doesPluginStillExist (const PluginDescription& desc)
{
    return pimpl->doesPluginStillExist (desc);
}

bool LV2PluginFormatHeadless::canScanForPlugins() const { return true; }
bool LV2PluginFormatHeadless::isTrivialToScan() const { return true; }

StringArray LV2PluginFormatHeadless::searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                                            bool recursive,
                                                            bool allowAsync)
{
    return pimpl->searchPathsForPlugins (directoriesToSearch, recursive, allowAsync);
}

FileSearchPath LV2PluginFormatHeadless::getDefaultLocationsToSearch()
{
    return pimpl->getDefaultLocationsToSearch();
}

bool LV2PluginFormatHeadless::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

void LV2PluginFormatHeadless::createPluginInstance (const PluginDescription& desc,
                                                    double sampleRate,
                                                    int bufferSize,
                                                    PluginCreationCallback callback)
{
    Pimpl::createPluginInstance<lv2_host::LV2AudioPluginInstanceHeadless> (*this, desc, sampleRate, bufferSize, std::move (callback));
}

} // namespace juce

#endif
