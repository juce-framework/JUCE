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

namespace juce
{

#if (JUCE_PLUGINHOST_LADSPA && JUCE_LINUX) || DOXYGEN

//==============================================================================
/**
    Implements a plugin format manager for LADSPA plugins.
*/
class JUCE_API  LADSPAPluginFormat   : public AudioPluginFormat
{
public:
    LADSPAPluginFormat();
    ~LADSPAPluginFormat();

    //==============================================================================
    String getName() const override                { return "LADSPA"; }
    void findAllTypesForFile (OwnedArray<PluginDescription>&, const String& fileOrIdentifier) override;
    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;
    bool pluginNeedsRescanning (const PluginDescription&) override;
    StringArray searchPathsForPlugins (const FileSearchPath&, bool recursive, bool) override;
    bool doesPluginStillExist (const PluginDescription&) override;
    FileSearchPath getDefaultLocationsToSearch() override;
    bool canScanForPlugins() const override        { return true; }

private:
    //==============================================================================
    void createPluginInstance (const PluginDescription&, double initialSampleRate,
                               int initialBufferSize, void* userData,
                               void (*callback) (void*, AudioPluginInstance*, const String&)) override;

    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept override;

private:
    void recursiveFileSearch (StringArray&, const File&, bool recursive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAPluginFormat)
};


#endif

}
