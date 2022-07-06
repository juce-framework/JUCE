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

namespace juce
{

#if (JUCE_PLUGINHOST_LV2 && (! (JUCE_ANDROID || JUCE_IOS))) || DOXYGEN

/**
    Implements a plugin format for LV2 plugins.

    @tags{Audio}
*/
class JUCE_API LV2PluginFormat  : public AudioPluginFormat
{
public:
    LV2PluginFormat();
    ~LV2PluginFormat() override;

    static String getFormatName()       { return "LV2"; }
    String getName() const override     { return getFormatName(); }

    void findAllTypesForFile (OwnedArray<PluginDescription>& results,
                              const String& fileOrIdentifier) override;

    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;

    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;

    bool pluginNeedsRescanning (const PluginDescription&) override;

    bool doesPluginStillExist (const PluginDescription&) override;

    bool canScanForPlugins() const override;

    bool isTrivialToScan() const override;

    StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                       bool recursive,
                                       bool allowPluginsWhichRequireAsynchronousInstantiation = false) override;

    FileSearchPath getDefaultLocationsToSearch() override;

private:
    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    void createPluginInstance (const PluginDescription&, double, int, PluginCreationCallback) override;

    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LV2PluginFormat)
};

#endif

} // namespace juce
