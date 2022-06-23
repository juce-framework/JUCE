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

#pragma once

#include "PluginGraph.h"


//==============================================================================
/**
    Manages the internal plugin types.
*/
class InternalPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    InternalPluginFormat();

    //==============================================================================
    const std::vector<PluginDescription>& getAllTypes() const;

    //==============================================================================
    static String getIdentifier()                                                       { return "Internal"; }
    String getName() const override                                                     { return getIdentifier(); }
    bool fileMightContainThisPluginType (const String&) override                        { return true; }
    FileSearchPath getDefaultLocationsToSearch() override                               { return {}; }
    bool canScanForPlugins() const override                                             { return false; }
    bool isTrivialToScan() const override                                               { return true; }
    void findAllTypesForFile (OwnedArray<PluginDescription>&, const String&) override   {}
    bool doesPluginStillExist (const PluginDescription&) override                       { return true; }
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override      { return fileOrIdentifier; }
    bool pluginNeedsRescanning (const PluginDescription&) override                      { return false; }
    StringArray searchPathsForPlugins (const FileSearchPath&, bool, bool) override      { return {}; }

private:
    class InternalPluginFactory
    {
    public:
        using Constructor = std::function<std::unique_ptr<AudioPluginInstance>()>;

        explicit InternalPluginFactory (const std::initializer_list<Constructor>& constructorsIn);

        const std::vector<PluginDescription>& getDescriptions() const       { return descriptions; }

        std::unique_ptr<AudioPluginInstance> createInstance (const String& name) const;

    private:
        const std::vector<Constructor> constructors;
        const std::vector<PluginDescription> descriptions;
    };

    //==============================================================================
    void createPluginInstance (const PluginDescription&,
                               double initialSampleRate, int initialBufferSize,
                               PluginCreationCallback) override;

    std::unique_ptr<AudioPluginInstance> createInstance (const String& name);

    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;

    InternalPluginFactory factory;
};
