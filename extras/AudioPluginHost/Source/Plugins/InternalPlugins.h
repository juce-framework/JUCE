/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
